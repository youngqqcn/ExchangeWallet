/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      获取区块监测程序的充币信息(所有平台有的币种)
*修改记录:

修改日期         修改人       修改说明                      版本号
********************************************************************/
#include "coincharge.h"
#include <QDateTime>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMutexLocker>
#include <QMutex>
#include <mutex>

#include "utils.h"

bool CCoinCharge::ms_bThreadExit = false;

QMutex      g_Mutex;


QQueue<CoinChargeInfo> g_queueCoinChargeInfo;       //线程共享队列资源即buffer


QSemaphore g_freeSpace(INT_SEMAPHORE_MAX);             //空闲的空间
QSemaphore g_usedSpace;                                //已使用的空间

//释放标志,当释放一个充币信息时,置为1
std::atomic_int8_t g_iReleaseFlag = 0;




//各个币种对应的请求区块数据的时间戳
QMap<QString, QVariant> g_mapBtcTimestamp;  std::mutex g_mutexMap_BTC;
QMap<QString, QVariant> g_mapEthBlockNumber; std::mutex g_mutexMap_ETH;
QMap<QString, QVariant> g_mapEtcBlockNumber; std::mutex g_mutexMap_ETC;
QMap<QString, QVariant> g_mapEosBlockNumber;  std::mutex g_mutexMap_EOS;
QMap<QString, QVariant> g_mapERC20BlcokNumber; std::mutex g_mutexMap_ERC20;
QMap<QString, QVariant> g_map_HRC20_BlcokNumber; std::mutex g_mutexMap_HRC20;
QMap<QString, QVariant> g_mapLtcTimestamp;  std::mutex g_mutexMap_LTC;
QMap<QString, QVariant> g_mapBsvTimestamp; std::mutex g_mutexMap_BSV;
QMap<QString, QVariant> g_mapBchTimestamp; std::mutex g_mutexMap_BCH;
QMap<QString, QVariant> g_mapDashTimestamp; std::mutex g_mutexMap_DASH;
QMap<QString, QVariant> g_mapUsdtBlockNumber;  std::mutex g_mutexMap_USDT;
QMap<QString, QVariant> g_mapUsdpBlockNumber; std::mutex g_mutexMap_USDP;
QMap<QString, QVariant> g_mapHtdfBlockNumber;  std::mutex g_mutexMap_HTDF;
QMap<QString, QVariant> g_mapHetBlockNumber;  std::mutex g_mutexMap_HET;
QMap<QString, QVariant> g_mapXrpTimestamp;  std::mutex g_mutexMap_XRP;
QMap<QString, QVariant> g_mapXlmTimestamp;  std::mutex g_mutexMap_XLM;
QMap<QString, QVariant> g_mapTrxBlockNumber;  std::mutex g_mutexMap_TRX;
QMap<QString, QVariant> g_mapXmrBlockNumber;  std::mutex g_mutexMap_XMR;




CCoinCharge::CCoinCharge(QThread *parent) : QThread(parent)
{
//    m_pNetworkMng = NULL;
    m_uProducerMsec = 0;
    m_strProducerUrl = "";

}

CCoinCharge::CCoinCharge(const QString &strUrl, uint64_t uMsec)
{
    this->m_strProducerUrl = strUrl;
    this->m_uProducerMsec = uMsec;
}

CCoinCharge::~CCoinCharge()
{
    //CCoinCharge析构
}

void CCoinCharge::setTimestamp(QMap<QString, QVariant> &mapTimestamp, const QString &strCoinType)
{
    if (0 == strCoinType.compare("BTC", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_BTC);
        if (!g_mapBtcTimestamp.isEmpty())
            g_mapBtcTimestamp.clear();
        g_mapBtcTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("LTC", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_LTC);
        if (!g_mapLtcTimestamp.isEmpty())
            g_mapLtcTimestamp.clear();
        g_mapLtcTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("BSV", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_BSV);
        if (!g_mapBsvTimestamp.isEmpty())
            g_mapBsvTimestamp.clear();
        g_mapBsvTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("BCH", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_BCH);
        if (!g_mapBchTimestamp.isEmpty())
            g_mapBchTimestamp.clear();
        g_mapBchTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("DASH", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_DASH);
        if (!g_mapDashTimestamp.isEmpty())
            g_mapDashTimestamp.clear();
        g_mapDashTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_USDT);
        if (!g_mapUsdtBlockNumber.isEmpty())
            g_mapUsdtBlockNumber.clear();
        g_mapUsdtBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("eth", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_ETH);
        if (!g_mapEthBlockNumber.isEmpty())
            g_mapEthBlockNumber.clear();
        g_mapEthBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("etc", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_ETC);
        if (!g_mapEtcBlockNumber.isEmpty())
            g_mapEtcBlockNumber.clear();
        g_mapEtcBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("usdp", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_USDP);
        if (!g_mapUsdpBlockNumber.isEmpty())
            g_mapUsdpBlockNumber.clear();
        g_mapUsdpBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("htdf", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_HTDF);
        if (!g_mapHtdfBlockNumber.isEmpty())
            g_mapHtdfBlockNumber.clear();
        g_mapHtdfBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("het", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_HET);
        if (!g_mapHetBlockNumber.isEmpty())
            g_mapHetBlockNumber.clear();
        g_mapHetBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("xrp", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_XRP);
        if (!g_mapXrpTimestamp.isEmpty())
            g_mapXrpTimestamp.clear();
        g_mapXrpTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("eos", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_EOS);
        if (!g_mapEosBlockNumber.isEmpty())
            g_mapEosBlockNumber.clear();
        g_mapEosBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("xlm", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_XLM);
        if (!g_mapXlmTimestamp.isEmpty())
            g_mapXlmTimestamp.clear();
        g_mapXlmTimestamp.insert("blocktime", mapTimestamp.value("blocktime").toUInt());
    }
    else if (0 == strCoinType.compare("trx", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_TRX);
        if (!g_mapTrxBlockNumber.isEmpty())
            g_mapTrxBlockNumber.clear();
        g_mapTrxBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }
    else if (0 == strCoinType.compare("xmr", Qt::CaseInsensitive))
    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_XMR);
        if (!g_mapXmrBlockNumber.isEmpty())
            g_mapXmrBlockNumber.clear();
        g_mapXmrBlockNumber.insert("blocknumber", mapTimestamp.value("blocknumber").toUInt());
    }

    for (auto it : g_vctERC20Tokens)
    {
        if (0 == it.compare(strCoinType, Qt::CaseInsensitive))
        {
            std::lock_guard<std::mutex>  guard(g_mutexMap_ERC20);
            if (g_mapERC20BlcokNumber.contains(strCoinType))
                g_mapERC20BlcokNumber.remove(strCoinType);
            g_mapERC20BlcokNumber.insert(strCoinType, mapTimestamp.value("blocknumber").toUInt());
        }
    }

    for (auto it : g_vct_HRC20_Tokens)
    {
        if (0 == it.compare(strCoinType, Qt::CaseInsensitive))
        {
            std::lock_guard<std::mutex>  guard(g_mutexMap_HRC20);
            if (g_map_HRC20_BlcokNumber.contains(strCoinType))
                g_map_HRC20_BlcokNumber.remove(strCoinType);
            g_map_HRC20_BlcokNumber.insert(strCoinType, mapTimestamp.value("blocknumber").toUInt());
        }
    }

}

//#define MY_DEBUG_FLAG
void CCoinCharge::run()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " CCoinCharge::run START 生产数据线程启动";
    while(!ms_bThreadExit)
    {
        try
        {
            if(g_queueCoinChargeInfo.size() < 10) //处理完再拿数据, 防止数据量太大
            {
                // 根据需要放开一下相应币种的充币功能
//                __GetBtcChargeData();
                __GetEthChargeData();
//                __GetEtcChargeData();
//                __GetUsdtChargeData(); // omni-usdt
//                __GetUsdpChargeData();
//                __GetBchChargeData();
//                __GetBsvChargeData();
//                __GetDashChargeData();
//                __GetLtcChargeData();
//                __GetHetChargeData();
//                __GetHtdfChargeData();
//                __GetXrpChargeData();
//                __GetEosChargeData();
//                __GetXlmChargeData();
//                __GetTrxChargeData();
//                __GetXmrChargeData();

                // ERC20充币相关, 从配置文件中读取
                for (auto it : g_vctERC20Tokens)
                {
                    if (ms_bThreadExit) break;
                    if (g_mapERC20Url.contains(it))
                        __GetERC20ChargeData(it, g_qstr_WalletHttpIpPort + g_mapERC20Url.value(it).toString(), g_mapERC20BlcokNumber );
                }

                // HRC20相关
                for (auto it : g_vct_HRC20_Tokens)
                {
                    if(0 == it.compare("BTU", Qt::CaseInsensitive))
                    {
                        continue;
                    }


                    if (ms_bThreadExit) break;
                    if (g_map_HRC20_Url.contains(it))
                        __Get_HRC20_ChargeData(it, g_qstr_WalletHttpIpPort + g_map_HRC20_Url.value(it).toString(), g_map_HRC20_BlcokNumber);
                }

            }
        }
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


        for (uint64_t i = 0; i < m_uProducerMsec/1000; i++)
        {
            if (ms_bThreadExit) break;
            sleep(1);
        }
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinCharge::run() 线程正常退出";
}

void CCoinCharge::requestAndParseCoinChargeInfo(const QString &strUrl, const  QMap<QString, QVariant> &mapKeyValue, const QString &strInCoinType)
{
     QNetworkRequest request;
    QString str_Url;

    if ((0 == strUrl.compare(STR_GET_BTC_DATA, Qt::CaseSensitive)) || (0 == strUrl.compare(STR_GET_LTC_DATA, Qt::CaseSensitive))
            || (0 == strUrl.compare(STR_GET_DASH_DATA, Qt::CaseSensitive)) || (0 == strUrl.compare(STR_GET_BSV_DATA, Qt::CaseSensitive))
            || (0 == strUrl.compare(STR_GET_BCH_DATA, Qt::CaseInsensitive)) || (0 == strUrl.compare(STR_GET_XRP_DATA, Qt::CaseSensitive))
            || (0 == strUrl.compare(STR_GET_XLM_DATA, Qt::CaseSensitive))
            )
    {
        str_Url = strUrl + QString("%1").arg(mapKeyValue.value("blocktime").toUInt());
    }
    else
    {
        str_Url = strUrl + QString("%1").arg(mapKeyValue.value("blocknumber").toUInt());
    }
    request.setUrl(QUrl(str_Url));

    QJsonDocument requestJsonDoc = QJsonDocument::fromVariant(mapKeyValue);
    QByteArray requestJson = requestJsonDoc.toJson();
    request.setRawHeader("Cache-Control","no-cache");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //request.setHeader(QNetworkRequest::ContentLengthHeader, requestJson.size());
    SET_HTTPS_SSL_CONFIG(request);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("post请求Url:") << str_Url << "  和json" << requestJson;
    //return m_pNetworkMng->post(request, requestJson);
     QNetworkAccessManager netAccessMng;
    QNetworkReply *pReply = netAccessMng.post(request, requestJson);



    QString strCoinType = strInCoinType.toUpper();
    try
    {
        ASYNC_TO_SYNC_CAN_THROW(pReply);
    }
    catch(std::exception &e)
    {
        QString strErrMsg = QString("http error: ") + QString(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        //pReply->deleteLater();
        //pReply = NULL;
        return;
    }
    catch(...)
    {
        QString strErrMsg = QString("http error: unknow error. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        //pReply->deleteLater();
        //pReply = NULL;
        return;
    }

    QByteArray replyContent = pReply->readAll();
    QString strTmp = replyContent;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "从接口获取的数据:" << strTmp;
    pReply->deleteLater();

    //json解析错误
    QJsonParseError jsonParseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyContent, &jsonParseErr);
    if (jsonParseErr.error != QJsonParseError::NoError)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "json数据解析出错,原因:" << jsonParseErr.errorString();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "end 生产数据之后队列中的数据量:" << g_queueCoinChargeInfo.size();
        //reply->deleteLater();
        //reply = NULL;
        return;
    }
    //json解析正确,判断内容
    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.value("success") != tr("true"))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测程序返回的数据有误";
        //reply->deleteLater();
        //reply = NULL;
        return;
    }
    QJsonValue jsonArrayValue = jsonObj.value("result");
    QJsonArray jsonArray = jsonArrayValue.toArray();
    //上面两部可简写为:QJsonArray jsonArray = jsonObj.value("result").toArray();
    int jsonArrCount = jsonArray.size();        //获取Json数组中的元素个数

    CoinChargeInfo coinChargeInfo;


    //使用区块时间请求充币数据：BTC系
    if ((0 == strCoinType.compare("btc", Qt::CaseInsensitive)) || (0 == strCoinType.compare("ltc",Qt::CaseInsensitive))
            || (0 == strCoinType.compare("bsv", Qt::CaseInsensitive)) || (0 == strCoinType.compare("bch", Qt::CaseInsensitive))
            || (0 == strCoinType.compare("dash", Qt::CaseInsensitive)))
    {
        for (int i = 0;  i < jsonArrCount; i++)
        {
            QJsonObject resultDataObj = jsonArray.at(i).toObject();     //将json数组中的元素转为QJsonObject
            if (resultDataObj.value("category") == tr("send"))
                continue;
            else if (resultDataObj.value("category") == tr("receive"))
            {
                coinChargeInfo.strChargeTime = QString("%1").arg(resultDataObj.value("blocktime").toInt());
                coinChargeInfo.strCoinType = strCoinType.toUpper();
                coinChargeInfo.strChargeNum = resultDataObj.value("amount").toString().trimmed();
//                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币数量:" << coinChargeInfo.strChargeNum;
//                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币时间:" << coinChargeInfo.strChargeTime ;

                coinChargeInfo.iChargeStatus = 0;
                coinChargeInfo.strTXID = resultDataObj.value("txid").toString().trimmed();
                coinChargeInfo.strDstAddr = resultDataObj.value("address").toString().trimmed();
                coinChargeInfo.iConfirmNum = resultDataObj.value("confirmations").toInt();


                g_freeSpace.acquire();
                g_Mutex.lock();
                g_queueCoinChargeInfo.push_back(coinChargeInfo);
                g_Mutex.unlock();
                g_usedSpace.release();        //共享空间中的可用数据,做加法

            }
        }
    }
    //使用区块时间请求数据
    else if (0 == strCoinType.compare("xrp", Qt::CaseInsensitive) || 0 == strCoinType.compare("xlm", Qt::CaseInsensitive))
    {
        for (int i = 0;  i < jsonArrCount; i++)
        {
            QJsonObject resultDataObj = jsonArray.at(i).toObject();     //将json数组中的元素转为QJsonObject
            coinChargeInfo.strChargeTime = QString("%1").arg(resultDataObj.value("blocktime").toInt()/* + 8 * 60 * 60*/);
//            coinChargeInfo.strBlockNumber = QString("%1").arg(resultDataObj.value("blockNumber").toInt());
            coinChargeInfo.strCoinType = strCoinType.toUpper();
            coinChargeInfo.strChargeNum = resultDataObj.value("value").toString().trimmed();
//            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币数量:" << coinChargeInfo.strChargeNum;
//            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币时间:" << coinChargeInfo.strChargeTime ;
            coinChargeInfo.iChargeStatus = 0;
            coinChargeInfo.strTXID = resultDataObj.value("txid").toString().trimmed();
            coinChargeInfo.strSrcAddr = resultDataObj.value("from").toString().trimmed();
            coinChargeInfo.strDstAddr = resultDataObj.value("to").toString().trimmed();
            coinChargeInfo.iConfirmNum = resultDataObj.value("confirmations").toInt();

            g_freeSpace.acquire();
            g_Mutex.lock();
            g_queueCoinChargeInfo.push_back(coinChargeInfo);
            g_Mutex.unlock();
            g_usedSpace.release();        //共享空间中的可用数据,做加法
        }
    }
    //使用区块高度请求充币数据:包括usdp，htdf,het,usdt,eth,erc20，eos代币
    else if (0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        int iPropertyid = 0;
        iPropertyid = (true == g_bBlockChainMainnet) ? (31) : (2);

        for (int i = 0;  i < jsonArrCount; i++)
        {
            QJsonObject joTransaction = jsonArray.at(i).toObject();     //将json数组中的元素转为QJsonObject
            if(!joTransaction.contains("referenceaddress") || !joTransaction.contains("txid") || !joTransaction.contains("confirmations") \
                    || !joTransaction.contains("amount") || !joTransaction.contains("block") || !joTransaction.contains("propertyid"))
            {
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"Json错误";
                continue;
            }

            if(iPropertyid == joTransaction.value("propertyid").toInt())
            {
                coinChargeInfo.iChargeStatus = 0;
                coinChargeInfo.iConfirmNum = joTransaction.value("confirmations").toInt();
                coinChargeInfo.strChargeNum = joTransaction.value("amount").toString().trimmed();
                coinChargeInfo.strChargeTime = QString("%1").arg(joTransaction.value("blocktime").toInt());
                coinChargeInfo.strBlockNumber = QString("%1").arg(joTransaction.value("block").toInt());
                coinChargeInfo.strCoinType = strCoinType.toUpper();
                coinChargeInfo.strDstAddr = joTransaction.value("referenceaddress").toString().trimmed();
                coinChargeInfo.strTXID = joTransaction.value("txid").toString().trimmed();

                g_freeSpace.acquire();
                g_Mutex.lock();
                g_queueCoinChargeInfo.push_back(coinChargeInfo);
                g_Mutex.unlock();
                g_usedSpace.release();        //共享空间中的可用数据,做加法
            }
        }
    }
    //公司自研币种系
    else if ((0 == strCoinType.compare("usdp", Qt::CaseInsensitive)) || (0 == strCoinType.compare("htdf", Qt::CaseInsensitive))
             || (0 == strCoinType.compare("het", Qt::CaseInsensitive))
             || ( utils::Is_HRC20_Token( strCoinType )  ))
    {
        for (int i = 0;  i < jsonArrCount; i++)
        {

            QJsonObject resultDataObj = jsonArray.at(i).toObject();     //将json数组中的元素转为QJsonObject

            coinChargeInfo.strChargeTime = QString("%1").arg(resultDataObj.value("blocktime").toInt() + 8 * 60 * 60);
            coinChargeInfo.strBlockNumber = QString("%1").arg(resultDataObj.value("blockNumber").toInt());
            coinChargeInfo.strCoinType = strCoinType.toUpper();
            coinChargeInfo.strChargeNum = resultDataObj.value("value").toString().trimmed();
//            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币数量:" << coinChargeInfo.strChargeNum;
//            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币时间:" << coinChargeInfo.strChargeTime ;
            coinChargeInfo.iChargeStatus = 0;
            coinChargeInfo.strTXID = resultDataObj.value("txid").toString().trimmed();
            coinChargeInfo.strSrcAddr = resultDataObj.value("from").toString().trimmed();
            coinChargeInfo.strDstAddr = resultDataObj.value("to").toString().trimmed();
            coinChargeInfo.iConfirmNum = resultDataObj.value("confirmations").toInt();

            g_freeSpace.acquire();
            g_Mutex.lock();
            g_queueCoinChargeInfo.push_back(coinChargeInfo);
            g_Mutex.unlock();
            g_usedSpace.release();        //共享空间中的可用数据,做加法
        }
    }
    else    //ETH系 和  TRX, EOS, XMR
    {
        for (int i = 0;  i < jsonArrCount; i++)
        {
            QJsonObject resultDataObj = jsonArray.at(i).toObject();     //将json数组中的元素转为QJsonObject
            coinChargeInfo.strChargeTime = QString("%1").arg(resultDataObj.value("blocktime").toInt());
            coinChargeInfo.strBlockNumber = QString("%1").arg(resultDataObj.value("blockNumber").toInt());
            coinChargeInfo.strCoinType = strCoinType.toUpper();
            coinChargeInfo.strChargeNum = resultDataObj.value("value").toString().trimmed();
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币数量:" << coinChargeInfo.strChargeNum;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "区块监测返回的充币时间:" << coinChargeInfo.strChargeTime ;
            coinChargeInfo.iChargeStatus = 0;
            coinChargeInfo.strTXID = resultDataObj.value("txid").toString().trimmed();
            coinChargeInfo.strSrcAddr = resultDataObj.value("from").toString().trimmed();
            coinChargeInfo.strDstAddr = resultDataObj.value("to").toString().trimmed();
            coinChargeInfo.iConfirmNum = resultDataObj.value("confirmations").toInt();

            g_freeSpace.acquire();
            g_Mutex.lock();
            g_queueCoinChargeInfo.push_back(coinChargeInfo);
            g_Mutex.unlock();
            g_usedSpace.release();        //共享空间中的可用数据,做加法
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "END 生产数据之后队列中的数据量:" << g_queueCoinChargeInfo.size();
    //reply->deleteLater();
    //reply = NULL;
}

void CCoinCharge::__GetBtcChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("BTC");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_BTC);
    if (g_mapBtcTimestamp.isEmpty())
    {
        g_mapBtcTimestamp.insert("blocktime", 0);
    }
    }

    //请求数据
    requestAndParseCoinChargeInfo( STR_GET_BTC_DATA, g_mapBtcTimestamp , strCoinType );
    //reply = postCoinChargeInfo(m_strProducerUrl, g_mapBtcTimestamp );
    //解析数据
    //parseCoinChargeInfo(reply, strCoinType);
}

void CCoinCharge::__GetEthChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("ETH");


    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_ETH);
        if (g_mapEthBlockNumber.isEmpty())
        {
            g_mapEthBlockNumber.insert("blocknumber", 0);
        }
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_ETH_DATA, g_mapEthBlockNumber);
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_ETH_DATA, g_mapEthBlockNumber , strCoinType );
}


void CCoinCharge::__GetEtcChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("ETC");

    {
        std::lock_guard<std::mutex>  guard(g_mutexMap_ETC);
        if (g_mapEtcBlockNumber.isEmpty())
        {
            g_mapEtcBlockNumber.insert("blocknumber", 0);
        }
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_ETC_DATA, g_mapEtcBlockNumber );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);


    requestAndParseCoinChargeInfo( STR_GET_ETC_DATA, g_mapEtcBlockNumber , strCoinType );
}



void CCoinCharge::__GetEosChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("EOS");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_EOS);
    if (g_mapEosBlockNumber.isEmpty())
        g_mapEosBlockNumber.insert("blocknumber", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_EOS_DATA, g_mapEosBlockNumber );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_EOS_DATA, g_mapEosBlockNumber , strCoinType );
}


void CCoinCharge::__GetTrxChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("TRX");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_TRX);
    if (g_mapTrxBlockNumber.isEmpty())
        g_mapTrxBlockNumber.insert("blocknumber", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_TRX_DATA, g_mapTrxBlockNumber );

//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_TRX_DATA, g_mapTrxBlockNumber , strCoinType );
}


void CCoinCharge::__GetXmrChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("XMR");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_XMR);
    if (g_mapXmrBlockNumber.isEmpty())
        g_mapXmrBlockNumber.insert("blocknumber", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_XMR_DATA, g_mapXmrBlockNumber );

//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_XMR_DATA, g_mapXmrBlockNumber , strCoinType );
}


void CCoinCharge::__GetUsdtChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("USDT");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_USDT);
    if (g_mapUsdtBlockNumber.isEmpty())
        g_mapUsdtBlockNumber.insert("blocknumber", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_USDT_DATA, g_mapUsdtBlockNumber );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_USDT_DATA, g_mapUsdtBlockNumber , strCoinType );
}

void CCoinCharge::__GetUsdpChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("USDP");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_USDP);
    if (g_mapUsdpBlockNumber.isEmpty())
        g_mapUsdpBlockNumber.insert("blocknumber", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_USDP_DATA, g_mapUsdpBlockNumber );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_USDP_DATA, g_mapUsdpBlockNumber , strCoinType );
}

void CCoinCharge::__GetHtdfChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("HTDF");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_HTDF);
    if (g_mapHtdfBlockNumber.isEmpty())
        g_mapHtdfBlockNumber.insert("blocknumber", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_HTDF_DATA, g_mapHtdfBlockNumber );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_HTDF_DATA, g_mapHtdfBlockNumber , strCoinType );
}

void CCoinCharge::__GetLtcChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("LTC");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_LTC);
    if (g_mapLtcTimestamp.isEmpty())
        g_mapLtcTimestamp.insert("blocktime", 0);
    }

//    //请求充币数据
//    reply = postCoinChargeInfo(STR_GET_LTC_DATA, g_mapLtcTimestamp );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_LTC_DATA, g_mapLtcTimestamp , strCoinType );

}

void CCoinCharge::__GetDashChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("DASH");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_DASH);
    if (g_mapDashTimestamp.isEmpty())
        g_mapDashTimestamp.insert("blocktime", 0);
    }

//    reply = postCoinChargeInfo(STR_GET_DASH_DATA, g_mapDashTimestamp );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_DASH_DATA, g_mapDashTimestamp , strCoinType );
}

void CCoinCharge::__GetBsvChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("BSV");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_BSV);
    if (g_mapBsvTimestamp.isEmpty())
        g_mapBsvTimestamp.insert("blocktime", 0);
    }

//    reply = postCoinChargeInfo(STR_GET_BSV_DATA, g_mapBsvTimestamp );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_BSV_DATA, g_mapBsvTimestamp , strCoinType );
}

void CCoinCharge::__GetBchChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("BCH");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_BCH);
    if (g_mapBchTimestamp.isEmpty())
        g_mapBchTimestamp.insert("blocktime", 0);
    }

//    reply = postCoinChargeInfo(STR_GET_BCH_DATA, g_mapBchTimestamp );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_BCH_DATA, g_mapBchTimestamp , strCoinType );
}

void CCoinCharge::__GetHetChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("HET");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_HET);
    if (g_mapHetBlockNumber.isEmpty())
        g_mapHetBlockNumber.insert("blocknumber", 0);
    }

//    reply = postCoinChargeInfo(STR_GET_HET_DATA, g_mapHetBlockNumber );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_HET_DATA, g_mapHetBlockNumber , strCoinType );
}

void CCoinCharge::__GetXrpChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("XRP");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_XRP);
    if (g_mapXrpTimestamp.isEmpty())
        g_mapXrpTimestamp.insert("blocktime", 0);
    }

//    reply = postCoinChargeInfo(STR_GET_XRP_DATA, g_mapXrpTimestamp );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( STR_GET_XRP_DATA, g_mapXrpTimestamp , strCoinType );
}

void CCoinCharge::__GetXlmChargeData()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;
    QString strCoinType = QString("XLM");

    {
    std::lock_guard<std::mutex>  guard(g_mutexMap_XLM);
    if (g_mapXlmTimestamp.isEmpty())
        g_mapXlmTimestamp.insert("blocktime", 0);
    }

//    reply = postCoinChargeInfo(STR_GET_XLM_DATA, g_mapXlmTimestamp );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);


    requestAndParseCoinChargeInfo( STR_GET_XLM_DATA, g_mapXlmTimestamp , strCoinType );
}

void CCoinCharge::__GetERC20ChargeData(const QString &strCoinType, const QString &strUrl,
                                       QMap<QString, QVariant> &mapBlockNum )
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;


    QMap<QString, QVariant> mapTmpBlockNum;
    mapTmpBlockNum.clear();
    mapTmpBlockNum.insert("blocknumber", mapBlockNum.value(strCoinType).toUInt());

//    reply = postCoinChargeInfo(strUrl, mapTmpBlockNum );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( strUrl, mapTmpBlockNum , strCoinType );
}

void CCoinCharge::__Get_HRC20_ChargeData(const QString &strCoinType, const QString &strUrl,
                                       QMap<QString, QVariant> &mapBlockNum )
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 生产数据之前队列中的数据量" << g_queueCoinChargeInfo.size();

//    QNetworkReply *reply;


    QMap<QString, QVariant> mapTmpBlockNum;
    mapTmpBlockNum.clear();
    mapTmpBlockNum.insert("blocknumber", mapBlockNum.value(strCoinType).toUInt());

//    reply = postCoinChargeInfo(strUrl, mapTmpBlockNum );
//    //解析充币数据
//    parseCoinChargeInfo(reply, strCoinType);

    requestAndParseCoinChargeInfo( strUrl, mapTmpBlockNum , strCoinType );
}


/*
* @Name:GetInfo
* @outPutQueue:传出数据
* @n:表示一次要获取的数据量
*/
int CCoinCharge::GetInfo(QQueue<CoinChargeInfo> &outPutQueue, int n)
{

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "START 消费数据之前队列中剩余的数据量:" << g_queueCoinChargeInfo.size();
    for(int i = 0;  i < n; i++)
    {
        g_usedSpace.acquire();
        g_Mutex.lock();
        if (!g_queueCoinChargeInfo.isEmpty())
        {
            outPutQueue.push_back(g_queueCoinChargeInfo.front());
            g_queueCoinChargeInfo.pop_front();
        }
        g_Mutex.unlock();
        g_freeSpace.release(); //可用空间, 做加法
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "END 消费数据之后队列中剩余的数据量:" << g_queueCoinChargeInfo.size();

    return 0;
}
