#include "get_login_user.h"

CGetLoginUser::CGetLoginUser(QObject *parent) : QObject(parent)
{

}

CGetLoginUser::CGetLoginUser(const QString &strDBName)
{
    __m_strDBConnectName = strDBName;
    int iRet = init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "获取用户登录状态部分: 初始化数据库失败";
    }
}

CGetLoginUser::~CGetLoginUser()
{

}

int CGetLoginUser::checkLoginStatus(const QString &strLoginName)
{
    QSqlQuery queryResult(__m_db);
    QString strSql_1 = QString("select login_status from tb_admin where login_name = '%1'").arg(strLoginName);

    if (!queryResult.exec(strSql_1))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库中用户登录状态失败,请检查数据库";
        return DB_ERR;
    }
    if (queryResult.next())
    {
        if (1 == queryResult.value(0).toInt())
        {
            return INVALID_USERNAME;
        }
    }
    else
    {
        return DB_ERR;
    }

    return NO_ERROR;
}

int CGetLoginUser::updateLoginStatus(const QString &strLoginName)
{
    if(!__m_db.isOpen() && !__m_db.open())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "reopen db error:" <<__m_db.lastError().text();
    }

    QSqlQuery queryResult(__m_db);
    QString strSql_1 = QString("update tb_admin set login_status = 0 where login_name = '%1'").arg(strLoginName);

    if (!queryResult.exec(strSql_1))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库中用户登录状态失败,请检查数据库";
        return DB_ERR;
    }
    return NO_ERROR;
}

int CGetLoginUser::init()
{

    if (g_bIsLAN)
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", __m_strDBConnectName);
        __m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
        __m_db.setHostName("192.168.10.81");
        __m_db.setPort(3306);
        __m_db.setUserName("root");
        __m_db.setPassword("MyNewPass4!");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }
    else
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", __m_strDBConnectName);
        __m_db.setDatabaseName("assertdb");
        __m_db.setHostName("rm-wz991r2rjs3wmd9t1io.mysql.rds.aliyuncs.com");
        __m_db.setPort(3306);
        __m_db.setUserName("accertdb");
        __m_db.setPassword("CG1R47JxIfBofG5uIofHcUKW0Ay1f8");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }

    if (!__m_db.open())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "连接(打开)数据库失败, 错误信息:" << __m_db.lastError().text();
        return DB_ERR;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "启动数据库成功!";
    return NO_ERROR;
}
