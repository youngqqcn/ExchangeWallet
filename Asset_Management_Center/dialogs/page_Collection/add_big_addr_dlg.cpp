/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加大地址对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "add_big_addr_dlg.h"
#include "ui_add_big_addr_dlg.h"
#include "page_charge_list.h"
#include <QListView>

CAdd_Big_Addr_Dlg::CAdd_Big_Addr_Dlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CAdd_Big_Addr_Dlg)
{
    ui->setupUi(this);
    _Init();
    m_strAddress.clear();
    m_strCoinType.clear();
    m_strVerifyCode.clear();

    __InitCoinType_cbx();
}

CAdd_Big_Addr_Dlg::~CAdd_Big_Addr_Dlg()
{
    delete ui;
}

void CAdd_Big_Addr_Dlg::__InitCoinType_cbx()
{
    ui->combo_CoinType->setView(new QListView);
    CPageChargeList *pChargeListHandles = ::GetHandle<CPageChargeList>(UINT_HANDLE::PAGE_CHARGE_LIST);
    if (nullptr == pChargeListHandles)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "动态类型转换失败";
        return;
    }

    QStringList strlist;
    int iRet = pChargeListHandles->CheckAllCoinType(strlist);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败";
        return;
    }
    ui->combo_CoinType->addItems(strlist);
}
void CAdd_Big_Addr_Dlg::_Init()
{
    return CAMDialogBase::_Init();
}
void CAdd_Big_Addr_Dlg::on_btn_OK_clicked()
{
    if(ui->le_verify_code->text().trimmed().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), QString("谷歌验证码未填写"));
        MESSAGE_BOX_OK;
        return;
    }
    if(ui->le_address->text().trimmed().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, QString("错误"), QString("地址未填写!"));
        MESSAGE_BOX_OK;
        return;
    }
    if(ui->combo_CoinType->currentText().trimmed().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, QString("错误"), QString("币种未选择!"));
        MESSAGE_BOX_OK;
        return;
    }

    m_strAddress = ui->le_address->text().trimmed();
    m_strVerifyCode = ui->le_verify_code->text().trimmed();
    m_strCoinType = ui->combo_CoinType->currentText().trimmed();
    close();
}

void CAdd_Big_Addr_Dlg::on_btn_Cancel_clicked()
{
    close();
}
