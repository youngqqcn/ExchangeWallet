#ifndef USDT_OFF_SIG_H
#define USDT_OFF_SIG_H
#include "btc/btc_off_sig.h"



class CUSDTOffSig : public CBTCOffSig
{
public:
    explicit CUSDTOffSig();
    virtual ~CUSDTOffSig();

    //交易离线签名接口
   int TxOfflineSignature(
           QString &strSignedRawTxHex, //签名后交易数据(十六进制)
           double dAmount,             //USDT交易金额
           const vector<QString> &vctTxInAddrs,  //交易输入地址
           const map<QString , double>  &mapTxOutMap,  //交易输出信息, 用于验证
           const QString &strUnsignedRawTxHex, //交易数据, 十六进制
           const vector<QString> &vctPrivKeys, //私钥
           const vector<UTXO> &vctPrevTx,//UTXO的信息
           const QString &strSigHashType = "ALL",//签名类型, 可选, 默认是ALL, "ALL" "NONE"  "SINGLE" "ALL|ANYONECANPAY"  "NONE|ANYONECANPAY"  "SINGLE|ANYONECANPAY"
           unsigned char nAddrType = BTCAddrType::P2SKH) noexcept(false)  ; //地址类型

};

#endif // USDT_OFF_SIG_H
