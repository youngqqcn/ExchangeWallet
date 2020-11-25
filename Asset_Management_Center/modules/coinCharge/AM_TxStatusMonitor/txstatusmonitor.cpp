/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      从Java获取充币订单编号和上传充币完成信息
*修改记录:

修改日期         修改人       修改说明                      版本号
********************************************************************/
#include "txstatusmonitor.h"
#include <QJsonDocument>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>

CTxStatusMonitor::CTxStatusMonitor(const QString &strUrl)
{
    //__m_pEncryptionModule = NULL;
    this->m_strUrl = strUrl;
//    m_pNetworkManager = new QNetworkAccessManager(this);
    //__m_pEncryptionModule = new CEncryptionModule();
}

CTxStatusMonitor::~CTxStatusMonitor()
{
//    if (m_pNetworkManager)
//    {
//        m_pNetworkManager->destroyed();
//        m_pNetworkManager = NULL;
//    }
//    if (NULL != __m_pEncryptionModule)
//    {
//        delete __m_pEncryptionModule;
//        __m_pEncryptionModule = NULL;
//    }
}

/*
* @Name:getOrderId
* @outOrderId:传出参数
*/
int CTxStatusMonitor::getOrderId(orderId &outOrderId, const QQueue<CoinChargeInfo> &queueData)
{
//    if (queueData.first().strTXID == "58f1f457b6db6abff867e9fec63ce4dd967b2f82bbc484ce6752229abb77bab2")
//    {
//        qDebug() << "监控的地址为: " << queueData.first().strDstAddr;
//    }

    QMap<QString, QVariant> mapJson;
    mapJson.insert("createTime", queueData.at(0).strChargeTime);
    mapJson.insert("coin", queueData.at(0).strCoinType);
    mapJson.insert("amount", queueData.at(0).strChargeNum);
    mapJson.insert("status", queueData.at(0).iChargeStatus);
    mapJson.insert("code", queueData.at(0).strTXID);
    //    mapJson.insert("来源地址", m_queueData.at(0).strSrcAddr);
    mapJson.insert("coinAddress", queueData.at(0).strDstAddr);
    QByteArray byteArrayReply;

    //获取数据
    int iRetRequest = requestData(m_strUrl, mapJson, byteArrayReply);
    if (0 != iRetRequest)
    {
        qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "requestData Err";
        return iRetRequest;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "requestData返回的数据:" << byteArrayReply.data();

    //解析数据
    int iRetParse = parseData(byteArrayReply, outOrderId);
    if (0 != iRetParse)
    {
        qDebug() << "parseData err";
        return iRetParse;
    }

    return 0;
}

int CTxStatusMonitor::sendCoinChargeFinish(orderId &orderIdTmp, const QQueue<CoinChargeFinish> &queueChargeFinish)
{
    QMap<QString, QVariant> mapJson;
    mapJson.insert("finishTime", queueChargeFinish.at(0).strChargeTime);
    mapJson.insert("coin", queueChargeFinish.at(0).strCoinType);
    mapJson.insert("orderId", queueChargeFinish.at(0).strOrderId);
    mapJson.insert("status", queueChargeFinish.at(0).iChargeStatus);
    QByteArray byteArrayReply;

    //获取数据
    int iRetRequest = requestData(m_strUrl, mapJson, byteArrayReply);
    if (0 != iRetRequest)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "requestData Err";
        return iRetRequest;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "requestData返回的数据:" << byteArrayReply.data();

    //解析数据
    int iRetParse = parseData(byteArrayReply, orderIdTmp);
    if (0 != iRetParse)
    {
        qDebug() << "parseData err";
        return iRetParse;
    }

    return 0;
}

int CTxStatusMonitor::parseData(QByteArray &byteArrayData, orderId &outOrderId)
{
    //json解析错误
    QJsonParseError jsonParseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(byteArrayData, &jsonParseErr);
    if (jsonParseErr.error != QJsonParseError::NoError)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "解析json数据出错:" << jsonParseErr.errorString();
        return 1;
    }

    //json解析正确,获取数据
    QJsonObject jsonObj = jsonDoc.object();                     //将JsonDocument转为JsonObject
    QJsonValue jsonValue = jsonObj.value("data");    //获取data ,


    int errCode = jsonObj.value("errCode").toInt();             //获取code,即交易ID
    if (10832 == errCode)
    {
        outOrderId.strOrderId = QString("third_party_coin");
        outOrderId.strTXID = jsonValue.toObject().value("code").toString();
        return 0;
    }
    if (0 == errCode)
    {
        outOrderId.strOrderId = jsonValue.toObject().value("orderId").toString();
        outOrderId.strTXID = jsonValue.toObject().value("code").toString();
        return 0;
    }
    else
    {
        return 1;
    }
    return 0;
}

int CTxStatusMonitor::requestData(const QString strUrl, QMap<QString, QVariant> mapObj, QByteArray &byteArrayReply)
{
//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CTxStatusMonitor::requestData START";
//    QNetworkRequest request;
//    request.setUrl(QUrl(strUrl));

//    QJsonDocument requestJsonDoc = QJsonDocument::fromVariant(mapObj);
//    QByteArray requestJson = requestJsonDoc.toJson();
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//    request.setHeader(QNetworkRequest::ContentLengthHeader, requestJson.size());
//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("post请求Url:") << strUrl << "  和json" << requestJson;
//    QNetworkReply *m_pNetworkReply = m_pNetworkManager->post(request, requestJson);

//    ASYNC_TO_SYNC(30 * 1000, m_pNetworkReply, &QNetworkReply::finished);
//    if (m_pNetworkReply->error() != QNetworkReply::NoError)
//    {
//        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请求出现网络错误:" << m_pNetworkReply->errorString();
//        m_pNetworkReply->close();
//        return 1;
//    }

//    byteArrayReply = m_pNetworkReply->readAll();

//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CTxStatusMonitor::requestData END";
//    m_pNetworkReply->close();

    //下面是调用加密接口
    QJsonDocument requestJsonDoc = QJsonDocument::fromVariant(mapObj);
    //接口请求需要的参数
    QByteArray requestJson = requestJsonDoc.toJson(QJsonDocument::Compact/*不加空白字符*/);


    try
    {
        CEncryptionModule encMoudle;
        encMoudle.postEx(strUrl, requestJson, byteArrayReply);
    }
    catch(std::exception &e)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请求Java充币数据失败:" << QString(e.what());
        return HTTP_ERR;
    }
    catch(...)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请求Java充币数据失败:未知错误.";
        return HTTP_ERR;
    }

    return 0;
}
