/*
 * 作者: yqq
 * 日期: 2020-02-11
 * 说明: XLM   离线签名相关
 *
 */

#ifndef __XLM_RAW_TX_UTILS_H__
#define __XLM_RAW_TX_UTILS_H__
#pragma execution_character_set("utf-8")

#include <QObject>
#include "raw_tx_utils_base.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数

extern bool g_bBlockChainMainnet;

#define STR_XLM_CHAINNET  ((g_bBlockChainMainnet) ? ("mainnet") : ("testnet"))




namespace rawtx
{
struct XlmAccountInfo;
Implement  CXlmRawTxUtilsImpl;
Implement Xlm_ExportRawTxItem;
Implement Xlm_CreateRawTxParam;
using Xlm_ExportRawTxParam = CImpExpParamBase<Xlm_ExportRawTxItem>;
using Xlm_ImportRawTxItem = Xlm_ExportRawTxItem;
using Xlm_ImportRawTxParam = Xlm_ExportRawTxParam;

Implement CXlmRawTxUtilsImpl : public IRawTxUtils
{
public:

    //创建交易
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override ;

    //导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

public:
    //获取账户信息
    virtual int GetAccountInfo(XlmAccountInfo &accountInfo)noexcept(false);

};


struct XlmAccountInfo
{
    explicit XlmAccountInfo()
    {
        bFound = false;
        dBalance = 0.0; //以 XLM为单位
        strSequence = "";
        uSequence = 0;
    }

    QString     strAddress;

    double      bFound; //是否已经激活
    double      dBalance ;
    QString     strSequence;
    uint64_t    uSequence;
};


Implement  Xlm_CreateRawTxParam : public IRawTxUtilsParamBase
{
    Xlm_CreateRawTxParam(){Init();}

    virtual void Init() override
    {
        strChainNet = "";

        strSrcAddr = "";
        strDstAddr = "";
        strAmount= "";
        dAmount= 0.0;
        //uDestinationTag = 1;
        uSequence = UINT_MAX;
        uBaseFee =  0; //设置为默认的手续费
        strPrivKey = "";

        strMemo = "";

        bDstAccountExists = true;

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


        if(!( strPrivKey.startsWith('S')
            && 56 == strPrivKey.length()
            ))
         {
            //!! 禁止输出明文私钥
            if(NULL != pStrErr) *pStrErr = QString("`strPrivKey`  is invalid. please check it.").arg(strPrivKey);
            return false;
         }


        if(!( strSrcAddr.startsWith('G')
            && 56 == strSrcAddr.length()
            ))
         {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1`  is invalid address . please check it.").arg(strSrcAddr);
            return false;
         }

        if(!( strDstAddr.startsWith('G')
            && 56 == strDstAddr.length()
            ))
         {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid address . please check it.").arg(strDstAddr);
            return false;
         }

        //注意单位, 是最小的单位   1XRP = 1000000
        if(!boost::all( strAmount.toStdString(), boost::is_alnum() || boost::is_any_of(".")) )
        {
           if(NULL != pStrErr) *pStrErr = QString("`strAmount`: `%1` is  invalid amount . please check it.").arg(strAmount);
           return false;
        }

        if( !( 0.000001 < dAmount && dAmount < 99999999))
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmountInXRP`: `%1` is  invalid amount . please check it.").arg(dAmount);
            return false;
        }

        /*if( ! (0 < uDestinationTag && uDestinationTag < UINT_MAX))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uDestinationTag`: `%1` is  invalid uDestinationTag . please check it.").arg(uDestinationTag);
            return false;
        }*/
        if(strMemo.length() > 28)
        {
            if(NULL != pStrErr) *pStrErr = QString("`strMemo`: `%1` is  invalid strMemo . please check it.").arg(strMemo);
            return false;
        }

        if( !(0 <= uSequence  && uSequence < ULLONG_MAX) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`uSequence`: `%1` is  invalid uSequence . please check it.").arg(uSequence);
            return false;
        }

        if( !( 5 <= uBaseFee && uBaseFee < 1000  ))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uFeeInDrop`: `%1` is not invalid uFee. please check it.").arg(uBaseFee);
                return false;
        }

        return true;
    }

public: //输入参数
    QString     strChainNet;        //testnet, mainnet

    QString     strSrcAddr;         //源地址
    QString     strDstAddr;         //目的地址
    QString     strAmount;          //转账金额
    double      dAmount;            //转账金额
    //uint64_t    uDestinationTag;    //转账标签
    QString     strMemo;            //转账memo
    uint64_t    uSequence;          //sequence
    uint        uBaseFee;           //base fee

    bool        bDstAccountExists;  //目的账号是否存在

    QString     strPrivKey;         //源地址私钥(seed)

public: //输出参数
    bool    bSigned;                // false:未签名, true:已签名
    QString  strSignedRawTx;        //签名后的交易tx_blob
};


Implement Xlm_ExportRawTxItem : public IRawTxUtilsParamBase
{
public:

    explicit Xlm_ExportRawTxItem(){Init();}

    virtual void Init()  override
    {
        strOrderId = "";
        bComplete = false;
        strCoinType = "XLM";
        strChainNet = "";

        strSrcAddr = "";
        strDstAddr = "";
        strAmount = "";
        dAmount = 0.0;
        //uDestinationTag = 1;
        strMemo = "";
        uSequence = UINT_MAX;
        uBaseFee =  0;

        bDstAccountExists = true;

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


        if(!( strSrcAddr.startsWith('G')
            && 56 == strSrcAddr.length()
            ))
         {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1`  is invalid address . please check it.").arg(strSrcAddr);
            return false;
         }

        if(!( strDstAddr.startsWith('G')
            && 56 == strDstAddr.length()
            ))
         {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid address . please check it.").arg(strDstAddr);
            return false;
         }

        if(!boost::all( strAmount.toStdString(), boost::is_alnum()  || boost::is_any_of(".")) )
        {
           if(NULL != pStrErr) *pStrErr = QString("`strAmountInDrop`: `%1` is not invalid amount . please check it.").arg(strAmount);
           return false;
        }

        if( !( 0.000001 < dAmount && dAmount < 9999999))
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmountInXRP`: `%1` is not invalid amount . please check it.").arg(dAmount);
            return false;
        }

        /*if( ! (0 < uDestinationTag && uDestinationTag < UINT_MAX))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uDestinationTag`: `%1` is not invalid uDestinationTag . please check it.").arg(uDestinationTag);
            return false;
        }*/
        if(strMemo.length() > 28)
        {
            if(NULL != pStrErr) *pStrErr = QString("`strMemo`: `%1` is  invalid strMemo . please check it.").arg(strMemo);
            return false;
        }

        if( !(0 <= uSequence  && uSequence < ULLONG_MAX) )
        {
            if(NULL != pStrErr) *pStrErr = QString("`uSequence`: `%1` is not invalid uSequence . please check it.").arg(uSequence);
            return false;
        }

        if( !( 5 <= uBaseFee && uBaseFee< 1000  ))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uFeeInDrop`: `%1` is not invalid uFee. please check it.").arg(uBaseFee);
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
    QString     strAmount;          //转账金额
    double      dAmount;            //转账金额
    //uint64_t    uDestinationTag;    //转账标签
    QString     strMemo;
    uint64_t    uSequence;          //sequence
    uint     uBaseFee;           //

    bool        bDstAccountExists;  //目的账号是否存在

public:
    QString     strSignedRawTx;    //签名后的交易tx_blob
};



Implement Xlm_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
    explicit Xlm_BroadcastRawTxParam(){ Init();}

     virtual void Init()  override
     {
        strOrderId = "";
        strCoinType = "XLM";
        strAmount = "";
        dAmount = 0.0;
        uSequence = UINT_MAX;
        strSrcAddr = "";
        strDstAddr = "";
        //uDestinationTag = 1;
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

        //检查是否是合法的  base64编码的 字符串
        QByteArray  bytesDecode =  QByteArray::fromBase64(strSignedRawTx.toLatin1());
        if(bytesDecode.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("strSignedRawTx's is invalid base64-encoded string, please check it!");
            return false;
        }


        if(! (0 == strCoinType.compare("XLM", Qt::CaseSensitive)))
        {
            if(NULL != pStrErr) *pStrErr = QString("strCoinType's %1 is invalid, must be 'XLM' .").arg(strCoinType);
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

        if(!( strSrcAddr.startsWith('G')
            && 56 == strSrcAddr.length()
            ))
         {
            if(NULL != pStrErr) *pStrErr = QString("`strSrcAddr`: `%1`  is invalid address . please check it.").arg(strSrcAddr);
            return false;
         }

        if(!( strDstAddr.startsWith('G')
            && 56 == strDstAddr.length()
            ))
         {
            if(NULL != pStrErr) *pStrErr = QString("`strDstAddr`: `%1` is  invalid address . please check it.").arg(strDstAddr);
            return false;
         }


        if(!boost::all( strAmount.toStdString(), boost::is_alnum() || boost::is_any_of(".")) )
        {
           if(NULL != pStrErr) *pStrErr = QString("`strAmount`: `%1` is not invalid amount . please check it.").arg(strAmount);
           return false;
        }

        if( !( 0.000001 < dAmount&& dAmount < 9999999))
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmountInXRP`: `%1` is not invalid amount . please check it.").arg(dAmount);
            return false;
        }

//        if( ! (0 < uDestinationTag && uDestinationTag < UINT_MAX))
//        {
//            if(NULL != pStrErr) *pStrErr = QString("`uDestinationTag`: `%1` is not invalid uDestinationTag . please check it.").arg(uDestinationTag);
//            return false;
//        }

        if( !(0 <= uSequence  && uSequence < ULLONG_MAX) )
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
    QString             strAmount;
    double              dAmount;
    uint64_t            uSequence;
    QString             strSrcAddr;
    QString             strDstAddr;
//    uint64_t            uDestinationTag;
    QString             strSignedRawTx; //交易

public:
    QString             strTxid;
    QString             strErrMsg;

};




}
#endif //__XLM_RAW_TX_UTILS_H__
