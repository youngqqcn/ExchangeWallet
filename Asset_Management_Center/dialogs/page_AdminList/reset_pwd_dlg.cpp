/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      重置密码对话框
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "reset_pwd_dlg.h"
#include "ui_reset_pwd_dlg.h"
#include "page_admin_list.h"

CResetPwdDlg::CResetPwdDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CResetPwdDlg)
{
    ui->setupUi(this);
    //调用父类初始化函数
    CAMDialogBase::_Init();
    //调用自身初始化函数
    _Init();
}

CResetPwdDlg::~CResetPwdDlg()
{
    delete ui;
}

void CResetPwdDlg::GetAdminId(const uint &uAdminId)
{
    __m_uAdminId = uAdminId;
}

void CResetPwdDlg::_Init()
{
    connect(ui->btn_Affirm, &QPushButton::clicked, this, &CResetPwdDlg::__OnAffirm);
    connect(ui->btn_Cancel, &QPushButton::clicked, this, &CResetPwdDlg::__OnCancel);
}

void CResetPwdDlg::__InitUIDisplay()
{
    ui->le_NewPwd->text().clear();
    ui->le_AffirmNewPwd->text().clear();
}

void CResetPwdDlg::__OnAffirm()
{
     if (ui->le_AffirmNewPwd->text().isEmpty() || ui->le_NewPwd->text().isEmpty())
     {
         qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "输入错误:输入的密码为空";
         QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请输入新密码"));
         MESSAGE_BOX_OK;
         return;
     }
     if (ui->le_NewPwd->text() != ui->le_AffirmNewPwd->text())
     {
         QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("注意:两次输入的密码不一致"));
         MESSAGE_BOX_OK;
         ui->le_AffirmNewPwd->clear();
         return;
     }
     //验证正则表达式
     QRegExp regExpPwd(REGEXP_PASSWD);
     QString strPwd = ui->le_NewPwd->text();
     if (!regExpPwd.exactMatch(strPwd))
     {
         QMessageBox msgbox(QMessageBox::Warning, tr("重置失败"), tr("重置密码失败:\n密码必须为8-20位字母和数字混合!"));
         MESSAGE_BOX_OK;
         return;
     }

     CPageAdminList *pAdminListHandles = ::GetHandle<CPageAdminList>(UINT_HANDLE::PAGE_ADMINLIST);
     if (nullptr == pAdminListHandles)
     {
         //动态类型转换失败
         qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
         __InitUIDisplay();
         return;
     }
     QString strConditionNew = ui->le_AffirmNewPwd->text();
     QString strConditionOld = ui->le_OldPwd->text();

     int iRet = pAdminListHandles->CheckOldPwd(strConditionOld, __m_uAdminId);
     if (NO_ERROR != iRet)
     {
         qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "校验失败,输入的旧密码不正确";
         QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("您输入的旧密码不正确"));
         MESSAGE_BOX_OK;
         return;
     }

     iRet = pAdminListHandles->ResetPwd(strConditionNew, __m_uAdminId);
     if (NO_ERROR != iRet)
     {
         //处理错误
         qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ResetPwd 重置密码失败";
         //请根据实际业务添加其他处理....
         QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请检查数据库"));
         MESSAGE_BOX_OK;
         __InitUIDisplay();
         return;
     }

     __InitUIDisplay();
     QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("密码重置成功"));
     MESSAGE_BOX_OK;
     this->close();
}

void CResetPwdDlg::__OnCancel()
{
    __InitUIDisplay();
    this->close();
}
