/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      充币业务代码头文件
*修改记录:

修改日期         修改人       修改说明                      版本号
********************************************************************/
#ifndef CCOINCHARGEMNG_H
#define CCOINCHARGEMNG_H

//#include "coincharge.h"
#include "txstatusmonitor.h"
#include <QObject>
#include <QThread>

class CCoinCharge;

class CCoinChargeMng : public QThread
{
    Q_OBJECT
public:
    explicit CCoinChargeMng(QThread *parent = nullptr);
    ~CCoinChargeMng();

    void Init();            //初始化时间戳,数据库等信息

    void initAllTimestamp();   //初始化时间戳总函数

    void initTimestamp(const QString &strCoinType);                         //初始化传给区块监测程序的时间戳

    void sendTimeStamp(uint &iTimestamp, uint &uBlockNumber, const QString &strCoinType);         //将时间戳传到CCoinCharge中

    void run();

    //1.内部等待子线程(生产)退出,
    //2.设置本线程的退出标识为为true
    //3.修改两个信号量, 让消费线程返回(否则会阻塞等待信号量变化)
    bool StopAll(unsigned long uTimeout);

    enum {NOCHECK, TOCHECK, DB_ERR};    //NOCHECK:自动充币      TOCHECK:需审核

public:
    //判断TXID对应的数据是否存在数据库中
    int databaseOperator(const QQueue<CoinChargeInfo> &queueTmp);
    //插入充币信息到数据库
    int insertData(const QQueue<CoinChargeInfo> &queueTmp, const int &iCheckFlag);
    //查询数据库中充币表中的最大序号
    uint queryMaxNum();
    //判断对应数据的订单编号是否为空
    int judgementOrderID(const QQueue<CoinChargeInfo> &queueTmp);
    //根据txid查询数据库中对应数据的订单编号
    void queryOrderID(const QQueue<CoinChargeInfo> &queueTmp, QString &strOrderId);
    //开始充币更新数据库中的订单编号,上传状态更新为上传成功
    void coinChargeStartSucc(const orderId &orderIdTmp, const QQueue<CoinChargeInfo> &queueTmp);
    //开始充币更新数据库中的上传状态更新为上传失败
    void coinChargeStartfaild(const QQueue<CoinChargeInfo> &queueTmp);
    //充币完成,上传状态更新为上传成功
    void coinChargeFinishSucc(const QQueue<CoinChargeFinish> &queueCoinChargeFinish);
    //充币完成,上传状态更新为上传失败
    void coinChargeFinishfaild(const QQueue<CoinChargeFinish> &queueCoinChargeFinish);
    //将数据库中数据的充币状态改为已入账,充币时间也要更新
    void coinChargeFinishUpdateDB(const QQueue<CoinChargeInfo> &queueTmp);
    //查询数据库中的上传状态
    int queryChargeStatus(const QQueue<CoinChargeInfo> &queueTmp);
    //查询数据库中已入账数据的充币时间最大的那个时间
    int queryChargeTimeAndBlkNum(uint &iTimestamp, uint &uBlockNumber, const QString &strCoinType);
    //查询数据库中上传失败数据的充币时间最小的那个时间
    int queryChargeFailedTimeAndBlkNum(uint &uTimestamp, uint &uBlockNumber, const QString &strCoinType);
    //初始化的时候,查询数据库充值表中的所有币种
    void SearchCoinTypeFromChargeTable();
    //判断数据的充币数量跟数据库设置的充币审核阀值的大小
    int checkChargeThreshold(const QQueue<CoinChargeInfo> &queueTmp);
    //查询数据库中审核通过并且待入账的数据
    int queryAuditedData(QQueue<CoinChargeInfo> &queueTmp);
    //定时查询数据库上传失败数据最小的区块高度(区块时间)
    int CheckUploadedFailedOrders();

public:
//    1.对象1
    CCoinCharge         *m_pCoinCharge;
//    CTxStatusMonitor    *m_pTxStatusMonitor;
//    2.对象2
    static uint ms_timestamp;
    static uint ms_blockNumber;

    static bool ms_bThreadExit;

private:
    QString             m_strTabName;
    QSqlDatabase        __m_db;        //数据库操作对象
};

#endif // CCOINCHARGEMNG_H
