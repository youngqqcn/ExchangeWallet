/*
 * 作者: yqq
 * 日期: 2019-12-30
 * 说明: EOS  离线签名相关
 *
 */

#ifndef __EOS_RAW_TX_UTILS__
#define __EOS_RAW_TX_UTILS__
#include <QObject>
#include "raw_tx_utils_base.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数

extern bool g_bBlockChainMainnet;
#define STR_EOS_CHAINNET  ((g_bBlockChainMainnet) ? ("mainnet") : ("testnet"))


namespace rawtx
{

struct EosAccountInfo;
Implement  CEosRawTxUtilsImpl;
Implement Eos_ExportRawTxItem;
Implement Eos_CreateRawTxParam;
using Eos_ExportRawTxParam = CImpExpParamBase<Eos_ExportRawTxItem>;
using Eos_ImportRawTxItem = Eos_ExportRawTxItem;
using Eos_ImportRawTxParam = Eos_ExportRawTxParam;


Implement CEosRawTxUtilsImpl : public IRawTxUtils
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
    virtual int SignTxDigestWithPrivKey(const QString &strTxDigest, const QString &strPrivKey, QString &strOutSigResult) noexcept(false);



    //获取账户信息( 余额,  资源(RAM, CPU, NET) )
    virtual int GetAccountInfo(EosAccountInfo &accountInfo)noexcept(false);

};

Implement Eos_CreateRawTxParam  : public IRawTxUtilsParamBase
{
    Eos_CreateRawTxParam(){Init();}

    virtual void Init() override
    {
        strChainNet = "";

        strCoinType = "EOS";
        dAmount = 0.0;
        strAmount = "0.0";
        strSrcAcct = "";
        strDstAcct = "";
        strMemo = "";

        bSigned = false;
        strTxDigestHex = "";
        strSignedRawTx = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
         if( !( (!g_bBlockChainMainnet  && 0 == strChainNet.compare("testnet", Qt::CaseSensitive))
                || (g_bBlockChainMainnet && 0 == strChainNet.compare("mainnet", Qt::CaseSensitive)) ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainNet's %1 is different with current env: %2.")
                                    .arg(strChainNet).arg( (g_bBlockChainMainnet) ? ("Main") : ("Test") );
            return false;
        }

        QRegularExpression  reExp("^[1-5a-z]{12}$");
        QRegularExpressionMatch matchSrcAcct = reExp.match( strSrcAcct );
        QRegularExpressionMatch matchDstAcct = reExp.match( strDstAcct );
        if( ! matchSrcAcct.hasMatch() )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAcct`: `%1`  is invalid . please check it.").arg(strSrcAcct);
                return false;
        }

        if( !matchDstAcct.hasMatch())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAcct`: `%1`  is invalid . please check it.").arg(strDstAcct);
                return false;
        }

        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        if(20 < strMemo.length() || strMemo.length() < 1)
        {
            if(NULL != pStrErr) *pStrErr = QString("`strMemo`:`%1` is invalid memo, too long or too short.").arg(strMemo);
                return false;
        }

        return true;
    }
public: //输入参数
    QString     strChainNet; //testnet,  mainnet

    QString     strCoinType;        //币种
    double      dAmount;            //金额(浮点数)
    QString     strAmount;          //金额(字符串形式)  保留4为小数
    QString     strSrcAcct;         //源账户
    QString     strDstAcct;         //目的账户
    QString     strMemo;            //转账备注

public: //输出参数
    bool        bSigned;

    //交易摘要, 即交易编码后的hash值, 由服务端传回,  这也是进行 ECDSA签名的关键数据
    QString     strTxDigestHex;

    //和BTC, ETH 裸交易是十六进制字符串不同, EOS的交易是 json字符串形式的
    //未签名时: 其中的signature字段为  "signature" : []
    //签名后: 其中的signature字段为 "signature" : ["SIG_XXXXXXXXXXXXXXXXXXXXXXXXXXXXX"]
    QString     strSignedRawTx;
};


Implement Eos_ExportRawTxItem : public IRawTxUtilsParamBase
{
public:

    explicit Eos_ExportRawTxItem(){Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        bComplete = false;
        strCoinType = "EOS";
        strChainNet = "";        //testnet, mainnet

        strAmount = "";          //金额(字符串形式)  保留4为小数
        strSrcAcct = "";         //源账户
        strDstAcct = "";         //目的账户
        strMemo = "";            //转账备注
        strTxDigestHex = "";     //交易摘要
        strSignedRawTx = "";     //签名后的交易
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {

        if( !( (!g_bBlockChainMainnet  && 0 == strChainNet.compare("testnet", Qt::CaseSensitive))
                || (g_bBlockChainMainnet && 0 == strChainNet.compare("mainnet", Qt::CaseSensitive)) ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainNet's %1 is different with current env: %2.")
                                    .arg(strChainNet).arg( (g_bBlockChainMainnet) ? ("Main") : ("Test") );
            return false;
        }

        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }

        QRegularExpression  reExp("^[1-5a-z]{12}$");
        QRegularExpressionMatch matchSrcAcct = reExp.match( strSrcAcct );
        QRegularExpressionMatch matchDstAcct = reExp.match( strDstAcct );
        if( ! matchSrcAcct.hasMatch() )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAcct`: `%1`  is invalid . please check it.").arg(strSrcAcct);
                return false;
        }

        if( !matchDstAcct.hasMatch())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAcct`: `%1`  is invalid . please check it.").arg(strDstAcct);
                return false;
        }

        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        if(20 < strMemo.length() || strMemo.length() < 1)
        {
            if(NULL != pStrErr) *pStrErr = QString("`strMemo`:`%1` is invalid memo, too long or too short.").arg(strMemo);
                return false;
        }

        //检查交易摘要
        if(strTxDigestHex.length() < 64 ||  !boost::all( strTxDigestHex.toStdString(), boost::is_xdigit() ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strTxDigestHex`:`%1` is invalid , please check it.").arg(strTxDigestHex);
                return false;
        }

        return true;
    }

public:
    QString     strOrderId;
    bool        bComplete;
    QString     strCoinType;
    QString     strChainNet;        //testnet, mainnet

    double      dAmount;
    QString     strAmount;          //金额(字符串形式)  保留4为小数
    QString     strSrcAcct;         //源账户
    QString     strDstAcct;         //目的账户
    QString     strMemo;            //转账备注

    QString     strTxDigestHex;     //交易摘要
public:
    QString     strSignedRawTx;     //签名后的交易

};



Implement Eos_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
    explicit Eos_BroadcastRawTxParam(){ Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        strCoinType = "EOS";
        strChainNet = "";        //testnet, mainnet

        strAmount = "";          //金额(字符串形式)  保留4为小数
        strSrcAcct = "";         //源账户
        strDstAcct = "";         //目的账户
        strMemo = "";            //转账备注
        strTxDigestHex = "";     //交易摘要
        strSignedRawTx = "";     //签名后的交易

        strTxid = "";
        strErrMsg = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {

        if( !( (!g_bBlockChainMainnet  && 0 == strChainNet.compare("testnet", Qt::CaseSensitive))
                || (g_bBlockChainMainnet && 0 == strChainNet.compare("mainnet", Qt::CaseSensitive)) ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainNet's %1 is different with current env: %2.")
                                    .arg(strChainNet).arg( (g_bBlockChainMainnet) ? ("Main") : ("Test") );
            return false;
        }

        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }

        QRegularExpression  reExp("^[1-5a-z]{12}$");
        QRegularExpressionMatch matchSrcAcct = reExp.match( strSrcAcct );
        QRegularExpressionMatch matchDstAcct = reExp.match( strDstAcct );
        if( ! matchSrcAcct.hasMatch() )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAcct`: `%1`  is invalid . please check it.").arg(strSrcAcct);
                return false;
        }

        if( !matchDstAcct.hasMatch())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAcct`: `%1`  is invalid . please check it.").arg(strDstAcct);
                return false;
        }

        if(dAmount < 0.0001 || dAmount > 9999999)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: `%1` is  invalid amount . please check it.").arg(dAmount);
                return false;
        }

        if(20 < strMemo.length() || strMemo.length() < 1)
        {
            if(NULL != pStrErr) *pStrErr = QString("`strMemo`:`%1` is invalid memo, too long or too short.").arg(strMemo);
                return false;
        }

        //检查交易摘要
        if(strTxDigestHex.length() < 64 ||  !boost::all( strTxDigestHex.toStdString(), boost::is_xdigit() ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`strTxDigestHex`:`%1` is invalid , please check it.").arg(strTxDigestHex);
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
    QString             strChainNet; //testnet, mainnet
    QString             strOrderId; //广播时带上订单号, 防止重复提币
    QString             strCoinType;

    QString             strAmount;
    double              dAmount;
    QString             strSrcAcct;
    QString             strDstAcct;
    QString             strMemo;

    QString             strTxDigestHex;
    QString             strSignedRawTx;

public:
    QString             strTxid;
    QString             strErrMsg;
};

struct EosAccountInfo
{
    explicit  EosAccountInfo()
    {
        strAccountName = "";

        dEOSBalance  = 0.0;

        uRAM_Total = 0;
        uRAM_Used = 0;

        uNET_Total = 0;
        uNET_Avail = 0;
        uNET_Used = 0;

        uCPU_Avail = 0;
        uCPU_Total = 0;
        uCPU_Used  = 0;
    }


    QString     strAccountName;

    double      dEOSBalance;

    uint64_t    uRAM_Total; //RAM 余量
    uint64_t    uRAM_Used; //已使用量

    uint64_t    uNET_Total; // NET 总量
    uint64_t    uNET_Used; //已用量
    uint64_t    uNET_Avail; //可用量

    uint64_t    uCPU_Total; // CPU 总量
    uint64_t    uCPU_Used; //已用量
    uint64_t    uCPU_Avail; //可用量
};

}


#endif //__EOS_RAW_TX_UTILS__
