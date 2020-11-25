#include "raw_tx_utils_base.h"
using namespace rawtx;

IRawTxUtils::IRawTxUtils(QObject *parent):QObject(parent)
{

}

IRawTxUtils::~IRawTxUtils()
{

}

bool IRawTxUtils::IsValidAddr(const QString &strAddr, QString *pStrErrMsg)noexcept(true)
{
    if(strAddr.isEmpty())
    {
        if(NULL != pStrErrMsg)
        {
            *pStrErrMsg = QString("地址为空");
        }
        return false;
    }

    return true;
}

int IRawTxUtils::_SyncPostByURL(const QString &strInUrl, QByteArray &byteRet) noexcept(false)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("开始http请求");

    QUrl url = strInUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求链接: ") << strInUrl;
    QJsonObject jsonObj;
    QNetworkAccessManager  netMngr;
    QNetworkReply *pReply = netMngr.post( req, QJsonDocument(jsonObj).toJson());

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(pReply);

    byteRet = pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求结果:") << byteRet;

    pReply->close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求成功");
    return NO_ERROR;
}


int IRawTxUtils::_SyncPostByJson(const QString &strUrl, const QJsonObject &joPostData, QByteArray &byteRet, QString strContType, QString strAuth, bool bLogReqData)noexcept(false)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("开始http请求") << strUrl;

    QUrl url =strUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, strContType);
    SET_HTTPS_SSL_CONFIG(req);

    if(false == strAuth.isEmpty())
    {
        QByteArray byteAuth = "Basic "+ strAuth.toLatin1().toBase64();
        req.setRawHeader("Authorization", byteAuth);
    }

    if(bLogReqData) //有些地方是不能打印日志的, 比如  离线签名的时候
    {
        qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求数据: ") << joPostData;
    }

    QNetworkAccessManager  netMngr;
    QNetworkReply *pReply = netMngr.post( req, QJsonDocument(joPostData).toJson(QJsonDocument::Compact));

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(pReply);

    byteRet = pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求结果:") << byteRet;


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求成功");

    pReply->close();
    return NO_ERROR;
}



int IRawTxUtils::_SyncGetByURL(const QString &strInUrl, QByteArray &byteRet) noexcept(false)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("开始http请求");

    QUrl url = strInUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求链接: ") << strInUrl;
     QNetworkAccessManager  netMngr;
    QNetworkReply *pReply = netMngr.get(req);

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(pReply);

    byteRet = pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求结果:") << byteRet;


    pReply->close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求成功");
    return NO_ERROR;
}

