/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员对话框
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "add_admin_dlg.h"
#include "ui_add_admin_dlg.h"
#include "page_admin_list.h"
#include "page_charge_list.h"
#include <QListView>

CAddAdminDlg::CAddAdminDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CAddAdminDlg)
{
    ui->setupUi(this);
    //调用父类初始化函数
    CAMDialogBase::_Init();
    //调用自身初始化函数
    _Init();
}

CAddAdminDlg::~CAddAdminDlg()
{
    delete ui;
}

void CAddAdminDlg::SetTitle(const QString &strTitle, const int iFlag)
{
    ui->label_2->setText(strTitle);
    __m_iFlag = iFlag;
}

void CAddAdminDlg::SetUIAdminInfo(const vector<QString> &vctCondition)
{
    ui->cbx_AdminType->setCurrentText(vctCondition[0].trimmed());
    ui->le_UserName->setText(vctCondition[1].trimmed());
    ui->le_Name->setText(vctCondition[2].trimmed());
    ui->le_Pwd->setText(vctCondition[3].trimmed());
    ui->le_AffirmPwd->setText(vctCondition[3].trimmed());
    ui->le_PhoneNum->setText(vctCondition[4].trimmed());
}

void CAddAdminDlg::_Init()
{
    connect(ui->btn_Affirm, &QPushButton::clicked, this, &CAddAdminDlg::__OnAffirm);
    connect(ui->btn_Cancel, &QPushButton::clicked, this, &CAddAdminDlg::__OnCancel);

    ui->cbx_AdminType->setView(new QListView);
    __InitAdminType_cbx();
}

void CAddAdminDlg::__InitUIDisplay()
{
    ui->cbx_AdminType->setCurrentIndex(0);
    ui->le_UserName->clear();
    ui->le_Name->clear();
    ui->le_Pwd->clear();
    ui->le_AffirmPwd->clear();
    ui->le_PhoneNum->clear();
    ui->le_GoogleVerifyCode->clear();
}

void CAddAdminDlg::__InitAdminType_cbx()
{
    CPageAdminList *pAdminListHandles = ::GetHandle<CPageAdminList>(UINT_HANDLE::PAGE_ADMINLIST);
    if(nullptr == pAdminListHandles) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        return;
    }

    QStringList strlist;
    int iRet = pAdminListHandles->SetAdminTypeCbx(strlist);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型失败,请检查数据库";
        return;
    }
    ui->cbx_AdminType->addItems(strlist);
}

void CAddAdminDlg::__OnAffirm()
{
    if (0 == ui->cbx_AdminType->currentIndex() || ui->le_AffirmPwd->text().isEmpty() || ui->le_GoogleVerifyCode->text().isEmpty()
            || ui->le_Name->text().isEmpty() || ui->le_PhoneNum->text().isEmpty() || ui->le_Pwd->text().isEmpty()
            || ui->le_UserName->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请填写完整的信息"));
        MESSAGE_BOX_OK;
        return;
    }

    CPageAdminList *pAdminListHandles = ::GetHandle<CPageAdminList>(UINT_HANDLE::PAGE_ADMINLIST);
    if(nullptr == pAdminListHandles) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        return;
    }
    if (0 != ui->le_Pwd->text().compare(ui->le_AffirmPwd->text(), Qt::CaseSensitive))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("添加失败"), tr("注意:两次密码不一致!"));
        MESSAGE_BOX_OK;
        return;
    }
    //验证用户名正则
    QRegExp regExpLoginName(REGEXP_LOGIN_NAME);
    QString strLoginName = ui->le_UserName->text();
    if (!regExpLoginName.exactMatch(strLoginName))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("添加失败"), tr("添加管理员失败:\n用户名必须为6-20位字母或数字组成!"));
        MESSAGE_BOX_OK;
        return;
    }
    //验证密码正则
    QRegExp regExpPwd(REGEXP_PASSWD);
    QString strPwd = ui->le_AffirmPwd->text();
    QString strAffiemPwd = ui->le_Pwd->text();
    if (!regExpPwd.exactMatch(strPwd) || !regExpPwd.exactMatch(strAffiemPwd))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("添加失败"), tr("添加管理员失败:\n密码必须为8-20位字母和数字混合!"));
        MESSAGE_BOX_OK;
        return;
    }
    //验证手机号正则
    QRegExp regExpPhone(REGEXP_PHONE);
    QString strPhone = ui->le_PhoneNum->text();
    if (!regExpPhone.exactMatch(strPhone))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("添加失败"), tr("添加管理员失败:\n手机号必须为十一位数字组成!"));
    }

    vector<QString> vctCondition;
    vctCondition.push_back(ui->cbx_AdminType->currentText().trimmed());
    vctCondition.push_back(ui->le_UserName->text().trimmed());
    vctCondition.push_back(ui->le_Name->text().trimmed());
    vctCondition.push_back(ui->le_Pwd->text().trimmed());
    vctCondition.push_back(ui->le_PhoneNum->text().trimmed());

    if (0 != __m_iFlag)
    {
        int iRet = pAdminListHandles->EditAdminAffirmBtn(vctCondition, __m_iFlag);
        if (NO_ERROR != iRet)
        {
            //处理错误
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "EditAdmin 编辑管理员失败";
            //请根据实际业务添加其他处理....
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请检查数据库或填写信息"));
            MESSAGE_BOX_OK;
            return;
        }

        __InitUIDisplay();
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("编辑管理员成功, 请在界面上查看"));
        MESSAGE_BOX_OK;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("成功退出__OnAffirm");
        this->close();
    }
    else
    {
        //首先判断用户名是否已存在
        int iRet = pAdminListHandles->SearchAdminLoginName(ui->le_UserName->text());
        if (HTTP_ERR == iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "已存在的登录名";
            QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("注意:该用户名已存在!"));
            MESSAGE_BOX_OK;
            return;
        }

        iRet = pAdminListHandles->AddAdminAffirmBtn(vctCondition);
        if (NO_ERROR != iRet)
        {
            //处理错误
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "AddAdmin 添加管理员失败";
            //请根据实际业务添加其他处理....
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请检查数据库或填写信息"));
            MESSAGE_BOX_OK;
            return;
        }

        __InitUIDisplay();
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("添加管理员成功, 请在界面上查看"));
        MESSAGE_BOX_OK;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("成功退出__OnAffirm");
        this->close();
    }
}

void CAddAdminDlg::__OnCancel()
{
    __InitUIDisplay();
    this->close();
}

void CAddAdminDlg::OnAddAdminTypeCbx(const QString &strAdminTypeName)
{
    ui->cbx_AdminType->addItem(strAdminTypeName);
}

void CAddAdminDlg::OnEditAdminTypeCbx(const QStringList &strlistAdminTypeName)
{
    ui->cbx_AdminType->clear();
    ui->cbx_AdminType->addItem("请选择管理员类型");
    for (int i = 0; i < strlistAdminTypeName.count(); i++)
    {
        ui->cbx_AdminType->addItem(strlistAdminTypeName.at(i));
    }
}
