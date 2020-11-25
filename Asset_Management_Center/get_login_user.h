#ifndef GET_LOGIN_USER_H
#define GET_LOGIN_USER_H

#include "comman.h"
#include <QObject>
#include <QSqlDatabase>

class CGetLoginUser : public QObject
{
    Q_OBJECT
public:
    explicit CGetLoginUser(QObject *parent = nullptr);
    explicit CGetLoginUser(const QString &strDBName);
    virtual ~CGetLoginUser();

    //程序启动的时候查询数据库中用户的登录状态,返回"login_status=1"的用户的用户名
    int checkLoginStatus(const QString &strLoginName);

    //程序关闭的时候更新数据库中的所有用户的login_status为0
    int updateLoginStatus(const QString &strLoginName);

private:
    int init();

private:
    QString         __m_strDBFilePath;
    QString         __m_strDBConnectName;
    QSqlDatabase    __m_db;
};

#endif // GET_LOGIN_USER_H
