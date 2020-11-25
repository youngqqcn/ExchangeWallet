#include "comman.h"
#include "manualconfirmdialog.h"
#include "ui_manualconfirmdialog.h"
#include <QMessageBox>


CManualConfirmDialog::CManualConfirmDialog(const QString& strTitle, QWidget *parent/*=0*/) :
    CAMDialogBase(parent),
    ui(new Ui::CManualConfirmDialog)
{
    ui->setupUi(this);
    CAMDialogBase::_Init(); //调用父类的默认的初始化函数
    ui->label_Title->setText(strTitle);
    m_strTitle = strTitle;
    m_strRemarks.clear();
    m_strVerifyCode.clear();
}

CManualConfirmDialog::~CManualConfirmDialog()
{
    delete ui;
}

void CManualConfirmDialog::_Init()
{
}

void CManualConfirmDialog::on_btn_OK_clicked()
{
    QString strGoogleCode = ui->le_verify_code->text().trimmed();
    QString strRemark = ui->le_remark->text().trimmed();

    if(strGoogleCode.isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), QString("谷歌验证码未填写"));
        MESSAGE_BOX_OK;
        return;
    }

    if(strRemark.isEmpty() && 0 == m_strTitle.compare(QString("审核驳回")))
    {
        QMessageBox msgbox(QMessageBox::Critical, QString("错误"), QString("备注未填写!"));
        MESSAGE_BOX_OK;
        return;
    }

    m_strRemarks = strRemark.isEmpty() ? ("---") : ( strRemark );
    m_strVerifyCode = strGoogleCode;

    this->done(QDialog::Accepted);
}

void CManualConfirmDialog::on_btn_Cancel_clicked()
{
    this->done(QDialog::Rejected);
}
