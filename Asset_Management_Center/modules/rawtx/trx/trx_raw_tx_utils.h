/*
 * date: 2020-03-04
 * author: yqq
 * descriptions:  TRX 的裸交易工具类, 离线签名等
 */


#ifndef __TRX_RAW_TX_UTILS_H__
#define __TRX_RAW_TX_UTILS_H__
#include "raw_tx_utils_base.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数
#include "rawtx_comman.h"
extern bool g_bBlockChainMainnet;

namespace rawtx
{

struct TrxAccountInfo;
Implement  CTrxRawTxUtilsImpl;
Implement Trx_ExportRawTxItem;
Implement Trx_CreateRawTxParam;
using Trx_ExportRawTxParam = CImpExpParamBase<Trx_ExportRawTxItem>;
using Trx_ImportRawTxItem = Trx_ExportRawTxItem;
using Trx_ImportRawTxParam = Trx_ExportRawTxParam;


Implement CTrxRawTxUtilsImpl : public IRawTxUtils
{
public:
    //创建裸交易的接口 EOS 的实现
    //调用服务端的createrawtransaction 接口进行创建交易 (不包含签名)
    // digest : 是交易的编码后的数据, 也是被签名的对象
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override ;

    //导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;


public:
    //签名函数
    virtual int SignTxidWithPrivKey(const QString &strTxid, const QString &strPrivKey, QString &strOutSigResult) noexcept(false);



    //获取账户信息( 余额, 等信息  )
    virtual int GetAccountInfo(TrxAccountInfo &accountInfo)noexcept(false);

};


Implement Trx_CreateRawTxParam  : public IRawTxUtilsParamBase
{
    Trx_CreateRawTxParam(){Init();}

    virtual void Init() override
    {
        strCoinType = "TRX";
        dAmount = 0.0;
        strAmount = "0.0";
        strSrcAcct = "";
        strDstAcct = "";

        bSigned = false;
        strTxid = "";
        strSignedRawTx = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        //检查  源地址  合法性,   只考虑  Base58 格式,  不考虑 hexString 格式
        if( ! (34 == strSrcAcct.length()
                &&  strSrcAcct.startsWith('T', Qt::CaseSensitive)
                &&  !strSrcAcct.contains('O', Qt::CaseSensitive )
                &&  !strSrcAcct.contains('l', Qt::CaseSensitive )
                )
            )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAcct`: `%1` is  invalid  . please check it.").arg(strSrcAcct);
            return false;
        }

        //检查  目的地址  合法性
        if(!(34 == strDstAcct.length()
             &&  strDstAcct.startsWith('T', Qt::CaseSensitive)
                &&  !strDstAcct.contains('O', Qt::CaseSensitive )
                &&  !strDstAcct.contains('l', Qt::CaseSensitive )
            )
        )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAcct`: `%1` is  invalid . please check it.").arg(strDstAcct);
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

    QString     strCoinType;        //币种
    double      dAmount;            //金额(浮点数)
    QString     strAmount;          //金额(字符串形式)  保留4为小数

    QString     strSrcAcct;         //源账户
    QString     strDstAcct;         //目的账户

public: //输出参数
    bool        bSigned;

    //交易摘要,   由 hashlib.sha256( unhexlify(tx['raw_data_hex']) ).hexdigest()  得到
    //交易摘要, 即交易编码后的hash值, 由服务端传回,  这也是进行 ECDSA签名的关键数据
    //  交易中的   tx["txID"],  签名时对 txID进行签名即可
    // 但是要注意: 广播时必须要等待广播成功后,才能叫txid写入数据, 不能直接使用此txid, 因为可能会广播失败
    QString     strTxid;

    //签名后: 其中的signature字段为 "signature" : ["xxxxxxxxxxxxxxxxxx"]
    QString     strSignedRawTx;
};




Implement Trx_ExportRawTxItem : public IRawTxUtilsParamBase
{
public:

    explicit Trx_ExportRawTxItem(){Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        bComplete = false;
        strCoinType = "TRX";

        strAmount = "";          //金额(字符串形式)  保留4为小数
        strSrcAcct = "";         //源账户
        strDstAcct = "";         //目的账户
        strTxid = "";     //交易摘要
        strSignedRawTx = "";     //签名后的交易
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }


        //检查  源地址  合法性,   只考虑  Base58 格式,  不考虑 hexString 格式
        if( ! (34 == strSrcAcct.length()
                &&  strSrcAcct.startsWith('T', Qt::CaseSensitive)
                &&  !strSrcAcct.contains('O', Qt::CaseSensitive )
                &&  !strSrcAcct.contains('l', Qt::CaseSensitive )
                )
            )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAcct`: `%1` is  invalid  . please check it.").arg(strSrcAcct);
            return false;
        }

        //检查  目的地址  合法性
        if(!(34 == strDstAcct.length()
             &&  strDstAcct.startsWith('T', Qt::CaseSensitive)
                &&  !strDstAcct.contains('O', Qt::CaseSensitive )
                &&  !strDstAcct.contains('l', Qt::CaseSensitive )
            )
        )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAcct`: `%1` is  invalid . please check it.").arg(strDstAcct);
            return false;
        }


        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        //检查交易摘要
        if(strTxid.length() != 64 ||  !boost::all( strTxid.toStdString(), boost::is_xdigit() ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strTxid`:`%1` is invalid , please check it.").arg(strTxid);
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
    QString     strSrcAcct;         //源账户
    QString     strDstAcct;         //目的账户

    QString     strTxid;     //交易摘要,   由 hashlib.sha256( unhexlify(tx['raw_data_hex']) ).hexdigest()  得到
public:
    QString     strSignedRawTx;     //签名后的交易

};



Implement Trx_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
    explicit Trx_BroadcastRawTxParam(){ Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        strCoinType = "TRX";

        strAmount = "";          //金额(字符串形式)  保留4为小数
        strSrcAcct = "";         //源账户
        strDstAcct = "";         //目的账户
        strTxid = "";     //交易摘要
        strSignedRawTx = "";     //签名后的交易

        strSuccessedTxid = "";
        strErrMsg = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {

        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }


        //检查  源地址  合法性,   只考虑  Base58 格式,  不考虑 hexString 格式
        if( ! (34 == strSrcAcct.length()
                &&  strSrcAcct.startsWith('T', Qt::CaseSensitive)
                &&  !strSrcAcct.contains('O', Qt::CaseSensitive )
                &&  !strSrcAcct.contains('l', Qt::CaseSensitive )
                )
            )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAcct`: `%1` is  invalid  . please check it.").arg(strSrcAcct);
            return false;
        }

        //检查  目的地址  合法性
        if(!(34 == strDstAcct.length()
             &&  strDstAcct.startsWith('T', Qt::CaseSensitive)
                &&  !strDstAcct.contains('O', Qt::CaseSensitive )
                &&  !strDstAcct.contains('l', Qt::CaseSensitive )
            )
        )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAcct`: `%1` is  invalid . please check it.").arg(strDstAcct);
            return false;
        }

        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        //检查交易摘要
        if(strTxid.length() != 64 ||  !boost::all( strTxid.toStdString(), boost::is_xdigit() ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strTxid`:`%1` is invalid , please check it.").arg(strTxid);
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
    QString             strOrderId; //广播时带上订单号, 防止重复提币
    QString             strCoinType;

    QString             strAmount;
    double              dAmount;
    QString             strSrcAcct;
    QString             strDstAcct;

    QString             strTxid;
    QString             strSignedRawTx;

public:
    QString             strSuccessedTxid;
    QString             strErrMsg;
};


struct TrxAccountInfo
{
    QString strAddress = "";
    double dBalance = 0.0;
    bool  isActive = false;
};

}// namespace  rawtx


#endif //__TRX_RAW_TX_UTILS_H__
