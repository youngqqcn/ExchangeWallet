/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-13  10:30
*文件说明:      充币业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
********************************************************************/
#include "ccoinchargemng.h"
#include "comman.h"
#include "user_info.h"
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <memory>
#include <QTimer>
#include "coincharge.h"
#include "dialogs/page_Collection/page_Collection.h"
#include "modules/rawtx/rawtx_comman.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/special_functions/gamma.hpp>
using float100 = boost::multiprecision::cpp_bin_float_100;


using namespace  std;
bool CCoinChargeMng::ms_bThreadExit = false;

#define ETH_BLOCK_NUMBER 4263537

//存储充值表中所有的币种
vector<QString> g_vctAllCoinTypeFromChargeTab;

uint CCoinChargeMng::ms_timestamp = 0;
uint CCoinChargeMng::ms_blockNumber = 0;

CCoinChargeMng::CCoinChargeMng(QThread *parent) : QThread(parent)
{
    m_pCoinCharge = NULL;
}

CCoinChargeMng::~CCoinChargeMng()
{

}

void CCoinChargeMng::Init()
{
    m_strTabName = QString("tb_recharge");

    if (g_bIsLAN)
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", m_strTabName);
        __m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
        __m_db.setHostName("192.168.10.81");
        __m_db.setPort(3306);
        __m_db.setUserName("root");
        __m_db.setPassword("MyNewPass4!");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }
    else
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", m_strTabName);
        __m_db.setDatabaseName("assertdb");
        __m_db.setHostName("rm-wz991r2rjs3wmd9t1io.mysql.rds.aliyuncs.com");
        __m_db.setPort(3306);
        __m_db.setUserName("accertdb");
        __m_db.setPassword("CG1R47JxIfBofG5uIofHcUKW0Ay1f8");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }

    if (!__m_db.open())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "连接(打开)数据库失败, 错误信息:" << __m_db.lastError();
        return;
    }
    else
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "打开数据库成功!";

    QString strUrlProducer = STR_GET_BTC_DATA;
    uint  nWaitMSec=  ( g_bBlockChainMainnet) ? (10 * 60 * 1000) : (60);
    m_pCoinCharge = new CCoinCharge(strUrlProducer, nWaitMSec);
    SearchCoinTypeFromChargeTable();
    initAllTimestamp();
    m_pCoinCharge->start();
}

void CCoinChargeMng::initAllTimestamp()
{
    QStringList strlist;
    strlist.clear();
    if (g_vctAllCoinTypeFromChargeTab.empty())
    {
        uint uTimestamp = ETH_BLOCK_NUMBER;
        sendTimeStamp(ms_timestamp, uTimestamp, "ETH");
        sendTimeStamp(ms_timestamp, uTimestamp, "ETC");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "BTC");
        for (auto it : g_vctERC20Tokens)
        {
            sendTimeStamp(ms_timestamp, ms_blockNumber, it);
        }
        for(auto it : g_vct_HRC20_Tokens)
        {
            sendTimeStamp(ms_timestamp, ms_blockNumber, it);
        }

        sendTimeStamp(ms_timestamp, ms_blockNumber, "USDT");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "USDP");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "HTDF");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "LTC");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "DASH");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "BSV");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "BCH");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "HET");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "XRP");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "XLM");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "EOS");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "TRX");
        sendTimeStamp(ms_timestamp, ms_blockNumber, "XMR");
    }
    else
    {
        for (auto it : g_vctAllCoinTypeFromChargeTab)
        {
            initTimestamp(it);
            sendTimeStamp(ms_timestamp, ms_blockNumber, it);
            strlist << it;
        }
        if (!strlist.contains("ETH", Qt::CaseInsensitive))
        {
            uint uTimestamp = ETH_BLOCK_NUMBER;
            sendTimeStamp(ms_timestamp, uTimestamp, "ETH");
        }
        if (!strlist.contains("ETC", Qt::CaseInsensitive))
        {
            uint uTimestamp = ETH_BLOCK_NUMBER;
            sendTimeStamp(ms_timestamp, uTimestamp, "ETC");
        }
        if (!strlist.contains("BTC", Qt::CaseInsensitive))
        {
            initTimestamp("BTC");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "BTC");
        }
        if (!strlist.contains("USDT", Qt::CaseInsensitive))
        {
            initTimestamp("USDT");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "USDT");
        }
        if (!strlist.contains("USDP", Qt::CaseInsensitive))
        {
            initTimestamp("USDP");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "USDP");
        }
        if (!strlist.contains("HTDF", Qt::CaseInsensitive))
        {
            initTimestamp("HTDF");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "HTDF");
        }
        if (!strlist.contains("LTC", Qt::CaseInsensitive))
        {
            initTimestamp("LTC");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "LTC");
        }
        if (!strlist.contains("DASH", Qt::CaseInsensitive))
        {
            initTimestamp("DASH");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "DASH");
        }
        if (!strlist.contains("BSV", Qt::CaseInsensitive))
        {
            initTimestamp("BSV");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "BSV");
        }
        if (!strlist.contains("BCH", Qt::CaseInsensitive))
        {
            initTimestamp("BCH");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "BCH");
        }
        if (!strlist.contains("HET", Qt::CaseInsensitive))
        {
            initTimestamp("HET");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "HET");
        }
//        if (!strlist.contains("BJC", Qt::CaseInsensitive))
//        {
//            initTimestamp("BJC");
//            sendTimeStamp(ms_timestamp, ms_blockNumber, "BJC");
//        }
//        if (!strlist.contains("YQB", Qt::CaseInsensitive))
//        {
//            initTimestamp("YQB");
//            sendTimeStamp(ms_timestamp, ms_blockNumber, "YQB");
//        }
        if (!strlist.contains("XRP", Qt::CaseInsensitive))
        {
            initTimestamp("XRP");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "XRP");
        }
        if (!strlist.contains("XLM", Qt::CaseInsensitive))
        {
            initTimestamp("XLM");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "XLM");
        }
        if (!strlist.contains("EOS", Qt::CaseInsensitive))
        {
            initTimestamp("EOS");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "EOS");
        }
        if (!strlist.contains("TRX", Qt::CaseInsensitive))
        {
            initTimestamp("TRX");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "TRX");
        }
        if (!strlist.contains("XMR", Qt::CaseInsensitive))
        {
            initTimestamp("XMR");
            sendTimeStamp(ms_timestamp, ms_blockNumber, "XMR");
        }


        for (auto it : g_vctERC20Tokens)
        {
            if (!strlist.contains(it, Qt::CaseInsensitive))
            {
                initTimestamp(it);
                sendTimeStamp(ms_timestamp, ms_blockNumber, it);
            }
        }

        for (auto it : g_vct_HRC20_Tokens)
        {
            if (!strlist.contains(it, Qt::CaseInsensitive))
            {
                initTimestamp(it);
                sendTimeStamp(ms_timestamp, ms_blockNumber, it);
            }
        }

    }
}

void CCoinChargeMng::initTimestamp(const QString &strCoinType)
{
    int iRet = queryChargeFailedTimeAndBlkNum(ms_timestamp, ms_blockNumber, strCoinType);
    if (1 == iRet)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种 " << strCoinType << " 上传失败数据的最小时间戳成功";
    }
    if (0 == iRet)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种 " << strCoinType << " 上传失败数据为空,继续查询币种 " << strCoinType << " 数据库中已入账数据最大时间戳";
        int uRet = queryChargeTimeAndBlkNum(ms_timestamp, ms_blockNumber, strCoinType);
        if (1 == uRet)
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中币种 " << strCoinType << " 的已入账数据时间戳最大值查询成功";
        }
        if (0 == uRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中币种 " << strCoinType << " 的已入账数据时间戳最大值查询失败,故时间戳不变";
        }
    }
}

void CCoinChargeMng::sendTimeStamp(uint &iTimestamp, uint &uBlockNumber, const QString &strCoinType)
{
    QMap<QString, QVariant> mapTimestamp;
    QMap<QString, QVariant> mapBlockNumber;
    mapTimestamp.clear();
    mapBlockNumber.clear();
    if ((0 == strCoinType.compare("btc", Qt::CaseInsensitive)) || (0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
            || (0 == strCoinType.compare("dash", Qt::CaseInsensitive)) || (0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
            || (0 == strCoinType.compare("bch", Qt::CaseInsensitive)) || (0 == strCoinType.compare("XRP", Qt::CaseInsensitive))
            || (0 == strCoinType.compare("XLM", Qt::CaseInsensitive))
            )
    {
        mapTimestamp.insert("blocktime", iTimestamp);
        m_pCoinCharge->setTimestamp(mapTimestamp, strCoinType);
    }
    else
    {
        mapBlockNumber.insert("blocknumber", uBlockNumber);
        m_pCoinCharge->setTimestamp(mapBlockNumber, strCoinType);
    }
}

void CCoinChargeMng::run()
{
    QQueue<CoinChargeInfo> queueTmp;        //储存区块监测程序返回的数据
    int iDataCount = 1;
    unsigned long long iTimeoutFlag = 0;
    //Java后台接口URL(54服务器)
    QString strGetOrderIdUrl = STR_GET_ORDER_ID_URL;
    QString strCoinChargeSuccess = STR_COIN_CHARGE_SUCCESS_URL;

    //使用局部变量
    CTxStatusMonitor getOrderProxy(strGetOrderIdUrl);
    CTxStatusMonitor chargeSuccessProxy(strCoinChargeSuccess);
    CTxStatusMonitor *pGetOrderIdUrl = &getOrderProxy;
    CTxStatusMonitor *pCoinChargeSuccess =  &chargeSuccessProxy;

    //while (!ms_bThreadExit)
    for(; !ms_bThreadExit;  iTimeoutFlag++ )
    {
        try
        {

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("标志计数:%1").arg(iTimeoutFlag);
        if (0 == iTimeoutFlag % (abs(INT_CHARGE_CHECK_UPLOADED_FAILED_ORDERS_COUNTER)+1) )
        {
            iTimeoutFlag = 0;
            CheckUploadedFailedOrders();
            //qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询上传失败数据之后的标志计数:%1").arg(iTimeoutFlag);
        }


        queueTmp.clear();
        int iChargeCheckFlag = -1;

        //首先查询数据库是否存在审核通过并且待入账的数据
        iChargeCheckFlag = queryAuditedData(queueTmp);
        if (NO_ERROR != iChargeCheckFlag)
        {
            queueTmp.clear();
            int iTmp = CCoinCharge::GetInfo(queueTmp, iDataCount);
            if (queueTmp.isEmpty() || 0 != iTmp)
            {
                qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("从GetInfo接口拿到的数据量为空!标志计数:%1").arg(iTimeoutFlag);
                continue;       //当从GetInfo接口拿到的数据量为空时,跳出本次循环
            }

            //判断充币金额和设置的充币审核阀值大小
            iChargeCheckFlag = checkChargeThreshold(queueTmp);
            if (0 == databaseOperator(queueTmp))        //数据库中不存在该txid的数据
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中不存在该txid的数据,故插入数据";

                int iRet = insertData(queueTmp, iChargeCheckFlag);
                if (0 == iRet)
                {
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工审核充币信息插入成功";
                }
                else if (1 == iRet)
                {
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("人工审核充币信息插入失败!标志计数:%1").arg(iTimeoutFlag);
                    continue;       //数据插入失败,结束本次循环
                }
            }
        }

        if (1 == databaseOperator(queueTmp))        //数据库中已存在该txid的数据
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中已存在该txid的数据";

            if (0 == judgementOrderID(queueTmp))    //数据中的订单编号为空
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中对应txid数据的订单编号为空,开始获取订单编号请求";
                //发送开始充币请求
                orderId structOrderId;                  //存储Java后台返回的订单编号
                if (0 != pGetOrderIdUrl->getOrderId(structOrderId, queueTmp))
                {
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " 请求订单编号失败,将会第二次请求!";
                    if (0 != pGetOrderIdUrl->getOrderId(structOrderId, queueTmp))
                    {
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " 请求订单编号失败,将会第三次请求!";
                        if (0 != pGetOrderIdUrl->getOrderId(structOrderId, queueTmp))
                        {
                            coinChargeStartfaild(queueTmp);
                            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " 第三次请求订单编号失败";
                        }
                        else
                        {
                            coinChargeStartSucc(structOrderId, queueTmp);
                            if (0 == structOrderId.strOrderId.compare(tr("third_party_coin"), Qt::CaseSensitive))
                            {
                                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("标志计数:%1").arg(iTimeoutFlag);
                                continue;
                            }
                        }
                    }
                    else
                    {
                        coinChargeStartSucc(structOrderId, queueTmp);
                        if (0 == structOrderId.strOrderId.compare(tr("third_party_coin"), Qt::CaseSensitive))
                        {
                            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("标志计数:%1").arg(iTimeoutFlag);
                            continue;
                        }
                    }
                }
                else
                {
                    coinChargeStartSucc(structOrderId, queueTmp);
                    if (0 == structOrderId.strOrderId.compare(tr("third_party_coin"), Qt::CaseSensitive))
                    {
                        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("标志计数:%1").arg(iTimeoutFlag);
                        continue;
                    }
                }
            }
            if (TOCHECK == iChargeCheckFlag)
                continue;
            if (tr("该地址属于黑名单") == queueTmp.first().strSrcAddrFlag || tr("该地址不确定") == queueTmp.first().strSrcAddrFlag
                    || 0 == queueTmp.first().strSrcAddrFlag.compare("is_black", Qt::CaseSensitive)
                    || 0 == queueTmp.first().strSrcAddrFlag.compare("unsure", Qt::CaseSensitive))
                continue;

            if (1 == judgementOrderID(queueTmp))//不为空
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("数据库中订单编号不为空");
                if (1 <= queueTmp.first().iConfirmNum)
                {
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "确认数大于等于6";
                    if (0 == queryChargeStatus(queueTmp))
                    {
                        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中的充币状态为待入账";
                        CoinChargeFinish coinChargeFinish;
                        orderId structOrderId;
                        QString strOrderId;
                        queryOrderID(queueTmp, strOrderId);

                        //如果订单号不是数字+英文组成
                        //处理异常充币订单:
                        //   1.  黑名单充币
                        //   2.  第三方上币充币
                        if(true)
                        {
                            std::string strTmpOrderId = strOrderId.toStdString();
                            if( !(boost::all(strTmpOrderId, boost::is_alnum() || boost::is_any_of("_")) ))
                            {
                                QString strErrMsg = tr("订单号:%1 不是正常的订单号, 不上传").arg(strOrderId);
                                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                                continue;
                            }
                        }

                        coinChargeFinish.iChargeStatus = 1;
                        coinChargeFinish.strChargeTime = queueTmp.first().strChargeTime + QString("000");
                        coinChargeFinish.strCoinType = QString("%1").arg(queueTmp.first().strCoinType);
                        coinChargeFinish.strOrderId = strOrderId;
                        QQueue<CoinChargeFinish> queueCoinChargeFinish;
                        queueCoinChargeFinish.clear();
                        queueCoinChargeFinish.push_back(coinChargeFinish);
                        //发送充币完成请求
                        if (0 != pCoinChargeSuccess->sendCoinChargeFinish(structOrderId, queueCoinChargeFinish))
                        {
                            coinChargeFinishfaild(queueCoinChargeFinish);
                            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单 %1 充币完成的数据上传失败")
                                           .arg(queueCoinChargeFinish.first().strOrderId);
                        }
                        else
                        {
                            coinChargeFinishSucc(queueCoinChargeFinish);
                            coinChargeFinishUpdateDB(queueTmp);

                            if (0 == g_iReleaseFlag)//为0表示没有使用"释放"按钮释放数据,故有充币完成的数据则正常更新时间戳或者区块高度
                            {
                                if (1 == queryChargeTimeAndBlkNum(ms_timestamp, ms_blockNumber, queueTmp.first().strCoinType))
                                {
                                    sendTimeStamp(ms_timestamp, ms_blockNumber, queueTmp.first().strCoinType);
                                }
                                else
                                {
                                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中的已入账数据时间戳最大值查询失败,故时间戳不变";
                                }
                            }
                            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单 %1 充币数据上传成功,这笔充币完成")
                                        .arg(queueCoinChargeFinish.first().strOrderId);
                        }
                    }
                    else if (1 == queryChargeStatus(queueTmp))
                    {
                        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("因数据库中充币状态为已入账,故不做操作!标志计数:%1").arg(iTimeoutFlag);
                        continue;
                    }
                }
                else
                {
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("因确认数小于1,故跳过当前循环,不做任何操作!标志计数:%1").arg(iTimeoutFlag);
                    continue;
                }
            }
        }


        }//try
        catch(std::exception &e)
        {
            //处理异常
            QString strErrMsg = QString(e.what());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"error:" << strErrMsg;
        }
        catch(...)
        {
            //处理异常
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"error:未知错误";
        }

    }


    queueTmp.clear();
    m_pCoinCharge->deleteLater(); //不要直接  delete m_pCoinCharge;    防止程序崩溃
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::run() 线程正常退出";
}


//1.内部等待子线程(生产)退出,
//2.设置本线程的退出标识为为true
//3.修改两个信号量, 让消费线程返回(否则会阻塞等待信号量变化)
bool CCoinChargeMng::StopAll(unsigned long uTimeout)
{
    //先等待子线程退出
    CCoinCharge::ms_bThreadExit = true;
    m_pCoinCharge->wait(uTimeout);

    //防止消费线程卡在 CCoinCharge::GetInfo() 中的 g_usedSpace.acquire();
    CCoinChargeMng::ms_bThreadExit = true;
    g_usedSpace.release(5);
    g_freeSpace.acquire(5);

    return QThread::wait(uTimeout);
}

int CCoinChargeMng::databaseOperator(const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::databaseOperator START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select * from tb_recharge where txid = \'%1\' and receive_addr = \'%2\' and coin_type = \'%3\';") \
            .arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }

    if (queryResult.next())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中存在相应的TXID记录";
        return 1;
    }
    else
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中不存在相应的TXID记录";
        return 0;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::databaseOperator END";
    return 2;
}

int CCoinChargeMng::insertData(const QQueue<CoinChargeInfo> &queueTmp, const int &iCheckFlag)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::insertData START";
    uint uCoinChargeTime = queueTmp.first().strChargeTime.toUInt();// + QString("000");
    uint iMaxNo = queryMaxNum();

    uint uBlockNum;
    if (queueTmp.first().strBlockNumber.isEmpty())
    {
        uBlockNum = 0;
    }
    else
    {
        uBlockNum = queueTmp.first().strBlockNumber.trimmed().toUInt();
    }

    QSqlQuery queryResult(__m_db);
    QString strSql;
    if (NOCHECK == iCheckFlag)
    {
        if (0 == queueTmp.first().strCoinType.compare("htdf", Qt::CaseInsensitive) && 0 == queueTmp.first().strSrcAddrFlag.compare("is_black", Qt::CaseSensitive))
        {
            strSql = QString("insert into tb_recharge values(%1, \'该地址属于黑名单\', %2, \'%3\', \'%4\', %5, %6, -1, \'--\', \'--\', \'%7\', \'%8\', \'%9\', %10, %11, %12);")
                    .arg(iMaxNo).arg(CUserInfo::loginUser().AdminID().toUInt()).arg(queueTmp.first().strCoinType).arg(queueTmp.first().strChargeNum)
                    .arg(0).arg(1).arg(queueTmp.first().strSrcAddr.trimmed()).arg(queueTmp.first().strDstAddr.trimmed()).arg(queueTmp.first().strTXID.trimmed())
                    .arg(uCoinChargeTime).arg(uBlockNum).arg(0);
        }
        else if (0 == queueTmp.first().strCoinType.compare("htdf", Qt::CaseInsensitive) && 0 == queueTmp.first().strSrcAddrFlag.compare("unsure", Qt::CaseSensitive))
        {
            strSql = QString("insert into tb_recharge values(%1, \'该地址不确定\', %2, \'%3\', \'%4\', %5, %6, -1, \'--\', \'--\', \'%7\', \'%8\', \'%9\', %10, %11, %12);")
                    .arg(iMaxNo).arg(CUserInfo::loginUser().AdminID().toUInt()).arg(queueTmp.first().strCoinType).arg(queueTmp.first().strChargeNum)
                    .arg(0).arg(1).arg(queueTmp.first().strSrcAddr.trimmed()).arg(queueTmp.first().strDstAddr.trimmed()).arg(queueTmp.first().strTXID.trimmed())
                    .arg(uCoinChargeTime).arg(uBlockNum).arg(0);
        }
        else
        {
            strSql = QString("insert into tb_recharge values(%1, \'\', %2, \'%3\', \'%4\', %5, %6, -1, \'--\', \'--\', \'%7\', \'%8\', \'%9\', %10, %11, %12);")
                    .arg(iMaxNo).arg(CUserInfo::loginUser().AdminID().toUInt()).arg(queueTmp.first().strCoinType).arg(queueTmp.first().strChargeNum)
                    .arg(0).arg(1).arg(queueTmp.first().strSrcAddr.trimmed()).arg(queueTmp.first().strDstAddr.trimmed()).arg(queueTmp.first().strTXID.trimmed())
                    .arg(uCoinChargeTime).arg(uBlockNum).arg(0);
        }
    }
    else if (TOCHECK == iCheckFlag)
    {
        if (0 == queueTmp.first().strCoinType.compare("htdf", Qt::CaseInsensitive) && 0 == queueTmp.first().strSrcAddrFlag.compare("is_black", Qt::CaseSensitive))
        {
            strSql = QString("insert into tb_recharge values(%1, \'该地址属于黑名单\', %2, \'%3\', \'%4\', %5, %6, -1, \'--\', \'--\', \'%7\', \'%8\', \'%9\', %10, %11, %12);")
                    .arg(iMaxNo).arg(CUserInfo::loginUser().AdminID().toUInt()).arg(queueTmp.first().strCoinType).arg(queueTmp.first().strChargeNum)
                    .arg(0).arg(1).arg(queueTmp.first().strSrcAddr.trimmed()).arg(queueTmp.first().strDstAddr.trimmed()).arg(queueTmp.first().strTXID.trimmed())
                    .arg(uCoinChargeTime).arg(uBlockNum).arg(0);
        }
        else if (0 == queueTmp.first().strCoinType.compare("htdf", Qt::CaseInsensitive) && 0 == queueTmp.first().strSrcAddrFlag.compare("unsure", Qt::CaseSensitive))
        {
            strSql = QString("insert into tb_recharge values(%1, \'该地址不确定\', %2, \'%3\', \'%4\', %5, %6, -1, \'--\', \'--\', \'%7\', \'%8\', \'%9\', %10, %11, %12);")
                    .arg(iMaxNo).arg(CUserInfo::loginUser().AdminID().toUInt()).arg(queueTmp.first().strCoinType).arg(queueTmp.first().strChargeNum)
                    .arg(0).arg(1).arg(queueTmp.first().strSrcAddr.trimmed()).arg(queueTmp.first().strDstAddr.trimmed()).arg(queueTmp.first().strTXID.trimmed())
                    .arg(uCoinChargeTime).arg(uBlockNum).arg(0);
        }
        else
        {
            strSql = QString("insert into tb_recharge values(%1, \'\', %2, \'%3\', \'%4\', %5, %6, 2, \'--\', \'--\', \'%7\', \'%8\', \'%9\', %10, %11, %12);")
                    .arg(iMaxNo).arg(CUserInfo::loginUser().AdminID().toUInt()).arg(queueTmp.first().strCoinType).arg(queueTmp.first().strChargeNum)
                    .arg(0).arg(1).arg(queueTmp.first().strSrcAddr.trimmed()).arg(queueTmp.first().strDstAddr.trimmed()).arg(queueTmp.first().strTXID.trimmed())
                    .arg(uCoinChargeTime).arg(uBlockNum).arg(0);
        }
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入数据库充币信息sql语句: " << strSql;

    if (queryResult.exec(strSql))
    {
        return 0;
    }
    else
    {
        return 1;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::insertData END";
    return 1;
}

uint CCoinChargeMng::queryMaxNum()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryMaxNum START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select max(charge_no) from tb_recharge; ");

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }

    while (queryResult.next())
    {
        if (!queryResult.value(0).toString().isEmpty())
        {
            qDebug() << "充币表中序号最大值:" << queryResult.value(0).toUInt();
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryMaxNum END";
            return queryResult.value(0).toUInt() + 1;
        }
        else
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryMaxNum END";
            return 0;
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryMaxNum END";
    return 0;
}

int CCoinChargeMng::judgementOrderID(const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::judgementOrderID START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select order_id from tb_recharge where txid = \'%1\' and receive_addr = \'%2\' and coin_type = \'%3\';")
            .arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }

    while (queryResult.next())
    {
        if (queryResult.value(0).toString().isEmpty())
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::judgementOrderID END";
    return 2;
}

void CCoinChargeMng::queryOrderID(const QQueue<CoinChargeInfo> &queueTmp, QString &strOrderId)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryOrderID START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select order_id from tb_recharge where txid = \'%1\' and receive_addr = \'%2\' and coin_type = \'%3\';")
            .arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return;
    }

    if (queryResult.next())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询订单ID成功";
        strOrderId = queryResult.value(0).toString();
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库订单ID数据为空";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryOrderID END";
}

void CCoinChargeMng::coinChargeStartSucc(const orderId &orderIdTmp, const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeStartSucc START";
    QSqlQuery queryResult(__m_db);
    QString strSql;
    if (0 == orderIdTmp.strOrderId.compare(tr("third_party_coin"), Qt::CaseSensitive))
    {
        strSql = QString("update tb_recharge set order_id = \'%1\', upload_status = 0, recharge_status = 1 "
                         "where txid = \'%2\' and receive_addr = \'%3\' and coin_type = \'%4\'; ")
                .arg(orderIdTmp.strOrderId).arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);
    }
    else
    {
        strSql = QString("update tb_recharge set order_id = \'%1\', upload_status = 0 where txid = \'%2\' and receive_addr = \'%3\' and coin_type = \'%4\'; ")
                .arg(orderIdTmp.strOrderId).arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);
    }

    if (queryResult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币开始更新数据库中的订单编号和上传状态成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币开始更新数据库中的订单编号和上传状态失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeStartSucc END";
}

void CCoinChargeMng::coinChargeStartfaild(const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeStartfaild START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("update tb_recharge set upload_status = 2 where txid = \'%1\' and receive_addr = \'%2\' and coin_type = \'%3\'; ")
            .arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);

    if (queryResult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币开始更新数据库中的上传状态成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币开始更新数据库中的上传状态失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeStartfaild END";
}

void CCoinChargeMng::coinChargeFinishSucc(const QQueue<CoinChargeFinish> &queueCoinChargeFinish)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeFinishSucc START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("update tb_recharge set upload_status = 0 where order_id = \'%1\'; ").arg(queueCoinChargeFinish.first().strOrderId);

    if (queryResult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币完成更新数据库中的上传状态成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币完成更新数据库中的上传状态失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeFinishSucc END";
}

void CCoinChargeMng::coinChargeFinishfaild(const QQueue<CoinChargeFinish> &queueCoinChargeFinish)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeFinishfaild START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("update tb_recharge set upload_status = 2 where order_id = \'%1\';").arg(queueCoinChargeFinish.first().strOrderId);

    if (queryResult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币完成更新数据库中的上传状态成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币完成更新数据库中的上传状态失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeFinishfaild END";
}

void CCoinChargeMng::coinChargeFinishUpdateDB(const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeFinishUpdateDB START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("update tb_recharge set upload_status = 0, recharge_status = %1, recharge_time = %2 "
                             "where txid = \'%3\' and receive_addr = \'%4\' and coin_type = \'%5\';")
            .arg(1).arg(queueTmp.first().strChargeTime.toUInt()).arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr)
            .arg(queueTmp.first().strCoinType);

    if (queryResult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币完成更新数据库中的充币状态和充币时间成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币完成更新数据库中的充币状态和充币时间失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::coinChargeFinishUpdateDB END";
}

int CCoinChargeMng::queryChargeStatus(const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeStatus START";
    //    SqlliteDetail query("queryChargeStatus");
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select * from tb_recharge where txid = \'%1\' and receive_addr = \'%2\' and coin_type = \'%3\'; ")
            .arg(queueTmp.first().strTXID).arg(queueTmp.first().strDstAddr).arg(queueTmp.first().strCoinType);

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }

    while (queryResult.next())
    {
        if (0 == queryResult.value(5).toInt())      //待入账
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询出的订单编号: " << queryResult.value(1).toString().trimmed()
                     << "查询出的充币状态为: " << queryResult.value(5).toInt();
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeStatus END";
            return 0;
        }
        else if (1 == queryResult.value(5).toInt()) //已入账
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询出的订单编号: " << queryResult.value(1).toString().trimmed()
                     << "查询出的充币状态为: " << queryResult.value(5).toInt();
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeStatus END";
            return 1;
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeStatus END";
    return 2;
}

int CCoinChargeMng::queryChargeTimeAndBlkNum(uint &iTimestamp, uint &uBlockNumber, const QString &strCoinType)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeTimeAndBlkNum START";
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select recharge_time, block_number, recharge_status from tb_recharge where recharge_status = 1 ");


    QString strTmpCoinTypeUpper = strCoinType.toUpper(); //全转为大写
    if (0 == strTmpCoinTypeUpper.compare("btc", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("usdt", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("eth", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("etc", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("usdp", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("htdf", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("ltc", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("dash", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("bsv", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("bch", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("het", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("xrp", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("xlm", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("recharge_time");
    else if (0 == strTmpCoinTypeUpper.compare("eos", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("trx", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    else if (0 == strTmpCoinTypeUpper.compare("xmr", Qt::CaseInsensitive))
        strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");

    for (auto it : g_vctERC20Tokens)
    {
        if (0 == strCoinType.compare(it, Qt::CaseInsensitive))
            strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    }

    for (auto it : g_vct_HRC20_Tokens)
    {
        if (0 == strCoinType.compare(it, Qt::CaseInsensitive))
            strSql += QString("and coin_type = \'%1\' order by %2 desc; ").arg(strTmpCoinTypeUpper).arg("block_number");
    }



    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql:" << strSql;

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }
    if (queryResult.next())
    {
        iTimestamp = queryResult.value(0).toUInt(0);
        uBlockNumber = queryResult.value(1).toUInt(0);
        if ((0 == strCoinType.compare("btc", Qt::CaseInsensitive)) || (0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
                || (0 == strCoinType.compare("dash", Qt::CaseInsensitive)) || (0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
                || (0 == strCoinType.compare("bch", Qt::CaseInsensitive)) || (0 == strCoinType.compare("xrp", Qt::CaseInsensitive) ||
                ( 0 == strCoinType.compare("xlm", Qt::CaseInsensitive))
                ))
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询到币种 " << strCoinType << " 已入账时间戳最大的数据,故更新时间戳为:" << iTimestamp;
        }
        else
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询到币种 " << strCoinType << " 已入账区块高度最大的数据,故更新区块高度为:" << uBlockNumber;
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeTimeAndBlkNum END";
        return 1;

    }
    else
    {
        iTimestamp = 0;
        if (0 == strCoinType.compare("eth", Qt::CaseInsensitive))
        {
            uBlockNumber = ETH_BLOCK_NUMBER;
        }
        else
        {
            uBlockNumber = 0;
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中没有币种 " << strCoinType << " 的上传失败数据,故时间戳设置为零";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeTimeAndBlkNum END";
        return 0;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeTimeAndBlkNum END";
    return 0;
}

int CCoinChargeMng::queryChargeFailedTimeAndBlkNum(uint &uTimestamp, uint &uBlockNumber, const QString &strCoinType)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeFailedTimeAndBlkNum START";
    //新的方式,不用sqlite封装类
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select recharge_time, block_number, upload_status from tb_recharge where upload_status = 2 ");

    if (0 == strCoinType.compare("btc", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("eth", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("etc", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("usdp", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("htdf", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("dash", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("bch", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("het", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("xrp", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("xlm", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("recharge_time");
    else if (0 == strCoinType.compare("eos", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("trx", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    else if (0 == strCoinType.compare("xmr", Qt::CaseInsensitive))
       strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    for (auto it : g_vctERC20Tokens)
    {
        if (0 == strCoinType.compare(it, Qt::CaseInsensitive))
            strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    }
    
    for (auto it : g_vct_HRC20_Tokens)
    {
        if (0 == strCoinType.compare(it, Qt::CaseInsensitive))
            strSql += QString("and coin_type = \'%1\' order by %2 asc; ").arg(strCoinType).arg("block_number");
    }

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }

    qDebug() << strSql;

    if (queryResult.next())
    {
        uTimestamp = queryResult.value(0).toUInt();
        uBlockNumber = queryResult.value(1).toUInt();
        if ((0 == strCoinType.compare("btc", Qt::CaseInsensitive)) || (0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
                || (0 == strCoinType.compare("dash", Qt::CaseInsensitive)) || (0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
                || (0 == strCoinType.compare("bch", Qt::CaseInsensitive)) || (0 == strCoinType.compare("xrp", Qt::CaseInsensitive))
                || (0 == strCoinType.compare("xlm", Qt::CaseInsensitive))
                )
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询到币种 " << strCoinType << " 上传失败时间戳最小的数据,故更新时间戳为:" << uTimestamp;
        }
        else
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询到币种 " << strCoinType << " 上传失败区块高度最小的数据,故更新区块高度为:" << uBlockNumber;
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeFailedTimeAndBlkNum END";
        return 1;
    }
    else
    {
        uTimestamp = 0;
        if (0 == strCoinType.compare("eth", Qt::CaseInsensitive))
        {
            uBlockNumber = ETH_BLOCK_NUMBER;
        }
        else
        {
            uBlockNumber = 0;
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中没有币种 " << strCoinType << " 的上传失败数据,故时间戳设置为零";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeFailedTimeAndBlkNum END";
        return 0;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryChargeFailedTime END";
    return 0;
}

void CCoinChargeMng::SearchCoinTypeFromChargeTable()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::SearchCoinTypeFromChargeTable START";
    QSqlQuery queryResult(__m_db);
//    QString strSql = QString("select distinct coin_nick from tb_coin_type ");
    QString strSql = QString("select distinct coin_type from tb_recharge; ");

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库失败,请检查数据库: " << queryResult.lastError();
        return;
    }
    while (queryResult.next())
    {
        g_vctAllCoinTypeFromChargeTab.push_back(queryResult.value(0).toString().trimmed());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::SearchCoinTypeFromChargeTable END";
}

int CCoinChargeMng::checkChargeThreshold(const QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::checkChargeThreshold START";

    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select coin_nick, charge_threshold from tb_coin_type where coin_nick = \'%1\';").arg(queueTmp.first().strCoinType);
    auto dSrcChargeCount = float100(queueTmp.first().strChargeNum.toStdString());
    double dChargeThreshold;
//    double dSrcChargeCount = queueTmp.first().strChargeNum.toDouble();

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询充币审核阀值失败,请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }
    if (queryResult.next())
    {
        dChargeThreshold = queryResult.value(1).toDouble();
    }
    else
        return NOCHECK;

    if (dSrcChargeCount >= dChargeThreshold)
        return TOCHECK;
    else
        return NOCHECK;

    return NOCHECK;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::checkChargeThreshold END";
}

int CCoinChargeMng::queryAuditedData(QQueue<CoinChargeInfo> &queueTmp)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryAuditedData START";

    CoinChargeInfo srcChargeInfo;
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select * from tb_recharge where audit_status = 1 and recharge_status = 0 limit 1 ");

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询充值表失败,请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }
    if (queryResult.next())
    {
        srcChargeInfo.iChargeStatus = 0;
        srcChargeInfo.iConfirmNum = 6;
        srcChargeInfo.strBlockNumber = QString("%1").arg(queryResult.value(14).toString());
        srcChargeInfo.strChargeNum = QString("%1").arg(queryResult.value(4).toUInt());
        srcChargeInfo.strChargeTime = QString("%1").arg(queryResult.value(13).toUInt());
        srcChargeInfo.strCoinType = QString("%1").arg(queryResult.value(3).toString());
        srcChargeInfo.strDstAddr = QString("%1").arg(queryResult.value(11).toString());
        srcChargeInfo.strSrcAddr = QString("%1").arg(queryResult.value(10).toString());
        srcChargeInfo.strSrcAddrFlag = QString("%1").arg(queryResult.value(1).toString());
        srcChargeInfo.strTXID = QString("%1").arg(queryResult.value(12).toString());
        queueTmp.push_back(srcChargeInfo);
    }
    else
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "没有相应的审核通过并且待入账的数据";
        return ARGS_ERR;
    }
    return NO_ERROR;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryAuditedData END";
}

int CCoinChargeMng::CheckUploadedFailedOrders()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::CheckUploadedFailedOrder START";

    QSqlQuery queryResult(__m_db);
    QString strSql_1 = QString("select distinct coin_type from tb_recharge where upload_status = 2;");
    vector<QString> strAllCoin;

    if (!queryResult.exec(strSql_1))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
        return DB_ERR;
    }

    while (queryResult.next())
    {
        strAllCoin.push_back(queryResult.value(0).toString());
    }

    for (auto it : strAllCoin)
    {
        queryResult.clear();
        QString strSql_2 = QString("select recharge_time, block_number from tb_recharge where upload_status = 2 ");
        if ((0 == it.compare("btc", Qt::CaseInsensitive)) || (0 == it.compare("ltc", Qt::CaseInsensitive))
                || (0 == it.compare("dash", Qt::CaseInsensitive)) || (0 == it.compare("bsv", Qt::CaseInsensitive))
                || (0 == it.compare("bch", Qt::CaseInsensitive)) || (0 == it.compare("xrp", Qt::CaseInsensitive))
                || (0 == it.compare("xlm", Qt::CaseInsensitive)))
        {
            strSql_2 += QString("and coin_type = \'%1\' order by recharge_time ").arg(it);
            if (!queryResult.exec(strSql_2))
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
                return DB_ERR;
            }
            if (queryResult.next())
            {
                g_iReleaseFlag = 1;
                ms_timestamp = queryResult.value(0).toUInt();
                sendTimeStamp(ms_timestamp, ms_blockNumber, it);
            }
        }
        else
        {
            strSql_2 += QString("and coin_type = \'%1\' order by block_number ").arg(it);
            if (!queryResult.exec(strSql_2))
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库出错, 请检查数据库: " << queryResult.lastError();
                return DB_ERR;
            }
            if (queryResult.next())
            {
                g_iReleaseFlag = 1;
                ms_blockNumber = queryResult.value(1).toUInt();
                sendTimeStamp(ms_timestamp, ms_blockNumber, it);
            }
        }
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::CheckUploadedFailedOrder END";
    return NO_ERROR;
}
