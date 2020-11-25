/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员类型对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_ADMIN_TYPE_LIST_H
#define PAGE_ADMIN_TYPE_LIST_H

#include "am_main.h"
#include <QObject>

class CPageAdminTypeList : public CAMMain
{
    Q_OBJECT
public:
//    explicit CPageAdminTypeList(QObject *parent = nullptr);
    explicit CPageAdminTypeList(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageAdminTypeList();

    virtual int Init() override;

public:
    //查询按钮
    int ConditionSearch(vector<QStringList> &vctAdminType);
    //冻结按钮(冻结数据库中所有这种类型的管理员账号)
    int FreezeAdminType(const uint &uAdminTypeId);
    //删除按钮
    int DeleteAdminType(const uint uAdminTypeId);

    //解冻按钮
    int UnfreezeAdminType(const uint &uAdminTypeId);

signals:

public slots:
};

#endif // PAGE_ADMIN_TYPE_LIST_H
