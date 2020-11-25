/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      短信设置对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef SHORT_MSG_SETTING_DLG_H
#define SHORT_MSG_SETTING_DLG_H
#include "comman.h"
#include "am_dialog_base.h"


namespace Ui {
class CShortMsgSettingDlg;
}

class CShortMsgSettingDlg : public CAMDialogBase       //继承资产管理对话框  基类
{
    Q_OBJECT

public:
    explicit CShortMsgSettingDlg(QWidget *parent = 0);
    ~CShortMsgSettingDlg();

protected:
    //重写基类的初始化函数
    virtual void _Init() override;

public slots:
    void OnClosePress();

private slots:  //公共槽函数, 用于MainWindow上的控件发来的消息

    //设置短信 槽函数
    void  __OnShortMsgSetting();

private:
    void __InitLineEdit();

private:
    Ui::CShortMsgSettingDlg *ui;

};

#endif // SHORT_MSG_SETTING_DLG_H
