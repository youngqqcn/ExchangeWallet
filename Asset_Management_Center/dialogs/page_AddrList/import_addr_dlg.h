/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      地址导入对话框
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef LEAD_IN_ADDR_DLG_H
#define LEAD_IN_ADDR_DLG_H

#include "am_dialog_base.h"
#include <QDialog>

namespace Ui {
class CImportAddrDlg;
}

class CImportAddrDlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CImportAddrDlg(QWidget *parent = 0);
    ~CImportAddrDlg();

private:
    void __InitUIControl();       //初始化界面上的控件的内容

protected:
    //重写基类的初始化函数
    virtual void _Init() override;

private slots:
    void __OnCancelPress();       //取消按钮点击
    void __OnAffirmPress();       //确认按钮点击
    void __OnChoosePress();       //选择按钮点击

private:
    Ui::CImportAddrDlg *ui;
};

#endif // LEAD_IN_ADDR_DLG_H
