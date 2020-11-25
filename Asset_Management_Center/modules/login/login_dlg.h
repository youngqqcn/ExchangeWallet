/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      登录界面代码头文件
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef LOGIN_DLG_H
#define LOGIN_DLG_H

#include "am_dialog_base.h"
#include "login_operate.h"
#include "user_info.h"
#include "get_login_user.h"

namespace Ui {
class CLoginDlg;
}

class CLoginDlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CLoginDlg(QWidget *parent = 0);
    ~CLoginDlg();

    int GetResultValue();          //获取返回值
    void fastToLogin(const QString strUser, const QString strPwd);

protected:
    virtual void _Init() override;

private slots:
    void __OnLoginPress();      //登录按钮槽函数
    void __OnClosePress();      //关闭按钮槽函数

private:
    Ui::CLoginDlg *ui;
    CLoginOperate *__m_pLoginOperate;
    CGetLoginUser   *__m_getLoginUser;

    int __m_iResultValue;

    QString     __m_strUser;
    QString     __m_strPwd;
};

#endif // LOGIN_DLG_H
