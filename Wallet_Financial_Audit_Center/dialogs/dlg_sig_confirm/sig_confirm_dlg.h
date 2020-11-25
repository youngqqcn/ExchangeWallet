#ifndef SIG_CONFIRM_DLG_H
#define SIG_CONFIRM_DLG_H

#include "rawtx_comman.h"
#include <QDialog>

namespace Ui {
class CSigConfirmDlg;
}

class CSigConfirmDlg : public QDialog
{
    Q_OBJECT

public:
    //explicit CSigConfirmDlg(QWidget *parent = 0);
    explicit CSigConfirmDlg(const QStringList& lstSrcAddr,  QString &strAmount, QWidget *parent = 0);
    ~CSigConfirmDlg();

public:
    QString m_strPrivKey;

private slots:
    void on_btn_Ok_clicked();

    void on_btn_Cancel_clicked();

private:
    Ui::CSigConfirmDlg *ui;
};

#endif // SIG_CONFIRM_DLG_H
