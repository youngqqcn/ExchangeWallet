/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加币种对话框
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef ADD_COIN_DLG_H
#define ADD_COIN_DLG_H

#include "comman.h"
#include "am_dialog_base.h"

namespace Ui {
class CAddCoinDlg;
}

class CAddCoinDlg : public CAMDialogBase     //继承自资产基类
{
    Q_OBJECT

public:
    explicit CAddCoinDlg(QWidget *parent = 0);
    ~CAddCoinDlg();

    void setTitle(const QString &strTitle, const int iFlag);     //设置标题
    void GetCoinInfo(const vector<QString> &vctCondition);

signals:
    void Send_AddCoinType(QString strCoinTypeName);    //添加币种成功够发送信号
    void Send_UpdateCoinType(QStringList strCoinTypeName);   //编辑币种成功后发送信号

private:
    void __InitCondition();         //初始化查询条件

protected:
    virtual void _Init() override;//重写父类初始化函数

private slots:
    void __OnAffirm();          //确认按钮
    void __OnCancel();          //取消按钮
    void __OnImportPrivKey();   //导入私钥文件

private:
    Ui::CAddCoinDlg *ui;

    int __m_iflag;
};

#endif // ADD_COIN_DLG_H
