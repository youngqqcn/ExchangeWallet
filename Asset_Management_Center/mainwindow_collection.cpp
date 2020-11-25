#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "comman.h"
#include "utils.h"
#include "ask_paytxfee_addr_dlg.h"  //usdt 弹出对话框 询问支付手续费的地址

//待归集列表-搜索
extern bool  g_bIsTipsNeeded;  //是否弹框标志

//归集创建-全选
void CMainWindow::page_CollectionCreateRawTx_OnSelAll()
{
    //如果当前是选中  则取消全选
    Qt::CheckState checkState = ( ui->checkBox_CollectionCreateRawTx_SelAll->isChecked() ) ? (Qt::Checked) : (Qt::Unchecked) ;


    QTableWidget *pTbw = ui->tbw_CollectionCreateRawTx;
    for(int iRow = 0; iRow < pTbw->rowCount(); iRow++ )
    {
        QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
        if(nullptr == pTbwItem)
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "获取失败";
            QMessageBox msgbox(QMessageBox::Warning, tr("获取失败"), tr("内部错误"));
            MESSAGE_BOX_OK;
            return;
        }

        pTbwItem->setCheckState(checkState);
    }

}

//创建未签名归集交易-查询按钮
void CMainWindow::page_CollectionCreateRawTx_OnSearch()
{
    QString strErrMsg;
    ui->checkBox_CollectionCreateRawTx_SelAll->setChecked(false);

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("创建未签名归集交易界面查询按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建未签名归集交易界面查询按钮操作插入操作日志表失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionCreateRawTx_OnSearch";


    if(0 == ui->cbx_CollectionCreateRawTx_CoinType->currentIndex())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("查询币种为空");
        QMessageBox msgbox(QMessageBox::Information, tr("查询失败"), tr("查询币种为空，请选择要查询的币种。"));
        MESSAGE_BOX_OK;
        return;
    }

    QString strCurSelCoinType =  ui->cbx_CollectionCreateRawTx_CoinType->currentText().trimmed();
    if(false
         || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
         || 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive)
         || 0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive)
    )
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("当前筛选的币种是\"%1\", 此币种所有用户共用充币地址,通过标签区分, 所以此不需归集.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }
    else if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) ||  utils::IsERC20Token(strCurSelCoinType)
            || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCurSelCoinType)
            || 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive)
           ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的归集功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    QString strCoinType = "";
    strCoinType = ui->cbx_CollectionCreateRawTx_CoinType->currentText().trimmed();
    vector<CollectionQueryRsp> vctCollectRsp;

    ui->btn_CollectionCreateRawTx_Search->setEnabled(false);
    ui->btn_CollectionCreateRawTx_Search->setText("查询中...");
    try
    {
        //清空
        QTableWidget *pTbw = ui->tbw_CollectionCreateRawTx;
        pTbw->clearContents();
        pTbw->setRowCount(0);


        __m_pCollection->CollectionQuery(strCoinType, vctCollectRsp );


        double dClcEthFee  = 0.0;
        if(utils::IsERC20Token(strCoinType))
        {
            dClcEthFee = utils::GetETH_ERC20_CollectionFee(strCoinType);
        }

        strErrMsg = QString("查询成功, 共%1条记录").arg(vctCollectRsp.size());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;



        int nRow = 0;
        for(uint i = 0; i < vctCollectRsp.size(); i++)
        {
            pTbw->insertRow(nRow);
            int iCol = 0;
            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(vctCollectRsp[i].strCoinType);
            pTbw->setItem(nRow, iCol++, checkBox);
            pTbw->setItem(nRow, iCol++, new QTableWidgetItem(vctCollectRsp[i].strAmount));
            pTbw->setItem(nRow, iCol++, new QTableWidgetItem(vctCollectRsp[i].strAddr));
            //pTbw->setItem(nRow, iCol++, new QTableWidgetItem(""));

            if(utils::IsERC20Token(strCoinType))
            {
                const std::vector<ETHCollectionQeuryRsp>  &ethClcRsp = __m_pCollection->GetETHCollectionRsp();
                double dEthBalance = boost::lexical_cast<double>(ethClcRsp.at(i).strETHBalance.toStdString());
                if(dEthBalance < dClcEthFee){
                    pTbw->setItem(nRow, iCol++, new QTableWidgetItem("不足"));
                }else{
                    pTbw->setItem(nRow, iCol++, new QTableWidgetItem("   "));
                }
            }
            else if( utils::Is_HRC20_Token(strCoinType) )
            {
                const std::vector<Hrc20TokensCollectionQueryRsp>  &hrc20ClsRsp = __m_pCollection->Get_HRC20_CollectionRsp();
                double dHTDFBalance = boost::lexical_cast<double>( hrc20ClsRsp.at(i).strHTDFBalance.toStdString()  );
                if( dHTDFBalance < 0.21 ){
                    pTbw->setItem(nRow, iCol++, new QTableWidgetItem("不足"));
                }else{
                    pTbw->setItem(nRow, iCol++, new QTableWidgetItem("   "));
                }

            }

            nRow++;
        }

        //USDT比较特殊, 需要查询一次能够归集多少个地址
        if(0 < vctCollectRsp.size()  && 0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            QString strCollectAddr = (g_bBlockChainMainnet) ? (/*主*/"16di2uD6pSH6C9KZgR8a6bx9KHsr1J9tC4") : (/*测*/"mpreew4dYwpZ9hsJLFzzQVzstgZobGjy4K");

            shared_ptr<CAskPayTxFeeAddrDlg> pDlg = std::make_shared<CAskPayTxFeeAddrDlg>(strCoinType, strCollectAddr);
            pDlg->exec();
            this->m_nOnceCount = pDlg->m_nOnceCount; //一次能归集几个地址
            this->m_strUsdtCollectionDstAddr = pDlg->m_strPayTxFeeAddr; //支付手续费的地址
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("查询错误, 错误信息: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Information, tr("查询失败"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("查询错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Information, tr("查询失败"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    ui->btn_CollectionCreateRawTx_Search->setEnabled(true);
    ui->btn_CollectionCreateRawTx_Search->setText("查询");
}

//创建未签名归集交易-根据选中地址创建归集交易并导出按钮
void CMainWindow::page_CollectionCreateRawTx_OnCreateRawTx()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("创建未签名归集交易界面根据选中地址创建归集交易并导出按钮");

    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建未签名归集交易界面根据选中地址创建归集交易并导出按钮操作插入操作日志表失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionCreateRawTx_OnCreateRawTx";


    QString  strBtnTextBak = ui->btn_CollectionCreateRawTx_CreateRawTx->text().trimmed();
    ui->btn_CollectionCreateRawTx_CreateRawTx->setEnabled(false);
    ui->btn_CollectionCreateRawTx_CreateRawTx->setText("创建交易中....");

    try
    {
        if(0 == ui->cbx_CollectionCreateRawTx_CoinType->currentIndex())
        {
            strErrMsg = QString("当前选择的币种是:%1, 请先选择一个具体的币种").arg(ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentText());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QString strCoinType;
        strCoinType = ui->cbx_CollectionCreateRawTx_CoinType->currentText().trimmed();

        QTableWidget *pTbw = ui->tbw_CollectionCreateRawTx;
        vector<int> vctRows;
        for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
        {
            QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
            if(nullptr == pTbwItem)
            {
                strErrMsg = QString("internal error: got checkbox returned nullptr.");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(Qt::Checked == pTbwItem->checkState())
            {
                vctRows.push_back(iRow);
            }
        }

        //获取所有选择范围
        if(vctRows.empty())
        {
            strErrMsg = "请选择数据,进行创建未签名交易";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        bool bFeeStatusOk = true;
        vector<QString>  vctAddrs; //被归集的用户的地址
        double dSum = 0.0;
        for(auto nRow : vctRows)
        {
            QString strAddr = pTbw->item(nRow, 2)->text().trimmed();
            vctAddrs.push_back(strAddr);

            QString strAmount = pTbw->item(nRow, 1)->text().trimmed();
            std::string cstrAmount = strAmount.toStdString();

            if(utils::IsERC20Token(strCoinType) || utils::Is_HRC20_Token(strCoinType))
            {
                auto pTmpItem =  pTbw->item(nRow, 3);
                if(NULL != pTmpItem) //防止空指针
                {
                    QString  strFeeStatus = pTmpItem->text().trimmed();
                    if(!strFeeStatus.isEmpty())
                    {
                        bFeeStatusOk = false;
                    }
                }
            }


            try
            {
                double dAmountTmp = boost::lexical_cast<double>(cstrAmount);
                dSum += dAmountTmp;
            }
            catch(boost::bad_lexical_cast &e)
            {
                strErrMsg = QString("internal error: bad_lexical_cast, %1").arg(e.what());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error( strErrMsg.toStdString() );
            }
        }

        if(!bFeeStatusOk)
        {
            /*QMessageBox msgbox(QMessageBox::Warning, tr("手续费不足提醒"), tr("提醒:检查到您选择了手续费不足的地址进行归集,极大概率会归集失败,是否强行继续!? "
                                                                  "  (如果您很确定那个地址的余额足以支付归集交易的手续费,您可以继续.否则,建议您不要强行继续.)"));
            MESSAGE_BOX_OK_CANCEL;
            if (QMessageBox::Ok != msgbox.exec())
            */

            //QMessageBox msgbox(QMessageBox::Critical, tr("手续费不足提醒"), tr("错误:您选择了手续费不足的地址! "));
            //MESSAGE_BOX_OK;

            {
                strErrMsg = "您选择了手续费不足的地址!";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }



        QString strSum = QString::asprintf("%.8f", dSum);
        uint32_t uAddrCount = vctRows.size();
        QString strDstAddr = "";
        QString strTxFeeSum = "---";

        //处理USDT归集  特殊
        if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            if(vctRows.size() > this->m_nOnceCount)
            {
                QString strLogText = tr("本次能归集的最大数量是:%1个地址, 您选择的数量是:%2个地址").arg(this->m_nOnceCount).arg(vctRows.size());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strLogText;
                throw  runtime_error( strLogText.toStdString() );
            }

            if(!utils::IsValidAddr("usdt",  this->m_strUsdtCollectionDstAddr))
            {
                QString strLogText = tr("支付手续费的地址(归集目的地址)非法!!! 地址:%1").arg(this->m_strUsdtCollectionDstAddr);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strLogText;
                throw  runtime_error( strLogText.toStdString() );
            }

            strDstAddr = (g_bBlockChainMainnet) ? (/*主*/"16di2uD6pSH6C9KZgR8a6bx9KHsr1J9tC4") : (/*测*/"mpreew4dYwpZ9hsJLFzzQVzstgZobGjy4K");
            strTxFeeSum = QString::asprintf("%.8f BTC",  vctRows.size() * 0.00001);
        }
        else if (0 == strCoinType.compare("btc", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("bch", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive) )
        {
            strDstAddr = (g_bBlockChainMainnet) ?  (/*主*/"1LBPnRsHNfHa5R1swhBCj5E7du25bNXuBN") :  (/*测*/"mxSK56qKAtt74ZhNN6iA9X6rKXiFqR9DP4");
        }
        else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive) )
        {
            strDstAddr = (g_bBlockChainMainnet) ?  (/*主*/"LRan4zGsDRSzbci5JVmuqLJuFZrjgRxKeJ") :  (/*测*/"mn9ft9QiM36uZTFJkpU5PV6c1VDoGNCtp3");
        }
        else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive) )
        {
            strDstAddr = (g_bBlockChainMainnet) ?  (/*主*/"XquQ8kbtHXSMHVjHKVJLgi4D3RYhAz3iES") :  (/*测*/"yiBiGAUe24kbifH23rXV2h5fsyRJ6kTqcV");
        }
        else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
            || 0 == strCoinType.compare("etc", Qt::CaseInsensitive))
        {
            //同提币, 如果ERC20代币有指定特定地址, 则使用指定的地址; 否则,使用ETH的地
            //以太坊相关的设置
            strDstAddr = (g_bBlockChainMainnet) ?  (/*主*/"0xdefebb1a43f749d77de260d7e3569aa5e4fedc48" ) :  (/*测*/"0xD7A4a381Fca4be6b8A645d858f1bDc3107Ac3f5D");

            double dClcEthFee  = 0.0;
            QString strGasLimit = utils::GetGasLimit(strCoinType);
            bool ok = false;
            double dGasLimit = strGasLimit.toDouble(&ok);
            if(!ok)
            {
                strErrMsg = QString("strGasLimit : %1").arg(strGasLimit);
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw std::runtime_error(strErrMsg.toStdString());
            }
            dClcEthFee = COLLECTION_GASPRICE_GWEI * dGasLimit / 1000000000;

            double dFeeTmp = dClcEthFee;
            QString strFeeType = (  0 == strCoinType.compare("etc", Qt::CaseInsensitive) ) ? ( "ETC") : ( "ETH");
            strTxFeeSum = QString::asprintf("%.8f ",  vctRows.size() * dFeeTmp) + strFeeType;
        }
        else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive))
        {
            //设置usdp归集地址
            strDstAddr = (g_bBlockChainMainnet) ? (/*主*/"usdp1k6y92hsjxq22t8p0lmvhx69gyhtn3zahp5gv9v") : (/*测*/"usdp1slqqph8wqwekksa7lpjjf778tdhpp58clxgdgv");
            strTxFeeSum = QString::asprintf("%.8f USDP",  vctRows.size() * 0.0000002);
        }
        else if(0 == strCoinType.compare("htdf", Qt::CaseInsensitive) ||  utils::Is_HRC20_Token(strCoinType))  //HTDF 和 HRC20代币
        {
            //设置htdf归集地址
            //strDstAddr = (g_bBlockChainMainnet) ? (/*主*/"htdf1c0njnzd93lh3zkt3qq28wftuyg3vqrsktj7eqc") : (/*测*/"htdf1ll9vc32wggxmh7enl2vggry0e4u3f5knmvq2d6");
            strDstAddr = (g_bBlockChainMainnet) ? (/*主*/"htdf17gcc420s4nhrglghmftj2dx37yz99lnfxye23p") : (/*测*/"htdf1ll9vc32wggxmh7enl2vggry0e4u3f5knmvq2d6");// 2020-08-04 更新
            strTxFeeSum = QString::asprintf("%.8f HTDF",  vctRows.size() * 0.0000002);
        }
        else if(0 == strCoinType.compare("het", Qt::CaseInsensitive))
        {
            //设置het归集地址
            strDstAddr = (g_bBlockChainMainnet) ? (/*主*/"0x1kzk8r8ydkpmk6xp9kpr95gaxdp4w3q2s64se3e") : (/*测*/"0x1duandu0tr7qzdj9x4f4e2ehmpvfzcc2kae3tx7");
            strTxFeeSum = QString::asprintf("%.8f HET",  vctRows.size() * 0.0000002);
        }
        else if(0 == strCoinType.compare("TRX", Qt::CaseInsensitive))
        {
            strDstAddr = (g_bBlockChainMainnet) ? (/*主*/"TAHLg7TnD18JPCZyyrwWFTdFYMfawRoRCB") : (/*测*/"TAAMnDDuRcHBfG53yVgC4m4TwquvAHasff");
            strTxFeeSum = QString::asprintf("%.8f HET",  vctRows.size() * 0.1);
        }


        strTxFeeSum += QString("(%1)").arg((g_bBlockChainMainnet)?("主链,认真核对归集地址."):("测试链,仅开发人员使用!!!"));

        //弹出对话框，验证谷歌验证码
        std::shared_ptr<CCollectionConfirmDialog> pColConfirmDlg(new CCollectionConfirmDialog(strSum, uAddrCount, strTxFeeSum, strCoinType, strDstAddr, QString("归集目的地址:")));
        if(QDialog::Rejected ==  pColConfirmDlg->exec()) //如果点击取消,直接退出
        {
            strErrMsg = "已取消创建";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        strDstAddr = pColConfirmDlg->m_strAddr.trimmed();

        if(!utils::IsValidAddr(strCoinType, strDstAddr))
        {
            strErrMsg = QString("非法地址: %1").arg(strDstAddr);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        QString strRetExportFilePath;
        __m_pCollection->CreateAndExportUnsignedRawTx(strCoinType, vctRows, vctAddrs, strDstAddr, strRetExportFilePath);

        QMessageBox msgbox(QMessageBox::Information, tr("信息"), tr("创建并导出成功,文件名:%1").arg(strRetExportFilePath));
        MESSAGE_BOX_OK;

        QDesktopServices::openUrl(QUrl::fromLocalFile( QCoreApplication::applicationDirPath() ));

    }
    catch(std::exception &e)
    {
        strErrMsg = QString("创建失败, 错误信息:%1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建失败:未知错误";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("创建失败:未知错误"));
        MESSAGE_BOX_OK;
    }


    //恢复按钮
    ui->btn_CollectionCreateRawTx_CreateRawTx->setEnabled(true);
    ui->btn_CollectionCreateRawTx_CreateRawTx->setText(strBtnTextBak);
}



// ERC20 补手续费
void CMainWindow::page_CollectionCreateRawTx_OnSupplyTxFee()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("归集创建页面,补手续费");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "归集创建页面,补手续费按钮操作插入操作日志表失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionCreateRawTx_OnERC20SupplyTxFee";



    try
    {
        if(0 == ui->cbx_CollectionCreateRawTx_CoinType->currentIndex())
        {
            strErrMsg = QString("当前选择的币种是:%1, 请先选择一个具体的币种").arg(ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentText());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QTableWidget *pTbw = ui->tbw_CollectionCreateRawTx;
        vector<int> vctRows;
        for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
        {
            QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
            if(nullptr == pTbwItem)
            {
                strErrMsg = QString("internal error: got checkbox returned nullptr.");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(Qt::Checked == pTbwItem->checkState())
            {
                vctRows.push_back(iRow);
            }
        }

        //获取所有选择范围
        if(vctRows.empty())
        {
            strErrMsg = "请选择数据,进行创建未签名交易";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //判断币种
        QString strCoinType;
        strCoinType = ui->cbx_CollectionCreateRawTx_CoinType->currentText().trimmed();
        QString strShowCoinType = ui->tbw_CollectionCreateRawTx->item(vctRows[0], 0)->text().trimmed(); //获取币种
        if(0 != strCoinType.compare(strShowCoinType, Qt::CaseInsensitive))
        {
            strErrMsg = QString("当前筛选的币种是:%1, 但是选择的是:%2的数据").arg(strCoinType).arg(strShowCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( !utils::IsERC20Token(strCoinType) && !utils::Is_HRC20_Token(strCoinType) )
        {
            strErrMsg = QString("当前选择的数据币种是:%1, 不是ERC20或HRC20代币,不需要进行补手续费.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //2.弹出确认对话框
        QString strMainCoinType = ( utils::IsERC20Token(strCoinType) ) ? ("ETH") : (STR_HTDF);
        QString strShowInfo =  "";
        if(0 == strMainCoinType.compare("ETH", Qt::CaseInsensitive) )
        {
            double dClcEthFee  = utils::GetETH_ERC20_CollectionFee(strCoinType) + 0.00001;
            strShowInfo = QString("共%1个地址,每个地址分别补 %2手续费, 共计%3%4").arg(vctRows.size())
                                    .arg(dClcEthFee).arg( dClcEthFee * vctRows.size()  ).arg(strMainCoinType);
        }
        else
        {
            strShowInfo = QString("共%1个地址,每个地址分别补 %2手续费, 共计%3%4").arg(vctRows.size())
                                    .arg(FLOAT_HRC20_SUPPLY_FEE ).arg( FLOAT_HRC20_SUPPLY_FEE * vctRows.size()  ).arg(strMainCoinType);
        }

        QMessageBox msgbox(QMessageBox::Information, QString("补手续费"),  strShowInfo);
        MESSAGE_BOX_OK_CANCEL;
        if(QMessageBox::Cancel == msgbox.exec())
        {
            strErrMsg = QString("已取消补手续费");
            throw runtime_error(strErrMsg.toStdString());
        }

        //3.进行转账操作
        if( utils::IsERC20Token(strCoinType) )
        {
            std::vector<QString> vctAddrs;
            const std::vector<ETHCollectionQeuryRsp>  vctEthClcRsp = __m_pCollection->GetETHCollectionRsp();
            for(auto iRow : vctRows) vctAddrs.push_back(vctEthClcRsp.at(iRow).strAddr);
            __m_pCollection->ERC20SupplyTxFee(vctAddrs, strCoinType);
        }
        else if( utils::Is_HRC20_Token(strCoinType) )
        {
             std::vector<QString> vctAddrs;
            const std::vector<Hrc20TokensCollectionQueryRsp>  vctHrc20ClcRsp = __m_pCollection->Get_HRC20_CollectionRsp();
            for(auto iRow : vctRows)
            {
                vctAddrs.push_back(vctHrc20ClcRsp.at(iRow).strAddr);
            }

            __m_pCollection->HRC20_SupplyTxFee(vctAddrs);
        }
        else
        {
            strErrMsg = "error, unkonw cointype";
            throw runtime_error(strErrMsg.toStdString());
        }


        //提示成功
        {
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("成功,可在归集列表查看txid,并查询区块浏览器,交易成功后即可进行归集操作."));
            MESSAGE_BOX_OK;
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("失败, 错误信息:%1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "失败:未知错误";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("失败:未知错误"));
        MESSAGE_BOX_OK;
    }


    return;
}


//归集交易广播-全选
void CMainWindow::page_CollectionBroadcast_OnSelAll()
{
    //如果当前是选中  则取消全选
    Qt::CheckState checkState = ( ui->checkBox_CollectionBroadcast_SelAll->isChecked() ) ? (Qt::Checked) : (Qt::Unchecked) ;

    QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
    for(int iRow = 0; iRow < pTbw->rowCount(); iRow++ )
    {
        QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);

        if(!(pTbwItem->flags() & Qt::ItemIsUserCheckable)) continue;
        //checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );

        if(nullptr == pTbwItem)
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "获取失败";
            QMessageBox msgbox(QMessageBox::Warning, tr("获取失败"), tr("内部错误"));
            MESSAGE_BOX_OK;
            return;
        }


        //未签名的不能选择
        if(pTbwItem->flags() & Qt::ItemIsUserCheckable)
        {
            pTbwItem->setCheckState(checkState);
        }
    }

}

//归集交易广播-搜索
void CMainWindow::page_CollectionBroadcast_OnSearch()
{
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("暂不支持"));
    MESSAGE_BOX_OK;
}

//归集交易广播-导入待广播文件
void CMainWindow::page_CollectionBroadcast_OnImportRawTx()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("归集交易广播界面导入待广播文件按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "归集交易广播界面导入待广播文件按钮操作插入操作日志表失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionBroadcast_OnImportRawTx";

    //选择广播币种
    if(0 == ui->cbx_CollectionBroadcast_CoinType->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请先选择要导入币种"));
        MESSAGE_BOX_OK;
        return;
    }

    QString strCurSelCoinType = ui->cbx_CollectionBroadcast_CoinType->currentText();

    if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive)  || utils::IsERC20Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)|| 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
           || utils::Is_HRC20_Token(strCurSelCoinType)
           ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的广播功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    QString strFilePath = QFileDialog::getOpenFileName(this, tr("选择交易文件"),tr("."), tr("josn文件(*.json);;所有文件(*.*)"));
    if(strFilePath.trimmed().isEmpty())
        return;

    QString strBtnText = ui->btn_CollectionBroadcast_Ok->text();
    ui->btn_CollectionBroadcast_Ok->setEnabled(false);
    ui->btn_CollectionBroadcast_Ok->setText("广播中...");

    try
    {
        QFile fileTmpImport(strFilePath);
        if(!fileTmpImport.exists())
        {
            strErrMsg = QString("operation error: import file: %1 do not exists, please check it.").arg(fileTmpImport.fileName());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_pCollection->ImportRawTxFile(strCurSelCoinType, strFilePath);

        //通过引用方式获取导入数据
        void const * const pData = __m_pCollection->GetImportData(strCurSelCoinType);
        if(NULL == pData)
        {
            strErrMsg = QString("internal error: get import data failed.");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QString strCoinType = ui->cbx_CollectionBroadcast_CoinType->currentText();
        if( 0 == strCoinType.compare("btc", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
         )
        {
            BTCImportData const * const pBtcImportData =  reinterpret_cast<BTCImportData const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
            if(NULL == pBtcImportData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }


            if(pBtcImportData->vctExportItems.empty())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pBtcImportData->vctExportItems.size());
            for(uint i = 0; i < pBtcImportData->vctExportItems.size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText(pBtcImportData->strCoinType.toUpper());
                if(false == pBtcImportData->vctExportItems[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setText( pBtcImportData->strCoinType );
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }

                pTbw->setItem(i, iCol++, checkBox);
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].strOrderID)); //订单编号

                //源地址
                QString strTmpSrcAddrs;
                for(auto strAddr : pBtcImportData->vctExportItems[i].vctSrcAddr)
                {
                    strTmpSrcAddrs += strAddr.trimmed();
                    strTmpSrcAddrs += "\r\n";
                }
                pTbw->setRowHeight(i, 20*pBtcImportData->vctExportItems[i].vctSrcAddr.size());
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strTmpSrcAddrs ));

                //目的地址
                QString strTmpDstAddrs;
                for(auto strAddr : pBtcImportData->vctExportItems[i].vctDstAddr)
                {
                    strTmpDstAddrs += strAddr.trimmed();
                    strTmpDstAddrs += "\r\n";
                }
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strTmpDstAddrs));

                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].strAmount));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].strTxFee ));

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "---"));
            }
        }
        else if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            BTCImportData const * const pUsdtImportData =  reinterpret_cast<BTCImportData const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
            if(NULL == pUsdtImportData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(pUsdtImportData->vctExportItems.empty())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pUsdtImportData->vctExportItems.size());
            for(uint i = 0; i < pUsdtImportData->vctExportItems.size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText(pUsdtImportData->strCoinType.toUpper() );
                if(false == pUsdtImportData->vctExportItems[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setText( pUsdtImportData->strCoinType );
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }

                pTbw->setItem(i, iCol++, checkBox);
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].strOrderID)); //订单编号

                //源地址
                QString strTmpSrcAddrs;
                for(auto strAddr : pUsdtImportData->vctExportItems[i].vctSrcAddr)
                {
                    strTmpSrcAddrs += strAddr.trimmed();
                    strTmpSrcAddrs += "\r\n";
                }
                pTbw->setRowHeight(i, 20 * (1 + pUsdtImportData->vctExportItems[i].vctSrcAddr.size()));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strTmpSrcAddrs ));

                //目的地址
                QString strTmpDstAddrs;
                for(auto strAddr : pUsdtImportData->vctExportItems[i].vctDstAddr)
                {
                    strTmpDstAddrs += strAddr.trimmed();
                    strTmpDstAddrs += "\r\n";
                }
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strTmpDstAddrs));

                QString strAmountShow = pUsdtImportData->vctExportItems[i].strAmount;
                strAmountShow += "BTC";
                strAmountShow += "\r\n";
                strAmountShow += pUsdtImportData->vctExportItems[i].strTokenAmount;
                strAmountShow += "USDT";
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strAmountShow));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].strTxFee + "BTC"));

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "---"));

            }
        }
        else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
            || 0 == strCoinType.compare("etc", Qt::CaseInsensitive))
        {
            rawtx::ETH_ImportRawTxParam const* const pEthImportData =  reinterpret_cast<rawtx::ETH_ImportRawTxParam const* const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);

            if(NULL == pEthImportData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pEthImportData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pEthImportData->size());

            for(size_t i = 0; i < pEthImportData->size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( strCoinType );

                if(false == (*pEthImportData)[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }

                pTbw->setItem(i, iCol++, checkBox);                      //币种,
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pEthImportData)[i].strOrderId) );//订单编号

                 //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pEthImportData)[i].strAddrFrom) );

                //目的地址
                QString strShowTo = (utils::IsERC20Token(strCurSelCoinType)) ? ( (*pEthImportData)[i].strERC20Recipient ) : ((*pEthImportData)[i].strAddrTo);
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowTo ));

                //金额
                QString strShowValue =  (utils::IsERC20Token(strCurSelCoinType)) ? (  (*pEthImportData)[i].strERC20TokenValue ) : ( (*pEthImportData)[i].strValue);
                strShowValue += strCurSelCoinType;
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowValue ));

                //手续费
                QString strShowFee = (utils::IsERC20Token(strCurSelCoinType)) ? ("0.002ETH") : ("0.000105ETH");
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowFee ) );

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "---"));

            }
        }
        else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
                || utils::Is_HRC20_Token(strCurSelCoinType)
                )
        {
            rawtx::Cosmos_ImportRawTxParam const* const pCosmosImportData =  reinterpret_cast<rawtx::Cosmos_ImportRawTxParam const* const>(pData);
            if(NULL == pCosmosImportData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pCosmosImportData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pCosmosImportData->size());

            for(size_t i = 0; i < pCosmosImportData->size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( (*pCosmosImportData)[i].strCoinType.toUpper() );

                if(false == (*pCosmosImportData)[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);                      //币种,


                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strOrderId)  );  //订单编号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strFrom) );     //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strTo) );       //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strValue) );      //金额

                try
                {
                    std::string cstrFeeAmount = (*pCosmosImportData)[i].strFeeAmount.toStdString();
                    double dFeeAmount = boost::lexical_cast<double>(cstrFeeAmount) / 100000000;
                    pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", dFeeAmount)));  //手续费, 以USDP, 或 HTDF为单位显示
                }
                catch(boost::bad_lexical_cast &e)
                {
                    strErrMsg =  QString("boost::lexical_cast<double>(cstrFeeAmount)  error: %1").arg( e.what());
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "---"));
            }
        }

        else if(0 ==  strCoinType.compare("trx", Qt::CaseInsensitive))
        {
            rawtx::Trx_ImportRawTxParam const* const pTrxImportData =  reinterpret_cast<rawtx::Trx_ImportRawTxParam const* const>(pData);

            if(NULL == pTrxImportData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pTrxImportData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pTrxImportData->size());

            for(size_t i = 0; i < pTrxImportData->size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( strCoinType );

                if(false == (*pTrxImportData)[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }

                pTbw->setItem(i, iCol++, checkBox);                      //币种,
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pTrxImportData)[i].strOrderId) );//订单编号

                 //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pTrxImportData)[i].strSrcAcct) );

                //目的地址
                QString strShowTo = (*pTrxImportData)[i].strDstAcct;
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowTo ));

                //金额
                QString strShowValue =  (*pTrxImportData)[i].strAmount;
                strShowValue += strCurSelCoinType;
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowValue ));

                //手续费
                QString strShowFee = "0.1 TRX";
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowFee ) );

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "---"));

            }

        }
        else
        {
            strErrMsg = QString("operation error: not support %1 in current version").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

    }
    catch(std::exception &e)
    {
        strErrMsg =  QString("导入错误, 错误信息:%1").arg(QString(e.what()));
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg =  QString("导入错误: 未知错误");
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    ui->btn_CollectionBroadcast_Ok->setEnabled(true);
    ui->btn_CollectionBroadcast_Ok->setText(strBtnText);
}

//归集交易广播-确认广播选中
void CMainWindow::page_CollectionBroadcast_OnOK()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("归集交易广播界面确认广播选中按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "归集交易广播界面确认广播选中按钮操作插入操作日志表失败";
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionBroadcast_OnOK";

    if(0 == ui->cbx_CollectionBroadcast_CoinType->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请先选择要广播币种"));
        MESSAGE_BOX_OK;
        return;
    }

    QString strCoinType = ui->cbx_CollectionBroadcast_CoinType->currentText();

    QString strBtnText = ui->btn_CollectionBroadcast_Ok->text();
    ui->btn_CollectionBroadcast_Ok->setEnabled(false);
    ui->btn_CollectionBroadcast_Ok->setText("广播中...");

    vector<QString> vctStrTxid;
    try
    {
        QTableWidget *pTbw = ui->tbw_CollectionBroadcast;
        vector<int> vctRows;
        for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
        {
            QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
            if(nullptr == pTbwItem)
            {
                strErrMsg = "internal error: got checkbox returned nullptr.";
                throw runtime_error(strErrMsg.toStdString());
            }

            if(Qt::Checked == pTbwItem->checkState())
            {
                vctRows.push_back(iRow);
            }
        }

        //获取所有选择范围
        if(vctRows.empty())
        {
            strErrMsg = tr("请先选择已签名交易,然后再进行广播");
            throw runtime_error(strErrMsg.toStdString());
        }

        if(0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("het", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("usdp", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType)
            )
        {
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("共%1笔%2归集交易,预计需要%3秒(%4分钟)\r\n是否继续?")
                               .arg(vctRows.size()).arg(strCoinType).arg(vctRows.size() * 10).arg(QString::asprintf("%.2f", (double)(vctRows.size()) * 10.0 / 60.0)));
            MESSAGE_BOX_OK_CANCEL;
            if(QMessageBox::Cancel == msgbox.exec())
            {
                strErrMsg = QString("已取消广播");
                throw runtime_error(strErrMsg.toStdString());
            }
        }


        __m_pCollection->BroadcastRawTx(strCoinType, vctRows, vctStrTxid);

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("广播完成"));
        MESSAGE_BOX_OK;
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("广播失败, 错误信息: %1").arg(e.what());
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("广播失败: 未知错误"));
        MESSAGE_BOX_OK;
    }

    ui->btn_CollectionBroadcast_Ok->setEnabled(true);
    ui->btn_CollectionBroadcast_Ok->setText(strBtnText);
}

//大地址监控-查询
void CMainWindow::page_BigAccountList_OnSearch()
{
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("暂不支持"));
    MESSAGE_BOX_OK;
}

//大地址监控-重置
void CMainWindow::page_BigAccountList_OnReset()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_BigAccountList_OnReset";
    ui->le_BigAccountList_Addr->clear();
    ui->cbx_BigAccountList_CoinType->setCurrentIndex(0);
}

//大地址监控-删除选中地址
void CMainWindow::page_BigAccountList_OnDeleteAddr()
{
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("暂不支持"));
    MESSAGE_BOX_OK;
}

//大地址监控-添加监控地址
void CMainWindow::page_BigAccountList_OnAddAddr()
{
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("暂不支持"));
    MESSAGE_BOX_OK;
}

//归集日志-查询
void CMainWindow::page_CollectionLog_OnSearch()
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionLog_OnSearch";
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("归集日志界面查询按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "归集日志界面查询按钮操作插入操作日志表失败";
    }

    ui->btn_CollectionLog_Search->setEnabled(false);
    ui->btn_CollectionLog_Search->setText("查询中...");


//    uint uCurrentDatetime = QDateTime::currentDateTime().toTime_t();
    uint uStartDatetime = QDateTime::fromString(ui->date_CollectionLog_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    uint uEndDatetime = QDateTime::fromString(ui->date_CollectionLog_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    if (UINT_TIMESTAMP == uEndDatetime)
        ui->date_CollectionLog_End->setDateTime(QDateTime::currentDateTime());

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_CollectionLog_Search->setEnabled(true);
        ui->btn_CollectionLog_Search->setText("查询");
        return;
    }
//    if ((uStartDatetime > uCurrentDatetime) || (uEndDatetime > uCurrentDatetime))
//    {
//        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "输入的开始时间或结束时间大于当前时间";
//        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("输入的开始时间或者结束时间大于当前时间"));
//        MESSAGE_BOX_OK;
//        ui->btn_CollectionLog_Search->setEnabled(true);
//        ui->btn_CollectionLog_Search->setText("查询");
//        return;
//    }

    //设置查询条件
    vector<Collection::COLLECTIONINFO> vctColInfo;
    Collection::SEARCHCOND searchCond;
    searchCond.strAdminID = "";
    searchCond.strAdminName = ui->le_CollectionLog_AdminName->text().trimmed();
    searchCond.strCoinType = (0 == ui->cbx_CollectionLog_CoinType->currentIndex()) ? ("") : (ui->cbx_CollectionLog_CoinType->currentText().trimmed());
    searchCond.uDatetimeStart = QDateTime::fromString(ui->date_CollectionLog_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    searchCond.uDatetimeEnd = QDateTime::fromString(ui->date_CollectionLog_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();

    if("待归集" == ui->cbx_CollectionLog_CollectionStatus->currentText())
    {
        searchCond.uiStatus = 0;
    }
    else if("已归集" == ui->cbx_CollectionLog_CollectionStatus->currentText())
    {
        searchCond.uiStatus = 1;
    }
    else
    {
        searchCond.uiStatus = SEARCHALL;
    }
    searchCond.strAddr = "";
    searchCond.uiQueryType = 1;

    //调用业务处理接口查询
    try
    {
        __m_pCollection->ConditionSearch_Collection_Log(searchCond, vctColInfo);


        //显示
        list<QStringList> listClcItems;
        QStringList strlstSrcAddr;
        QStringList strlstDstAddr;
        for(vector<Collection::COLLECTIONINFO>::iterator it = vctColInfo.begin(); it != vctColInfo.end(); ++it)
        {
            QString strTmpSrcAddr;
            strlstSrcAddr = it->strSrcAddr.split(',');
            for(auto strAddr : strlstSrcAddr)
            {
                strTmpSrcAddr += strAddr + "\r\n";
//                strTmpSrcAddr += strAddr + "\n";
            }

            QString strTmpDstAddr;
            strlstDstAddr = it->strDstAddr.split(',');
            for(auto strAddr : strlstDstAddr)
            {
                strTmpDstAddr += strAddr + "\r\n";
//                strTmpDstAddr += strAddr + "\n";
            }

            QString strShowAmount;
            QString strShowTxFee;
            if(0 ==  it->strCoinType.compare("USDT", Qt::CaseInsensitive) )
            {
                strShowAmount += it->strAmount + "BTC\r\n";
                strShowAmount += it->strTokenAmount + "USDT\r\n";
//                strShowAmount += it->strAmount + "BTC\n";
//                strShowAmount += it->strTokenAmount + "USDT\n";

                strShowTxFee += it->strTxFee + "BTC";
            }
            else
            {
                strShowAmount += it->strAmount + it->strCoinType ;
                strShowTxFee += it->strTxFee + it->strCoinType;
            }


            QStringList listclcItem;
            listclcItem << it->strCoinType << it->strOrderID << strTmpSrcAddr << strTmpDstAddr \
                        << strShowAmount << strShowTxFee << it->strTxid << it->strTime << it->strAdminName;
            listClcItems.push_back(listclcItem);
        }


        //显示数据
        ui->tbw_CollectionLog->clearContents();
        ui->tbw_CollectionLog->setRowCount(0);

        strErrMsg = QString("查询数据成功, 共%1条记录.").arg(listClcItems.size());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询归集日志表成功";
        QMessageBox msgbox(QMessageBox::Information, tr("查询成功"), strErrMsg);
        MESSAGE_BOX_OK;

        auto pTableWidget = ui->tbw_CollectionLog;
        int nRow = 0;
        for (auto it : listClcItems)
        {
            pTableWidget->insertRow(nRow);
            pTableWidget->setRowHeight(nRow, 20 * (1 + max(strlstDstAddr.size(), strlstSrcAddr.size())));
            for (int j = 0; j < pTableWidget->columnCount(); j++)
            {
                pTableWidget->setItem(nRow, j, new QTableWidgetItem(it.at(j)));
            }
            nRow++;
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("查询错误, 错误信息:%1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("查询错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    ui->btn_CollectionLog_Search->setEnabled(true);
    ui->btn_CollectionLog_Search->setText("查询");
}

//归集日志-重置
void CMainWindow::page_CollectionLog_OnReset()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "page_CollectionLog_OnReset";
    ui->le_CollectionLog_AdminName->clear();
    ui->cbx_CollectionLog_CoinType->setCurrentIndex(0);
    ui->cbx_CollectionLog_CollectionStatus->setCurrentIndex(0);
}

void CMainWindow::page_CollectionLog_OnExportExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("归集日志列表界面导出EXCEL按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "归集日志列表界面导出EXCEL按钮操作插入操作日志表失败";
    }
    if (0 == ui->tbw_CollectionLog->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有归集日志数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中不存在数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_CollectionLog);
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "归集日志数据导出EXCEL文件失败";
        QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("EXCEL文件导出失败!"));
        MESSAGE_BOX_OK;
        return;
    }

    QMessageBox msgbox(QMessageBox::Information, tr("成功"), tr("导出EXCEL成功!"));
    MESSAGE_BOX_OK;
}
