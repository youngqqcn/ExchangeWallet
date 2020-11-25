/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      登录业务代码,主要操作数据库
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "login_operate.h"
#include "config.h"

CLoginOperate::CLoginOperate(QObject *parent) : QObject(parent)
{
    int iRet = Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "登录部分: 初始化数据库失败";
    }

}

CLoginOperate::~CLoginOperate()
{

}

int CLoginOperate::Init()
{

    __m_strDBConnectName = QString("LoginOperate");


    //如果是局域网, 则使用开发测试环境
    if(g_bIsLAN)
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", __m_strDBConnectName);
        __m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
        __m_db.setHostName("192.168.10.81");
        __m_db.setPort(3306);
        __m_db.setUserName("root");
        __m_db.setPassword("MyNewPass4!");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }
    else //生产环境
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
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "连接(打开)数据库失败, 错误信息:" << __m_db.lastError();
        return DB_ERR;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "启动数据库成功!";
    return NO_ERROR;
}

int CLoginOperate::QueryUserName(LoginOperate::AdminInfo &adminInfo)
{
    QSqlQuery queryResult(__m_db);
    QString strSql = QString("select * from tb_admin where login_name = binary(\'%1\') ").arg(adminInfo.UserName);

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库中管理员信息失败" << queryResult.lastError();
        return DB_ERR;
    }

    if (queryResult.next())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询管理员信息成功");
        adminInfo.AdminID = queryResult.value(0).toString().trimmed();
        adminInfo.AdminTypeID = queryResult.value(1).toString().trimmed();
        adminInfo.UserName = queryResult.value(2).toString().trimmed();
        adminInfo.FamilyName = queryResult.value(4).toString().trimmed();
        adminInfo.Passwd = queryResult.value(3).toString().trimmed();
        adminInfo.Tel = queryResult.value(5).toString().trimmed();
        adminInfo.Status = queryResult.value(6).toString().trimmed();
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败,没有这个管理员" << queryResult.lastError();
        return INVALID_USERNAME;
    }

    return NO_ERROR;
}

int CLoginOperate::updateLoginStatus(const QString adminID)
{
    QSqlQuery queryResult(__m_db);
    QString strSql_1 = QString("update tb_admin set login_status = 1 where admin_id = %1").arg(adminID);
    QString strSql_2 = QString("update tb_admin set login_status = 0 where admin_id != %1").arg(adminID);

    if (!queryResult.exec(strSql_1))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新登录用户的登录状态为1失败!!!" << queryResult.lastError();
        return DB_ERR;
    }

    queryResult.clear();
    if (!queryResult.exec(strSql_2))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新其他用户的登录状态为0失败!!!" << queryResult.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}
