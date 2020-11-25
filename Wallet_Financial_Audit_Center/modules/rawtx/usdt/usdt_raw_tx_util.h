#ifndef USDT_RAW_TX_UTIL_H
#define USDT_RAW_TX_UTIL_H
#include "btc/btc_raw_tx_util.h"
#include "comman.h"


class CUSDTRawTxUtil : public CBTCRawTxUtil
{
public:
    explicit CUSDTRawTxUtil();
    virtual ~CUSDTRawTxUtil();

    //提币创建交易
    //调用createrawtransaction创建交易  支持  1对n  批量提币
    int CreateRawTransaction(const QString &strURL, const QString &strSrcAddr,  USDTExportData &usdtExportData) noexcept(false); //注意异常处理

    //归集, 目的地址作为支付手续费的地址
    int CreateRawTransactionEx_Collection(const QString &strURL, const vector<QString> &vctstrSrcAddr,
                                          const QString &strDstAddr, BTCExportData &btcExportData) noexcept(false);


};

#endif // USDT_RAW_TX_UTIL_H
