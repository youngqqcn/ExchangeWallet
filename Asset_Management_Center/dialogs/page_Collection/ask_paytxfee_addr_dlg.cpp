#include "ask_paytxfee_addr_dlg.h"
#include "ui_ask_paytxfee_addr_dlg.h"

#include "comman.h"
#include "utils.h"

CAskPayTxFeeAddrDlg::CAskPayTxFeeAddrDlg(const QString &strCoinType, const QString &strCollectAddr, bool bEnableEdit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAskPayTxFeeAddrDlg)
{
    ui->setupUi(this);

    ui->le_paytxfee_addr->setText(strCollectAddr);

    if ((0 == strCoinType.compare("usdt", Qt::CaseInsensitive) && !bEnableEdit)
            || (0 == strCoinType.compare("btc", Qt::CaseInsensitive) && !bEnableEdit))
    {
        ui->le_paytxfee_addr->setReadOnly(true);
    }

    m_strPayTxFeeAddr = "";
    m_nOnceCount = 0;

    //ui->btn_Back->setEnabled(false);
}

CAskPayTxFeeAddrDlg::~CAskPayTxFeeAddrDlg()
{
    delete ui;
}

void CAskPayTxFeeAddrDlg::on_btn_Query_clicked()
{
    QString strTmpAddr =  ui->le_paytxfee_addr->text().trimmed();

    if (!utils::IsValidAddr("usdt", strTmpAddr))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("地址错误"), tr("输入的地址非法!!"));
        MESSAGE_BOX_OK;
        return;
    }



    auto  fnTmpGetOnceCount = [&]() -> int{
        QString strUrl = g_qstr_WalletHttpIpPort + STR_usdt_getcollectiononcecount;
        strUrl += QString("payfee_address=") + strTmpAddr;

        QByteArray bytesRsp;
        int iRet = utils::HttpUtils::SyncGetData(strUrl, bytesRsp);
        if(NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("SyncGetData() 调用失败");
            throw runtime_error("请求服务接口HTTP错误,检查网络连接, 接口: /usdt/getcollectiononcecount ");
        }

        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"Json解析错误" ;
            throw runtime_error("解析服务端返回数据失败: Json解析错误");
        }

        if(!jsonRsp.isObject())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "服务器返回的json格式错误";
            throw runtime_error("解析服务端返回数据失败: Json解析错误");
        }

        QJsonObject joRes = jsonRsp.object();

        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "服务器返回的json格式错误";
            throw runtime_error("解析服务端返回数据失败: Json解析错误");
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "服务器返回的json格式错误";
            throw runtime_error("请求服务接口返回失败,检查参数,接口: /usdt/getcollectiononcecount ");
        }

        int nCount = joRes.value("result").toInt(0);
        return nCount;
    };

    try{
        m_nOnceCount = fnTmpGetOnceCount();
        m_strPayTxFeeAddr = strTmpAddr;

        ui->le_once_count->setText(QString::asprintf("%d", m_nOnceCount));
        //ui->btn_Back->setEnabled(true);

    }catch(std::exception &e){
        QString strErrMsg(e.what());
        QMessageBox msgbox(QMessageBox::Warning, tr("错误:"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }

}

void CAskPayTxFeeAddrDlg::on_btn_Back_clicked()
{
    this->close();
    //(0);
}
