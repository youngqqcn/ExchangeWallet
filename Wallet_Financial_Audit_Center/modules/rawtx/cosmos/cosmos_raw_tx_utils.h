/*************************************************************************************************
*文件作者:   yangqingqing
*作者邮箱:   youngqqcn@gmail.com
*创建日期:   2019-05-10  14:41
*文件说明:   cosmos裸交易工具类  支持 USDP, HTDF 的交易创建和签名  以及 广播数据的拼装编码 等
*修改记录:

修改日期          修改人                  修改说明                                             版本号
2019-05-10      yangqingqing            创建文件                                            v0.1

*************************************************************************************************/



#ifndef _COSMOS_RAW_TX_UTILS_
#define _COSMOS_RAW_TX_UTILS_
#pragma execution_character_set("utf-8")

#include <QObject>
#include "raw_tx_utils_base.h"
#include "cosmos/CCosmos.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数

namespace rawtx
{
Implement CCosmosRawTxUtilsImpl;
Implement Cosmos_ExportRawTxParam;
Implement Cosmos_ExportRawTxItem;

//导入 参数类
using Cosmos_ImportRawTxParam = Cosmos_ExportRawTxParam;
using Cosmos_ImportRawTxItem = Cosmos_ExportRawTxItem;


typedef struct _CosmosAccountInfo
{
    explicit _CosmosAccountInfo()
    {
        strCoinType         = "";
        strAddress          = "";

        bValidAccountFlag   = false;

        dBalance            = 0.0;
        uSequence           = INTMAX_MAX;
        uAccountNumber      = INTMAX_MAX;
    }

public: //输入
    QString     strCoinType;            //usdp  htdf
    QString     strAddress;

public: //输出
    bool        bValidAccountFlag;      //标识此账户目前是否是正常账户(必须有交易记录)

    double      dBalance;               //账户余额
    uint64_t    uSequence;              //交易次数,离签时的参数
    uint64_t    uAccountNumber;         //账户序号,离签时的参数
}CosmosAccountInfo;


Implement Cosmos_CreateRawTxParam : public IRawTxUtilsParamBase
{
    friend Implement CCosmosRawTxUtilsImpl;

    explicit Cosmos_CreateRawTxParam() { Init(); }
    virtual ~Cosmos_CreateRawTxParam(){}

    virtual void Init() override
    {
        csRawTx         = cosmos::CsRawTx();
        strPrivKey      = "";
        strHexData      = "";

        _strPubKey       = "";
        _csBcastTx       = cosmos::CsBroadcastTx();
    }

    virtual bool ParamsCheck(QString *pStrErr) noexcept(true) override
    {
        if(UINT_PRIV_KEY_LEN != strPrivKey.size() / 2)
        {
            if(NULL != pStrErr) *pStrErr = "strPrivKey's length is not 32-bytes . privkey must be 32 bytes.";
            return false;
        }

        /*if(UINT_PUB_KEY_LEN != _strPubKey.size() / 2)
        {
            if(NULL != pStrErr) *pStrErr = "strPubKey's length is not 33-bytes. pubkey must be 33 bytes.";
            return false;
        }*/

        std::string cstrErrMsg;
        if(false == csRawTx.ParamsCheck(cstrErrMsg))
        {
            if(NULL != pStrErr) *pStrErr = QString( cstrErrMsg.c_str() );
            return false;
        }

        return true;
    }


    std::string GetPubKey(bool bBase64Encode=true)
    {
        std::string cstrPrivKey  = this->strPrivKey.toStdString();
        std::string cstrPubKey;
        cosmos::PrivKeyToCompressPubKey(cstrPrivKey, cstrPubKey);
        std::string cstrPubKeyBin = cosmos::HexToBin(cstrPubKey);
        std::string cstrB64Pubkey =  cosmos::Base64Encode(cstrPubKeyBin.data(), cstrPubKeyBin.size(), false);
        return (bBase64Encode) ? cstrB64Pubkey : (cstrPubKey);
    }

public: //输入
    cosmos::CsRawTx         csRawTx;                //交易信息
    QString                 strPrivKey;             //私钥(十六进制字符串形式, 不是二进制形式)
public: //输出
    QString                 strHexData;             //编码后的广播数据(  hexencode(_csBcastTx.ToString()) )

protected:
    QString                 _strPubKey;              //公钥 : 由私钥推到出公钥(压缩), 不需要用户手动输入公钥
    cosmos::CsBroadcastTx   _csBcastTx;              //签完名后组装的数据
};

Implement Cosmos_ExportRawTxItem : public IRawTxUtilsParamBase
{
    explicit Cosmos_ExportRawTxItem() {Init();}
    virtual void Init() override
    {
        strOrderId              = "";
        strCoinType             = "";
        bComplete               = false;

        strChainId              = "";
        strSequence             = "";
        strFrom                 = "";
        strTo                   = "";
        strValue                = "";
        strFeeAmount            = "";
        strFeeGas               = "";
        strMemo                 = "";

        strSignedRawTxHex       = "";
    }

    virtual bool ParamsCheck(QString *pStrErr) override
    {
        if(strOrderId.isEmpty() || !boost::all(strOrderId.toStdString(),  boost::is_digit())   /*|| strOrderId.size() != UINT_ORDER_ID_LEN*/)
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId is empty");
            return false;
        }

//        if(!(STR_HTDF == strCoinType || STR_USDP == strCoinType ||  STR_HET == strCoinType))
//        {
//            if(NULL != pStrErr) *pStrErr = QString("strCoinType is invalid. Must be %1 or %2 or %3.").arg(STR_HTDF).arg(STR_USDP).arg(STR_HET);
//            return false;
//        }

        if(!(STR_MAINCHAIN == strChainId || STR_TESTCHAIN == strChainId))
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainId is  invalid. Must be %1 or %2.").arg(STR_MAINCHAIN).arg(STR_TESTCHAIN);
            return false;
        }

        if( !boost::all( strAccountNumber.toStdString(), boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strAccountNumber: %1, is invalid digit. Must be digit string.").arg(strAccountNumber);
            return false;
        }

        if(strSequence.isEmpty() || !boost::all( strSequence.toStdString(), boost::is_digit()))
        {
            if(NULL != pStrErr) *pStrErr = QString("strSquence: %1, is invalid digit. Must be digit string.").arg(strSequence);
            return false;
        }

        int nAddrLen = UINT_ADDR_LEN(strFrom.toStdString());
        if(strFrom.isEmpty() || nAddrLen != strFrom.size()  || !(strFrom.startsWith("usdp1") || strFrom.startsWith("htdf1") || strFrom.startsWith("0x")))
        {
            if(NULL != pStrErr) *pStrErr = QString("strFrom: %1 , is invalid address. Must start with  `usdp1` or `htdf1` or `0x`.").arg(strFrom);
            return false;
        }

        if(strTo.isEmpty() || nAddrLen != strTo.size()  || !(strTo.startsWith("usdp1") || strTo.startsWith("htdf1") || strTo.startsWith("0x") ))
        {
            if(NULL != pStrErr) *pStrErr = QString("strTo: %1 , is invalid address. Must be %2 bytes AND Must start with  `usdp1` or `htdf1` or `0x`.")
                                                    .arg(strFrom).arg(nAddrLen);
            return false;
        }

        //必须是浮点数字符串
        if(strValue.isEmpty() ||  !boost::all(strValue.toStdString(), boost::is_from_range('0', '9') || boost::is_any_of(".")) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strValue: %1, is invalid value, Must float string is dec.").arg(strValue);
            return false;
        }

        //必须是整数字符串
        if(strFeeAmount.isEmpty() ||  !boost::all(strFeeAmount.toStdString(), boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strFeeAmount: %1, is invalid value, Must dec string.").arg(strFeeAmount);
            return false;
        }

        //必须是整数字符串2
        if(strFeeGas.isEmpty() ||  !boost::all(strFeeGas.toStdString(), boost::is_digit()) )
        {
            if(NULL != pStrErr) *pStrErr = QString("strFeeGas: %1, is invalid value, Must dec string.").arg(strFeeGas);
            return false;
        }

        //如果不为空(签完名之后,导出已签名交易的时候才有这种情况, 所以, 此判断是为已签名交易导出时做参数判断提供的)
        if(false == strSignedRawTxHex.isEmpty())
        {
            //必须是十六进制字符串
            if(!boost::all(strSignedRawTxHex.toStdString(), boost::is_xdigit()))
            {
                if(NULL != pStrErr) *pStrErr = QString("strSignedRawTxHex is invalid hex str. strSignedRawTxHex : %1.").arg(strSignedRawTxHex);
                return false;
            }
        }

        return true;
    }

public: //参数
    QString             strOrderId;         //订单编号
    QString             strCoinType;        //币名 usdp , htdf
    bool                bComplete;          //是否已完成签名

public:
    QString             strChainId;         //chaiid , mainchain 或者  testchain
    QString             strSequence;        //交易次数  类似eth中的nonce
    QString             strAccountNumber;   //账户号
    QString             strFrom;            //from 源地址
    QString             strTo;              //to 目的地址
    QString             strValue;           //金额
    QString             strFeeAmount;       //手续费最大金额??  TODO:具体手续费计算规则还需讨论
    QString             strFeeGas;          //gasprice ??? TODO:同feeamount 需讨论
    QString             strMemo;           //memo 默认填写订单号

public:
    QString             strSignedRawTxHex;  //已签名的交易数据(已经组装完成)可以直接进行广播的数据
};

struct Cosmos_ExportRawTxParam : public CImpExpParamBase<Cosmos_ExportRawTxItem>
{
};


//交易广播参数类
Implement Cosmos_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
    explicit Cosmos_BroadcastRawTxParam() { Init(); }
    virtual void Init() override
    {
        strCoinType         = "";
        strChainId          = "";
        strValue            = "";
        strSequence         = "";
        strFrom             = "";
        strTo               = "";
        strSignedRawTxHex   = "";

        strTxid             = "";
        strErrMsg           = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) noexcept(true) override
    {
//        if( !(0 == strCoinType.compare(STR_USDP, Qt::CaseInsensitive)
//                ||  0 == strCoinType.compare(STR_HTDF, Qt::CaseInsensitive)
//                ||  0 == strCoinType.compare(STR_HET, Qt::CaseInsensitive)) )
//        {
//            if(NULL != pStrErr) *pStrErr = QString("strCoinType is invalid. Must be %1 or %2 or %3.").arg(STR_HTDF).arg(STR_USDP).arg(STR_HET);
//            return false;
//        }

        if(!(0 == strChainId.compare(STR_MAINCHAIN, Qt::CaseInsensitive) || 0 == strChainId.compare(STR_TESTCHAIN, Qt::CaseInsensitive)))
        {
            if(NULL != pStrErr) *pStrErr = QString("strChainId is  invalid. Must be %1 or %2.").arg(STR_MAINCHAIN).arg(STR_TESTCHAIN);
            return false;
        }

        if(strSequence.isEmpty() || !boost::all( strSequence.toStdString(), boost::is_digit()))
        {
            if(NULL != pStrErr) *pStrErr = QString("strSquence: %1, is invalid digit. Must be digit string.").arg(strSequence);
            return false;
        }

        int nAddrLen = UINT_ADDR_LEN(strFrom.toStdString());
        if(strFrom.isEmpty() || nAddrLen != strFrom.size()  || !(strFrom.startsWith("usdp1") || strFrom.startsWith("htdf1") || strFrom.startsWith("0x") ))
        {
            if(NULL != pStrErr) *pStrErr = QString("strFrom: %1, is invalid address. Must start with  `usdp1` or `htdf1` or `0x`.").arg(strFrom);
            return false;
        }

        if(strTo.isEmpty() || nAddrLen != strTo.size()  || !(strTo.startsWith("usdp1") || strTo.startsWith("htdf1") || strTo.startsWith("0x") ))
        {
            if(NULL != pStrErr) *pStrErr = QString("strTo: %1, is invalid address. Must be %2 bytes AND Must start with  `usdp1` or `htdf1` or `0x`.")
                                                    .arg(strTo).arg(nAddrLen);
            return false;
        }

        //必须是浮点数字符串
//        if(strValue.isEmpty() ||  !boost::all(strValue.toStdString(), boost::is_from_range('0', '9') || boost::is_any_of(".")) )
//        {
//            if(NULL != pStrErr) *pStrErr = QString("strValue: %1, is invalid value, Must float string is dec.").arg(strValue);
//            return false;
//        }

        //必须是十六进制字符串
        if(strSignedRawTxHex.isEmpty() || !boost::all(strSignedRawTxHex.toStdString(), boost::is_xdigit()))
        {
            if(NULL != pStrErr) *pStrErr = QString("strSignedRawTxHex is invalid hex str. strSignedRawTxHex : %1.").arg(strSignedRawTxHex);
            return false;
        }


        //广播完成后才有txid, 此参数判断不用于入参判断
        if( !strTxid.isEmpty() )
        {
            if(UINT_TXID_CHAR_LEN != strTxid.size())
            {
                if(NULL != pStrErr) *pStrErr = QString("strTxid's length is %1. Txid length must  %1 char length .").arg(strTxid.size()).arg(UINT_TXID_CHAR_LEN);
                return false;
            }

            if(false == boost::all(strTxid.toStdString(), boost::is_xdigit()) )
            {
                if(NULL != pStrErr) *pStrErr = QString("strTxid is invalid hex str.");
                return false;
            }
        }
        return true;
    }

public: //输入
    QString             strOrderId;             //订单号(防止交易重发)

    QString             strCoinType;            //币种
    QString             strChainId;             //chainId
    QString             strValue;               //金额value
    QString             strSequence;            //交易次数  sequence
    QString             strFrom;                //目的地址
    QString             strTo;                  //源地址
    QString             strSignedRawTxHex;      //广播数据(已签名的交易十六进制字符串, 可以直接广播)
public: //输出
    QString             strTxid;                //txid  交易id
    QString             strErrMsg;              //错误信息
    bool                bSure;                  //广播的这笔交易, 是否可以确定成功,   true:100%成功   false:已经广播但还不能确定是否能够成功
    //QString             strGasUsed;             //实际用掉的gas
};


Implement CCosmosRawTxUtilsImpl : public IRawTxUtils
{
public:
    //创建裸交易的接口  Cosmos的实现
    //注意:  此接口包含了创建交易 + 签名交易 两部分,
    //  其中  Cosmos_CreateRawTxParam.strHexData  是签名后并组装后的十六进制字符串
    //  是可以直接调用服务端接口进行广播的, 不需要额外进行拼装操作
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override ;

    //导出交易接口  Cosmos的实现
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //导入交易接口  Cosmos的实现
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //广播交易接口  Cosmos的实现
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;


public:
    //获取账户信息
    virtual int GetAccountInfo(CosmosAccountInfo &accountInfo)noexcept(false);
    virtual int GetBalance(const QString& strCoinType, const QString& strAddr, double& dBalance);
    virtual int GetBalance_HRC20(const QString &strAddr,  std::map<QString, double> &mapBalances ) noexcept(false);


public:
    //2020-04-15  yqq
    //HRC20    将bech32格式地址转为  hexstr 格式,  不补齐32字节(不在左边填充 '0' )
    static bool Bech32AddrToHexStrAddr(const QString &strBech32AddrIn, QString &strHexStrAddrOut)  noexcept(false);

    //static bool  HexStrAddrToBech32Addr(const QString &strHrp, const QString &strHexStrAddrIn, QString &strBech32AddrOut);

    //构造  HRC20 transfer 数据
    static int Make_HRC20_Transfer_Data(const QString &strTo, const QString &strHexValueInWei, std::string &cstrRetHexStrData) noexcept(false);

    //将 HRC20 浮点金额  转为  wei , 同 ERC20  wei
    static int ToWeiStr(double dAmount , int nDecimals, std::string &cstrRet) noexcept(false) ;


};



}




#endif //_COSMOS_RAW_TX_UTILS_
