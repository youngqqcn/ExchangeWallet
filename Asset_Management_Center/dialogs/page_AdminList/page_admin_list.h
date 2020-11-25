/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员数据库操作代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_ADMIN_LIST_H
#define PAGE_ADMIN_LIST_H

#include "am_main.h"

class CPageAdminList : public CAMMain
{
public:
    explicit CPageAdminList(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageAdminList();

    virtual int Init() override;

public:
    //添加管理员对话框界面确认按钮业务代码
    int AddAdminAffirmBtn(const vector<QString> &vctCondition);

    //编辑管理员界面确认按钮
    int EditAdminAffirmBtn(const vector<QString> &vctCondition, const int &__m_iFlag);

    //重置密码对话框界面确认按钮业务代码
    int ResetPwd(const QString &strCondition, const uint &uAdminId);
    //重置密码对话框确认按钮业务
    int CheckOldPwd(const QString &strCondition, const uint &uAdminId);

    //管理员界面搜索按钮业务代码
    int SearchAdminInfo(const vector<QString> &vctCondition, vector<QStringList> &vctAdminInfo);

    //管理员界面删除按钮业务代码
    int DeleteAdminInfo(const uint &uAdminId);

    //管理员界面冻结按钮业务
    int FreezeAdmin(const uint &uAdminId);

    //管理员界面冻结按钮业务
    int UnfreezeAdmin(const uint &uAdminId);

    //设置管理员类型combobox框
    int SetAdminTypeCbx(QStringList &strlist);

    //判断数据库中是否已存在相同的管理员登录名
    int SearchAdminLoginName(const QString &strAdminLoginName);

private:
    //查询数据库中管理员类型表中的管理员类型ID,根据管理员类型名
    int __QueryAdminTypeID(const QString &strAdminTypeName, vector<QString> &vctAdminInfo);
    //查询数据库中管理员表中管理员ID最大值
    int __QueryMaxAdminID(uint &uAdminId);
    //根据管理员类型ID查询对应的管理员类型名
    int __QueryAdminTypeName(const uint &uAdminTypeId, QString &strAdminTypeName);

};

#endif // PAGE_ADMIN_LIST_H
