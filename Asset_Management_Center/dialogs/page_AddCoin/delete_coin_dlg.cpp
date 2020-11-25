/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      删除币种对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "delete_coin_dlg.h"
#include "ui_delete_coin_dlg.h"
#include "page_add_coin.h"
#include "page_charge_list.h"

CDeleteCoinDlg::CDeleteCoinDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CDeleteCoinDlg)
{
    ui->setupUi(this);
    //调用父类的默认的初始化函数
    CAMDialogBase::_Init();
    //调用自身初始化
    _Init();

}

CDeleteCoinDlg::~CDeleteCoinDlg()
{
    delete ui;
}

void CDeleteCoinDlg::getAddr(const QString &strAddr, const QString &strCoinType)
{
    __m_strWithdrawAddr = strAddr;
    __m_strCoinType = strCoinType;
}

void CDeleteCoinDlg::_Init()
{
    connect(ui->Btn_Affirm, &QPushButton::clicked, this, &CDeleteCoinDlg::__OnAffirm);
    connect(ui->btn_Cancel, &QPushButton::clicked, this, &CDeleteCoinDlg::__OnCancel);
}

void CDeleteCoinDlg::__InitLed()
{
    ui->le_GoogleCode->clear();
}

void CDeleteCoinDlg::__OnAffirm()
{
    if (ui->le_GoogleCode->text().isEmpty())
    {

        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("未填写谷歌验证码"));
        MESSAGE_BOX_OK;
        return;
    }

    CPageAddCoin *pDeleteCoin = ::GetHandle<CPageAddCoin>(UINT_HANDLE::PAGE_ADD_COIN_LIST);
    if(nullptr == pDeleteCoin) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        __InitLed();
        return;
    }

    CPageChargeList *pChargeListHandles = ::GetHandle<CPageChargeList>(UINT_HANDLE::PAGE_CHARGE_LIST);
    if(nullptr == pChargeListHandles) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        __InitLed();
        return;
    }

    QString strGoogleCode = ui->le_GoogleCode->text().trimmed();
    int iRet = -1;
    iRet = pDeleteCoin->DeleteCoin(__m_strWithdrawAddr, __m_strCoinType, strGoogleCode);
    if(NO_ERROR != iRet)
    {
        //处理错误
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "AddCoin 添加币种失败";
        //请根据实际业务添加其他处理....
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请检查数据库或谷歌验证码的正确性"));
        MESSAGE_BOX_OK;
        __InitLed();
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该币种的该地址删除成功"));
    MESSAGE_BOX_OK;
    //删除币种成功后发送信号
    QStringList strlist;
    iRet = pChargeListHandles->CheckAllCoinType(strlist);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询所有币种失败";
        return;
    }
    emit Send_UpdateCoinType(strlist);

    this->close();
}

void CDeleteCoinDlg::__OnCancel()
{
    __InitLed();
    this->close();
}
