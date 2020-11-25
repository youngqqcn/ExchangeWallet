#ifndef BTC_OFF_SIG_H
#define BTC_OFF_SIG_H
/*************************************************************************************************************
 *
 * 模块: BTC离线签名
 *
 *
 *
***************************************************************************************************************/


#include "rawtx_comman.h"



//测试demo
//#define MY_DEBUG_TEST

typedef struct _txout_t
{
    int n;
    QString txid;
    QString scriptPubKey;
}txout_t;



enum BTCAddrType{
        P2SKH = 0,  //p2skh地址
        P2SH = 1,  //p2sh地址
        P2WSH = 2  //隔离见证地址
};

class CBTCOffSig : public QObject
{
    Q_OBJECT
public:
    explicit CBTCOffSig(QObject *parent = nullptr);
    virtual ~CBTCOffSig();


public:

    enum {
        NO_ERROR = 0,
        ARGS_ERR,
        HTTP_ERR,
        JSON_ERR,
        RAWTX_ERR,
        OUTTXMAP_ERR,
        TXMAPCHECK_ERR,
        TXDECODE_ERR,
    };


    //交易离线签名接口
   int TxOfflineSignature(
           bool  bIsCollection,
           QString &strSignedRawTxHex, //签名后交易数据(十六进制)
           const vector<QString> &vctTxInAddrs,  //交易输入地址
           const map<QString , double>  &mapTxOutMap,  //交易输出信息, 用于验证
           const QString &strUnsignedRawTxHex, //交易数据, 十六进制
           const vector<QString> &vctPrivKeys, //私钥
           const vector<UTXO> &vctPrevTx,//UTXO的信息
           const QString &strSigHashType = "ALL",//签名类型, 可选, 默认是ALL, "ALL" "NONE"  "SINGLE" "ALL|ANYONECANPAY"  "NONE|ANYONECANPAY"  "SINGLE|ANYONECANPAY"
           unsigned char nAddrType = BTCAddrType::P2SKH) noexcept(false) ; //地址类型

protected:
   int _SyncPostData(const QString &strUrl, const QJsonObject &joPostData, QByteArray &byteArray) noexcept(false);

public:
    QString                 m_strCoinType;
    QString                 m_strRPCAuth;
    QString                 m_strDeamonIpPort;

public:
    QNetworkAccessManager    *m_pNetAccMgr;
    QNetworkReply            *m_pReply;
};



class CLTCOffSig : public CBTCOffSig
{
  Q_OBJECT
public:
    explicit CLTCOffSig(QObject *parent = nullptr) : CBTCOffSig(parent)
    {
        m_strRPCAuth = "ltc:ltc2018";
        m_strDeamonIpPort = STR_LTC_NODE_IP_PORT;
        m_strCoinType = "LTC";
    }
    virtual ~CLTCOffSig()
    {

    }

};

class CBCHOffSig : public CBTCOffSig
{
  Q_OBJECT
public:
    explicit CBCHOffSig(QObject *parent = nullptr) : CBTCOffSig(parent)
    {
        m_strRPCAuth = "bch:bch2018";
        m_strDeamonIpPort = STR_BCH_NODE_IP_PORT;
        m_strCoinType = "BCH";
    }
    virtual ~CBCHOffSig()
    {

    }

};

class CBSVOffSig : public CBTCOffSig
{
  Q_OBJECT
public:
    explicit CBSVOffSig(QObject *parent = nullptr) : CBTCOffSig(parent)
    {
        //m_strRPCAuth = "bsv:bsv2018";
        //m_strDeamonIpPort = STR_BSV_NODE_IP_PORT;

        //直接使用BCH的进程进行签名
        m_strRPCAuth = "bch:bch2018";
        m_strDeamonIpPort = STR_BCH_NODE_IP_PORT;
        m_strCoinType = "BSV";
    }

    virtual ~CBSVOffSig()
    {

    }
};

class CDASHOffSig : public CBTCOffSig
{
  Q_OBJECT
public:
    explicit CDASHOffSig(QObject *parent = nullptr) : CBTCOffSig(parent)
    {
        m_strRPCAuth = "dash:dash2018";
        m_strDeamonIpPort = STR_DASH_NODE_IP_PORT;
        m_strCoinType = "DASH";
    }
    virtual ~CDASHOffSig()
    {

    }
};



#endif // BTC_OFF_SIG_H
