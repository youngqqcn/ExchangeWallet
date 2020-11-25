/*
 * 作者: yqq
 * 日期: 2019-08-23
 * 说明:重构 BTC代码, 以适应BTC系列的币种
 *
 */

#ifndef _BTC_RAW_TX_UTILS_H_
#define _BTC_RAW_TX_UTILS_H_


#include "raw_tx_utils_base.h"
#include "rawtx_comman.h"
#include "boost/algorithm/string.hpp"  //使用boost 字符串工具
#include "boost/lexical_cast.hpp"   //使用boost 转出函数

#include "utils.h"

namespace   rawtx
{
namespace  btc
{

Implement CBTCRawTxUtilsImpl;
Implement BTC_ExportRawTxParam;
Implement BTC_ExportRawTxItem;

//导入 参数类
using BTC_ImportRawTxParam = BTC_ExportRawTxParam;
using BTC_ImportRawTxItem = BTC_ExportRawTxItem;


/*
typedef struct _UTXO
{
    _UTXO()
    {
        txid = "";
        address = "";
        vout = 0xffffff;
        scriptPubKey = "";
        redeemScript = "";
        amount = 0.00000;
    }

    QString  txid;             //txid
    QString  address;          //地址
    unsigned int vout;         //此utxo在交易中output的序号
    QString scriptPubKey;      //公钥脚本
    QString redeemScript;    //赎回脚本, p2sh地址需要填写, p2skh不需要此字段
    double amount;             //金额
}UTXO;
*/

Implement CBTCRawTxUtilsImpl : public IRawTxUtils
{
public:
    explicit CBTCRawTxUtilsImpl(){}
    virtual ~CBTCRawTxUtilsImpl(){}

public:
    //实现创建交易接口
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override ;

    //实现,导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override;

    //实现导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //实现广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override;

public:

    //签名接口
    virtual int SignRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) ;


};

//创建交易的参数类
Implement BTC_CreateRawTxParam : public IRawTxUtilsParamBase
{
public:
    explicit BTC_CreateRawTxParam()  {  Init();  }
    virtual ~BTC_CreateRawTxParam() {}

    //所有子类必须实现Init, 并在构造函数总调用
    virtual void Init() noexcept(true) override
    {
        strCoinType = "";
        strURL = "";
        vctDstAddr.clear();
        vctSrcAddr.clear();
        dAmount = 0.0;

        dTxFee = 0.0;
        mapTxOut.clear();
        vctRefUTXOs.clear();
        strRawTxHex = "";
    }

    virtual bool ParamsCheck(QString *pStrErr)  noexcept(true) override
    {
        if(! (0 == strCoinType.compare(strCoinType, Qt::CaseInsensitive)))
        {
            if(NULL != pStrErr) *pStrErr = QString("`strCoinType` is '%1', not support.").arg(strCoinType);
            return false;
        }

        if(strURL.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strURL` is empty.").arg(strCoinType);
            return false;
        }

        if(vctSrcAddr.empty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`vctSrcAddr` is empty.").arg(strCoinType);
            return false;
        }

        if(vctDstAddr.empty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`vctDstAddr` is empty.").arg(strCoinType);
            return false;
        }

        //检查源地址
        for(QString strAddr : vctSrcAddr)
        {
            if(!utils::IsValidAddr(strCoinType, strAddr))
            {
                if(NULL != pStrErr) *pStrErr = QString("address check: %1 is invalid %2 address").arg(strAddr).arg(strCoinType);
                return false;
            }
        }

        //检查源地址
        for(QString strAddr : vctDstAddr)
        {
            if(!utils::IsValidAddr(strCoinType, strAddr))
            {
                if(NULL != pStrErr) *pStrErr = QString("address check: %1 is invalid %2 address").arg(strAddr).arg(strCoinType);
                return false;
            }
        }

        if(dAmount < 0.000001  || dAmount >= 2100*10000)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dAmount`: %1  is illegal.").arg(QString::asprintf("%.8f", dAmount));
            return false;
        }

        if(mapTransfer.empty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`mapTransfer` is empty.");
            return false;
        }


        //是否检查输出???
        /*if(dTxFee > 0.1)
        {
            if(NULL != pStrErr) *pStrErr = QString("`dTxFee`:%1 is too large").arg(QString::asprintf("%.8f", dTxFee));
            return false;
        }

        if(mapTxOut.size() < 2)
        {
            if(NULL != pStrErr) *pStrErr = QString("`mapTxOut`'s size is %1. Dangerous! Please check whether you missing payback?? ");
            return false;
        }

        if(vctRefUTXOs.empty())
        {
            if(NULL != pStrErr) *pStrErr = "`vctRefUTXOs` is empty, please check it.";
            return false;
        }

        if(strRawTxHex.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = "`strRawTxHex` is  empty";
            return false;
        }*/

        return true;
    }

public://输入参数
    QString strCoinType; //币种
    QString strURL; //请求的URL
    std::vector<QString> vctSrcAddr; //输入地址
    std::vector<QString> vctDstAddr; //输出地址
    double dAmount;//总金额
    std::map<QString, double> mapTransfer; //以便支持 n-n方式的转账

public://输出参数

    double dTxFee; //消耗的手续费
    std::map<QString, double> mapTxOut; //txout
    std::vector<UTXO> vctRefUTXOs; //引用的UTXOs
    QString strRawTxHex;//裸交易

};


//交易导入导出元素类
Implement BTC_ExportRawTxItem : public IRawTxUtilsParamBase
{
public:
    explicit BTC_ExportRawTxItem()  {  Init();  }
    virtual ~BTC_ExportRawTxItem() {}

    //所有子类必须实现Init, 并在构造函数总调用
    virtual void Init() noexcept(true) override
    {
        strCoinType = "";
        strOrderID =  "";
        strAmount = "";
        strTxFee = "";
        vctSrcAddr.clear();
        vctDstAddr.clear();
        strRawTxHex = "";
        vctRefUTXOs.clear();
        mapTxOut.clear();
        bComplete = false;
        uTokenId = 0xFFFFFF;
        strTokenAmount = "";
    }

    virtual bool ParamsCheck(QString *pStrErr)  noexcept(true) override
    {
        if(! (0 == strCoinType.compare("BTC", Qt::CaseInsensitive)))
        {
            if(NULL != pStrErr) *pStrErr = QString("`strCoinType` is '%1', not support.").arg(strCoinType);
            return false;
        }

        if(strOrderID.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strOrderID` is empty.");
            return false;
        }


        if(vctSrcAddr.empty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`vctstrSrcAddr` is empty.").arg(strCoinType);
            return false;
        }

        if(vctDstAddr.empty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`vctstrDstAddr` is empty.").arg(strCoinType);
            return false;
        }

        if(strAmount.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strAmount` is empty.");
            return false;
        }

        //是否检查输出???
        if(strTxFee.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strTxFee` is empty");
            return false;
        }

        if(mapTxOut.size() < 2)
        {
            if(NULL != pStrErr) *pStrErr = QString("`mapTxOut`'s size is %1. Dangerous! Please check whether you missing payback?? ");
            return false;
        }

        if(vctRefUTXOs.empty())
        {
            if(NULL != pStrErr) *pStrErr = "`vctRefUTXOs` is empty, please check it.";
            return false;
        }

        if(strRawTxHex.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = "`strRawTxHex` is  empty";
            return false;
        }

        return true;
    }

public:
    QString strCoinType;                     //币种
    QString strOrderID;                      //交易ID
    QString strAmount;                       //金额(保留8位小数)
    QString strTxFee;                        //矿工费(保留8位小数)
    std::vector<QString> vctSrcAddr;         //源地址
    std::vector<QString> vctDstAddr;         //目的地址
    QString strRawTxHex;                        //交易
    vector<UTXO>  vctRefUTXOs;                  //utxo集
    map<QString, double> mapTxOut;           //交易输出

    bool bComplete;//完成状态    true:已完成   false:未完成

    //usdt
    uint    uTokenId; // testnet-USDT : 2    mainnet-USDT:31
    QString   strTokenAmount; //代币的金额 即 usdt的金额

};


//交易导入导出参数类
struct BTC_ExportRawTxParam : public CImpExpParamBase<BTC_ExportRawTxItem>
{
};


//交易广播参数类
Implement BTC_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
public:
    explicit BTC_BroadcastRawTxParam()  {  Init();  }
    virtual ~BTC_BroadcastRawTxParam() {}

    //所有子类必须实现Init, 并在构造函数总调用
    virtual void Init() noexcept(true) override
    {
        strURL = "";
        strCoinType = "";
        strSignedRawTxHex = "";

        strTxid = "";
    }

    virtual bool ParamsCheck(QString *pStrErr)  noexcept(true) override
    {
        if(strURL.isEmpty())
        {
            if(NULL == pStrErr) *pStrErr = "`strURL` is empty.";
            return false;
        }

        if(! (0 == strCoinType.compare("BTC", Qt::CaseInsensitive)))
        {
            if(NULL == pStrErr) *pStrErr = QString("`strCoinType`:'%1', not support").arg(strCoinType);
            return false;
        }

        if(strSignedRawTxHex.isEmpty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`strSignedRawTxHex` is empty");
            return false;
        }

        //strTxid
        return true;
    }

public:
    QString strURL;
    QString strCoinType;
    QString strSignedRawTxHex; //已签名的交易

public:
    QString strTxid;
};


//交易签名参数类
Implement BTC_SignRawTxParam : public IRawTxUtilsParamBase
{
public:
    explicit BTC_SignRawTxParam()  {  Init();  }
    virtual ~BTC_SignRawTxParam() {}

    virtual void Init() noexcept(true) override
    {
        strCoinType.clear();
        bIsCollection = false;
        vctInAddrs.clear();
        mapTxout.clear();
        strUnsignedRawTxHex.clear();
        vctPrivKeys.clear();
        vctUTXOs.clear();

        strSignedRawTxHex.clear();
    }

    virtual bool ParamsCheck(QString *pStrErr)  noexcept(true) override
    {
        if(strCoinType.isEmpty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`strCoinType` is empty");
            return false;
        }

        if(vctInAddrs.empty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`vctInAddrs` is empty");
            return false;
        }

        if(mapTxout.empty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`mapTxout` is empty");
            return false;
        }

        if(strUnsignedRawTxHex.isEmpty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`strUnsignedRawTxHex` is empty");
            return false;
        }

        if(vctPrivKeys.empty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`vctPrivKeys` is empty");
            return false;
        }

        if(vctUTXOs.empty())
        {
            if(NULL == pStrErr) *pStrErr = QString("`vctUTXOs` is empty");
            return false;
        }

        //输入地址数和私钥个数不相等
        if(vctInAddrs.size() != vctPrivKeys.size())
        {
            if(NULL == pStrErr) *pStrErr = QString("vctTxInAddrs's size (%1) is not equals  vctPrivKeys's size(%2).\
                    you must missing some address's priv-keys. please check it. ")
                    .arg(vctInAddrs.size()).arg(vctPrivKeys.size());
            return false;
        }

        return true;
    }

public: //输入参数
    QString  strCoinType;               //币种
    bool bIsCollection;                 //是否为归集
    std::vector<QString> vctInAddrs;    //输入地址
    map<QString, double> mapTxout;      //txout, 交易输出
    QString strUnsignedRawTxHex;        //未签名的裸交易
    std::vector<QString> vctPrivKeys;   //私钥
    std::vector<UTXO> vctUTXOs;         //UTXO


public: //输出参数
    QString strSignedRawTxHex;          //签名后的交易

};

}

namespace ltc {
    using namespace btc;
    using  CLTCRawTxUtilsImpl = CBTCRawTxUtilsImpl;
    using  LTC_CreateRawTxParam = BTC_CreateRawTxParam;
    using LTC_ExportRawTxItem = BTC_ExportRawTxItem;
    using  LTC_ExportRawTxParam  = BTC_ExportRawTxParam;
    using LTC_BroadcastRawTxParam = BTC_BroadcastRawTxParam;
    using LTC_SignRawTxParam = BTC_SignRawTxParam;

}

}




#endif //_BTC_RAW_TX_UTILS_H_
