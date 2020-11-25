/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      删除选中地址对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef DELETE_BIG_ADDR_DLG_H
#define DELETE_BIG_ADDR_DLG_H

#include <QDialog>
#include "am_dialog_base.h"
namespace Ui {
class CDelete_Big_Addr_Dlg;
}

class CDelete_Big_Addr_Dlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CDelete_Big_Addr_Dlg(const QString& strDelAddr, QWidget *parent = 0);
    ~CDelete_Big_Addr_Dlg();
    QString m_strVerifyCode;
protected:
    virtual void _Init();


private slots:
    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();

private:
    Ui::CDelete_Big_Addr_Dlg *ui;
};

#endif // DELETE_BIG_ADDR_DLG_H
