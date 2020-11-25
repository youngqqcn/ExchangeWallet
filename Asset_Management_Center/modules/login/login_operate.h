/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      登录业务代码头文件
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef LOGIN_OPERATE_H
#define LOGIN_OPERATE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <vector>
#include <QString>
#include <QDebug>
#pragma execution_character_set("utf-8")

using namespace std;

namespace LoginOperate {
typedef struct _AdminInfo
{
    _AdminInfo()
    {
        UserName = "";
        Passwd = "";
        AdminID = "";
        AdminTypeID = "";
        Tel = "";
        Status = "";
    }
    QString UserName;               //用户名
    QString FamilyName;             //管理员姓名
    QString Passwd;                 //密码
    QString AdminID;                //管理员ID
    QString AdminTypeID;            //管理员类型ID
    QString Tel;                    //电话
    QString Status;                 //状态(0:冻结  1:正常)
}AdminInfo;
}

class CLoginOperate : public QObject
{
    Q_OBJECT
public:
    explicit CLoginOperate(QObject *parent = nullptr);
//    explicit CLoginOperate(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CLoginOperate();

    int Init();

public:
    enum{
        NO_ERROR = 0,
        DB_ERR = 10001,
        INVALID_USERNAME = 3,
    };

public:
    int QueryUserName(LoginOperate::AdminInfo &adminInfo);
//    int QueryPassword(const QString &strPassword);
    //登录成功之后,更新登录用的登录状态为1,其他的都更新为0
    int updateLoginStatus(const QString adminID);

private:
    QString         __m_strDBFilePath;
    QString         __m_strDBConnectName;
    QSqlDatabase    __m_db;

};

#endif // LOGIN_OPERATE_H
