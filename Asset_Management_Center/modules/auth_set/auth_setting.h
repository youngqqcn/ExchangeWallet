/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      权限设置模块
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     无修改                       v0.0.1
********************************************************************/
#ifndef AUTH_SETTING_H
#define AUTH_SETTING_H

#include <QObject>
#include <QTreeWidget>
#include <QPushButton>
#include "comman.h"

class CAuthSetting : public QObject
{
    Q_OBJECT
public:
    explicit CAuthSetting(QObject *parent = nullptr);
    virtual ~CAuthSetting();

    int Init();

public:
    void InitAuthlist(QTreeWidget *treeWidget);     //初始化给定的treeWidget

    void updateParentItem(QTreeWidgetItem *item);       //更新父节点的选中状态

    int AddAdminType(const uint admin_Type_ID, QTreeWidget *treeWidget);        //添加管理员权限

    int EditAdminType(const uint admin_type_ID, QTreeWidget *treeWidget);        //编辑管理员权限

    int DisplayAuth(const uint uAdminTypeID, QTreeWidget *treeWidget);  //在编辑管理员类型的时候显示权限标志,当登录的时候判断有哪些权限

    int SetUserBtnAuth(const uint uAdminTypeID, QPushButton *array[]);      //登录成功,设置按钮权限

public slots:
    void OnTreeItemChanged(QTreeWidgetItem *treeWidget);     //树上节点选择变更时响应槽函数

private:
    int __ExecTransaction(const int &admin_Type_ID, QTreeWidget *treeWidget);     //执行节点选中状态

private:
    QString         __m_strDBFilePath;
    QString         __m_strDBConnectName;
    QSqlDatabase    __m_db;
};

#endif // AUTH_SETTING_H
