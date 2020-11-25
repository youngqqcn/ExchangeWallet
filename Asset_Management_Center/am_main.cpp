#include "am_main.h"


CAMMain::CAMMain(QObject *parent) : QObject(parent)
{
    this->_m_pNetManager = NULL;
    this->_m_pReply = NULL;
}

CAMMain::CAMMain(const QString &strDBFilePath, const QString &strDBConnectName):
    _m_strDBFilePath(strDBFilePath), _m_strConnectName(strDBConnectName)
{
    this->_m_pNetManager = NULL;
    this->_m_pReply = NULL;
}

CAMMain::~CAMMain()
{
    if(_m_pNetManager)
    {
        delete _m_pNetManager;
        _m_pNetManager = NULL;
    }
}

void CAMMain::SetDBFilePath(const QString &strDBFilePath)
{
    _m_strDBFilePath = strDBFilePath;
}

void CAMMain::SetDBConnectName(const QString &strConnectName)
{
    _m_strConnectName = strConnectName;
}

int CAMMain::SyncGetData(const QString &strInUrl, QByteArray &byteArray) noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始http get请求";

    QUrl url = strInUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");


    SET_HTTPS_SSL_CONFIG(req);

    //QSslConfiguration sslConfig = req.sslConfiguration();
    //sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //sslConfig.setProtocol(QSsl::TlsV1SslV3);
    //req.setSslConfiguration(sslConfig);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求链接: " << strInUrl;

    _m_pNetManager->disconnect();
    _m_pReply = _m_pNetManager->get(req);

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(_m_pReply);

    byteArray = _m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArray;

    _m_pReply->close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求成功";
    return NO_ERROR;
}

int CAMMain::SyncPostByJson(const QString &strUrl,  const QJsonObject &joParams, QByteArray &byteArrayRet, QString strContentType/*="application/json"*/)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("开始http请求");

    QUrl url =strUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, strContentType);
    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求数据: ") << joParams;
    QNetworkAccessManager  netAccessMng;
    QNetworkReply *pReply  = netAccessMng.post( req, QJsonDocument(joParams).toJson(QJsonDocument::Compact));

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(pReply);
    byteArrayRet = pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("http请求结果:") << byteArrayRet;
    pReply->deleteLater();
    return NO_ERROR;
}


//初始化函数
int CAMMain::Init()
{
    if(_m_strDBFilePath.isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("数据库文件路径为空");
        return DB_ERR;
    }
    if(_m_strConnectName.isEmpty())
    {
        //禁止数据库连接名为空
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库连接名为空";
        return DB_ERR;
    }


    if (g_bIsLAN)
    {
        //测试网
        _m_db = QSqlDatabase::addDatabase("QMYSQL", _m_strConnectName);
        _m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
        _m_db.setHostName("192.168.10.81");
        _m_db.setPort(3306);
        _m_db.setUserName("root");
        _m_db.setPassword("MyNewPass4!");
        _m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }
    else
    {
        //主网
        _m_db = QSqlDatabase::addDatabase("QMYSQL", _m_strConnectName);
        _m_db.setDatabaseName("assertdb");
        _m_db.setHostName("rm-wz991r2rjs3wmd9t1io.mysql.rds.aliyuncs.com");
        _m_db.setPort(3306);
        _m_db.setUserName("accertdb");
        _m_db.setPassword("CG1R47JxIfBofG5uIofHcUKW0Ay1f8");
        _m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }

    if (!_m_db.open())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "连接(打开)数据库失败, 错误信息:" << _m_db.lastError().text();
        return DB_ERR;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "启动数据库成功!" ;
//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << _m_db.driver()->hasFeature(QSqlDriver::BatchOperations);

    //初始化网络连接
    _m_pNetManager = new QNetworkAccessManager(this);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("初始化网络连接成功!");
    return NO_ERROR;
}
