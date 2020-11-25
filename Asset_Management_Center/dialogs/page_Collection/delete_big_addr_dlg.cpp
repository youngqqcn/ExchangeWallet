/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      删除选中地址对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "delete_big_addr_dlg.h"
#include "ui_delete_big_addr_dlg.h"

CDelete_Big_Addr_Dlg::CDelete_Big_Addr_Dlg(const QString& strDelAddr, QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CDelete_Big_Addr_Dlg)
{
    ui->setupUi(this);
    _Init();
    ui->label_Delete_address->setText(strDelAddr);
    m_strVerifyCode.clear();
}

CDelete_Big_Addr_Dlg::~CDelete_Big_Addr_Dlg()
{
    delete ui;
}
void CDelete_Big_Addr_Dlg::_Init()
{
    return CAMDialogBase::_Init();
}
void CDelete_Big_Addr_Dlg::on_btn_OK_clicked()
{
    if(ui->le_verify_code->text().trimmed().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), QString("谷歌验证码未填写"));
        MESSAGE_BOX_OK;
        return;
    }

    m_strVerifyCode = ui->le_verify_code->text().trimmed();
    close();
}

void CDelete_Big_Addr_Dlg::on_btn_Cancel_clicked()
{
    close();
}
