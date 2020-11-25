/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      登录的用户信息管理模块
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef USER_INFO_H
#define USER_INFO_H

#include <QString>

class CUserInfo
{
public:
    explicit CUserInfo();
    QString LoginName() const;
    void SetLoginName(const QString &strLoginName);

    QString FamilyName() const;
    void SetFamilyName(const QString &strFamilyName);

    QString Password() const;
    void SetPassword(const QString &password);

    QString AdminID() const;
    void SetAdminID(const QString &strAdminID);

    QString AdminTypeID() const;
    void SetAdminTypeID(const QString &strAdminTypeID);

    QString Tel() const;
    void SetTel(const QString &strTel);

    QString Status() const;
    void SetStatus(const QString &strStatus);

    static CUserInfo& loginUser();

private:
    QString __m_UserName;               //用户名
    QString __m_FamilyName;             //管理员姓名
    QString __m_Passwd;                 //密码
    QString __m_AdminID;                //管理员ID
    QString __m_AdminTypeID;            //管理员类型ID
    QString __m_Tel;                    //电话
    QString __m_Status;                 //状态(0:冻结  1:正常)

    static CUserInfo __ms_LoginUser;
};

#endif // USER_INFO_H
