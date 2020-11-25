/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加币种对话框界面代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "add_coin_dlg.h"
#include "ui_add_coin_dlg.h"
#include "page_add_coin.h"
#include "page_charge_list.h"
#include "utils.h"

//匹配key
#define REGEXP_ENCODEKEY "^\\w{10,10}$"

CAddCoinDlg::CAddCoinDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CAddCoinDlg)
{
    ui->setupUi(this);
    CAMDialogBase::_Init(); //调用父类的默认的初始化函数
    _Init();
}

CAddCoinDlg::~CAddCoinDlg()
{
    delete ui;
}


void CAddCoinDlg::setTitle(const QString &strTitle, const int iFlag)
{
    ui->label_2->setText(strTitle);
    __m_iflag = iFlag;
}

void CAddCoinDlg::GetCoinInfo(const vector<QString> &vctCondition)
{
    if (9 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return;
    }
    ui->le_CoinTypeNick->setText(vctCondition[0]);
    ui->le_No->setText(vctCondition[1]);
    ui->le_autoWithdrawAddr->setText(vctCondition[3]);
    ui->le_MinWithDrawCount->setText(vctCondition[4]);
    ui->le_SemiAutoWithdrawAmount->setText( vctCondition[5] );
    ui->le_DayWithDrawCount->setText(vctCondition[6]);
    ui->le_blockBrowser->setText(vctCondition[8]);
    ui->le_chargeThreshold->setText(vctCondition[7]);
}
void CAddCoinDlg::__InitCondition()
{
    ui->le_CoinTypeNick->clear();
    ui->le_No->clear();
    ui->le_autoWithdrawAddr->clear();
    ui->le_privateKey->clear();
    ui->le_DecodePwd->clear();
    ui->le_googleVerificationCode->clear();
    ui->le_DayWithDrawCount->clear();
    ui->le_MinWithDrawCount->clear();
    ui->le_SemiAutoWithdrawAmount->clear(); //半自动额度上限  2020-04-26 yqq
    ui->le_blockBrowser->clear();
    ui->le_chargeThreshold->clear();
    ui->le_autoWithdrawAddr->setEnabled(true);
    ui->le_CoinTypeNick->setEnabled(true);
    ui->le_DecodePwd->setEnabled(true);
}

void CAddCoinDlg::_Init()
{
    connect(ui->btn_affirm, &QPushButton::clicked, this, &CAddCoinDlg::__OnAffirm);
    connect(ui->btn_cancel, &QPushButton::clicked, this, &CAddCoinDlg::__OnCancel);
    connect(ui->btn_ImportPrivKey, &QPushButton::clicked, this, &CAddCoinDlg::__OnImportPrivKey);
    __InitCondition();
}

void CAddCoinDlg::__OnAffirm()
{
    if (ui->le_autoWithdrawAddr->text().isEmpty() || ui->le_googleVerificationCode->text().isEmpty()
            || ui->le_CoinTypeNick->text().isEmpty() || ui->le_DayWithDrawCount->text().isEmpty()
            || ui->le_MinWithDrawCount->text().isEmpty() || ui->le_DecodePwd->text().isEmpty()
            || ui->le_privateKey->text().isEmpty() || ui->le_blockBrowser->text().isEmpty()
            || ui->le_chargeThreshold->text().isEmpty()
            || ui->le_SemiAutoWithdrawAmount->text().isEmpty()
            )
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "您有未填写的信息";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:您有未填写的信息"));
        MESSAGE_BOX_OK;
        return;
    }

    QRegExp expEncodeKey(REGEXP_ENCODEKEY);
    QString strEncodeKey = ui->le_DecodePwd->text().trimmed();
    if (!expEncodeKey.exactMatch(strEncodeKey))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:请填写10位数字或字母组成的KEY!"));
        MESSAGE_BOX_OK;
        return;
    }
    bool use_chinese = strEncodeKey.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
    if(use_chinese)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:解密密码不能有中文!"));
        MESSAGE_BOX_OK;
        return;
    }

    //TODO:添加地址规则检查, 私钥检查, 以及密码检查等等等...  by yqq  2019-05-19

    QString strTmpCoinType = ui->le_CoinTypeNick->text().trimmed().toUpper();
    QString strTmpAddr = ui->le_autoWithdrawAddr->text().trimmed();

    if(0 == strTmpCoinType.compare("ERC20FEE", Qt::CaseInsensitive))
        strTmpCoinType = "ETH";
    else if( 0 == strTmpCoinType.compare("HRC20FEE", Qt::CaseInsensitive))
        strTmpCoinType = "HTDF";

    if(false == utils::IsValidAddr(strTmpCoinType, strTmpAddr))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("注意:自动提币地址 %1 非法！请检查后,谨慎输入地址!").arg(strTmpAddr);
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:自动提币地址非法！请检查后,谨慎输入地址!"));
        MESSAGE_BOX_OK;
        ui->le_autoWithdrawAddr->setEnabled(true);
        ui->le_CoinTypeNick->setEnabled(true);
        ui->le_DecodePwd->setEnabled(true);
        return;
    }

    QString strTmpPrivKey = ui->le_privateKey->text().trimmed();
    if(false == utils::IsValidPrivKey(strTmpCoinType, strTmpPrivKey, g_bBlockChainMainnet))
    {
        //警告!! 禁止直接将私钥输出到日志!!  只能输出提示信息!!
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("自动提币地址的私钥非法！请检查后,谨慎输入私钥");
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:自动提币地址的私钥非法！请检查后,谨慎输入私钥!"));
        MESSAGE_BOX_OK;
        ui->le_autoWithdrawAddr->setEnabled(true);
        ui->le_CoinTypeNick->setEnabled(true);
        ui->le_DecodePwd->setEnabled(true);
        return;
    }

    QString strMinWithdrawOnce = ui->le_MinWithDrawCount->text();
    double dAutoWithdrawAmount = 0.0;
    try
    {
        dAutoWithdrawAmount = boost::lexical_cast<double>(strMinWithdrawOnce.toStdString());

    }
    catch(boost::bad_lexical_cast &e)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(e.what());
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:自动提币上限金额不是有效数字！请检查后,谨慎输入!"));
        MESSAGE_BOX_OK;
        return;
    }

    if( dAutoWithdrawAmount < 0 || dAutoWithdrawAmount > 99999999 )
    {
        QString strErrMsg = QString("全自动提币金额非法!");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }

    QString strSemiWithdrawAmount = ui->le_SemiAutoWithdrawAmount->text();
    double dSemiWindrawAmount = 0.0;
    try
    {
        dSemiWindrawAmount = boost::lexical_cast<double>(strSemiWithdrawAmount.toStdString());
    }
    catch(boost::bad_lexical_cast &e)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(e.what());
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:半自动提币上限金额不是有效数字！请检查后,谨慎输入!"));
        MESSAGE_BOX_OK;
        return;
    }

    if( dSemiWindrawAmount < 0 || dSemiWindrawAmount > 99999999 )
    {
        QString strErrMsg = QString("半自动提币上限金额非法!");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }



    QString strDataWithdrawAmount = ui->le_DayWithDrawCount->text().trimmed();
    double dDayWithdrawAmount = 0.0;
    try
    {
        dDayWithdrawAmount = boost::lexical_cast<double>(strDataWithdrawAmount.toStdString());

    }
    catch(boost::bad_lexical_cast &e)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("注意:单日累计提币金额不是有效数字！请检查后,谨慎输入! errMsg: %1").arg(QString(e.what()));
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:单日累计提币金额不是有效数字！请检查后,谨慎输入!"));
        MESSAGE_BOX_OK;
        return;
    }

    if( dDayWithdrawAmount < 0 || dDayWithdrawAmount > 99999999 )
    {
        QString strErrMsg = QString("单日累计提币金额非法!");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }



    QString strChargeThreshold = ui->le_chargeThreshold->text().trimmed();
    double dChargeThreshold = 0.0;
    try
    {
        dChargeThreshold = boost::lexical_cast<double>(strChargeThreshold.toStdString());
    }
    catch(boost::bad_lexical_cast &e)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("注意:充币审核阀值不是有效数字！请检查后,谨慎输入! errMsg: %1").arg(QString(e.what()));
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:充币审核阀值不是有效数字！请检查后,谨慎输入!"));
        MESSAGE_BOX_OK;
        return;
    }

    //对以上三个金额进行判断
    if( dSemiWindrawAmount <= dAutoWithdrawAmount )
    {
        QString strErrMsg = QString("半自动提币上限 必须大于 全自动上限额度!");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }

    if( dDayWithdrawAmount <= dSemiWindrawAmount  )
    {
        QString strErrMsg = QString("单日单币种累计提币额度 必须大于 半自动上限额度!");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }




    vector<QString> vctCondition;
    vctCondition.push_back(ui->le_No->text());
    vctCondition.push_back(ui->le_CoinTypeNick->text().trimmed().toUpper());
    vctCondition.push_back(ui->le_autoWithdrawAddr->text().trimmed());
    vctCondition.push_back(ui->le_privateKey->text().trimmed());
    vctCondition.push_back(ui->le_DecodePwd->text().trimmed());
    vctCondition.push_back(ui->le_MinWithDrawCount->text().trimmed());
    vctCondition.push_back( ui->le_SemiAutoWithdrawAmount->text().trimmed() ); //半自动
    vctCondition.push_back(ui->le_DayWithDrawCount->text().trimmed());
    vctCondition.push_back(ui->le_blockBrowser->text().trimmed());
    vctCondition.push_back(ui->le_chargeThreshold->text().trimmed());
    //vctCondition.push_back(strTmpCollectionAddr);
    QString strGoogleCode = ui->le_googleVerificationCode->text().trimmed();

    CPageAddCoin *pAddCoinHandles = ::GetHandle<CPageAddCoin>(UINT_HANDLE::PAGE_ADD_COIN_LIST);
    if(nullptr == pAddCoinHandles) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("系统错误! 请联系管理员!"));
        MESSAGE_BOX_OK;
        ui->le_autoWithdrawAddr->setEnabled(true);
        ui->le_CoinTypeNick->setEnabled(true);
        ui->le_DecodePwd->setEnabled(true);
        return;
    }

    CPageChargeList *pChargeListHandles = ::GetHandle<CPageChargeList>(UINT_HANDLE::PAGE_CHARGE_LIST);
    if(nullptr == pChargeListHandles) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
         QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("系统错误! 请联系管理员!"));
        MESSAGE_BOX_OK;
        ui->le_autoWithdrawAddr->setEnabled(true);
        ui->le_CoinTypeNick->setEnabled(true);
        ui->le_DecodePwd->setEnabled(true);
        return;
    }

    if (-1 != __m_iflag)
    {
        int iRet = -1;
        iRet = pAddCoinHandles->EditCoin(vctCondition);
        if(NO_ERROR != iRet)
        {
            //处理错误
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "AddCoin 编辑币种失败";
            //请根据实际业务添加其他处理....
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请检查数据库或填写信息"));
            MESSAGE_BOX_OK;
            return;
        }

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("编辑币种成功, 请在界面上的列表查看"));
        MESSAGE_BOX_OK;
        //编辑币种成功后发送信号
        QStringList strlist;
        iRet = pChargeListHandles->CheckAllCoinType(strlist);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询所有币种失败";
            this->close();
            return;
        }

        emit Send_UpdateCoinType(strlist);
        __InitCondition();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("成功退出__OnAffirm");
        this->close();
    }
    else
    {
        if (ui->le_autoWithdrawAddr->text().isEmpty() || ui->le_googleVerificationCode->text().isEmpty()
                || ui->le_CoinTypeNick->text().isEmpty() || ui->le_DayWithDrawCount->text().isEmpty()
                || ui->le_MinWithDrawCount->text().isEmpty() || ui->le_DecodePwd->text().isEmpty()
                || ui->le_privateKey->text().isEmpty() || ui->le_blockBrowser->text().isEmpty()
                || ui->le_chargeThreshold->text().isEmpty()
                || ui->le_SemiAutoWithdrawAmount->text().isEmpty()
                )
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("注意:您有未填写的信息"));
            MESSAGE_BOX_OK;
            return;
        }

        int iRet = -1;
        iRet = pAddCoinHandles->AddCoin(vctCondition, strGoogleCode);
        if(NO_ERROR != iRet)
        {
            //处理错误
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "AddCoin 添加币种失败";
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请检查数据库或填写信息"));
            MESSAGE_BOX_OK;
            return;
        }

        //添加币种成功后发送信号
        emit Send_AddCoinType(ui->le_CoinTypeNick->text().trimmed());
        __InitCondition();
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("添加币种成功, 请在界面上的列表查看"));
        MESSAGE_BOX_OK;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("成功退出__OnAffirm");
        this->close();
    }
}

void CAddCoinDlg::__OnCancel()
{
    __InitCondition();
    this->close();
}

void CAddCoinDlg::__OnImportPrivKey()
{
    if (ui->le_CoinTypeNick->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("导入私钥之前,必须填写币种!"));
        MESSAGE_BOX_OK;
        return;
    }
    if (ui->le_autoWithdrawAddr->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("导入私钥之前,必须填写提币地址!"));
        MESSAGE_BOX_OK;
        return;
    }
    if (ui->le_DecodePwd->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("导入私钥之前,必须填写解密密码!"));
        MESSAGE_BOX_OK;
        return;
    }

    QString strFilePath = QFileDialog::getOpenFileName(this, tr("选择加密私钥文件"), ".", tr("dat file (*.dat)"));

    if(strFilePath.trimmed().isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "文件选中失败";
        return;
    }

    //判断输入的密码是否跟选中的所有文件匹配
    QFile readFile(strFilePath);
    if(!readFile.open(QFile::ReadOnly))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "打开地址私钥加密文件失败!";
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("地址私钥文件打开失败!"));
        MESSAGE_BOX_OK;
        return;
    }
    QDataStream readStream(&readFile);
    QString strEncodePrivKey = "";
    QString strDecodePrivKey = "";
    readStream >> strEncodePrivKey;
    QStringList strlistPrivKeyTmp = strEncodePrivKey.trimmed().split("\t");

    //应该先判断界面输入的地址与文件中第一个地址是否一致
    QString strFileAddrTmp =  strlistPrivKeyTmp.at(1);
    if(0 != strFileAddrTmp.trimmed().compare( ui->le_autoWithdrawAddr->text().trimmed() , Qt::CaseSensitive ) )
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("文件\"%1\"的中第一个地址与界面上输入的地址不一致!").arg(strFilePath);
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("私钥文件的中第一个地址与界面上输入的地址不一致!"));
        MESSAGE_BOX_OK;
        return;
    }

    strEncodePrivKey = strlistPrivKeyTmp.at(2);
    utils::decodePrivKey(strEncodePrivKey, strDecodePrivKey);
    if (0 != ui->le_DecodePwd->text().compare(strDecodePrivKey.mid(0, 10), Qt::CaseSensitive))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("文件\"%1\"的密码对不上输入的解密密码密码!").arg(strFilePath);
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("文件的加密密码和输入的解密密码不一致!"));
        MESSAGE_BOX_OK;
        return;
    }
    readFile.close();

    ui->le_privateKey->clear();
    QMessageBox msgbox(QMessageBox::Information, tr("导入成功"), tr("私钥导入成功"));
    MESSAGE_BOX_OK;
    ui->le_privateKey->setText(strDecodePrivKey.mid(10));
    ui->le_autoWithdrawAddr->setEnabled(false);
    ui->le_CoinTypeNick->setEnabled(false);
    ui->le_DecodePwd->setEnabled(false);
}

