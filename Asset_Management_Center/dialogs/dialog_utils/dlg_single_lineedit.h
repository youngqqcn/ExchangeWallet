#ifndef DLG_SINGLE_LINEEDIT_H
#define DLG_SINGLE_LINEEDIT_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include "am_dialog_base.h"

namespace Ui {
class CDlg_single_lineedit;
}

class CDlg_single_lineedit : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit CDlg_single_lineedit(QWidget *parent = 0);
    explicit CDlg_single_lineedit(QString strLabelText, QString strTitle, QWidget *parent = 0,  QLineEdit::EchoMode echoMode = QLineEdit::Normal);
    ~CDlg_single_lineedit();

private slots:
    void on_btn_Ok_clicked();

    void on_btn_Cancel_clicked();

public:

    virtual void _Init() ;

    QString m_strLineText;
    void Clear ();

private:
    Ui::CDlg_single_lineedit *ui;
};



#endif // DLG_SINGLE_LINEEDIT_H
