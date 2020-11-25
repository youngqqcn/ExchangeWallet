/*
 * 作者: yqq
 * 日期: 2019-12-12
 * 说明: XRP (Ripple)  离线签名相关
 *
 */

#ifndef __XRP_RAW_TX_UTILS_H__
#define __XRP_RAW_TX_UTILS_H__
#pragma execution_character_set("utf-8")

#include <QObject>
#include "raw_tx_utils_base.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数

extern bool g_bBlockChainMainnet;

#define STR_XRP_CHAINNET  ((g_bBlockChainMainnet) ? ("mainnet") : ("testnet"))
#define UINT_XRP_DEFAULT_FEE_IN_DROP        (12)   //12 drop,   0.000012 XRP

namespace rawtx
{
struct XrpAccountInfo;
Implement  CXrpRawTxUtilsImpl;
Implement Xrp_ExportRawTxItem;
Implement Xrp_CreateRawTxParam;
using Xrp_ExportRawTxParam = CImpExpParamBase<Xrp_ExportRawTxItem>;
using Xrp_ImportRawTxItem = Xrp_ExportRawTxItem;
using Xrp_ImportRawTxParam = Xrp_ExportRawTxParam;

Implement CXrpRawTxUtilsImpl  : public IRawTxUtils
{
public:
    //创建裸交易的接口  XRP 的实现
    //注意:  此接口包含了创建交易 + 签名交易 两部分,
    //  是可以直接调用服务端接口进行广播的, 不需要额外进行拼装操作
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override ;

    //导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;



public:
    //获取账户信息
    virtual int GetAccountInfo(XrpAccountInfo &accountInfo)noexcept(false);

public: //静态工具函数
    static QString  GetAddrFrom_Addr_Tag(const QString &str_Addr_Tag)noexcept(true);
    static uint32_t  GetTagFrom_Addr_Tag(const QString &str_Addr_Tag)noexcept(false);

private:
    //代理
    virtual int __SignProxyByRippled( Xrp_CreateRawTxParam *pParam  ) noexcept(false) ;

};


struct XrpAccountInfo
{
    explicit XrpAccountInfo()
    {
        bActivate = false;
        dBalanceInXRP = 0.0; //以 XRP为单位
        uSequqnce = UINT_MAX;
    }

    QString     strAddress;

    double      bActivate; //是否已经激活
    double      dBalanceInXRP;
    uint64_t    uSequqnce;
};


Implement  Xrp_CreateRawTxParam : public IRawTxUtilsParamBase
{
    Xrp_CreateRawTxParam(){Init();}

    virtual void Init() override
    {
        strChainNet = "";

        strSrcAddr = "";
        strDstAddr = "";
        strAmountInDrop = "";
        dAmountInXRP = 0.0;
        uDestinationTag = 1;
        uSequence = UINT_MAX;
        uFeeInDrop =  UINT_XRP_DEFAULT_FEE_IN_DROP;
        strPrivKey = "";

        bSigned = false;
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


        if(!( strPrivKey.startsWith('s')
            && 29 == strPrivKey.length()
            && (!strPrivKey.contains('I')  && !strPrivKey.contains('O') && !strPrivKey.contains('l') && !strPrivKey.contains('0') ))
            )
         {
            //!! 禁止输出明文私钥
            if(NULL != pStrErr) *pStrErr = QString("`strPrivKey`  is invalid. please check it.").arg(strPrivKey);
            return false;
         }


        if(!( strSrcAddr.startsWith('r')
            && 34 == strSrcAddr.length()
            && (!strSrcAddr.contains('I')  && !strSrcAddr.contains('O') && !strSrcAddr.contains('l') && !strSrcAddr.contains('0') ))
            )
         {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1`  is invalid address . please check it.").arg(strSrcAddr);
            return false;
         }

        if(!( strDstAddr.startsWith('r')
            && 34 == strDstAddr.length()
            && (!strDstAddr.contains('I')  && !strDstAddr.contains('O') && !strDstAddr.contains('l') && !strDstAddr.contains('0') ))
            )
         {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid address . please check it.").arg(strDstAddr);
            return false;
         }

        //注意单位, 是最小的单位   1XRP = 1000000
        if(!boost::all( strAmountInDrop.toStdString(), boost::is_alnum()) )
        {
           if(NULL != pStrErr) *pStrErr = QString("`strAmount`: `%1` is  invalid amount . please check it.").arg(strAmountInDrop);
           return false;
        }

        if( !( 0.000001 < dAmountInXRP && dAmountInXRP < 99999999))
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmountInXRP`: `%1` is  invalid amount . please check it.").arg(dAmountInXRP);
            return false;
        }

        if( ! (0 < uDestinationTag && uDestinationTag < UINT_MAX))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uDestinationTag`: `%1` is  invalid uDestinationTag . please check it.").arg(uDestinationTag);
            return false;
        }

        if( !(0 <= uSequence  && uSequence < UINT_MAX) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`uSequence`: `%1` is  invalid uSequence . please check it.").arg(uSequence);
            return false;
        }

        if( !( 5 <= uFeeInDrop && uFeeInDrop < 100  ))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uFeeInDrop`: `%1` is not invalid uFee. please check it.").arg(uFeeInDrop);
                return false;
        }

        return true;
    }

public: //输入参数
    QString     strChainNet;        //testnet, mainnet

    QString     strSrcAddr;         //源地址
    QString     strDstAddr;         //目的地址
    QString     strAmountInDrop;          //转账金额 , 以最小单位为单位,  1000000 = 1XRP
    double      dAmountInXRP;            //转账金额  , 以XRP为单位,   1XRP = 1000000
    uint64_t    uDestinationTag;    //转账标签
    uint64_t    uSequence;          //sequence
    uint8_t     uFeeInDrop;               //默认12 即可

    QString     strPrivKey;         //源地址私钥(seed)

public: //输出参数
    bool    bSigned;                // false:未签名, true:已签名
    QString  strSignedRawTx;        //签名后的交易tx_blob
};

Implement Xrp_ExportRawTxItem : public IRawTxUtilsParamBase
{
public:

    explicit Xrp_ExportRawTxItem(){Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        bComplete = false;
        strCoinType = "XRP";
        strChainNet = "";

        strSrcAddr = "";
        strDstAddr = "";
        strAmountInDrop = "";
        dAmountInXRP = 0.0;
        uDestinationTag = 1;
        uSequence = UINT_MAX;
        uFeeInDrop =  UINT_XRP_DEFAULT_FEE_IN_DROP;

        strSignedRawTx = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        if( !( (!g_bBlockChainMainnet  && 0 == strChainNet.compare("testnet", Qt::CaseSensitive))
                || (g_bBlockChainMainnet && 0 == strChainNet.compare("mainnet", Qt::CaseSensitive)) ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainNet's %1 is different with current env: %2 .")
                                    .arg(strChainNet).arg( (g_bBlockChainMainnet) ? ("Main") : ("Test") );
            return false;
        }


        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }

         if(!( strSrcAddr.startsWith('r')
            && 34 == strSrcAddr.length()
            && (!strSrcAddr.contains('I')  && !strSrcAddr.contains('O') && !strSrcAddr.contains('l') && !strSrcAddr.contains('0') ))
            )
         {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1`  is invalid address . please check it.").arg(strSrcAddr);
            return false;
         }

        if(!( strDstAddr.startsWith('r')
            && 34 == strDstAddr.length()
            && (!strDstAddr.contains('I')  && !strDstAddr.contains('O') && !strDstAddr.contains('l') && !strDstAddr.contains('0') ))
            )
         {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is not invalid address . please check it.").arg(strDstAddr);
            return false;
         }

        //注意单位, 是最小的单位   1XRP = 1000000
        if(!boost::all( strAmountInDrop.toStdString(), boost::is_alnum()) )
        {
           if(NULL != pStrErr) *pStrErr = QString("`strAmountInDrop`: `%1` is not invalid amount . please check it.").arg(strAmountInDrop);
           return false;
        }

        if( !( 0.000001 < dAmountInXRP && dAmountInXRP < 9999999))
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmountInXRP`: `%1` is not invalid amount . please check it.").arg(dAmountInXRP);
            return false;
        }

        if( ! (0 < uDestinationTag && uDestinationTag < UINT_MAX))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uDestinationTag`: `%1` is not invalid uDestinationTag . please check it.").arg(uDestinationTag);
            return false;
        }

        if( !(0 <= uSequence  && uSequence < UINT_MAX) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`uSequence`: `%1` is not invalid uSequence . please check it.").arg(uSequence);
            return false;
        }

        if( !( 5 <= uFeeInDrop && uFeeInDrop < 100  ))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uFeeInDrop`: `%1` is not invalid uFee. please check it.").arg(uFeeInDrop);
                return false;
        }

        return true;
    }

public:
    QString     strOrderId;
    bool        bComplete;
    QString     strCoinType;
    QString     strChainNet;        //testnet, mainnet

    QString     strSrcAddr;         //源地址
    QString     strDstAddr;         //目的地址
    QString     strAmountInDrop;          //转账金额 , 以最小单位为单位,  10000000 drop = 1XRP
    double      dAmountInXRP;            //转账金额  , 以XRP为单位,   1XRP = 1000000 drop
    uint64_t    uDestinationTag;    //转账标签
    uint64_t    uSequence;          //sequence
    uint8_t     uFeeInDrop;               //默认12 即可

public:
    QString     strSignedRawTx;    //签名后的交易tx_blob
};




Implement Xrp_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
    explicit Xrp_BroadcastRawTxParam(){ Init();}

     virtual void Init()  override
     {
        strOrderId = "";
        strCoinType = "XRP";
        strAmountInDrop = "";
        dAmountInXRP = 0.0;
        uSequence = UINT_MAX;
        strSrcAddr = "";
        strDstAddr = "";
        uDestinationTag = 1;
        strSignedRawTx = "";

        strTxid = "";
        strErrMsg = "";
     }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        if( !( (!g_bBlockChainMainnet  && 0 == strChainNet.compare("testnet", Qt::CaseSensitive))
                || (g_bBlockChainMainnet && 0 == strChainNet.compare("mainnet", Qt::CaseSensitive)) ) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainNet's %1 is different with current env: %2,  .")
                                    .arg(strChainNet).arg( (g_bBlockChainMainnet) ? ("Main") : ("Test") );
            return false;
        }

        if( strSignedRawTx.length() < 32 ||  !boost::all(strSignedRawTx.toStdString(),  boost::is_xdigit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strSignedRawTx's is invalid, please check it!");
            return false;
        }


        if(! (0 == strCoinType.compare("XRP", Qt::CaseSensitive)))
        {
            if(NULL != pStrErr) *pStrErr = QString("strCoinType's %1 is invalid, must be 'XRP' .").arg(strCoinType);
            return false;
        }

        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId's %1 is invalid .").arg(strOrderId);
            return false;
        }

        if( !( 0 == strChainNet.compare("testnet", Qt::CaseSensitive)
                ||0 == strChainNet.compare("mainnet", Qt::CaseSensitive)) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainNet's %1 is invalid, must be 'testnet' or 'mainnet' .").arg(strChainNet);
            return false;
        }

         if(!( strSrcAddr.startsWith('r')
            && (25 <= strSrcAddr.length() && strSrcAddr.length() <= 35)
            && (!strSrcAddr.contains('I')  && !strSrcAddr.contains('O') && !strSrcAddr.contains('l') && !strSrcAddr.contains('0') ))
            )
         {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1`  is invalid address . please check it.").arg(strSrcAddr);
            return false;
         }

        if(!( strDstAddr.startsWith('r')
            && 34 == strDstAddr.length()
            && (!strDstAddr.contains('I')  && !strDstAddr.contains('O') && !strDstAddr.contains('l') && !strDstAddr.contains('0') ))
            )
         {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is not invalid address . please check it.").arg(strDstAddr);
            return false;
         }

        //注意单位, 是最小的单位   1XRP = 1000000
        if(!boost::all( strAmountInDrop.toStdString(), boost::is_alnum()) )
        {
           if(NULL != pStrErr) *pStrErr = QString("`strAmountInDrop`: `%1` is not invalid amount . please check it.").arg(strAmountInDrop);
           return false;
        }

        if( !( 0.000001 < dAmountInXRP && dAmountInXRP < 9999999))
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmountInXRP`: `%1` is not invalid amount . please check it.").arg(dAmountInXRP);
            return false;
        }

        if( ! (0 < uDestinationTag && uDestinationTag < UINT_MAX))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uDestinationTag`: `%1` is not invalid uDestinationTag . please check it.").arg(uDestinationTag);
            return false;
        }

        if( !(0 <= uSequence  && uSequence < UINT_MAX) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`uSequence`: `%1` is not invalid uSequence . please check it.").arg(uSequence);
            return false;
        }



        return true;
    }

public:
    QString             strChainNet; //testnet, mainnet

    QString             strOrderId; //广播时带上订单号, 防止重复提币

    QString             strCoinType;
    QString             strAmountInDrop;
    double              dAmountInXRP;
    uint64_t            uSequence;
    QString             strSrcAddr;
    QString             strDstAddr;
    uint64_t            uDestinationTag;
    QString             strSignedRawTx; //交易

public:
    QString             strTxid;
    QString             strErrMsg;

};



}


#endif //__XRP_RAW_TX_UTILS_H__
