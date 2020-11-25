/*
 * author: yqq
 * date: 2020-04-07
 * description: XMR (monero) 离线签名相关操作函数封装
 */

#ifndef  __XMR_RAW_TX_UTILS_H__
#define __XMR_RAW_TX_UTILS_H__
#pragma execution_character_set("utf-8")

#include <QObject>
#include "raw_tx_utils_base.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数

extern bool g_bBlockChainMainnet;

namespace rawtx
{

struct CXmrRawTxUtilsImpl;
struct XMR_ExportRawTxItem;
struct XMR_CreateRawTxParam;
using XMR_ExportRawTxParam = CImpExpParamBase<XMR_ExportRawTxItem>;
using XMR_ImportRawTxItem = XMR_ExportRawTxItem;
using XMR_ImportRawTxParam = XMR_ExportRawTxParam;

struct  CXmrRawTxUtilsImpl : public IRawTxUtils
{
public:
    //1. hot wallet 导出 交易输出
    virtual int ExportTxOutputs(const QString &strReqUrl, QString &strOutputsDataHex) noexcept(false) ;

    //2. cold wallet 导入 交易输出
    virtual int ImportTxOutputs(const QString &strOutputsDataHex) noexcept(false) ;

    //3. cold wallet 导出 签名后的 key images
    virtual int ExportKeyImages( QString  &strSignedKeyImages) noexcept(false) ;

    //4. hot wallet 导入 已签名的 key images
    virtual int ImportKeyImages(const QString &strReqUrl, const QString &strSignedKeyImages,   double &dAvailBalance )noexcept(false) ;

    //签名交易
    int  SignTx(  const QString &strUnsignedTxSet,  QString &strSignedTxHex ) noexcept(false);

public:
    //创建交易
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override ;

    //导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override;

    //导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override;

    //广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override;


};

// monero源码: monero/src/common/base58.cpp
// Monero-Special Base58  Character Set : "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
inline bool IsValidXmrAddr(const QString &strAddr)
{
    //只考虑标准地址, 不考虑支持  intergrated-address
    if(95 == strAddr.length()
        && !strAddr.contains('I', Qt::CaseSensitive)
        && !strAddr.contains('O', Qt::CaseSensitive)
        && !strAddr.contains('l', Qt::CaseSensitive)
         )
    {
        // 主网:  '8' 开头的subAddress,  '4'开头的 standardAddress(或称 masterAddress)
        if(g_bBlockChainMainnet && (strAddr.startsWith('8') || strAddr.startsWith('4')))
            return true;

        // stagenet测试网:  '7' 开头的subAddress,  '5'开头的 standardAddress(或称 masterAddress)
        if(!g_bBlockChainMainnet && (strAddr.startsWith('7') || strAddr.startsWith('5')))
            return true;
    }

    return false;
}


struct XMR_CreateRawTxParam  : public IRawTxUtilsParamBase
{
public:

    XMR_CreateRawTxParam(){Init();}

    virtual void Init() override
    {
        bIsManualWithdraw = false;
        strCoinType = "XMR";
        dAmount = 0.0;
        strAmount = "0.0";
        strSrcAddr = "";
        strDstAddr = "";

        bSigned = false;
        strSignedRawTx = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        if(!IsValidXmrAddr(strSrcAddr))
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1` is  invalid  . please check it.").arg(strSrcAddr);
            return false;
        }

        //检查  目的地址  合法性
        if(!IsValidXmrAddr(strDstAddr) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid . please check it.").arg(strDstAddr);
            return false;
        }


        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        return true;
    }
public: //输入参数
    bool        bIsManualWithdraw;  //是否是手动提币

    QString     strCoinType;        //币种
    double      dAmount;            //金额(浮点数)
    QString     strAmount;          //金额(字符串形式)

    QString     strSrcAddr;         //源地址
    QString     strDstAddr;         //目的地址

public: //输出参数
    bool        bSigned;
    QString     strSignedRawTx;
};



struct XMR_ExportRawTxItem : public IRawTxUtilsParamBase
{
public:

    explicit XMR_ExportRawTxItem(){Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        bComplete = false;
        strCoinType = "XMR";

        strAmount = "";          //金额(字符串形式)  保留4为小数
        strSrcAddr= "";         //源账户
        strDstAddr = "";         //目的账户
        strSignedRawTx = "";     //签名后的交易
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }

        if(!IsValidXmrAddr(strSrcAddr))
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1` is  invalid  . please check it.").arg(strSrcAddr);
            return false;
        }

        //检查  目的地址  合法性
        if(!IsValidXmrAddr(strDstAddr) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid . please check it.").arg(strDstAddr);
            return false;
        }

        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        return true;
    }

public:
    QString     strOrderId;
    bool        bComplete;
    QString     strCoinType;

    double      dAmount;
    QString     strAmount;          //金额(字符串形式)  保留4为小数
    QString     strSrcAddr;         //源账户
    QString     strDstAddr;         //目的账户

public:
    QString     strSignedRawTx;     //签名后的交易

};


struct XMR_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
public:
    explicit XMR_BroadcastRawTxParam(){ Init();}

    virtual void Init()  override
    {
        bIsManualWithdraw = false;

        strOrderId = "";
        strCoinType = "XMR";

        strAmount = "";          //金额(字符串形式)  保留4为小数
        strSrcAddr = "";         //源账户
        strDstAddr = "";         //目的账户
        strSignedRawTx = "";     //签名后的交易


        //输出
        strTxid = "";
        strErrMsg = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {

        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }

        if(!IsValidXmrAddr(strSrcAddr))
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1` is  invalid  . please check it.").arg(strSrcAddr);
            return false;
        }

        //检查  目的地址  合法性
        if(!IsValidXmrAddr(strDstAddr) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid . please check it.").arg(strDstAddr);
            return false;
        }

        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        if(strSignedRawTx.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSignedRawTx is empty, please check it.");
                return false;
        }

        return true;
    }

public:
    bool        bIsManualWithdraw;  //是否是手动提币

    QString             strOrderId; //广播时带上订单号, 防止重复提币
    QString             strCoinType;

    QString             strAmount;
    double              dAmount;
    QString             strSrcAddr;
    QString             strDstAddr;

    QString             strSignedRawTx;

public: //输出
    QString             strTxid;
    QString             strErrMsg;
};




} //namespace rawtx

#endif //__XMR_RAW_TX_UTILS_H__
