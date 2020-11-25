#include "dlg_single_lineedit.h"
#include "ui_dlg_single_lineedit.h"

#include <QMessageBox>

CDlg_single_lineedit::CDlg_single_lineedit(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CDlg_single_lineedit)
{
    ui->setupUi(this);
    CAMDialogBase::_Init();
}

CDlg_single_lineedit::CDlg_single_lineedit(QString strLabelText, QString strTitle, QWidget *parent, QLineEdit::EchoMode echoMode /*= QLineEdit::Normal*/):
        CAMDialogBase(parent),
        ui(new Ui::CDlg_single_lineedit)
{
    ui->setupUi(this);

    ui->lb_label->setText(strLabelText);
    this->setWindowTitle(strTitle);
    ui->le_Key->setEchoMode(echoMode);

    CAMDialogBase::_Init();

}

CDlg_single_lineedit::~CDlg_single_lineedit()
{
    delete ui;
}

void CDlg_single_lineedit::_Init()
{
}

void CDlg_single_lineedit::on_btn_Ok_clicked()
{
    QString strTmp = ui->le_Key->text().trimmed();
    if(strTmp.isEmpty())
    {
        QMessageBox::warning(this, "错误", "内容为空, 请填写内容");
        return;
    }

    this->m_strLineText = strTmp;
    done(QDialog::Accepted);
}

void CDlg_single_lineedit::on_btn_Cancel_clicked()
{
    this->m_strLineText = ui->le_Key->text().trimmed();
    done(QDialog::Rejected);
}

void CDlg_single_lineedit::Clear()
{
    ui->le_Key->clear();
}
