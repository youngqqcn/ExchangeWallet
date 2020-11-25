/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员对话框
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef ADD_ADMIN_DLG_H
#define ADD_ADMIN_DLG_H

#include "am_dialog_base.h"

namespace Ui {
class CAddAdminDlg;
}

class CAddAdminDlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CAddAdminDlg(QWidget *parent = 0);
    ~CAddAdminDlg();

    void SetTitle(const QString &strTitle, const int iFlag);        //设置标题
    void SetUIAdminInfo(const vector<QString> &vctCondition);       //设置界面上的编辑框中的内容

protected:
    virtual void _Init() override;

private:
    //初始化界面上的各种编辑框
    void __InitUIDisplay();
    //初始化界面上的管理员类型选择框
    void __InitAdminType_cbx();

private slots:
    void __OnAffirm();
    void __OnCancel();

public slots:
    //当addAdminType界面新增了一个管理员类型时会发送信号,这边会新增一个管理员类型项
    void OnAddAdminTypeCbx(const QString &strAdminTypeName);
    //当addAdminType界面编辑了一个管理员类型时会发送信号,这边会更新管理员类型名这个combobox的内容
    void OnEditAdminTypeCbx(const QStringList &strlistAdminTypeName);
    //当addAdminType界面删除了一个管理员类型时会发送信号,这边会更新管理员类型名这个combobox的内容
//    void OnDeleteAdminTypeCbx(const QString &strItem);

private:
    Ui::CAddAdminDlg *ui;

    int __m_iFlag;
};

#endif // ADD_ADMIN_DLG_H
