#ifndef MANUALCONFIRMDIALOG_H
#define MANUALCONFIRMDIALOG_H

//#include <QDialog>
#include "am_dialog_base.h"

namespace Ui {
class CManualConfirmDialog;
}

class CManualConfirmDialog : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CManualConfirmDialog(const QString& strTitle,QWidget *parent = 0);
    ~CManualConfirmDialog();
    QString m_strVerifyCode;
    QString m_strRemarks;
    QString m_strTitle;
protected:
    virtual void _Init();

private slots:
    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();

private:
    Ui::CManualConfirmDialog *ui;


};

#endif // MANUALCONFIRMDIALOG_H
