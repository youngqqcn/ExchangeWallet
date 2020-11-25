/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      重置密码对话框
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef RESET_PWD_DLG_H
#define RESET_PWD_DLG_H

#include "am_dialog_base.h"

namespace Ui {
class CResetPwdDlg;
}

class CResetPwdDlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CResetPwdDlg(QWidget *parent = 0);
    ~CResetPwdDlg();

    void GetAdminId(const uint &uAdminId);

protected:
    virtual void _Init() override;

private:
    //初始化界面上的各种编辑框
    void __InitUIDisplay();

private slots:
    void __OnAffirm();
    void __OnCancel();

private:
    Ui::CResetPwdDlg *ui;

    uint __m_uAdminId;
};

#endif // RESET_PWD_DLG_H
