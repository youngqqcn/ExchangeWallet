#include "audit_dlg.h"
#include "ui_audit_dlg.h"

audit_dlg::audit_dlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::audit_dlg)
{
    ui->setupUi(this);
    CAMDialogBase::_Init();

    _Init();
}

audit_dlg::~audit_dlg()
{
    delete ui;
}

void audit_dlg::_Init()
{
    connect(ui->pushButton, &QPushButton::clicked, this, &audit_dlg::onOkClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &audit_dlg::onCancelClicked);
}

void audit_dlg::setDlgTitle(const QString &strTitle, const uint &uFlag, const ChargeList::CHARGEINFO &selectedChargeInfo)
{
    ui->label->setText(strTitle);
    m_iFlag = uFlag;  //0表示审核,1表示驳回
    __m_selectedChargeInfo = selectedChargeInfo;

    if (0 == m_iFlag)
        ui->lineEdit->setPlaceholderText("请输入审核通过的备注信息...");
    else
        ui->lineEdit->setPlaceholderText("请输入审核驳回的备注信息...");
}

uint audit_dlg::getFlag()
{
    return __m_uFlag;
}

void audit_dlg::onOkClicked()
{
    ui->pushButton->setEnabled(false);
    if (ui->lineEdit->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"),tr("没有输入备注信息..."));
        MESSAGE_BOX_OK;
        ui->pushButton->setEnabled(true);
        ui->lineEdit->setFocus();
        return;
    }

    CPageChargeList *pChargeListHandle = ::GetHandle<CPageChargeList>(UINT_HANDLE::PAGE_CHARGE_LIST);
    if(nullptr == pChargeListHandle) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        ui->lineEdit->clear();
        ui->pushButton->setEnabled(true);
        this->close();
    }

    __m_selectedChargeInfo.strAuditRemark = ui->lineEdit->text();
    __m_selectedChargeInfo.strAuditor = CUserInfo::loginUser().FamilyName();
    if (0 == m_iFlag && !__m_selectedChargeInfo.strTxid.isEmpty()) //审核
        pChargeListHandle->coinChargeFinishUpdateDB(__m_selectedChargeInfo);

    ui->lineEdit->clear();
    ui->pushButton->setEnabled(true);
    __m_uFlag = QDialog::Accepted;
    this->close();
}

void audit_dlg::onCancelClicked()
{
    ui->lineEdit->clear();
    __m_uFlag = QDialog::Rejected;
    this->close();
}
