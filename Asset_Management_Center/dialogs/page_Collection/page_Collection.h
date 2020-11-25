/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      归集列表界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_COLLECTION_H
#define PAGE_COLLECTION_H

/*******************************************************************************************************************
 *
 * 说明: 处理充值列表的业务主要业务
 *
*******************************************************************************************************************/

#include "comman.h"
#include "am_main.h"
#include "modules/rawtx/btc/btc_raw_tx_util.h"
#include "modules/rawtx/usdt/usdt_raw_tx_util.h"
#include "modules/rawtx/eth/eth_raw_tx_utils.h"
#include "modules/rawtx/cosmos/cosmos_raw_tx_utils.h"
#include "trx/trx_raw_tx_utils.h"
#include "utils.h"

//充币列表的 命名空间
namespace Collection {

typedef struct _BigAccount
{
    QString strCoinType;  //币种
    QString strBalance;   //余额
    QString strAddr;      //地址
    QString strlastTime;  //最后一次查询时间
}BigAccount;


//搜索条件
typedef struct _SEARCHCOND
{
    QString strAddr;
    QString strCoinType;
    QString strAdminID;
    QString strAdminName;
    uint    uiStatus;
    uint    uiQueryType;
    uint    uDatetimeStart;
    uint    uDatetimeEnd;
}SEARCHCOND;


//搜索结果  充币信息
typedef struct _COLLECTIONINFO
{
    QString strOrderID;
    QString strCoinType;
    QString strAmount;
    QString strTokenAmount; //代币金额
    QString strTxFee;
    QString strSrcAddr;
    QString strOperator;
    QString strStatus;
    QString strTime;
    QString strAdminName;
    QString strDstAddr;
    QString strTxid;
}COLLECTIONINFO;
}

typedef struct _CollectionQueryRsp
{
    QString strCoinType;
    QString strAddr;

    //如果币种是ETH, 此值是ETH余额; 如果是ERC20 ,此值是代币的余额;
    //如果是HTDF, 此值是HTDF余额;  如果HRC20, 此值是代币的余额,
    //至于HRC20地址的HTDF  则在子类中保存  2020-04-20 yqq
    QString strAmount;

}CollectionQueryRsp;


typedef struct _ETHCollectionQeuryRsp : public CollectionQueryRsp
{
    quint32 uNonce;

    QString strETHBalance; //如果币种是ETH, 此值是ETH余额; 如果是ERC20 ,此值是ETH的余额
}ETHCollectionQeuryRsp;



typedef struct _CosmosCollectionQueryRsp : public CollectionQueryRsp
{
    quint32 uSequence; //sequence
    quint32 uAccountNumber; //账户序号
}CosmosCollectionQueryRsp;

typedef struct  _Hrc20TokensCollectionQueryRsp : public _CosmosCollectionQueryRsp
{
    QString strHTDFBalance; //HRC20 地址的 HTDF余额   2020-04-20  yqq
}Hrc20TokensCollectionQueryRsp;



class CPageCollection : public CAMMain     //继承自资产基类
{
    Q_OBJECT
public:
    explicit CPageCollection(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageCollection();


public:
    virtual int Init() override;


public:

    //调用服务端接口查询需要归集的地址余额
    int CollectionQuery(const QString &strCoinType, vector<CollectionQueryRsp> &vctColRsp) noexcept(false);


    //根据条件查询数据库
    int ConditionSearch_Collection_Log(Collection::SEARCHCOND &searchCond, vector<Collection::COLLECTIONINFO> &vctCollectionInfo) noexcept(false);


    //创建并导出
    int CreateAndExportUnsignedRawTx(
            const QString &strCoinType,
            const vector<int> &vctSelRows, //界面上选择的行索引
            const vector<QString> &vctSrcAddrs, //源地址
            const QString& strDstAddr, //目的地址
            QString &strRetExportFilePath) noexcept(false);   //创建并导出未签名归集交易


    //导入待广播文件
    int ImportRawTxFile(const QString& strCoinType, const QString &strImportFilePath) noexcept(false);


    //归集广播
    int BroadcastRawTx(const QString& strCoinType, const vector<int> &vctRowsSel,  vector<QString> &vctStrTxid) noexcept(false);

    //获取数据
    const void *const GetImportData(const QString &strCoinType)const
    {
        QString strCoinTypeTmp = strCoinType;
        strCoinTypeTmp = strCoinType.trimmed();

        if(0 == strCoinTypeTmp.compare("btc", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
        ){
            return  (void const * const)(&__m_btcImportData);
        }
        else if( 0 == strCoinType.compare("usdt", Qt::CaseInsensitive ))
        {
            return (void const * const)(&__m_usdtImportData);
        }
        else if(0 == strCoinTypeTmp.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinTypeTmp)
              || 0 == strCoinTypeTmp.compare("etc", Qt::CaseInsensitive)  )
        {
            return (void const * const)(&__m_ethImportRawTxParams);
        }
        else if(0 == strCoinTypeTmp.compare("usdp", Qt::CaseInsensitive)|| 0 == strCoinTypeTmp.compare("htdf", Qt::CaseInsensitive)
                || 0 == strCoinTypeTmp.compare("het", Qt::CaseInsensitive)
                || utils::Is_HRC20_Token(strCoinTypeTmp)
                )
        {
            return (void const * const)(&__m_cosmosImportRawTxParams);
        }
        else if( 0 == strCoinType.compare("trx", Qt::CaseInsensitive ))
        {
            return (void const * const)(&__m_trxImportRawTxParams);
        }



        return (void const* const)NULL;
    }

    const std::vector<ETHCollectionQeuryRsp> &GetETHCollectionRsp() const
    {
        return __m_vctEthCollectionQueryRsp;
    }

    const std::vector<Hrc20TokensCollectionQueryRsp> &Get_HRC20_CollectionRsp() const
    {
        return __m_vctHrc20CollectionQueryRsp;
    }


    int ERC20SupplyTxFee( const vector<QString> &vctAddrs, const QString& strERC20TokenName) noexcept(false);

    //HRC20补充手续费 2020-04-20
    int HRC20_SupplyTxFee( const vector<QString> &vctAddrs ) noexcept(false);

protected:


    //BTC广播
    int _BTC_BroadcastRawTx(const vector<int> &vctRowsSel,  vector<QString> &vctStrTxid) noexcept(false);


    //USDT广播
    int _USDT_BroadcastRawTx(const vector<int> &vctRowsSel,  vector<QString> &vctStrTxid) noexcept(false);


//    //ETH创建
//    int _ETH_CreateUnsignedRawTx(const vector<int> &vctRowsSel, const QString& strDestAddr)  noexcept(false);

    //ETH广播
    int _ETH_BroadcastRawTx(const vector<int> &vctRowsSel,  QString strCoinType) noexcept(false);



//    //USDP和HTDF 创建
//    int _COSMOS_CreateUnsignedRawTx(const vector<int> &vctRowsSel, const QString& strDestAddr) noexcept(false);

//    //USDP和HTDF 广播
   int _Cosmos_BroadcastRawTx(const vector<int> &vctRowsSel) noexcept(false);

    //TRX 广播
    int _TRX_BroadcastRawTx(const vector<int> &vctRowsSel,  QString strCoinType) noexcept(false);

private:
    std::map<int, QString>   __m_mapCollectionStatus;
    std::vector<Collection::COLLECTIONINFO> __m_vctCollectionListItem;
    std::vector<Collection::COLLECTIONINFO> __m_vctCollectionCreateItem;
    std::vector<Collection::COLLECTIONINFO> __m_vctCollectionBroadcastItem;

    BTCExportData __m_btcExportData; //导出的数据
    BTCImportData __m_btcImportData; //导入的数据
    //CBTCRawTxUtil __m_btcRawTxUtil;

    //USDT数据
    BTCExportData __m_usdtExportData; //导出的数据
    BTCImportData __m_usdtImportData; //导入的数据
    CUSDTRawTxUtil __m_usdtRawTxUtil;

    //ETH裸交易工具
    rawtx::CETHRawTxUtilsImpl       __m_ethRawTxUtils;              //eth工具对象
    rawtx::ETH_ExportRawTxParam     __m_ethExportRawTxParams;       //导出的数据
    rawtx::ETH_ImportRawTxParam     __m_ethImportRawTxParams;       //导入的数据

    std::vector<ETHCollectionQeuryRsp> __m_vctEthCollectionQueryRsp; //eth归集查询结果


    //TRX
    rawtx::CTrxRawTxUtilsImpl    __m_trxRawTxUtils;
    rawtx::Trx_ExportRawTxParam     __m_trxExportRawTxParams;       //导出的数据
    rawtx::Trx_ImportRawTxParam     __m_trxImportRawTxParams;       //导入的数据

    std::vector<CollectionQueryRsp> __m_vctTrxCollectionQueryRsp; //TRX



    //USDP 和 HTDF  , HET 裸交易
    rawtx::CCosmosRawTxUtilsImpl    __m_cosmosRawTxUtils;
    rawtx::Cosmos_ExportRawTxParam  __m_cosmosExportRawTxParams;
    rawtx::Cosmos_ImportRawTxParam  __m_cosmosImportRawTxParams;


    std::vector<CosmosCollectionQueryRsp> __m_vctCosmosCollectionQueryRsp; //cosmos归集查询结果

    std::vector<Hrc20TokensCollectionQueryRsp> __m_vctHrc20CollectionQueryRsp; //HRC20 归集查询结果

signals:

public slots:

};

#endif // PAGE_COLLECTION_H
