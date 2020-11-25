/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加大地址对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef ADD_BIG_ADDR_DLG_H
#define ADD_BIG_ADDR_DLG_H

#include <QDialog>
#include "am_dialog_base.h"

namespace Ui {
class CAdd_Big_Addr_Dlg;
}

class CAdd_Big_Addr_Dlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CAdd_Big_Addr_Dlg(QWidget *parent = 0);
    ~CAdd_Big_Addr_Dlg();
    QString m_strCoinType;
    QString m_strAddress;
    QString m_strVerifyCode;

private:
    void __InitCoinType_cbx();      //初始化币种选择框

protected:
    virtual void _Init();

private slots:
    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();

private:
    Ui::CAdd_Big_Addr_Dlg *ui;
};

#endif // ADD_BIG_ADDR_DLG_H
