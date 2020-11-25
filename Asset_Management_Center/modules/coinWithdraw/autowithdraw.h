#ifndef AUTOWITHDRAW_H
#define AUTOWITHDRAW_H
/***************************************************************************************************
 *
 * 模块:  自动提币模块
 *
 *
 *
***************************************************************************************************/
#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <QMap>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QDebug>
#include <QJsonArray>
#include <QObject>
#include <QTimer>
#include <QEventLoop>
#include <QThread>
#include <QSqlDatabase> //数据库支持
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QVariantList>
#include <QStringList>
#include <QMutex>
#include <atomic>  //原子操作
#include "autowithdraw.h"
//#include "dialogs/page_ManualWithdraw/page_manual_withdraw.h"
#include "btc/btc_raw_tx_util.h"
#include "usdt/usdt_raw_tx_util.h"
#include "btc/btc_off_sig.h"
#include "usdt/usdt_off_sig.h"
#include "encryption_module.h"
#include "user_info.h"

#if defined(BTC_SERIES_EX)
#include "btc/btc_raw_tx_utils.h"
#endif

using namespace  std;







#define  AUTOWITH_WAITTIME  5*60
typedef struct _OrderData
{
    _OrderData()
    {
        iAuditStatus =0;
        dAmount = 0;
        dTradeFee = 0;
        dTxFee =0;
        uTimeAudit =0;
        uTimeOrderCreate =0;
        iWay = 0;
        iTxStatus =0;
        iUploadStatus = 0;
        uTimeTxComplete =0;
        iOrderStatus = 0;

        strOrderId = "";
        strUserId = "";
        strCoinType = "";
        strDstAddr = "";
        strSrcAddr = "";
        strTxid = "";
        strAuditor = "";
        strRemarks = "";

        //eth附加参数
        uNonce = ULONG_MAX;
    }

    //从Java后台接收到的数据
    QString strOrderId; //订单编号
    QString strUserId; //用户id
    QString strCoinType; //币种
    QString strDstAddr; //目的地址
    QString strSrcAddr; //出币地址
    double dAmount; //金额
    double dTradeFee;  //手续费
    double dTxFee; //矿工费
    uint64_t uTimeOrderCreate; //订单创建的时间戳
    int  iWay; //提币方式  0:自动提币,  1:人工提币

    //提交给Java后台的数据
    QString strTxid; //用于返回给Java后台
    uint64_t uTimeTxComplete; //交易完成时间
    int  iTxStatus; //交易状态: 0:未完成  1:成功    不存在失败的概念
    int  iOrderStatus; //订单状态 (0:待完成  1:成功, 2:客户端失败   3:服务器错误   4:网络错误)
    int  iAuditStatus; //审核状态(0:待审核  1:成功, 2:失败)
    int  iUploadStatus;  //上传状态 (0:待上传   1:上传成功  2:上传失败)
    uint64_t uTimeAudit; //审核时间
    QString strAuditor; //审核人
    QString strRemarks;  //备注

public: //ETH 附加参数,  2019-04-28  by yqq
    uint64_t        uNonce;

}OrderData;


class CAutoWithdraw : public QThread
{
public:
    explicit CAutoWithdraw();
    virtual ~CAutoWithdraw();

    enum CommanCode{
        NO_ERROR = 0,
        HTTP_ERR = 1,
        JSON_ERR = 2,
        REQ_ERR = 3,
        ARGS_ERR = 4,
        DB_ERR = 5,
        INTERFACE_ERR=6,  //接口返回的错误
        EMPTY_DATA = 7,
        FILE_OPEN_ERR = 8,
        AMOUNT_ERR = 9,
        BALANCE_NOT_ENOUGH = 10, //余额不足
    };

    enum TxStatus{   //交易状态: 0:未完成  1:成功    不存在失败的概念
        TX_INCOMPLETE = 0,
        TX_SUCCESS = 1
    };

    enum OrderStatus{//订单状态 (0:待处理  1:成功, 2:失败  3:客户端失败   4:服务器错误   5:网络错误)
      ORDER_PENDING = 0,
      ORDER_SUCCESS = 1,
      ORDER_FAILED = 2,
      ORDER_CLIENT_ERR = 3,
      ORDER_SERVER_ERR = 4,
      ORDER_NET_ERR = 5,
    };

    enum AuditStatus{
        AUDIT_PENDING = 0, //待审核, Java后台不会接收此状态的提币完成信息
        AUDIT_SUCCESS = 1, //成功:复审通过
        AUDIT_FAILED = 2, //审核失败
        AUDIT_FIRST_PASS = 3, //初审通过
    };

    enum UploadStatus{
        UPLOAD_PENDING = 0, //待上传
        UPLOAD_SUCCESS = 1, //上传成功
        UPLOAD_FAILED = 2,//上传失败
    };

    enum WithdrawWay{ //提币方式
        WAY_AUTO = 0,  //自动
        WAY_MANUAL = 1,  //人工
    };

public:
    virtual void run() override; //重写run方法


public:
    //解析服务返回的数据
    int ParseBytesToOrderData(const QByteArray &byteArrOrderData, vector<OrderData> &vctOrderData)noexcept(false);

    //从服务器获取订单数据
    int PullOrderDataFromServer(const QString &strURL, QString &strStartTime,  QByteArray &byteArrOrderData)noexcept(false);

    //更新用户信息表
    int UpdateUserInfoToDB()noexcept(false);

    //从数据库中获取 已存在的订单的数据 (还未完成的订单的数据)
    int GetExistOrderDataFromDB(vector<OrderData> &vctOrderData)noexcept(false);

    //将订单数据存入数据库(insert or ignore)
    int PushFreshOrderIntoDB(const vector<OrderData> &vctOrderData)noexcept(false);

public: //btc
    //比特币自动提币接口
    int BTC_AutoWithdraw( OrderData &orderData)noexcept(false);
    int USDT_AutoWithdraw( OrderData &orderData) noexcept(false);
    int LTC_AutoWithdraw( OrderData &orderData)noexcept(false);
    int BCH_AutoWithdraw( OrderData &orderData)noexcept(false);
    int BSV_AutoWithdraw( OrderData &orderData)noexcept(false);
    int DASH_AutoWithdraw( OrderData &orderData)noexcept(false);

public: //eth
    //以太坊 ETH自动提币(直接使用私钥创建交易,并发送)
    int ETH_AutoWithdraw(OrderData &orderData)noexcept(false);

    //以太坊 保存交易nonce值
    int ETH_PushNonceIntoDB(OrderData &orderData)noexcept(false);

    //以太坊 获取nonce
    int ETH_GetNonceFromDB(OrderData &orderData)noexcept(false);


    //检查余额是否低于警戒值
    //处理余额不足的临界情况
    //必要时短信通知管理员
    //但是, 短信不能发送太频繁
    int __GetBalance(const QString &strURL, const QString &strSrcAddr, double &dBalance) noexcept(false);
    int __GetAllCoinBalance(const std::set<QString> &setCurCoinTypes, std::map<QString, double> &mapAllCoinBalances)noexcept(false);
    int BalanceThresholdCheckAndSMSNotify(vector<OrderData> &vctOrderData)noexcept(false);


public: //cosmos --> usdp 和 htdf 自动提币   2019-05-12  added by yqq

    //usdp , htdf 自动提币(创建交易并用本地私钥签名, 并广播)
    int COSMOS_AutoWithdraw(OrderData &orderData)noexcept(false);


    int HRC20_AutoWithdraw(OrderData &orderData)noexcept(false);



    //TODO: 是否有必要保存sequence值, 还需深入理解cosmos的sequence机制
    //      如果sequence类似eth中的nonce进制, 那么就必须保存sequence
    //      如果sequence仅仅是记录交易次数, 那么, 目前不必保存sequence, 以后如果要
    //         实现特定需求时,再保存sequence值.

    //usdp, htdf 保存交易sequence值
    //int COSMOS_PushSequenceIntoDB(OrderData &orderData){ return CAutoWithdraw::NO_ERROR; }

    //usdp, htdf 获取交易的sequence值 获取nonce
    //int COSMOS_GetSequenceFromDB(OrderData &orderData){ return CAutoWithdraw::NO_ERROR; }

public:
    //XRP 自动提币(创建交易的同时并用本地私钥签名, 然后并广播)
    int XRP_AutoWithdraw( OrderData &orderData ) noexcept(false);


    //EOS 自动提币(创建交易的同时并用本地私钥签名, 然后并广播)
    int EOS_AutoWithdraw( OrderData &orderData ) noexcept(false);

    //XLM 自动提币 (创建交易, 并签名广播)
    //注意: 需要考虑目的账户是否存在, 所以需要将账户是否存在的参数传入创建交易的函数
    int XLM_AutoWithdraw( OrderData &orderData ) noexcept(false);

    //TRX 自动提币 (创建交易-->签名--->广播)
    //TODO: 后期如有需要, 添加TRC20-USDT提币功能
    int TRX_AutoWithdraw( OrderData &orderData ) noexcept(false);


    //XMR 自动提币
    int XMR_AutoWithdraw( OrderData &orderData ) noexcept(false);

public:

    //更新数据库的订单数据, 如 交易状态, txid, 等
    int UpdateDBOrderData(const OrderData &orderData)noexcept(false);


    //组装推送给Java后台的的数据
    int MakePushOrderData(const OrderData &orderData,  QByteArray &byteArrPushData)noexcept(true);

    //推送已完成订单数据给Java后台
    int PushOrderDataToServer(const QString &strURL, QByteArray &byteArrPushData)noexcept(false);

    //处理订单数据
    int HandleOrder()noexcept(false);


public:

    //初始化网络和数据库
    int Init()noexcept(false);

private:

    //启动数据库服务
    int __StartDBService()noexcept(false);

    //读取币种表, 获取所有币种, 以及自动提币地址及私钥
    int __GetAllCoinConfigFromDB()noexcept(false);

    //判断是否符合自动提币规则
    //bool __IsAutoWithdraw(const QString &strCoinType, const QString &strUserId, const QString &strOrderId, const double dAmount) const noexcept(true);

    //根据提币规则, 修改提币订单的 提币方式 和 订单审核状态
    int __ModifyOrderWithdrawWayAndAuditStatusByWithdrawRules( OrderData &OrderData );

    //判断是否符合上传服务器要求
    bool __CheckUploadable(const OrderData &orderData)const noexcept(true);


    //检查钱包服务端的健康情况, 如果发现计数器超过最大值则发送短信通知开发人员
    void  __WalletServerHealthStateMonitor()noexcept(false);

public:
    static qint64 ms_uTimestamp;
    static bool ms_bExitThread;
    //std::atomic_bool  bExitThread (false);




private:
    /*
        需要在  run() 进行变量定义,  否则会导致   QNetworkReply 的 deleteLater 释放不了内存, 内存泄露
        因为 ,  deleteLater 只是通过 postEvent 将删除事件提交到事件队列, 并不会立即释放内存
        参考 void QObject::deleteLater() 的官方文档,

        [slot] void QObject::deleteLater()

         Schedules this object for deletion.
         The object will be deleted when control returns to the event loop. If the event loop is not running when
         this function is called (e.g. deleteLater() is called on an object before QCoreApplication::exec()), the
         object will be deleted once the event loop is started. If deleteLater() is called after the main event loop
         has stopped, the object will not be deleted. Since Qt 4.8, if deleteLater() is called on an object that lives
         in a thread with no running event loop, the object will be destroyed when the thread finishes.
        Note that entering and leaving a new event loop (e.g., by opening a modal dialog) will not perform the deferred
        deletion; for the object to be deleted, the control must return to the event loop from which deleteLater() was called.

      翻译 by yqq:
         为对象安排删除操作,
         当程序返回到事件循环中.这个对象将会被删除.
         如果调用deleteLater()时事件循环还没运行, 那么当事件循环启动后,该对象会被释放.
         如果在主事件循环已经停止之后调用deleteLater() ,那么该对象将不会被释放.
         从Qt4.8起, 如果在一个没有运行事件循环的线程中,通过一个线程局部对象调用deleteLater(),那么在线程结束后该对象会被释放.
         注意: 进入或者退出一个新的事件循环,将不会执行延迟删除操作; 必须返回到deleteLater()的调用处所在的事件循环中, 对象才能够被释放.
     */


    QSqlDatabase   __m_db; //数据库
    QMutex   __m_mutexBakFile; //备份文件的互斥量
    CUSDTRawTxUtil __m_usdtRawTxUtil;
    CUSDTOffSig  __m_usdtOffSign;

#if defined(BTC_SERIES_EX)
    rawtx::btc::CBTCRawTxUtilsImpl __m_btcRawTxUtilEx;
#endif

private:

    QMap<QString , QMap<QString, QVariant>> __m_qmapAllCoinConfig;

    //短信发送记录, 记录短信发送的时间, 以控制短信发送的频率
    std::map<QString, qint64>   __m_mapSMSSentRecord;


private:
    uint   __m_nWalletServerHealthStateMonitorCounter;

};

#endif // AUTOWITHDRAW_H
