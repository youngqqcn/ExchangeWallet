#ifndef BTC_RAW_TX_UTIL_H
#define BTC_RAW_TX_UTIL_H

#include "rawtx_comman.h"

/*
 *
 * 模块:   比特币创建交易模块
 *
 *1.创建rawtransaction
 *
 *流程:
 *    1.列出utxo
 *    2.组装数据
 *    3.调用createrawtransaction 创建rawtransactin
 *    4.返回数据
 */



class CBTCRawTxUtil : public QObject
{
    Q_OBJECT
public:
    explicit CBTCRawTxUtil(QObject *parent = nullptr);
    virtual ~CBTCRawTxUtil();

    enum {
      NO_ERROR = 0,
      HTTP_ERR = 1,
      INVALID_DATA_ERR = 2, //无data字段
      JSON_ERR = 3 ,//解析JSon出错
      ARGS_ERR=4,//函数参数错误
      INVALID_ADDR_ERR = 5,//提币地址不合法
      FILE_NOT_EXIST= 6, //文件不存在
      FILE_OPEN_FAIL = 7, //打开文件失败
      AMOUNT_NOT_ENOUGH = 10009, //冷地址金额不足
      TRANSACTION_ALREADY_DONE = 10008,  //交易已存在
      MIN_TXFEE_NOT_MET = 10010,       //矿工费不够
    };




    //调用createrawtransaction创建交易
    //strRawTxHex:返回的交易的十六进制字符串
     //vctUTXOs: 创建交易所引用到的UTXOs
    int CreateRawTransaction(const QString &strURL,
                             const QString &strSrcAddr,
                             const QString &strDstAddr,
                             const double &dAmount,
                             QString &strRawTxHex,
                             map<QString, double> &mapTxout,
                             vector<UTXO>  &vctRefUTXOs) noexcept(false);

    //归集
    int CreateRawTransactionEx_Collection(const QString &strURL,
                                          vector<QString> &vctstrSrcAddr,
                                          vector<QString> &vctstrDstAddr,
                                          QString &strRawTxHex,
                                          map<QString,double> &mapTxout,
                                          vector<UTXO>  &vctRefUTXOs,
                                          double &dTxFee,
                                          double &dAmount) noexcept(false);


    //将交易导出为json文件
    int ExportRawTxToJsonFile(const QString &strJsonFilePath, const BTCExportData &btcExportData)noexcept(false);


    //从json文件中导入交易数据(其中最核心的是 txHex, 即已签名的交易)
    int ImportRawTxFromJsonFile(const QString &strJsonFilePath, BTCImportData &btcImportData)noexcept(false);


    //广播已签名裸交易
    int BroadcastRawTx(const QString &strURL, const QString &strSignedRawTxHex, QString &strTxid)noexcept(false);

    int SyncGetData(const QString &strInUrl, QByteArray &byteArray)noexcept(false);

    //同步方式post数据, 并获取返回数据
    int SyncPostData(const QString &strInUrl, const QJsonObject &joPostData, QByteArray &byteArray)noexcept(false) ;

    //解析获取到的json数据
    int ParseRawData(const QByteArray &byteJsonData, QString &strRawTxHex, map<QString, double> &mapTxout,  vector<UTXO> &vctRefUTXOs) noexcept(false);

    //获取地址余额
    int GetBalance(const QString &strURL, const QString &strSrcAddr, double &dBalance)noexcept(false);

    //导入私钥
    virtual int ImportPrivKey(const QString& strFilePath, map<QString, QString>& mapAddrPrivKey) noexcept(false);


public:
    QString                 m_strCoinType;

public:
    QNetworkAccessManager    *m_pNetAccMgr;
    QNetworkReply            *m_pReply;
};

class CLTCRawTxUtil : public CBTCRawTxUtil
{
    Q_OBJECT
public:
    explicit CLTCRawTxUtil(QObject *parent = nullptr) : CBTCRawTxUtil(parent)
    {
        m_strCoinType = "LTC";
    }
    virtual ~CLTCRawTxUtil()
    {

    }
};

class CBCHRawTxUtil : public CBTCRawTxUtil
{
    Q_OBJECT
public:
    explicit CBCHRawTxUtil(QObject *parent = nullptr) : CBTCRawTxUtil(parent)
    {
        m_strCoinType = "BCH";
    }
    virtual ~CBCHRawTxUtil()
    {

    }
};

class CBSVRawTxUtil : public CBTCRawTxUtil
{
    Q_OBJECT
public:
    explicit CBSVRawTxUtil(QObject *parent = nullptr) : CBTCRawTxUtil(parent)
    {
        m_strCoinType = "BSV";
    }
    virtual ~CBSVRawTxUtil()
    {

    }
};

class CDASHRawTxUtil : public CBTCRawTxUtil
{
    Q_OBJECT
public:
    explicit CDASHRawTxUtil(QObject *parent = nullptr) : CBTCRawTxUtil(parent)
    {
        m_strCoinType = "DASH";
    }
    virtual ~CDASHRawTxUtil()
    {

    }
};


#endif // BTC_RAW_TX_UTIL_H
