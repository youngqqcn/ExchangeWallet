/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      确认归集选中对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef CCOLLECTIONCONFIRMDIALOG_H
#define CCOLLECTIONCONFIRMDIALOG_H

#include <QDialog>
#include "am_dialog_base.h"

namespace Ui {
class CCollectionConfirmDialog;
}

class CCollectionConfirmDialog : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CCollectionConfirmDialog(const QString& strTotalAmount, uint uiAddrCount, const QString& strTxFee,
                                      const QString& strCoinType, const QString& strDstAddr, const QString& strlabel,
                                       bool bAddrEditable=false, QWidget *parent = 0);
    ~CCollectionConfirmDialog();

public:
    QString         m_strAddr;
    QString         m_strVerifyCode;
    QString         m_strTotalAmount;
    uint            m_uAddrCount;
    QString         m_strTxFee;
    QString         m_strCoinType;

protected:
    virtual void _Init();

private slots:
    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();

private:
    Ui::CCollectionConfirmDialog *ui;
};

#endif // CCOLLECTIONCONFIRMDIALOG_H
