#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <QDebug>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QThread>
#include <QSqlDatabase> //数据库支持
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QVariantList>
#include <QStringList>
#include <QTableWidget>
#include <QMutex>
#include <QMessageBox>
#include <QTextCodec>
#include <QList>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <QProcess>
#include <QStringList>
#include <algorithm>
#include <random>  //使用std::mt19937
#include <chrono>
#include <memory>  //使用智能指针
#include <QCryptographicHash>       //使用SHA256加密
#include <QDesktopServices>
#include <tuple>
#include <set>

#pragma execution_character_set("utf-8")

#define STR_UAT_ASSERTDB_DB_NAME   "uat_assertdb"
#define STR_SIT_ASSERTDB_DB_NAME   "assertdb"
#define STR_PRO_ASSERTDB_DB_NAME   "assertdb"

//#define UAT_TEST_VERSION
#ifdef UAT_TEST_VERSION
    #ifdef G_STR_EXE_VERSION
        #undef G_STR_EXE_VERSION
        #define G_STR_EXE_VERSION "AM-UAT特别版-20-01-16"
    #endif
    #define STR_TEST_ASSERTDB_DB_NAME  STR_UAT_ASSERTDB_DB_NAME
    #pragma message("---------------这是UAT版本------------")
#else
    #define STR_TEST_ASSERTDB_DB_NAME  STR_SIT_ASSERTDB_DB_NAME
#endif

// gas_price 在 [50, 500 ]  区间浮动
#define MAX_GASPRICE    500000000000    // 500 Gwei
#define MIN_GASPRICE    50000000000     // 50 Gwei
#define STR_NORMAL_ETH_TX_GASLIMIT  "21000"
#define COLLECTION_GASPRICE_GWEI     200 //100 Gwei
#define STR_COLLECTION_GASPRICE_WEI     "200000000000" //100 Gwei
#define FLOAT_HRC20_SUPPLY_FEE  0.42   // 2 * 0.21


extern std::vector<QString> g_vctERC20Tokens;
extern const std::map<QString, std::tuple<QString, QString, int, QString> >  gc_mapERC20TokensCfg;
extern QMap<QString, QVariant> g_mapERC20Url;

extern std::vector<QString> g_vct_HRC20_Tokens;
extern QMap<QString, QVariant> g_map_HRC20_Url;
extern const std::map<QString, std::tuple<QString, QString, int> >  gc_map_HRC20_tokens_config;  //HRC20


extern bool  g_bIsLAN; //是否是局域网, 通过  utils::Ping("192.168.10.1") 来检查即可

extern QString g_qstr_PrivFilePwd; //RSA 私钥文件加密密码

extern QString g_qstr_MainExeName;

extern bool g_bBlockChainMainnet;
extern bool g_bEnableWithdraw;
extern bool g_bEnableCharge;
extern bool g_bEnableBitcoindExe;
extern bool g_bEnableOmnicoredExe;
extern bool g_bEnableLitecoindExe;
extern bool g_bEnableBchcoindExe;
extern bool g_bEnableBsvcoindExe;
extern bool g_bEnableDashdExe;
extern bool g_bEnableRippledExe;
extern bool g_bEnableMoneroWalletRpcExe;
extern QString g_qstr_EncryptionKey;


extern QString g_qstr_JavaHttpIpPort; //格式   http://192.168.10.243:9001
extern QString g_qstr_WalletHttpIpPort; //格式  http://192.168.10.79:9000

//提币相关的接口URL
extern QString g_qstr_JAVA_selectWithdrawOrderInfo;
extern QString g_qstr_JAVA_upWithdrawOrderStatus;

//用户资产相关接口URL
extern QString g_qstr_JAVA_getUserAsset;
extern QString g_qstr_JAVA_getUserFlow;

//充币相关接口
extern QString g_qstr_JAVA_saveRechargeOrder;
extern QString g_qstr_JAVA_upRechargeOrderStatus;


extern QString g_qstr_WalletAPI_btc_createrawtransaction;
extern QString g_qstr_WalletAPI_btc_sendrawtransaction;
extern QString g_qstr_WalletAPI_ltc_createrawtransaction;
extern QString g_qstr_WalletAPI_ltc_sendrawtransaction;
extern QString g_qstr_WalletAPI_bch_createrawtransaction;
extern QString g_qstr_WalletAPI_bch_sendrawtransaction;
extern QString g_qstr_WalletAPI_bsv_createrawtransaction;
extern QString g_qstr_WalletAPI_bsv_sendrawtransaction;
extern QString g_qstr_WalletAPI_dash_createrawtransaction;
extern QString g_qstr_WalletAPI_dash_sendrawtransaction;


extern QString g_qstr_http_or_https;

extern QProcess *g_ptr_qprocess_Bitcoind;
extern QProcess *g_ptr_qprocess_Omnicored;
extern QProcess *g_ptr_qprocess_Litecoind;
extern QProcess *g_ptr_qprocess_Bchcoind;
//extern QProcess *g_ptr_qprocess_Bsvcoind;
extern QProcess *g_ptr_qprocess_Dashd;
extern QProcess *g_ptr_qprocess_Rippled;
extern QProcess *g_ptr_qprocess_MoneroWalletRpc;


extern QString g_qstr_Usdt_OffSig_Port;
extern QString g_qstr_Btc_OffSig_Port;
extern QString g_qstr_Ltc_OffSig_Port;
extern QString g_qstr_Bch_OffSig_Port;
extern QString g_qstr_Bsv_OffSig_Port;
extern QString g_qstr_Dash_OffSig_Port;
extern QString g_qstr_Xmr_OffSig_Port;

int InitEnvironment(QString &strErrMsg);
bool IsValidPEMFilePassword(const QString &strPwd , QString &strErrMsg);


#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)

#define STR_HTTP_ERROR  ("http error :")



//HTTPS的SSL配置
#if !defined(SET_HTTPS_SSL_CONFIG)
#define SET_HTTPS_SSL_CONFIG(req)\
do{\
    if(req.url().toString().startsWith("https")){\
    QSslConfiguration sslConfig = req.sslConfiguration();\
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);\
    sslConfig.setProtocol(QSsl::TlsV1SslV3);\
    req.setSslConfiguration(sslConfig);\
    }\
}while(0);
#endif

//异步转同步宏函数定义
#if !defined(ASYNC_TO_SYNC)
#define ASYNC_TO_SYNC(_uMillsecTimeout, _sender, _signal)\
do{\
    int timeout = _uMillsecTimeout;\
    QTimer t;\
    t.setSingleShot(true);\
    QEventLoop q;\
    connect(&t, &QTimer::timeout, &q, &QEventLoop::quit, Qt::DirectConnection);\
    connect(_sender, _signal, &q, &QEventLoop::quit, Qt::DirectConnection); \
    t.start(timeout);\
    q.exec();\
    if (!t.isActive()){qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << "]" <<  "http request timeout!";} \
    disconnect(&t, &QTimer::timeout, &q, &QEventLoop::quit);\
    disconnect(_sender, _signal, &q, &QEventLoop::quit);\
}while(0);
#endif


//如果连接失败或者连接超时会抛异常
#if !defined(ASYNC_TO_SYNC_CAN_THROW)
#define ASYNC_TO_SYNC_CAN_THROW(_sender)\
do{\
    int timeout = 30*1000;\
    QTimer t;\
    t.setSingleShot(true);\
    QEventLoop q;\
    connect(&t, &QTimer::timeout, &q, &QEventLoop::quit, Qt::DirectConnection);\
    connect(_sender,  &QNetworkReply::finished, &q, &QEventLoop::quit, Qt::DirectConnection); \
    t.start(timeout);\
    q.exec();\
    if (!t.isActive()){\
        QString strErrMsg = QString(STR_HTTP_ERROR) + QString("http request timeout!");\
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;\
        _sender->abort();\
        _sender->deleteLater();\
        throw std::runtime_error(strErrMsg.toStdString());\
    }\
    disconnect(&t, &QTimer::timeout, &q, &QEventLoop::quit);\
    disconnect(_sender,  &QNetworkReply::finished, &q, &QEventLoop::quit);\
    if(QNetworkReply::NoError != _sender->error()){\
        QString strErrMsg = QString(STR_HTTP_ERROR) + _sender->errorString();\
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;\
        _sender->abort();\
        _sender->deleteLater();\
        throw std::runtime_error( strErrMsg.toStdString() );\
    }\
}while(0);
#endif

//ERC20 transfer函数的标识符
#define STR_erc20_transfer_sig "a9059cbb"

//HRC20 transfer
#define STR_HRC20_transfer_sig "a9059cbb"

#define STR_web_btb_btbOrder   "/web/btb/btbOrder/"
#define STR_web_btb_btbDetailLog "/web/btb/btbDetailLog/"
#define STR_selectWithdrawOrderInfo "selectWithdrawOrderInfo"
#define STR_upWithdrawOrderStatus "upWithdrawOrderStatus"
#define STR_getMemberBtbCoin "getMemberBtbCoin"
#define STR_getBtbDetailLog "getBtbDetailLog"

#define STR_saveRechargeOrder  "saveRechargeOrder"
#define STR_upRechargeOrderStatus  "upRechargeOrderStatus"

#define STR_btc_createrawtransaction "/btc/createrawtransaction?"
#define STR_btc_sendrawtransaction "/btc/sendrawtransaction?"
#define STR_btc_collectionquery "/btc/collectionquery" //不需要参数
#define STR_btc_getbalance "/btc/getbalance?"
#define STR_btc_createrawtransactionex_collection "/btc/createrawtransactionex_collection"


//fix_bug: 2020-03-30 yqq
//如果是 USDT 则使用 USDT 自己的节点广播, 不然会导致 交易池的差异,
//导致双花问题, 因为创建的时候使用USDT节点, 而广播的使用使用的是 USDT节点,
#define STR_usdt_sendrawtransaction "/usdt/sendrawtransaction?"
#define STR_usdt_createrawtransaction "/usdt/createrawtransaction?"
#define STR_usdt_collectionquery  "/usdt/collectionquery" //不需要参数
#define STR_usdt_getcollectiononcecount "/usdt/getcollectiononcecount?" //
#define STR_usdt_createrawtransactionex_collection "/usdt/createrawtransactionex_collection"
#define STR_usdt_getbalance "/usdt/getbalance?"
#define STR_usdt_btc_getbalance "/usdt/btc/getbalance?"


#define STR_eth_collectionquery  "/eth/collectionquery" //不需要参数
#define STR_etc_collectionquery  "/etc/collectionquery" //不需要参数
#define STR_usdp_collectionquery "/usdp/collectionquery"
#define STR_htdf_collectionquery "/htdf/collectionquery"
#define STR_het_collectionquery "/het/collectionquery"

#define STR_ltc_createrawtransaction "/ltc/createrawtransaction?"
#define STR_ltc_sendrawtransaction "/ltc/sendrawtransaction?"
#define STR_ltc_collectionquery "/ltc/collectionquery" //不需要参数
#define STR_ltc_getbalance "/ltc/getbalance?"
#define STR_ltc_createrawtransactionex_collection "/ltc/createrawtransactionex_collection"


#define STR_bch_createrawtransaction "/bch/createrawtransaction?"
#define STR_bch_sendrawtransaction "/bch/sendrawtransaction?"
#define STR_bch_collectionquery "/bch/collectionquery" //不需要参数
#define STR_bch_getbalance "/bch/getbalance?"
#define STR_bch_createrawtransactionex_collection "/bch/createrawtransactionex_collection"

#define STR_bsv_createrawtransaction "/bsv/createrawtransaction?"
#define STR_bsv_sendrawtransaction "/bsv/sendrawtransaction?"
#define STR_bsv_collectionquery "/bsv/collectionquery" //不需要参数
#define STR_bsv_getbalance "/bsv/getbalance?"
#define STR_bsv_createrawtransactionex_collection "/bsv/createrawtransactionex_collection"


#define STR_dash_createrawtransaction "/dash/createrawtransaction?"
#define STR_dash_sendrawtransaction "/dash/sendrawtransaction?"
#define STR_dash_collectionquery "/dash/collectionquery" //不需要参数
#define STR_dash_getbalance "/dash/getbalance?"
#define STR_dash_createrawtransactionex_collection "/dash/createrawtransactionex_collection"



#define STR_btc_crawltransactions_blocktime "/btc/crawltransactions?blocktime="
#define STR_eth_crawltransactions_blocktime "/eth/crawltransactions?blknumber="
#define STR_etc_crawltransactions_blocktime "/etc/crawltransactions?blknumber="
#define STR_usdt_crawltransactions_blocktime "/usdt/crawltransactions?blknumber="
#define STR_usdp_crawltransactions_blocktime "/usdp/crawltransactions?blknumber="

#define STR_htdf_crawltransactions_blocktime "/htdf/crawltransactions?blknumber="

#define STR_het_crawltransactions_blocktime "/het/crawltransactions?blknumber="
#define STR_ltc_crawltransactions_blocktime "/ltc/crawltransactions?blocktime="
#define STR_DASH_crawltransactions_blocktime "/dash/crawltransactions?blocktime="
#define STR_BCH_crawltransactions_blocktime "/bch/crawltransactions?blocktime="
#define STR_BSV_crawltransactions_blocktime "/bsv/crawltransactions?blocktime="
#define STR_Xrp_crawltransactions_blocktime "/xrp/crawltransactions?blocktime="
#define STR_Eos_crawltransactions_blocktime "/eos/crawltransactions?blknumber="
#define STR_Xlm_crawltransactions_blocktime "/xlm/crawltransactions?blocktime="
#define STR_Trx_crawltransactions_blknumber "/trx/crawltransactions?blknumber="
#define STR_Xmr_crawltransactions_blknumber "/xmr/crawltransactions?blknumber="

#define STR_usdp_getaccountinfo_address "/usdp/getaccountinfo?address="
#define STR_usdp_getbalance_address "/usdp/getbalance?address="
#define STR_usdp_sendrawtransaction_tx "/usdp/sendrawtransaction?tx="
#define STR_htdf_getaccountinfo_address "/htdf/getaccountinfo?address="
#define STR_htdf_getbalance_address "/htdf/getbalance?address="
#define STR_htdf_sendrawtransaction_tx "/htdf/sendrawtransaction?tx="
#define STR_het_getaccountinfo_address "/het/getaccountinfo?address="
#define STR_het_getbalance_address "/het/getbalance?address="
#define STR_het_sendrawtransaction_tx "/het/sendrawtransaction?tx="

#define STR_htdf_getbalance "/htdf/getbalance?"
#define STR_usdp_getbalance "/usdp/getbalance?"
#define STR_het_getbalance "/het/getbalance?"



#define STR_eth_gettransactioncount_address "/eth/gettransactioncount?address="
#define STR_eth_getbalance_address "/eth/getbalance?address="
#define STR_eth_sendrawtransaction_data "/eth/sendrawtransaction?data="
#define STR_eth_gasprice "/eth/gasprice"

#define STR_etc_gettransactioncount_address "/etc/gettransactioncount?address="
#define STR_etc_getbalance_address "/etc/getbalance?address="
#define STR_etc_getbalance "/etc/getbalance?"
#define STR_etc_sendrawtransaction_data "/etc/sendrawtransaction?data="

#define STR_xrp_getaccountinfo_address "/xrp/getaccountinfo?address="
#define STR_xrp_getbalance_address "/xrp/getbalance?address="
#define STR_xrp_sendrawtransaction_data "/xrp/sendrawtransaction?data="
#define STR_xrp_getbalance  "/xrp/getbalance?"


#define STR_xlm_getaccountinfo_address "/xlm/getaccountinfo?address="
#define STR_xlm_getbalance_address "/xlm/getbalance?address="
#define STR_xlm_sendrawtransaction_data "/xlm/sendrawtransaction?data="
#define STR_xlm_getbalance  "/xlm/getbalance?"

#define STR_eos_getaccountinfo "/eos/getaccountinfo?address="
#define STR_eos_getbalance "/eos/getbalance?"
#define STR_eos_sendrawtransaction "/eos/sendrawtransaction"
#define STR_eos_createrawtransaction "/eos/createrawtransaction"

#define STR_trx_createrawtransaction "/trx/createrawtransaction"
#define STR_trx_sendrawtransaction "/trx/sendrawtransaction"
#define STR_trx_getaccountinfo "/trx/getaccountinfo?address="
#define STR_trx_getbalance "/trx/getbalance?"
#define STR_trx_collectionquery "/trx/collectionquery"


#define STR_xmr_manual_exporttxoutputs "/xmr/manual/exporttxoutputs"
#define STR_xmr_manual_importkeyimages "/xmr/manual/importkeyimages"
#define STR_xmr_manual_createrawtransactions "/xmr/manual/createrawtransactions"
#define STR_xmr_manual_getbalance  "/xmr/manual/getbalance"
#define STR_xmr_manual_sendrawtransaction "/xmr/manual/sendrawtransaction"

#define STR_xmr_auto_exporttxoutputs "/xmr/auto/exporttxoutputs"
#define STR_xmr_auto_importkeyimages "/xmr/auto/importkeyimages"
#define STR_xmr_auto_createrawtransactions "/xmr/auto/createrawtransactions"
#define STR_xmr_auto_getbalance  "/xmr/auto/getbalance"
#define STR_xmr_auto_sendrawtransaction "/xmr/auto/sendrawtransaction"


#define STR_BTC_NODE_IP_PORT        QString("http://127.0.0.1:")+g_qstr_Btc_OffSig_Port
#define STR_USDT_NODE_IP_PORT       QString("http://127.0.0.1:")+g_qstr_Usdt_OffSig_Port
#define STR_LTC_NODE_IP_PORT        QString("http://127.0.0.1:") + g_qstr_Ltc_OffSig_Port
#define STR_BCH_NODE_IP_PORT        QString("http://127.0.0.1:") + g_qstr_Bch_OffSig_Port
#define STR_BSV_NODE_IP_PORT        QString("http://127.0.0.1:") + g_qstr_Bsv_OffSig_Port
#define STR_DASH_NODE_IP_PORT       QString("http://127.0.0.1:") + g_qstr_Dash_OffSig_Port
#define STR_XMR_LOCAL_DEAMON_IP_PORT  QString("http://127.0.0.1:") + g_qstr_Xmr_OffSig_Port + QString("/json_rpc")


#define UINT_USDT_PROPERTYID            ((g_bBlockChainMainnet)?(31):(2))






#endif // CONFIG_H
