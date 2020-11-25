/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      删除币种对话框界面代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef DELETE_COIN_DLG_H
#define DELETE_COIN_DLG_H

#include "am_dialog_base.h"

namespace Ui {
class CDeleteCoinDlg;
}

class CDeleteCoinDlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CDeleteCoinDlg(QWidget *parent = 0);
    ~CDeleteCoinDlg();

public:
    //获取自动提现地址
    void getAddr(const QString &strAddr, const QString &strCoinType);

signals:
    void Send_UpdateCoinType(QStringList strCoinTypeName);   //删除币种成功后发送信号

protected:
    //纯虚函数,  子类继承, 必须实现Init方法
   virtual void _Init() override;

private:
    void __InitLed();

private slots:
    void __OnAffirm();          //确认按钮
    void __OnCancel();          //取消按钮

private:
    Ui::CDeleteCoinDlg *ui;

    QString __m_strWithdrawAddr;    //自动提现地址
    QString __m_strCoinType;        //币种
};

#endif // DELETE_COIN_DLG_H
