#ifndef _WITHDRAW_COMMAN_H
#define _WITHDRAW_COMMAN_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QObject>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>
#include <QDateTime>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QTextCodec>
#include <QTimer>
#include <QEventLoop>
#include <QStringList>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QVariant>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

#include <algorithm>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <memory>
#include "comman.h"
using namespace std;




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



//比特币数据导出
typedef struct _BTCExportItem
{
    _BTCExportItem()
    {
        strOrderID = "";
        strAmount = "";
        strTxFee = "";
        vctSrcAddr.clear();
        vctDstAddr.clear();
        strTxHex = "";
        vctUTXOs.clear();
        mapTxOut.clear();
        bComplete = false;
    }

    QString strOrderID;          //交易ID
    QString strAmount;          //金额(保留8位小数)
    QString strTxFee;            //矿工费(保留8位小数)
    vector<QString> vctSrcAddr;  //源地址
    vector<QString> vctDstAddr;  //目的地址
    QString strTxHex;            //交易
    vector<UTXO>  vctUTXOs;     //utxo集
    map<QString, double> mapTxOut; //交易输出

    bool bComplete;//完成状态    true:已完成   false:未完成

    //usdt
    uint    uTokenId; // testnet-USDT : 2    mainnet-USDT:31
    QString   strTokenAmount; //代币的金额 即 usdt的金额
}BTCExportItem, BTCImportItem, USDTExportItem, USDTImportItem;


typedef struct _BTCExportData
{
    _BTCExportData()
    {
        bIsCollection = false;
        strCoinType = "";
        uTimestamp = 0;
        vctExportItems.clear();
    }

    //info
    bool    bIsCollection;
    QString strCoinType;
    quint64 uTimestamp;

    //items
    vector<BTCExportItem> vctExportItems;
}BTCExportData, BTCImportData, USDTExportData, USDTImportData;

namespace RAWTX {
enum{
    NO_ERROR = 0,
    HTTP_ERR = 1,
    JSON_ERR = 3 ,//解析JSon出错
    FILE_NOT_EXIST= 6, //文件不存在
    FILE_OPEN_FAIL = 7, //打开文件失败

    ARGS_ERR = 10002,
    DB_ERR = 10001,
    PARAM_ERR = 1003,
    JSONPARSE_ERR = 1004,
    STATUS_ERR = 1005,
    INVALID_DATA_ERR = 1006,
    FILE_ERROR = 1007,
    FILE_OPEN_ERR = 1008,

    //manual withdraw
    NOT_SUPPORT_COIN_TYPE = 1500,
    COIN_TYPE_ERROR = 1501,
    COIN_TYPE_NOT_THE_SAME = 1502,

    //eth
    GET_NONCE_FAILED_ERR = 2000,
    INVALID_NONCE_ERR = 2001,
    INVALID_ETH_ADDR  = 2002,
    INVALID_ETH_PRIV_KEY = 2003,
    BROADCAST_FAILED_ERR = 2004,

    AMOUNT_NOT_ENOUGH = 10009, //冷地址金额不足
    TRANSACTION_ALREADY_DONE = 10008,  //交易已存在

};

}


//enum COMMAN_CODE{
//    NO_ERROR = 0,
//};

#endif // _WITHDRAW_COMMAN_H
