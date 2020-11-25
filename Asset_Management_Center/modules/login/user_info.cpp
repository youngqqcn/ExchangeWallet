/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      存储登录的用户信息
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "user_info.h"

CUserInfo CUserInfo::__ms_LoginUser;

CUserInfo::CUserInfo()
{

}

QString CUserInfo::LoginName() const
{
    return __m_UserName;
}

void CUserInfo::SetLoginName(const QString &strLoginName)
{
    __m_UserName = strLoginName;
}

QString CUserInfo::FamilyName() const
{
    return __m_FamilyName;
}

void CUserInfo::SetFamilyName(const QString &strFamilyName)
{
    __m_FamilyName = strFamilyName;
}

QString CUserInfo::Password() const
{
    return __m_Passwd;
}

void CUserInfo::SetPassword(const QString &password)
{
    __m_Passwd = password;
}

QString CUserInfo::AdminID() const
{
    return __m_AdminID;
}

void CUserInfo::SetAdminID(const QString &strAdminID)
{
    __m_AdminID = strAdminID;
}

QString CUserInfo::AdminTypeID() const
{
    return __m_AdminTypeID;
}

void CUserInfo::SetAdminTypeID(const QString &strAdminTypeID)
{
    __m_AdminTypeID = strAdminTypeID;
}

QString CUserInfo::Tel() const
{
    return __m_Tel;
}

void CUserInfo::SetTel(const QString &strTel)
{
    __m_Tel = strTel;
}

QString CUserInfo::Status() const
{
    return __m_Status;
}

void CUserInfo::SetStatus(const QString &strStatus)
{
    __m_Status = strStatus;
}

CUserInfo& CUserInfo::loginUser()
{
    return __ms_LoginUser;
}
