#include "sig_confirm_dlg.h"
#include "ui_sig_confirm_dlg.h"
#include <QMessageBox>

//CSigConfirmDlg::CSigConfirmDlg(QWidget *parent) :
//    QDialog(parent),
//    ui(new Ui::CSigConfirmDlg)
//{
//    ui->setupUi(this);
//}

CSigConfirmDlg::CSigConfirmDlg(const QStringList& lstSrcAddr,  QString &strAmount, QWidget *parent/*=0*/):
    QDialog(parent),
    ui(new Ui::CSigConfirmDlg)
{
     ui->setupUi(this);

     ui->le_Amount->setText(strAmount);
     ui->le_SrcAddr->setText(lstSrcAddr.join(","));

     //隐藏边框和标题
     this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
     this->setAttribute(Qt::WA_TranslucentBackground);
}

CSigConfirmDlg::~CSigConfirmDlg()
{
    delete ui;
}

void CSigConfirmDlg::on_btn_Ok_clicked()
{
    QString strPrivKey = ui->le_PrivKey->text().trimmed();
    if(strPrivKey.length() < 45/* || strPrivKey.length() > 64*/)//不卡最大长度，多签会很长
    {
        QMessageBox::warning(this, tr("错误"), tr("私钥非法!!!"));
        return;
    }

    m_strPrivKey = strPrivKey;
    QDialog::close();
}

void CSigConfirmDlg::on_btn_Cancel_clicked()
{
    QDialog::close();
}
