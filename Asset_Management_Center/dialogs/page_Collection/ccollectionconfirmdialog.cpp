/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      确认归集选中对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "ccollectionconfirmdialog.h"
#include "ui_ccollectionconfirmdialog.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "utils.h"

CCollectionConfirmDialog::CCollectionConfirmDialog(
        const QString& strTotalAmount, uint uiAddrCount, const QString& strTxFee, \
        const QString& strCoinType, const QString &strAddr, const QString& strlabel, bool bAddrEditable/*=false*/, QWidget *parent/*=NULL*/)
        :CAMDialogBase(parent), ui(new Ui::CCollectionConfirmDialog)
{
    ui->setupUi(this);
    _Init(); //调用父类的默认的初始化函数
    ui->le_Addr->setText(strAddr);
    ui->label_Amount->setText(QString("%1 %2").arg(strTotalAmount).arg(strCoinType));
    ui->label_AddressCount->setText(QString("%1个").arg(uiAddrCount));
    ui->label_TxFee->setText(QString("%1").arg(strTxFee));
    ui->lbl_CollectAffirm_collectAddr->setText(strlabel);

    if((0 == strCoinType.compare("usdt", Qt::CaseInsensitive) && false == bAddrEditable)
            || (0 == strCoinType.compare("btc", Qt::CaseInsensitive) && false == bAddrEditable)
            || (0 == strCoinType.compare("eth", Qt::CaseInsensitive) && false == bAddrEditable)
            || (0 == strCoinType.compare("htdf", Qt::CaseInsensitive) && false == bAddrEditable)
            || (0 == strCoinType.compare("usdp", Qt::CaseInsensitive) && false == bAddrEditable)
            || (0 == strCoinType.compare("het", Qt::CaseInsensitive) && false == bAddrEditable)
            )
    {
        //ui->le_Addr->setEnabled(false);
        ui->le_Addr->setReadOnly(true);
    }

    m_strAddr.clear();
    m_strVerifyCode.clear();

    m_strAddr = strAddr;


    //获取值保存在成员变量
    m_strCoinType = strCoinType.trimmed();
    m_strTotalAmount = strTotalAmount.trimmed();
    m_uAddrCount = uiAddrCount;
    m_strTxFee = m_strTxFee;
    m_strVerifyCode = m_strVerifyCode;

}

CCollectionConfirmDialog::~CCollectionConfirmDialog()
{
    delete ui;
}

void CCollectionConfirmDialog::_Init()
{
    return CAMDialogBase::_Init();
}

void CCollectionConfirmDialog::on_btn_OK_clicked()
{
    if(ui->le_Verify_code->text().trimmed().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), QString("谷歌验证码未填写"));
        MESSAGE_BOX_OK;
        return;
    }

    if(ui->le_Addr->text().trimmed().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, QString("错误"), QString("归币地址未填写!"));
        MESSAGE_BOX_OK;
        return;
    }

    QString strAddr = ui->le_Addr->text().trimmed();
    QString strVerifyCode = ui->le_Verify_code->text().trimmed();

    //TODO: 添加地址检查规则(不同币种)  可以参考 CAutowithdraw::__IsValidAddr()
    if(false == utils::IsValidAddr(m_strCoinType, strAddr))
    {
        QMessageBox msgbox(QMessageBox::Warning, QString("错误"), QString("地址非法, 请谨慎输入地址,并检查!"));
        MESSAGE_BOX_OK;
        return;
    }

    m_strAddr = strAddr;
    m_strVerifyCode = strVerifyCode;

    this->done(QDialog::Accepted);
}

void CCollectionConfirmDialog::on_btn_Cancel_clicked()
{
    this->done(QDialog::Rejected);
}


