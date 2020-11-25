/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员业务代码模块
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_ADD_ADMIN_TYPE_H
#define PAGE_ADD_ADMIN_TYPE_H

#include "am_main.h"
#include "comman.h"
#include <QObject>

class CPageAddAdminType : public CAMMain
{
    Q_OBJECT
public:
//    explicit CPageAddAdminType(QObject *parent = nullptr);
    explicit CPageAddAdminType(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageAddAdminType();

    virtual int Init() override;

public:
    //添加按钮
    int AddAdminType(const vector<QString> &vctCondition, uint &uAdminTypeID);
    //编辑按钮
    int UpdateAdminType(const uint &uFlag, const vector<QString> &vctCondition);
    //判断数据库是否已存在管理员类型名
    int CheckAdminType(const QString &strAdminTypeName);

private:
    //查询最大管理员类型ID
    uint __QueryMaxAdminTypeId();

private:
    //插入管理员类型
    int __InsertAdminType(const vector<QString> &vctCondition, uint &uAdminTypeID);

    //更新管理员类型数据
    int __UpdateAdminType(const uint uFlag, const vector<QString> &vctCondition);

private:
    uint uAdminTypeId;  //管理员类型表中最大管理员Id
};

#endif // PAGE_ADD_ADMIN_TYPE_H
