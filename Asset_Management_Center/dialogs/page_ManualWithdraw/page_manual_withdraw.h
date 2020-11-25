#ifndef _WITHDRAW_H_
#define _WITHDRAW_H_

/*******************************************************************************************************************
 *
 * 说明: 处理充值列表的业务主要业务
 *
*******************************************************************************************************************/

#include "am_main.h"
#include "comman.h"
#include "rawtx_comman.h"
#include "btc/btc_off_sig.h"
#include "btc/btc_raw_tx_util.h"
#include "usdt/usdt_off_sig.h"
#include "usdt/usdt_raw_tx_util.h"
#include "modules/coinWithdraw/autowithdraw.h"
#include "eth/eth_raw_tx_utils.h"  //以太坊裸交易工具类 add by yqq 2019-04-29
#include "cosmos/cosmos_raw_tx_utils.h" //USDP 和 HTDF  added by yqq 2019-05-13
#include "xrp/xrp_raw_tx_utils.h" //xrp工具类  2019-12-16
#include "eos/eos_raw_tx_utils.h"  //EOS 工具类  2020-01-02
#include "xlm/xlm_raw_tx_utils.h"  //XLM 工具类,  2020-02-12
#include "trx/trx_raw_tx_utils.h" //TRX 工具类  2020-03-05
#include "xmr/xmr_raw_tx_utils.h" //XMR 工具类 2020-03-08
#include "utils.h"

#if defined(BTC_SERIES_EX)
#include "btc/btc_raw_tx_utils.h"
#endif



//充币列表的 命名空间
namespace NS_Withdraw{


//搜索条件
typedef struct _SEARCHCOND
{
    _SEARCHCOND()
    {
        uDatetimeEnd = 0;
        uDatetimeStart = 0;
    }
    QString strUserID;     //用户ID
    QString strUserTel;    //用户电话
    QString strTxID;       //交易ID
    QString strAddr;       //交易地址
    QString strCoinType;   //币种
    uint uiWithdrawStatus; //提现状态
    uint uiAuditStatus;    //审核状态
    uint uiQueryType;      //查询类型0:自动提币表 1:人工提币-初审 2:人工提币-复审 3: 人工提币-提币清单
    uint uDatetimeStart;
    uint uDatetimeEnd;
}SEARCHCOND;

//搜索结果  提币信息
typedef struct _WITHDRAWINFO
{
    QString strOrderID;      //订单编号
    QString strTime;         //提现时间
    QString strUserID;       //用户ID
    QString strUserTel;      //用户手机
    QString strUserName;     //用户名称
    QString strCoinType;     //提现币种
    QString strAmount;        //提现数量
    QString strTranFee;       //手续费( 手续费 包含矿工费)
    QString strTxFee;        //矿工费用
    QString strAuditStatus;  //审核状态
    QString strOrderStatus;  //提现状态
    QString strTxID;         //TxID
    QString strDstAddr;         //提现地址
    QString strAuditor;      //审核人
    QString strRemarks;      //审核备注
    QString strAuditTime;    //审核时间
    QString strCompleteTime; //完成时间
}WITHDRAWINFO;



typedef struct _WithdrawData
{
    QString strCoinType;     //币种
    QString strOrderID;     //订单编号
    QString strDstAddr;     //目的地址
    QString strSrcAddr;     //源地址
    QString strAmount;      //金额
    QString strTxFee;       //矿工费
}WithdrawData;




struct _FlowData;
struct _UserInfo;
struct _AssetData;

typedef struct _UserInfo
{
    QString strUserId;  //用户id
    QString strTelNo; //用户手机号码
    QString strUserName; //用户姓名
    QString strUserEmail; //用户邮箱
}UserInfo;


typedef struct _UserFlow
{
    _UserInfo   userBasicInfo;  //用户基本信息
    list<_FlowData>  lstFlowData;  //用户流水

}UserFlow;

typedef struct _UserAsset
{
    _UserInfo  userBasicInfo;   //用户基本信息
    list<_AssetData>  lstAssetData; //用户资产
}UserAsset;


typedef struct _FlowData
{
    QString strCoinName;  //币种
    QString strTxType;    //交易类型
    QString strCount;     //交易数量
    QString strBalance;   //余额
    QString strTime;      //交易时间
}FlowData;



typedef struct _AssetData
{
    _AssetData()
    {
        strCoinName  = "---";
        strTotalInCoin= "---";
        strTotalOutCoin= "---";
        strTotalAssets= "---";
        strAvailVol= "---";
        strStatus  = "---";
        strWithdrawAmount= "---";
        strOrderId= "---";
        strPlatformFee= "---";
        strFreezeVol= "---";
        strLockVol= "---";
        strWithdrawFreeze = "---";
        strDelegateVol= "---";
    }

    QString strCoinName;    //币种
    QString strTotalInCoin; //充币总数量
    QString strTotalOutCoin; //提币总数量
    QString strTotalAssets;   //总余额
    QString strAvailVol;   //可提币余额
    QString strStatus;     //当前提现状态
    QString strWithdrawAmount;  //当前提现金额

    QString strOrderId; //订单号
    QString strPlatformFee; //手续费(平台手续费)  不是矿工费

    QString strFreezeVol;//冻结金额
    QString strLockVol; //锁定金额
    QString strDelegateVol; //委托数量
    QString strWithdrawFreeze; //提币冻结
}AssetData;

}




class CPageManualWithdraw : public CAMMain     //继承自资产基类
{
public:
    enum
    {
        AUTOWITHDRAW = 0,       //自动提币表
        MANUALWITHDRAW_1 = 1,    //初审
        MANUALWITHDRAW_2 = 2,    //复审
        MANUALWITHDRAW_ALL = 3,  //人工提币表总表
    };
public:
    //explicit CWithdraw(QObject *parent = nullptr);
    explicit CPageManualWithdraw(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageManualWithdraw();


public:
    virtual int Init() override;


public://(耗时操作)

    //获取用户资产数据接口
    int PullUserAssetDataFromServer(const QString &strInUrl, const QString &strInUserID) noexcept(false);

     //获取用户流水接口
    int PullUserFlowFromServer(const QString &strInUrl, const QString &strInUserID) noexcept(false);


public:

    //根据条件查询数据库
    int  ConditionSearch(const NS_Withdraw::SEARCHCOND &searchCond,  vector<NS_Withdraw::WITHDRAWINFO> &vctWithdrawInfo) noexcept(false);

    //查询自动提币表
    int ConditionSearch_AutoWithDraw(const NS_Withdraw::SEARCHCOND &searchCond,  vector<NS_Withdraw::WITHDRAWINFO> &vctWithdrawInfo) noexcept(false);

    //更新订单审核状态
    int  UpdateAuditStatus(const QString& strOrderID, const QString& strAuditor, const QString& strRemark, int iStatus) noexcept(false);


    //查询复审已通过的订单数据
    int SearchManualWithdrawOrder(const QString &strCoinType, const QString &strDstAddr,  vector<NS_Withdraw::WithdrawData> &vctManualWithdrawData) noexcept(false);

    //创建未签名交易
    int CreateUnsignedRawTx(const QString &strCoinType, const vector<int> &vctRowsSel, const QString &strSrcAddr) noexcept(false);

    //导出未签名交易
    int ExportRawTxFile(const QString &strCoinType, const QString &strExportFilePath)noexcept(false);

    //导入已签名交易
    int ImportRawTxFile(const QString &strCoinType,  const QString &strImportFilePath)noexcept(false);

    //广播已签名的交易
    int BroadcastRawTx(const QString &strCoinType, const vector<int> &vctRowsSel) noexcept(false);

    //获取订单状态, 是否可以被广播
    bool JudgeBroadcastable(const QString &strOrderID) noexcept(false);

    //计算提现额度
    QString CalculateTotalAmount( const vector<int> &vctRowsSel) noexcept(true);

    QString CalculateTxFee(const QString& strCoinType, const vector<int> &vctRowsSel) noexcept(true);

    //验证数据库中的订单审核状态
    bool VerifyAuditStatus(const QString &strOrderId, int iStatus) noexcept(false);


public: //处理XMR

    //导出 交易输出
    int XMR_ExportTxOutputs(QString &strExportFilePath)  noexcept(false);

    //导入已签名的 key-images
    int XMR_ImportKeyImages(const QString &strImportFilePath)  noexcept(false);


public:
    //返回导出交易的个数
    int GetExportDataCount(const QString &strCoinType) noexcept(true);

    //2019-04-30 yqq, 将获取导入数据的接口, 修改为模板成员函数, 以适配多币种
    const void *const GetImportData(const QString &strCoinType)const noexcept(true)
    {
        QString strCoinTypeTmp = strCoinType;
        strCoinTypeTmp = strCoinType.trimmed();

        if(0 == strCoinTypeTmp.compare("btc", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
        )
        {
#if defined(BTC_SERIES_EX)
            return  (void const * const)( &__m_btcImportRawTxParams );
#else
            return  (void const * const)( &__m_btcImportData );
#endif
        }
        if (0 == strCoinTypeTmp.compare("usdt", Qt::CaseInsensitive))
            return  (void const * const)( &__m_usdtImportData );
        else if(0 == strCoinTypeTmp.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
                || 0 == strCoinTypeTmp.compare("etc", Qt::CaseInsensitive) )
            return (void const * const)( &__m_ethImportRawTxParams );
        else if(0 == strCoinTypeTmp.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinTypeTmp.compare("htdf", Qt::CaseInsensitive)
                || 0 == strCoinTypeTmp.compare("het", Qt::CaseInsensitive)
                || utils::Is_HRC20_Token(strCoinTypeTmp)
                )
        {
            return (void const * const)( &__m_cosmosImportRawTxParams);
        }
        else if( 0 == strCoinTypeTmp.compare("xrp", Qt::CaseInsensitive))
        {
            return (void const * const)( &__m_xrpImportRawTxParams);
        }
        else if( 0 == strCoinTypeTmp.compare("eos", Qt::CaseInsensitive))
        {
            return (void const * const)( &__m_eosImportRawTxParams);
        }
        else if( 0 == strCoinTypeTmp.compare("xlm", Qt::CaseInsensitive))
        {
            return (void const * const)( &__m_xlmImportRawTxParams);
        }
        else if( 0 == strCoinTypeTmp.compare("trx", Qt::CaseInsensitive))
        {
            return (void const * const)( &__m_trxImportRawTxParams);
        }
        else if( 0 == strCoinTypeTmp.compare("xmr", Qt::CaseInsensitive))
        {
            return (void const * const)( &__m_xmrImportRawTxParams);
        }

        return (void const* const)NULL;
    }

    const void *const GetUserFlowData( )const noexcept(true)
    {

        return (void const * const)( &__m_userFlowData);
    }

    const void *const GetUserAssetData( )const noexcept(true)
    {
        return (void const * const)( &__m_userAssetData);
    }

    void ClearImportData() noexcept(true)
    {
        __m_cosmosImportRawTxParams.clear();
        __m_btcImportData.vctExportItems.clear();
        __m_usdtImportData.vctExportItems.clear();
        __m_ethImportRawTxParams.clear();
        __m_xrpImportRawTxParams.clear();
        __m_eosImportRawTxParams.clear();
        __m_xlmImportRawTxParams.clear();
        __m_trxImportRawTxParams.clear();
        __m_xmrImportRawTxParams.clear();
    }


protected:

private:
    int __ParseUserAssetData(const QByteArray &arrayJsonData) noexcept(false);
    int __ParseUserFlowData(const QByteArray &arrayJsonData, int *poutPageCnt) noexcept(false) ;
private:

    vector<NS_Withdraw::WithdrawData> __m_vctManualWithdrawData; //复审已通过的订单数据
    BTCExportData __m_btcExportData; //导出的数据
    BTCImportData __m_btcImportData; //导入的数据
//    CBTCRawTxUtil __m_btcRawTxUtil;
//    CLTCRawTxUtil __m_ltcRawTxUtil;


#if defined(BTC_SERIES_EX)
    rawtx::btc::CBTCRawTxUtilsImpl   __m_btcRawTxUtilEx;
    rawtx::btc::BTC_ExportRawTxParam __m_btcExportRawTxParams;
    rawtx::btc::BTC_ImportRawTxParam __m_btcImportRawTxParams;
#endif


    //USDT数据
    BTCExportData __m_usdtExportData; //导出的数据
    BTCImportData __m_usdtImportData; //导入的数据
    CUSDTRawTxUtil __m_usdtRawTxUtil;


    //ETH裸交易工具
    rawtx::CETHRawTxUtilsImpl       __m_ethRawTxUtils;              //eth工具对象
    rawtx::ETH_ExportRawTxParam     __m_ethExportRawTxParams;       //导出的数据
    rawtx::ETH_ImportRawTxParam     __m_ethImportRawTxParams;       //导入的数据


    //added by yqq 2019-05-13
    //USDP 和 HTDF 裸交易
    rawtx::CCosmosRawTxUtilsImpl    __m_cosmosRawTxUtils;
    rawtx::Cosmos_ExportRawTxParam  __m_cosmosExportRawTxParams;
    rawtx::Cosmos_ImportRawTxParam  __m_cosmosImportRawTxParams;



    //xrp相关数据
    rawtx::CXrpRawTxUtilsImpl     __m_xrpRawTxUtils;
    rawtx::Xrp_ExportRawTxParam     __m_xrpExportRawTxParams;
    rawtx::Xrp_ImportRawTxParam     __m_xrpImportRawTxParams;

    //eos 相关
    rawtx::CEosRawTxUtilsImpl   __m_eosRawTxUtils;
    rawtx::Eos_ExportRawTxParam     __m_eosExportRawTxParams;
    rawtx::Eos_ImportRawTxParam     __m_eosImportRawTxParams;

    //xlm相关数据
    rawtx::CXlmRawTxUtilsImpl     __m_xlmRawTxUtils;
    rawtx::Xlm_ExportRawTxParam     __m_xlmExportRawTxParams;
    rawtx::Xlm_ImportRawTxParam     __m_xlmImportRawTxParams;


    //trx相关数据
    rawtx::CTrxRawTxUtilsImpl     __m_trxRawTxUtils;
    rawtx::Trx_ExportRawTxParam     __m_trxExportRawTxParams;
    rawtx::Trx_ImportRawTxParam     __m_trxImportRawTxParams;

    //xmr相关数据
    rawtx::CXmrRawTxUtilsImpl     __m_xmrRawTxUtils;
    rawtx::XMR_ExportRawTxParam     __m_xmrExportRawTxParams;
    rawtx::XMR_ImportRawTxParam     __m_xmrImportRawTxParams;

    //用户流水和用户资产
    NS_Withdraw::UserFlow    __m_userFlowData;
    NS_Withdraw::UserAsset   __m_userAssetData;

    vector<NS_Withdraw::WITHDRAWINFO>  __m_vctWithdrawData;

public slots:

private:
    QMap<QString, QString> __m_mapTradeType;
    QMap<QString, QString> __m_mapAuditStatus;
    QMap<int, QString> __m_mapOrderStatus;
    map<QString, NS_Withdraw::UserInfo> __m_mapUserInfo;    //用户基本信息
};

#endif // WITHDRAW_H_
