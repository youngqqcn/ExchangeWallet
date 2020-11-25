#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "modules/coinWithdraw/autowithdraw.h"
#include "user_info.h"
#include "utils.h"
#include "boost/algorithm/string.hpp"
#include <QDesktopServices>
#include <QUrl>

#define UINT_POS_ORDERID    0
#define UINT_POS_USERID     2
#define UNIT_POS_AUDITSTATUS 9
#define UNIT_AUTO_AND_MANUAL_AUDITSTATUS 10
#define UINT_POS_MANUAL_COINTYPE  5
#define UINT_POS_MANUAL_AMOUNT    6
#define UINT_POS_MANUAL_STATUS    9

bool  g_bIsTipsNeeded = true;  //是否弹框标志

//人工审核-初审-搜索
void CMainWindow::page_ManulWithdraw1_OnSearch()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("初审列表界面搜索按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初审列表界面搜索按钮操作插入操作日志表失败";
    }
    ui->btn_ManualWithdraw1_Search->setEnabled(false);
    ui->btn_ManualWithdraw1_Search->setText("搜索中...");
    //使用 ChargeList 命名空间中的 结构体
    NS_Withdraw::SEARCHCOND  searchCond;   //搜索条件
    vector<NS_Withdraw::WITHDRAWINFO> vctWithdrawInfo; //搜索结果

    if(0 == ui->cbx_ManualWithdraw1_CoinType->currentIndex())
        searchCond.strCoinType = "";
    else
        searchCond.strCoinType = ui->cbx_ManualWithdraw1_CoinType->currentText().trimmed();
    searchCond.strUserID = ui->le_ManualWithdraw1_UserID->text().trimmed();
    searchCond.strUserTel = ui->le_ManualWithdraw1_UserTel->text().trimmed();
    searchCond.strAddr = ui->le_ManualWithdraw1_DstAddr->text().trimmed();
    searchCond.strTxID = "";

    if("已初审" == ui->cbx_ManualWithdraw1_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FIRST_PASS;
    }
    else if("待初审" == ui->cbx_ManualWithdraw1_Status->currentText().trimmed())
    {
        //searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_PENDING;
    }
    else if("已拒绝" == ui->cbx_ManualWithdraw1_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FAILED;
    }
    else
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = SEARCHALL;
    }

    try
    {
        __m_pWithdraw->ConditionSearch(searchCond, vctWithdrawInfo);

        int  nSemiAutoWithdrawOrderCounter = 0; //半自动的笔数
        //double dSumSemiAutoAmount = 0.0; //半自动 提币总金额
        std::map<QString, double> mapSumSemiAutoAmount;

        vector<QStringList> vctWdItems;
        for(vector<NS_Withdraw::WITHDRAWINFO>::iterator it = vctWithdrawInfo.begin(); it != vctWithdrawInfo.end(); ++it)
        {
            QStringList listWdItem;
            listWdItem<< it->strOrderID << it->strTime << it->strUserID << it->strUserTel << it->strUserName \
                        << it->strCoinType << it->strAmount << it->strTranFee << it->strTxFee << it->strAuditStatus \
                        << it->strRemarks << it->strDstAddr << it->strAuditor;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << listWdItem;
            vctWdItems.push_back(listWdItem);


            if(it->strRemarks.contains("半自动")
                && (0 == it->strAuditStatus.compare("待初审") || 0 == it->strAuditStatus.compare("已初审") ))
            {
                nSemiAutoWithdrawOrderCounter += 1;
                //dSumSemiAutoAmount += it->strAmount.toDouble();
                std::map<QString, double>::iterator itItem = mapSumSemiAutoAmount.find(  it->strCoinType);
                if(mapSumSemiAutoAmount.end() !=  itItem)
                {
                    itItem->second += it->strAmount.toDouble();
                }
                else
                {
                     mapSumSemiAutoAmount.insert( std::make_pair(it->strCoinType, it->strAmount.toDouble())  );
                }
            }

        }

        if(true == g_bIsTipsNeeded)
        {
            strErrMsg = QString("查询数据成功, 共%1条记录.").arg(vctWdItems.size());
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), strErrMsg);
            MESSAGE_BOX_OK;

            QString strTipMsg = "";
            if(  nSemiAutoWithdrawOrderCounter > 0 )
            {
                strTipMsg  = QString("!注意!: 检测到有\"半自动\"提币订单, 请通知管理员向自动出币地址进行充值, 待充值到账后才能进行\"半自动\"提币订单的审核!!!  累计金额为如下: ");
                strTipMsg += "\r\n";
                for(auto item : mapSumSemiAutoAmount)
                {
                    strTipMsg += "\t" + item.first + " 半自动提币累计金额 " + QString::asprintf("%.8f", item.second) + "\r\n";
                }
                QMessageBox msgbox(QMessageBox::Warning, tr("重要提示!!"), strTipMsg);
                MESSAGE_BOX_OK;
            }
        }

        //清除上次查询显示的内容
        ui->tbw_ManualWithdraw1->clearContents();
        ui->tbw_ManualWithdraw1->setRowCount(0);
        if(vctWdItems.size() > 0)
        {
            SetTableWidgetFillData(ui->tbw_ManualWithdraw1, vctWdItems);
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    ui->btn_ManualWithdraw1_Search->setEnabled(true);
    ui->btn_ManualWithdraw1_Search->setText("搜索");
}

//人工审核-初审-重置
void CMainWindow::page_ManulWithdraw1_OnReset()
{
    ui->cbx_ManualWithdraw1_CoinType->setCurrentIndex(0);
    ui->le_ManualWithdraw1_UserID->clear();
    ui->le_ManualWithdraw1_UserTel->clear();
    ui->le_ManualWithdraw1_DstAddr->clear();
    ui->cbx_ManualWithdraw1_Status->setCurrentIndex(0);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("重置查询条件");
}

//人工提币-初审-审核驳回
void CMainWindow::page_ManulWithdraw1_OnReject()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("初审列表界面审核驳回按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初审列表界面审核驳回按钮操作插入操作日志表失败";
    }

    //先判断是否选中有效行
    QTableWidgetItem *pOrderIdItem = ui->tbw_ManualWithdraw1->item(ui->tbw_ManualWithdraw1->currentRow(), UINT_POS_ORDERID);
    if(NULL == pOrderIdItem )
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("选择无效");
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        return;
    }

    //先判断是否为待初审状态
    QTableWidgetItem *pAuditItem =  ui->tbw_ManualWithdraw1->item(ui->tbw_ManualWithdraw1->currentRow(), UNIT_POS_AUDITSTATUS);
    if(0 != pAuditItem->text().trimmed().compare("待初审"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单当前状态为\"%1\",不能进行驳回操作").arg(pAuditItem->text().trimmed());
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单当前状态为\"%1\",不能进行驳回操作").arg(pAuditItem->text().trimmed()));
        MESSAGE_BOX_OK;
        return;
    }

    try
    {
        //除了验证界面上的,还要验证数据库中的状态
        QString strOderID = pOrderIdItem->text();
        if (false == __m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_PENDING))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单状态已不再是\'待初审\',\n请刷新界面数据");
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单状态已不再是\'待初审\',\n请刷新界面数据"));
            MESSAGE_BOX_OK;
            return;
        }

        //弹出对话框，验证谷歌验证码和添加备注
        std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("审核驳回")));
        if(QDialog::Rejected ==  pConfirmDlg->exec())
            return;

         QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
        __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, pConfirmDlg->m_strRemarks, CAutoWithdraw::AUDIT_FAILED);

        g_bIsTipsNeeded = false;
        page_ManulWithdraw1_OnSearch();
        g_bIsTipsNeeded = true;

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("驳回成功");
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误:%1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
    catch(...)
    {
        strErrMsg = QString("错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
}

//人工提币-初审-审核
void CMainWindow::page_ManulWithdraw1_OnAudit()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("初审列表界面审核按钮");
    if (NO_ERROR !=  __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初审列表界面审核按钮操作插入操作日志表失败";
    }

    //先判断是否选中有效行
    QTableWidgetItem *pOrderItem =  ui->tbw_ManualWithdraw1->item(ui->tbw_ManualWithdraw1->currentRow(), UINT_POS_ORDERID);
    if(NULL == pOrderItem)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        return;
    }
    //先判断是否为待初审状态
    QTableWidgetItem *pAuditItem =  ui->tbw_ManualWithdraw1->item(ui->tbw_ManualWithdraw1->currentRow(), UNIT_POS_AUDITSTATUS);
    if(0 != pAuditItem->text().trimmed().compare("待初审"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("状态不是\"待初审\",不能进行审核操作");
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("状态不是\"待初审\",不能进行审核操作"));
        MESSAGE_BOX_OK;
        return;
    }

    QTableWidgetItem *pRemarkItem =  ui->tbw_ManualWithdraw1->item(ui->tbw_ManualWithdraw1->currentRow(), UNIT_POS_AUDITSTATUS+1);
    QString strRemark =  pRemarkItem->text().trimmed();
    if(strRemark.contains("半自动"))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("提示"), tr("此笔提币是\"半自动\"提币, 请查询自动出币地址余额, 确保余额充足, 否则,会导致提币失败!"));
        //MESSAGE_BOX_OK;
        MESSAGE_BOX_OK_CANCEL;
        if(QMessageBox::Cancel ==  msgbox.exec())
            return;
    }

    try
    {
        //除了验证界面上的,还要验证数据库中的状态
        QString strOderID = pOrderItem->text();

        if (false == __m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_PENDING))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单状态已不再是\'待初审\',故不能再次进行审核操作");
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单状态已不再是\'待初审\',\n请刷新界面数据"));
            MESSAGE_BOX_OK;
            return;
        }

        //弹出对话框，验证谷歌验证码和添加备注
        std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("确认审核")));
        if (QDialog::Rejected == pConfirmDlg->exec())
            return;

        //TODO:不要用这种从界面上读的方式,而是用一个变量存储搜索出来的数据, 通过选择的行的索引去变量中获取对应的订单信息 , 李玲, 请记得修改 2019-04-13 yqq留
        QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
        QString strNewRemark = ( strRemark.contains("半自动")  ) ? (strRemark +  pConfirmDlg->m_strRemarks ) : (pConfirmDlg->m_strRemarks);

        __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, strNewRemark, CAutoWithdraw::AUDIT_FIRST_PASS);


        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("审核成功");

        g_bIsTipsNeeded = false;
        page_ManulWithdraw1_OnSearch();
        g_bIsTipsNeeded = true;
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误:%1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;

    }
    catch(...)
    {
        strErrMsg = QString("错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
}

//查看用户资产
void CMainWindow::OnShowAssetPage(QStringList strListOrderInfo)
{
    QString strErrMsg;

    if(4 > strListOrderInfo.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "传递信息错误" << strListOrderInfo;
        return;
    }

    //TODO:不要用这种从界面上读的方式,而是用一个变量存储搜索出来的数据, 通过选择的行的索引去变量中获取对应的订单信息 , 李玲, 请记得修改 2019-04-13 yqq留
   QString strUserID = strListOrderInfo[0];
   QString strCoinType = strListOrderInfo[1];
   QString strAmount = strListOrderInfo[2];
   QString strStatus = strListOrderInfo[3];
   QString strUrl = g_qstr_JAVA_getUserAsset;
   qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strUrl;

   try
   {
       __m_pWithdraw->PullUserAssetDataFromServer(strUrl, strUserID);
   }
   catch(std::exception &e)
   {
       strErrMsg = QString("获取用户资产数据失败:%1").arg(e.what());
       qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
       QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
       MESSAGE_BOX_OK;
       return;
   }
   catch(...)
   {
       strErrMsg = QString("错误: 未知错误");
       qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
       QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
       MESSAGE_BOX_OK;
       return;
   }


   void const * const pData = __m_pWithdraw->GetUserAssetData();
   if(NULL == pData)
   {
       QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("获取用户数据失败"));
       MESSAGE_BOX_OK;
   }

   NS_Withdraw::UserAsset const * const pUserAssetData =  reinterpret_cast<NS_Withdraw::UserAsset const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
   if(NULL == pUserAssetData)
   {
       QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败"));
       MESSAGE_BOX_OK;
   }

   vector<QStringList> vctAssetData;
   for(auto it = pUserAssetData->lstAssetData.begin(); it != pUserAssetData->lstAssetData.end(); it++)
   {
       QStringList listTemp;
       listTemp << it->strCoinName << it->strOrderId << it->strTotalInCoin << it->strTotalOutCoin << it->strTotalAssets<< \
                   it->strAvailVol << it->strFreezeVol << it->strWithdrawFreeze << it->strLockVol  <<  it->strWithdrawAmount << it->strPlatformFee \
                << it->strStatus << "";
       vctAssetData.push_back(listTemp);
   }
   ui->lb_UserAssertData_UserID->setText(pUserAssetData->userBasicInfo.strUserId);
   ui->lb_UserAssertData_UserName->setText(pUserAssetData->userBasicInfo.strUserName);
   ui->lb_UserAssertData_UserTel->setText(pUserAssetData->userBasicInfo.strTelNo.isEmpty()?pUserAssetData->userBasicInfo.strUserEmail:pUserAssetData->userBasicInfo.strTelNo);
   ui->tbw_UserAssetdata->clearContents();
   ui->tbw_UserAssetdata->setRowCount(0);
   SetTableWidgetFillData(ui->tbw_UserAssetdata, vctAssetData);
   ui->stackedWidget->setCurrentIndex(PAGE_INDEX_USERASSET);
   qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("已显示用户%1的资产信息").arg(pUserAssetData->userBasicInfo.strUserId);
}

//人工提币-初审-查看用户资产
void CMainWindow::page_ManulWithdraw1_OnCheckAsset()
{
    ui->btn_ManualWithdraw1_CheckAsset->setEnabled(false);
    ui->btn_UserAssertData_Audit->show();
    ui->btn_UserAssertData_secAudit->hide();
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("初审列表界面查看用户资产按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初审列表界面查看用户资产按钮操作插入操作日志表失败";
    }

    int iCurRow = ui->tbw_ManualWithdraw1->currentRow();
    QTableWidgetItem* pUserIdItem = ui->tbw_ManualWithdraw1->item(iCurRow, UINT_POS_USERID);
    if(NULL == pUserIdItem)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdraw1_CheckAsset->setEnabled(true);
        return;
    }

    QStringList strList;
    strList<< ui->tbw_ManualWithdraw1->item(iCurRow, UINT_POS_USERID)->text().trimmed()\
           << ui->tbw_ManualWithdraw1->item(iCurRow, UINT_POS_MANUAL_COINTYPE)->text().trimmed()\
           << ui->tbw_ManualWithdraw1->item(iCurRow, UINT_POS_MANUAL_AMOUNT)->text().trimmed()\
           << ui->tbw_ManualWithdraw1->item(iCurRow, UINT_POS_MANUAL_STATUS)->text().trimmed();
    emit Send_ShowAssetPage(strList);
    ui->btn_ManualWithdraw1_CheckAsset->setEnabled(true);
}

//人工提币-初审-查看用户流水
void CMainWindow::page_ManulWithdraw1_OnCheckFlow()
{
    QString strErrMsg;

    ui->btn_ManualWithdraw1_CheckFlow->setEnabled(false);
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("初审列表界面查看用户流水按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初审列表界面查看用户流水按钮操作插入操作日志表失败";
    }

    QTableWidgetItem* pSelectItem = ui->tbw_ManualWithdraw1->item(ui->tbw_ManualWithdraw1->currentRow(), UINT_POS_USERID);
    if(0 == pSelectItem)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdraw1_CheckFlow->setEnabled(true);
        return;
    }


    QString strUserID = pSelectItem->text();
    QString strUrl = g_qstr_JAVA_getUserFlow;

    try
    {
        __m_pWithdraw->PullUserFlowFromServer(strUrl, strUserID);
    }
    catch(std::exception &e)
    {
        strErrMsg = QString(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("获取用户流水数据失败: %1").arg(strErrMsg));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdraw1_CheckFlow->setEnabled(true);
        return ;
    }
    catch(...)
    {
        strErrMsg = QString("未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("获取用户流水数据失败: %1").arg(strErrMsg));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdraw1_CheckFlow->setEnabled(true);
        return ;

    }


    void const * const pData = __m_pWithdraw->GetUserFlowData();
    if(NULL == pData)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("获取用户数据失败"));
        MESSAGE_BOX_OK;
    }

    NS_Withdraw::UserFlow const * const pUserFlowData =  reinterpret_cast<NS_Withdraw::UserFlow const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
    if(NULL == pUserFlowData)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败"));
        MESSAGE_BOX_OK;
    }

    vector<QStringList> vctFlowData;

    for(auto it = pUserFlowData->lstFlowData.begin(); it != pUserFlowData->lstFlowData.end(); it++)
    {
        QStringList listTemp;
        listTemp << it->strCoinName << it->strTxType << it->strCount << it->strBalance << it->strTime;
        vctFlowData.push_back(listTemp);
    }
    ui->lb_UserStatements_UserID->setText(pUserFlowData->userBasicInfo.strUserId);
    ui->lb_UserStatements_UserName->setText(pUserFlowData->userBasicInfo.strUserName);
    ui->lb_UserStatements_UserTel->setText(pUserFlowData->userBasicInfo.strTelNo);
    ui->tbw_UserStatements->clearContents();
    ui->tbw_UserStatements->setRowCount(0);
    SetTableWidgetFillData(ui->tbw_UserStatements, vctFlowData);
    ui->stackedWidget->setCurrentIndex(PAGE_INDEX_USERFLOW);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "当前选择的用户ID = " << strUserID;
    ui->btn_ManualWithdraw1_CheckFlow->setEnabled(true);
}

//人工提币-复审-审核驳回
void CMainWindow::page_ManulWithdraw2_OnReject()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("复审列表界面审核驳回按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "复审列表界面审核驳回按钮操作插入操作日志表失败";
    }

    //先判断是否选中有效行
    QTableWidgetItem *pOrderItem = ui->tbw_ManualWithdraw2->item(ui->tbw_ManualWithdraw2->currentRow(), UINT_POS_ORDERID);
    if(NULL == pOrderItem )
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        return;
    }

    //先判断是否为已初审状态
    QTableWidgetItem *pAuditItem =  ui->tbw_ManualWithdraw2->item(ui->tbw_ManualWithdraw2->currentRow(), UNIT_POS_AUDITSTATUS);
    if(0 != pAuditItem->text().trimmed().compare("已初审"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单当前状态为\"%1\",不能进行驳回操作").arg(pAuditItem->text().trimmed());
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单当前状态为\"%1\",不能进行驳回操作").arg(pAuditItem->text().trimmed()));
        MESSAGE_BOX_OK;
        return;
    }


    //除了验证界面上的,还要验证数据库中的状态
    QString strOderID = pOrderItem->text();
    if (false == __m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_FIRST_PASS))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("数据库中该订单当前审核状态已不再为\'已初审\',故不能再次进行审核驳回操作");
        QMessageBox msgbox(QMessageBox::Information, tr("驳回失败"), tr("该订单状态已不再是\'已初审\',\n请刷新界面数据"));
        MESSAGE_BOX_OK;
        return;
    }

    //弹出对话框，验证谷歌验证码和添加备注
    std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("审核驳回")));
    if(QDialog::Rejected ==  pConfirmDlg->exec())
        return;


    try
    {
        QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
        __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, pConfirmDlg->m_strRemarks, CAutoWithdraw::AUDIT_FAILED);


        g_bIsTipsNeeded = false;
        page_ManulWithdraw2_OnSearch();
        g_bIsTipsNeeded = true;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("复审驳回成功");
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "未知错误";
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("未知错误"));
        MESSAGE_BOX_OK;
    }

}

//人工提币-复审-审核通过
void CMainWindow::page_ManulWithdraw2_OnAudit()
{
    QString strErrMsg;
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("复审列表界面审核通过按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "复审列表界面审核通过按钮操作插入操作日志表失败";
    }


    //先判断是否选中有效行
    QTableWidgetItem *pOrderIdItem = ui->tbw_ManualWithdraw2->item(ui->tbw_ManualWithdraw2->currentRow(), UINT_POS_ORDERID);
    if(NULL == pOrderIdItem)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        return;
    }

    //先判断是否为已初审状态
    QTableWidgetItem *pAuditItem =  ui->tbw_ManualWithdraw2->item(ui->tbw_ManualWithdraw2->currentRow(), UNIT_POS_AUDITSTATUS);
    if(0 != pAuditItem->text().trimmed().compare("已初审"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单当前状态为\"%1\",不能进行\"审核\"操作").arg(pAuditItem->text().trimmed());
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("该订单当前状态为\"%1\",不能进行\"审核\"操作").arg(pAuditItem->text().trimmed()));
        MESSAGE_BOX_OK;
        return;
    }

    QTableWidgetItem *pRemarkItem =  ui->tbw_ManualWithdraw2->item(ui->tbw_ManualWithdraw2->currentRow(), UNIT_POS_AUDITSTATUS+1);
    QString strRemark =  pRemarkItem->text().trimmed();
    if(strRemark.contains("半自动"))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("提示"), tr("此笔提币是\"半自动\"提币, 请查询自动出币地址余额, 确保余额充足, 否则,会导致提币失败!"));
        //MESSAGE_BOX_OK;
        MESSAGE_BOX_OK_CANCEL;
        if(QMessageBox::Cancel ==  msgbox.exec())
            return;
    }

    try
    {
        //除了验证界面上的,还要验证数据库中的状态
        QString strOderID = pOrderIdItem->text();
        if (false == __m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_FIRST_PASS))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("数据库中该订单当前审核状态已不再为\'已初审\',故不能再次进行审核驳回操作");
            QMessageBox msgbox(QMessageBox::Information, tr("驳回失败"), tr("该订单状态已不再是\'已初审\',\n请刷新界面数据"));
            MESSAGE_BOX_OK;
            return;
        }

        //弹出对话框，验证谷歌验证码和添加备注
        std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("确认审核")));
        if( QDialog::Rejected == pConfirmDlg->exec())
            return;

        QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
        QString strNewRemark = ( strRemark.contains("半自动")  ) ? (strRemark +  pConfirmDlg->m_strRemarks ) : (pConfirmDlg->m_strRemarks);

        __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, strNewRemark, CAutoWithdraw::AUDIT_SUCCESS);


        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("复审审核成功");
        g_bIsTipsNeeded = false;
        page_ManulWithdraw2_OnSearch();
        g_bIsTipsNeeded = true;
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "未知错误";
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("未知错误"));
        MESSAGE_BOX_OK;
    }
}

//人工提币-复审-查看用户资产
void CMainWindow::page_ManulWithdraw2_OnCheckAsset()
{
    ui->btn_ManualWithdraw2_CheckAsset->setEnabled(false);
    ui->btn_UserAssertData_Audit->hide();
    ui->btn_UserAssertData_secAudit->show();
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("复审列表界面查看用户资产按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "复审列表界面查看用户资产按钮操作插入操作日志表失败";
    }

    int iCurRow = ui->tbw_ManualWithdraw2->currentRow();
    QTableWidgetItem* pSelectItem = ui->tbw_ManualWithdraw2->item(iCurRow, UINT_POS_USERID);
    if(NULL == pSelectItem)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdraw2_CheckAsset->setEnabled(true);
        return;
    }


    QStringList strList;
    strList<< ui->tbw_ManualWithdraw2->item(iCurRow, UINT_POS_USERID)->text().trimmed()\
           << ui->tbw_ManualWithdraw2->item(iCurRow, UINT_POS_MANUAL_COINTYPE)->text().trimmed()\
           << ui->tbw_ManualWithdraw2->item(iCurRow, UINT_POS_MANUAL_AMOUNT)->text().trimmed()\
           << ui->tbw_ManualWithdraw2->item(iCurRow, UINT_POS_MANUAL_STATUS)->text().trimmed();

    ui->btn_ManualWithdraw2_CheckAsset->setEnabled(true);
    emit Send_ShowAssetPage(strList);
}

//人工提币-复审-查看用户流水
void CMainWindow::page_ManulWithdraw2_OnCheckFlow()
{
    QString strErrMsg;

    ui->btn_ManualWithdraw2_CheckFlow->setEnabled(false);

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("复审列表界面查看用户流水按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "复审列表界面查看用户流水按钮操作插入操作日志表失败";
    }


    QTableWidgetItem* pSelectItem = ui->tbw_ManualWithdraw2->item(ui->tbw_ManualWithdraw2->currentRow(), UINT_POS_USERID);
    if(0 == pSelectItem)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdraw2_CheckFlow->setEnabled(true);
        return;
    }
    QString strUserID = pSelectItem->text();
    QString strUrl = g_qstr_JAVA_getUserFlow;

    try
    {
        __m_pWithdraw->PullUserFlowFromServer(strUrl, strUserID);

        void const * const pData = __m_pWithdraw->GetUserFlowData();
        if(NULL == pData)
        {
            strErrMsg = QString("internal error: get user flow data returns nullptr.");
            throw runtime_error(strErrMsg.toStdString());
        }

        NS_Withdraw::UserFlow const * const pUserFlowData =  reinterpret_cast<NS_Withdraw::UserFlow const * const>(pData);
        if(NULL == pUserFlowData)
        {
            strErrMsg = QString("internal error: reinterpret_cast failed.");
            throw runtime_error(strErrMsg.toStdString());
        }


        vector<QStringList> vctFlowData;

        for(auto it = pUserFlowData->lstFlowData.begin(); it != pUserFlowData->lstFlowData.end(); it++)
        {
            QStringList listTemp;
            listTemp << it->strCoinName << it->strTxType << it->strCount << it->strBalance << it->strTime;
            vctFlowData.push_back(listTemp);
        }
        ui->lb_UserStatements_UserID->setText(pUserFlowData->userBasicInfo.strUserId);
        ui->lb_UserStatements_UserName->setText(pUserFlowData->userBasicInfo.strUserName);
        ui->lb_UserStatements_UserTel->setText(pUserFlowData->userBasicInfo.strTelNo);
        ui->cbx_UserStatements_CoinType->setCurrentIndex(0);
        ui->cbx_UserStatements_StatementType->setCurrentIndex(0);
        ui->tbw_UserStatements->clearContents();
        ui->tbw_UserStatements->setRowCount(0);
        SetTableWidgetFillData(ui->tbw_UserStatements, vctFlowData);
        ui->stackedWidget->setCurrentIndex(PAGE_INDEX_USERFLOW);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "当前选择的用户ID = " << strUserID;
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "未知错误";
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("未知错误"));
        MESSAGE_BOX_OK;
    }


    ui->btn_ManualWithdraw2_CheckFlow->setEnabled(true);
}

//人工审核-复审-搜索
void CMainWindow::page_ManulWithdraw2_OnSearch()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("复审列表界面搜索按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "复审列表界面搜索按钮操作插入操作日志表失败";
    }
    ui->btn_ManualWithdraw2_Search->setEnabled(false);
    ui->btn_ManualWithdraw2_Search->setText("搜索中...");

    //使用 ChargeList 命名空间中的 结构体
    NS_Withdraw::SEARCHCOND  searchCond;   //搜索条件
    vector<NS_Withdraw::WITHDRAWINFO> vctWithdrawInfo; //搜索结果

    if(0 == ui->cbx_ManualWithdraw2_CoinType->currentIndex())
        searchCond.strCoinType = "";
    else
        searchCond.strCoinType = ui->cbx_ManualWithdraw2_CoinType->currentText().trimmed();

    searchCond.strUserID = ui->le_ManualWithdraw2_UserID->text().trimmed();
    searchCond.strAddr = ui->le_ManualWithdraw2_DstAddr->text().trimmed();
    searchCond.strUserTel = ui->le_ManualWithdraw2_UserTel->text().trimmed();
    searchCond.strTxID = "";

    if("已初审" == ui->cbx_ManualWithdraw2_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_2;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FIRST_PASS;
    }
    else if("已复审" == ui->cbx_ManualWithdraw2_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_SUCCESS;
    }
    else if("已拒绝" == ui->cbx_ManualWithdraw2_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FAILED;
    }
    else
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_2;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = SEARCHALL;
    }

    try
    {
        __m_pWithdraw->ConditionSearch(searchCond, vctWithdrawInfo);

        int  nSemiAutoWithdrawOrderCounter = 0; //半自动的笔数
        //double dSumSemiAutoAmount = 0.0; //半自动 提币总金额
        std::map<QString, double> mapSumSemiAutoAmount;

        vector<QStringList> vctWdItems;
        for(vector<NS_Withdraw::WITHDRAWINFO>::iterator it = vctWithdrawInfo.begin(); it != vctWithdrawInfo.end(); ++it)
        {
            QStringList listWdItem;

            listWdItem<< it->strOrderID << it->strTime << it->strUserID << it->strUserTel << it->strUserName \
                        << it->strCoinType << it->strAmount << it->strTranFee << it->strTxFee << it->strAuditStatus \
                         << it->strRemarks << it->strDstAddr << it->strAuditor;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << listWdItem;
            vctWdItems.push_back(listWdItem);

             if(it->strRemarks.contains("半自动")
                && (0 == it->strAuditStatus.compare("待初审") || 0 == it->strAuditStatus.compare("已初审") ))
            {
                 nSemiAutoWithdrawOrderCounter += 1;
                //dSumSemiAutoAmount += it->strAmount.toDouble();
                std::map<QString, double>::iterator itItem = mapSumSemiAutoAmount.find(  it->strCoinType);
                if(mapSumSemiAutoAmount.end() !=  itItem)
                {
                    itItem->second += it->strAmount.toDouble();
                }
                else
                {
                     mapSumSemiAutoAmount.insert( std::make_pair(it->strCoinType, it->strAmount.toDouble())  );
                }
            }
        }

        if(true == g_bIsTipsNeeded)
        {
            strErrMsg = QString("查询数据成功, 共%1条记录.").arg(vctWdItems.size());
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), strErrMsg);
            MESSAGE_BOX_OK;


            QString strTipMsg = "";
            if(  nSemiAutoWithdrawOrderCounter > 0 )
            {
                strTipMsg  = QString("!注意!: 检测到有\"半自动\"提币订单, 请通知管理员向自动出币地址进行充值, 待充值到账后才能进行\"半自动\"提币订单的审核!!!  累计金额为如下: ");
                strTipMsg += "\r\n";
                for(auto item : mapSumSemiAutoAmount)
                {
                    strTipMsg += "\t" + item.first + " 半自动提币累计金额: " + QString::asprintf("%.8f", item.second)+ "\r\n";
                }
                QMessageBox msgbox(QMessageBox::Warning, tr("重要提示!!"), strTipMsg);
                MESSAGE_BOX_OK;
            }
        }

        //清除上次查询显示的内容
        ui->tbw_ManualWithdraw2->clearContents();
        ui->tbw_ManualWithdraw2->setRowCount(0);

        if(vctWdItems.size() > 0)
        {
            SetTableWidgetFillData(ui->tbw_ManualWithdraw2, vctWdItems);
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }


    ui->btn_ManualWithdraw2_Search->setEnabled(true);
    ui->btn_ManualWithdraw2_Search->setText("搜索");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询-复审表成功";
}

//人工审核-复审-重置
void CMainWindow::page_ManulWithdraw2_OnReset()
{
    ui->cbx_ManualWithdraw2_CoinType->setCurrentIndex(0);
    ui->le_ManualWithdraw2_UserID->clear();
    ui->le_ManualWithdraw2_UserTel->clear();
    ui->le_ManualWithdraw2_DstAddr->clear();
    ui->cbx_ManualWithdraw2_Status->setCurrentIndex(0);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("重置查询条件");
}

//自动提币-停止提现
void CMainWindow::page_AutoWithdraw_OnReject()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("自动提币列表界面停止提现按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "自动提币列表界面停止提现按钮操作插入操作日志表失败";
    }


    //先判断是否选中有效行
    QTableWidgetItem *pOrderItem = ui->tbw_AutoWithdraw->item(ui->tbw_AutoWithdraw->currentRow(), UINT_POS_ORDERID);
    if(NULL == pOrderItem )
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        return;
    }

    //先判断是否为待提现状态
    QTableWidgetItem *pAuditItem =  ui->tbw_AutoWithdraw->item(ui->tbw_AutoWithdraw->currentRow(), UNIT_AUTO_AND_MANUAL_AUDITSTATUS);
    if(!(0 == pAuditItem->text().trimmed().compare("待提现")))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单当前状态为\"%1\",不能进行停止提现操作").arg(pAuditItem->text().trimmed());
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单当前状态为\"%1\",不能进行停止提现操作").arg(pAuditItem->text().trimmed()));
        MESSAGE_BOX_OK;
        return;
    }

    try
    {
        //弹出对话框，验证谷歌验证码和添加备注
        std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("审核驳回")));
        pConfirmDlg->exec();


        QString strOderID = pOrderItem->text();
        QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
        __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, pConfirmDlg->m_strRemarks, CAutoWithdraw::AUDIT_FAILED);


        g_bIsTipsNeeded = false;
        page_AutoWithdraw_OnSearch();
        g_bIsTipsNeeded = true;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("停止提现成功");
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("驳回失败: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("驳回失败: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }


}

//自动提币-搜索
void CMainWindow::page_AutoWithdraw_OnSearch()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("自动提币列表界面搜索按钮");
    if (NO_ERROR !=  __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "自动提币列表界面搜索按钮操作插入操作日志表失败";
    }
    ui->btn_AutoWithdraw_Search->setEnabled(false);
    ui->btn_AutoWithdraw_Search->setText("搜索中...");


    //使用 ChargeList 命名空间中的 结构体
    NS_Withdraw::SEARCHCOND  searchCond;   //搜索条件
    vector<NS_Withdraw::WITHDRAWINFO> vctWithdrawInfo; //搜索结果

    if(0 == ui->cbx_AutoWithdraw_CoinType->currentIndex())
        searchCond.strCoinType = "";
    else
        searchCond.strCoinType = ui->cbx_AutoWithdraw_CoinType->currentText().trimmed();

    searchCond.strUserID = ui->le_AutoWithdraw_UserID->text().trimmed();
    searchCond.strUserTel = ui->le_AutoWithdraw_UserTel->text().trimmed();
    searchCond.strAddr = ui->le_AutoWithdraw_DstAddr->text().trimmed();
    searchCond.strTxID = ui->le_AutoWithdraw_Txid->text().trimmed();
    searchCond.uiWithdrawStatus = ui->cbx_AutoWithdraw_Status->currentIndex();
    searchCond.uiAuditStatus = 0;
    searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询‘%1’交易").arg(ui->cbx_ManualWithdrawList_Status->currentText());


    if("待提现" == ui->cbx_AutoWithdraw_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = SEARCHALL;
    }
    else if("已提现" == ui->cbx_AutoWithdraw_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_SUCCESS;
        searchCond.uiAuditStatus = SEARCHALL;
    }
    else if("已拒绝" == ui->cbx_AutoWithdraw_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus =  CAutoWithdraw::AUDIT_FAILED;
    }
    else
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = SEARCHALL;
    }

    try
    {
        __m_pWithdraw->ConditionSearch_AutoWithDraw(searchCond, vctWithdrawInfo);

        vector<QStringList> vctWdItems;
        for(vector<NS_Withdraw::WITHDRAWINFO>::iterator it = vctWithdrawInfo.begin(); it != vctWithdrawInfo.end(); ++it)
        {
            QStringList listWdItem;
//            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << it->strOrderID << it->strAuditStatus << it->strOrderStatus;

            listWdItem << it->strOrderID << it->strTime << it->strCompleteTime << it->strUserID << it->strUserTel\
                       <<it->strUserName << it->strCoinType << it->strAmount<< it->strTranFee <<it->strTxFee \
                      << it->strAuditStatus << it->strRemarks << it->strTxID << it->strDstAddr;
            vctWdItems.push_back(listWdItem);
        }


        strErrMsg = QString("查询数据成功, 共%1条记录.").arg(vctWdItems.size());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;

        //清除上次查询显示的内容
        ui->tbw_AutoWithdraw->clearContents();
        ui->tbw_AutoWithdraw->setRowCount(0);

        if(vctWdItems.size() > 0)
        {
            SetTableWidgetFillData(ui->tbw_AutoWithdraw, vctWdItems);
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("查询错误: %1").arg(e.what());
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



    ui->btn_AutoWithdraw_Search->setEnabled(true);
    ui->btn_AutoWithdraw_Search->setText("搜索");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询-自动提币表成功";
}

//自动提币-重置
void CMainWindow::page_AutoWithdraw_OnReset()
{
    ui->cbx_AutoWithdraw_CoinType->setCurrentIndex(0);
    ui->le_AutoWithdraw_UserID->clear();
    ui->le_AutoWithdraw_UserTel->clear();
    ui->le_AutoWithdraw_Txid->clear();
    ui->le_AutoWithdraw_DstAddr->clear();
    ui->cbx_AutoWithdraw_Status->setCurrentIndex(0);
}

void CMainWindow::page_AutoWithdraw_OnExportExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("自动提币列表界面导出EXCEL按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "自动提币列表界面导出EXCEL按钮操作插入操作日志表失败";
    }

    NS_Withdraw::SEARCHCOND  searchCond;   //搜索条件
    vector<NS_Withdraw::WITHDRAWINFO> vctWithdrawInfo; //搜索结果

    uint uStartDatetime = QDateTime::fromString(ui->date_AutoWithdraw_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    uint uEndDatetime = QDateTime::fromString(ui->date_AutoWithdraw_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    if (UINT_TIMESTAMP == uEndDatetime)
        ui->date_AutoWithdraw_End->setDateTime(QDateTime::currentDateTime());

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_AutoWithdraw_Search->setEnabled(true);
        ui->btn_AutoWithdraw_Search->setText("搜索");
        return;
    }

    searchCond.uDatetimeStart = QDateTime::fromString(ui->date_AutoWithdraw_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    searchCond.uDatetimeEnd = QDateTime::fromString(ui->date_AutoWithdraw_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    if(0 == ui->cbx_AutoWithdraw_CoinType->currentIndex())
        searchCond.strCoinType = "";
    else
        searchCond.strCoinType = ui->cbx_AutoWithdraw_CoinType->currentText().trimmed();

    searchCond.strUserID = ui->le_AutoWithdraw_UserID->text().trimmed();
    searchCond.strUserTel = ui->le_AutoWithdraw_UserTel->text().trimmed();
    searchCond.strAddr = ui->le_AutoWithdraw_DstAddr->text().trimmed();
    searchCond.strTxID = ui->le_AutoWithdraw_Txid->text().trimmed();
    searchCond.uiWithdrawStatus = ui->cbx_AutoWithdraw_Status->currentIndex();
    searchCond.uiAuditStatus = 0;
    searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;

    qDebug() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << QString("查询‘%1’交易").arg(ui->cbx_ManualWithdrawList_Status->currentText());


    if("待提现" == ui->cbx_AutoWithdraw_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = SEARCHALL;
    }
    else if("已提现" == ui->cbx_AutoWithdraw_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_SUCCESS;
        searchCond.uiAuditStatus = SEARCHALL;
    }
    else if("已拒绝" == ui->cbx_AutoWithdraw_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus =  CAutoWithdraw::AUDIT_FAILED;
    }
    else
    {
        searchCond.uiQueryType = CPageManualWithdraw::AUTOWITHDRAW;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = SEARCHALL;
    }

    __m_pWithdraw->ConditionSearch_AutoWithDraw(searchCond, vctWithdrawInfo);
    if (vctWithdrawInfo.size() <= 0)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("查询到的数据为空!"));
        MESSAGE_BOX_OK;
        return;
    }
    vector<QStringList> vctWdItems;
    for(vector<NS_Withdraw::WITHDRAWINFO>::iterator it = vctWithdrawInfo.begin(); it != vctWithdrawInfo.end(); ++it)
    {
        QStringList listWdItem;
//            qDebug() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << it->strOrderID << it->strAuditStatus << it->strOrderStatus;

        listWdItem << it->strOrderID << it->strTime << it->strCompleteTime << it->strUserID << it->strUserTel\
                   <<it->strUserName << it->strCoinType << it->strAmount<< it->strTranFee <<it->strTxFee \
                  << it->strAuditStatus << it->strRemarks << it->strTxID << it->strDstAddr;
        vctWdItems.push_back(listWdItem);
    }

    //清除上次查询显示的内容
    ui->tbw_AutoWithdraw->clearContents();
    ui->tbw_AutoWithdraw->setRowCount(0);

    if(vctWdItems.size() > 0)
    {
        SetTableWidgetFillData(ui->tbw_AutoWithdraw, vctWdItems);
    }

    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("数据查询成功,\n点击确定进行导出操作"));
    MESSAGE_BOX_OK_CANCEL;
    if (QMessageBox::Ok != msgbox.exec())
    {
        return;
    }

    if (0 == ui->tbw_AutoWithdraw->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有自动提币数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中不存在数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_AutoWithdraw);
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "自动提币数据导出EXCEL文件失败";
        QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("EXCEL文件导出失败!"));
        MESSAGE_BOX_OK;
        return;
    }

    {
        QMessageBox msgbox(QMessageBox::Information, tr("成功"), tr("导出EXCEL成功!"));
        MESSAGE_BOX_OK;
    }
}

//用户资产-确定
void CMainWindow::page_UserAssetData_OnOK()
{
    //QMessageBox msgbox(QMessageBox::Information, tr("提示"), "暂不支持");
    //MESSAGE_BOX_OK;
#if 1
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户资产界面确认按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户资产界面确认按钮操作插入操作日志表失败";
    }

    QString strCoinType = (0 == ui->cbx_UserAssertData_CoinType->currentIndex()) ? QString("") : ui->cbx_UserAssertData_CoinType->currentText();
    QString strTradeStatus = (0 == ui->cbx_UserAssertData_Status->currentIndex()) ? QString("") : ui->cbx_UserAssertData_Status->currentText();


    void const * const pData = __m_pWithdraw->GetUserAssetData();
    if(NULL == pData)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("获取用户数据失败"));
        MESSAGE_BOX_OK;
        return;
    }

    NS_Withdraw::UserAsset const * const pUserAssetData =  reinterpret_cast<NS_Withdraw::UserAsset const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
    if(NULL == pUserAssetData)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败"));
        MESSAGE_BOX_OK;
        return;
    }

    vector<QStringList> vctAssetData;
    for(auto it = pUserAssetData->lstAssetData.rbegin(); it != pUserAssetData->lstAssetData.rend(); it++)
    {
        if(("" == strCoinType || strCoinType == it->strCoinName)
            && ("" == strTradeStatus || strTradeStatus == it->strStatus || "---" == it->strStatus/*把某个币的资产情况也展示出来*/ ))
        {
            QStringList listTemp;
            listTemp << it->strCoinName<< it->strOrderId << it->strTotalInCoin << it->strTotalOutCoin << it->strTotalAssets<< \
                        it->strAvailVol << it->strFreezeVol << it->strWithdrawFreeze << it->strLockVol  <<  it->strWithdrawAmount << it->strPlatformFee \
                     << it->strStatus << "";
            vctAssetData.push_back(listTemp);
        }

    }



    if(  vctAssetData.empty() )
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("未查询到%1 \"%2\"的数据").arg(strCoinType).arg(strTradeStatus));
        MESSAGE_BOX_OK;
        return;
    }



    ui->lb_UserAssertData_UserID->setText(pUserAssetData->userBasicInfo.strUserId);
    ui->lb_UserAssertData_UserName->setText(pUserAssetData->userBasicInfo.strUserName);
    ui->lb_UserAssertData_UserTel->setText((pUserAssetData->userBasicInfo.strTelNo.isEmpty())? (pUserAssetData->userBasicInfo.strUserEmail) : (pUserAssetData->userBasicInfo.strTelNo));
    ui->tbw_UserAssetdata->clearContents();
    ui->tbw_UserAssetdata->setRowCount(0);
    SetTableWidgetFillData(ui->tbw_UserAssetdata, vctAssetData);
    ui->stackedWidget->setCurrentIndex(PAGE_INDEX_USERASSET);
#endif
}

//用户资产-重置
void CMainWindow::page_UserAssetData_OnReset()
{
    ui->cbx_UserAssertData_Status->setCurrentIndex(0);
    ui->cbx_UserAssertData_CoinType->setCurrentIndex(0);
}
//用户资产初审
void CMainWindow::page_UserAssetData_OnAudit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户资产界面审核按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户资产界面审核按钮操作插入操作日志表失败";

    ui->btn_UserAssertData_Audit->setEnabled(false);
    int iSelectedRow = ui->tbw_UserAssetdata->currentRow();
    if (0 == ui->tbw_UserAssetdata->item(iSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("请选择有数据的行!"));
        MESSAGE_BOX_OK;
        ui->btn_UserAssertData_Audit->setEnabled(true);
        return;
    }
    if (tr("待初审") != ui->tbw_UserAssetdata->item(iSelectedRow, 11)->text())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("请选择状态为\'待初审\'的数据!"));
        MESSAGE_BOX_OK;
        ui->btn_UserAssertData_Audit->setEnabled(true);
        return;
    }

    //除了验证界面上的,还要验证数据库中的状态
    QString strOderID = ui->tbw_UserAssetdata->item(iSelectedRow, 1)->text();

    if (false == __m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_PENDING))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单状态已不再是\'待初审\',故不能再次进行审核操作");
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单状态已不再是\'待初审\',\n请刷新界面数据"));
        MESSAGE_BOX_OK;
        ui->btn_UserAssertData_Audit->setEnabled(true);
        return;
    }

    //弹出对话框，验证谷歌验证码和添加备注
    std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("确认审核")));
    if (QDialog::Rejected == pConfirmDlg->exec())
    {
        ui->btn_UserAssertData_Audit->setEnabled(true);
        return;
    }

    QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
    __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, pConfirmDlg->m_strRemarks, CAutoWithdraw::AUDIT_FIRST_PASS);

    g_bIsTipsNeeded = false;
    page_ManulWithdraw1_OnSearch();
    g_bIsTipsNeeded = true;

    QStringList strList;
    strList<< ui->lb_UserAssertData_UserID->text() << "111" << "222" << "333";
    emit Send_ShowAssetPage(strList);

    ui->btn_UserAssertData_Audit->setEnabled(true);
}
//用户资产复审
void CMainWindow::page_UserAssetData_OnSecAudit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户资产界面审核按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户资产界面审核按钮操作插入操作日志表失败";

    ui->btn_UserAssertData_secAudit->setEnabled(false);
    int iSelectedRow = ui->tbw_UserAssetdata->currentRow();
    if (0 == ui->tbw_UserAssetdata->item(iSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("请选择有数据的行!"));
        MESSAGE_BOX_OK;
        ui->btn_UserAssertData_secAudit->setEnabled(true);
        return;
    }
    if (tr("已初审") != ui->tbw_UserAssetdata->item(iSelectedRow, 11)->text())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("请选择状态为\'已初审\'的数据!"));
        MESSAGE_BOX_OK;
        ui->btn_UserAssertData_secAudit->setEnabled(true);
        return;
    }

    //除了验证界面上的,还要验证数据库中的状态
    QString strOderID = ui->tbw_UserAssetdata->item(iSelectedRow, 1)->text();

    if (false == __m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_FIRST_PASS))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单状态已不再是\'待初审\',故不能再次进行审核操作");
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单状态已不再是\'已初审\',\n请刷新界面数据"));
        MESSAGE_BOX_OK;
        ui->btn_UserAssertData_secAudit->setEnabled(true);
        return;
    }

    //弹出对话框，验证谷歌验证码和添加备注
    std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("确认审核")));
    if( QDialog::Rejected == pConfirmDlg->exec())
    {
        ui->btn_UserAssertData_secAudit->setEnabled(true);
        return;
    }

    QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
    __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, pConfirmDlg->m_strRemarks, CAutoWithdraw::AUDIT_SUCCESS);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("用户资产复审成功");
    g_bIsTipsNeeded = false;
    page_ManulWithdraw2_OnSearch();
    g_bIsTipsNeeded = true;

    QStringList strList;
    strList<< ui->lb_UserAssertData_UserID->text() << "111" << "222" << "333";
    emit Send_ShowAssetPage(strList);

    QMessageBox msgbox(QMessageBox::Information, tr("审核成功"), tr("复审审核成功!"));
    MESSAGE_BOX_OK;
    ui->btn_UserAssertData_secAudit->setEnabled(true);
}

//用户流水-查询
void CMainWindow::page_UserFlowData_OnSearch()
{
    //QMessageBox msgbox(QMessageBox::Information, tr("提示"), "暂不支持搜索");
    //MESSAGE_BOX_OK;


    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户流水界面查询按钮");
    if (NO_ERROR !=  __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户流水界面查询按钮操作插入操作日志表失败";
    }

    QString strCoinType = (0 == ui->cbx_UserStatements_CoinType->currentIndex())?QString("") : ui->cbx_UserStatements_CoinType->currentText();
    QString strTradeType = (0 == ui->cbx_UserStatements_StatementType->currentIndex())?QString("") : ui->cbx_UserStatements_StatementType->currentText();

    void const * const pData = __m_pWithdraw->GetUserFlowData();
    if(NULL == pData)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("获取用户数据失败"));
        MESSAGE_BOX_OK;
        return;
    }

    NS_Withdraw::UserFlow const * const pUserFlowData =  reinterpret_cast<NS_Withdraw::UserFlow const * const>(pData);
    if(NULL == pUserFlowData)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败"));
        MESSAGE_BOX_OK;
        return;
    }

    vector<QStringList> vctFlowData;

    //不筛选, 默认全部显示
    for(auto it = pUserFlowData->lstFlowData.rbegin(); it != pUserFlowData->lstFlowData.rend(); it++)
    {
        if( ("" == strCoinType || strCoinType == it->strCoinName)
            && ("" == strTradeType || strTradeType == it->strTxType))
        {
            QStringList listTemp;
            listTemp << it->strCoinName << it->strTxType << it->strCount << it->strBalance << it->strTime;
            vctFlowData.push_back(listTemp);
        }
    }

    if(  vctFlowData.empty() )
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("未查询到%1 \"%2\"的数据").arg(strCoinType).arg(strTradeType));
        MESSAGE_BOX_OK;
        return;
    }


    ui->lb_UserStatements_UserID->setText(pUserFlowData->userBasicInfo.strUserId);
    ui->lb_UserStatements_UserName->setText(pUserFlowData->userBasicInfo.strUserName);
    ui->lb_UserStatements_UserTel->setText(pUserFlowData->userBasicInfo.strTelNo);
    ui->tbw_UserStatements->clearContents();
    ui->tbw_UserStatements->setRowCount(0);
    SetTableWidgetFillData(ui->tbw_UserStatements, vctFlowData);
    ui->stackedWidget->setCurrentIndex(PAGE_INDEX_USERFLOW);


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "当前选择的用户ID = " << pUserFlowData->userBasicInfo.strUserId;

    ui->btn_ManualWithdraw1_CheckFlow->setEnabled(true);

}

//用户流水-初审
void CMainWindow::page_UserFlowData_OnJumptoManualWithdraw1()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户流水界面初审按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户流水界面初审按钮操作插入操作日志表失败";
    }
    ui->stackedWidget->setCurrentIndex(PAGE_INDEX_MANUALWITHDRAW1);
}

//用户流水-复审
void CMainWindow::page_UserFlowData_OnJumptoManualWithdraw2()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户流水界面复审按钮");
    if (NO_ERROR !=  __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户流水界面复审按钮操作插入操作日志表失败";
    }
    ui->stackedWidget->setCurrentIndex(PAGE_INDEX_MANUALWITHDRAW2);
}

//用户流水-查看资产数据
void CMainWindow::page_UserFlowData_OnJumptoAssetData()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("用户流水界面查看数据按钮");
    if (NO_ERROR !=  __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "用户流水界面查看数据按钮操作插入操作日志表失败";
    }

    QStringList strList;
    strList<< ui->lb_UserStatements_UserID->text().trimmed()<< ""<< ""<<"";
    emit Send_ShowAssetPage(strList);
}

//创建交易页面-全选
void CMainWindow::page_ManualWithdrawCreateRawTx_OnSelAll()
{

    //如果当前是选中  则取消全选
    Qt::CheckState checkState = ( ui->checkBox_ManualWithdrawCreateRawTx_SelAll->isChecked() ) ? (Qt::Checked) : (Qt::Unchecked) ;


    QTableWidget *pTbw = ui->tbw_ManualWithdrawCreateRawTx;
    for(int iRow = 0; iRow < pTbw->rowCount(); iRow++ )
    {
        QTableWidgetItem  *pTbwItem =  pTbw->item(iRow, 0);
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

//创建交易页面-搜索
void CMainWindow::page_ManualWithdrawCreateRawTx_OnSearch()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("创建未签名提币交易界面搜索按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建未签名提币交易界面搜索按钮操作插入操作日志表失败";
    }


    ui->btn_ManualWithdrawCreateRawTx_Search->setEnabled(false);
    ui->btn_ManualWithdrawCreateRawTx_Search->setText("搜索中...");
    //1.获取控件上的文本
    QString strCoinType = ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentText().trimmed();
    QString strDstAddr = ui->le_ManualWithdrawCreateRawTx_DstAddr->text().trimmed();
    if(strCoinType.isEmpty() || 0 == strCoinType.compare("所有币种"))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("币种为空, 请选择查询币种"));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdrawCreateRawTx_Search->setEnabled(true);
        ui->btn_ManualWithdrawCreateRawTx_Search->setText("搜索");
        return;
    }


    //2.调用业务处理接口
    vector<NS_Withdraw::WithdrawData> vctManualWithdrawData;
    try
    {
        QTableWidget *pTbw = ui->tbw_ManualWithdrawCreateRawTx;
        pTbw->clearContents();
        pTbw->setRowCount(0);


        __m_pWithdraw->SearchManualWithdrawOrder(strCoinType, strDstAddr, vctManualWithdrawData);


        strErrMsg = QString("查询成功, 共%1条记录.").arg(vctManualWithdrawData.size());
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;


        //3.显示返回数据
        QStringList strLstHeader;
        strLstHeader << LIST_STR_CREATE_UNSIGNED_RAW_TX_OF_COLLECTION_TABLE_HEADER;

        pTbw->setRowCount( vctManualWithdrawData.size());
        for(uint i = 0; i < vctManualWithdrawData.size(); i++)
        {
            int iCol = 0;
             QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(vctManualWithdrawData[i].strCoinType);
            pTbw->setItem(i, iCol++, checkBox);

            pTbw->setItem(i, iCol++, new QTableWidgetItem(vctManualWithdrawData[i].strOrderID));
            pTbw->setItem(i, iCol++, new QTableWidgetItem(vctManualWithdrawData[i].strDstAddr));
            pTbw->setItem(i, iCol++, new QTableWidgetItem(vctManualWithdrawData[i].strSrcAddr));
            pTbw->setItem(i, iCol++, new QTableWidgetItem(vctManualWithdrawData[i].strAmount));
            pTbw->setItem(i, iCol++, new QTableWidgetItem(vctManualWithdrawData[i].strTxFee));
        }

    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }


    ui->btn_ManualWithdrawCreateRawTx_Search->setEnabled(true);
    ui->btn_ManualWithdrawCreateRawTx_Search->setText("搜索");
}

//将已选中的订单,创建未签名的交易
void CMainWindow::page_ManualWithdrawCreateRawTx_OnOk()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("创建未签名提币交易界面根据选中订单创建未签名交易按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建未签名提币交易界面根据选中订单创建未签名交易按钮操作插入操作日志表失败";
    }


    //禁用按钮
    QString strBtnTextBak =  ui->btn_ManualWithdrawCreateRawTx_Ok->text();
    ui->btn_ManualWithdrawCreateRawTx_Ok->setEnabled(false);
    ui->btn_ManualWithdrawCreateRawTx_Ok->setText("创建交易中...");

    try
    {
        if(0 == ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentIndex())
        {
            strErrMsg = QString("当前选择的币种是:%1, 请先选择一个具体的币种").arg(ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentText());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QTableWidget *pTbw = ui->tbw_ManualWithdrawCreateRawTx;
        vector<int> vctRows;
        for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
        {
            QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
            if(nullptr == pTbwItem)
            {
                strErrMsg = "internal error: got check box returned nullptr";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
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
            strErrMsg = "请选择提币订单,进行创建未签名交易";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //弹出对话框，验证谷歌验证码
        QString strCoinType = ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentText();
        QString strTotalAmount = __m_pWithdraw->CalculateTotalAmount(vctRows);
        QString strTxFee = __m_pWithdraw->CalculateTxFee(strCoinType, vctRows);
        uint uiAddrCount = vctRows.size();
        QString strSrcAddr = "";


         //TODO:  添加  XMR 导出  txoutputs 处理 ?
        if(  0 == strCoinType.compare("xmr", Qt::CaseInsensitive) )
        {
            int iRet = XMR_PreworkHandler();
            if(QMessageBox::No == iRet || QMessageBox::Yes == iRet)
            {
                //恢复按钮
                ui->btn_ManualWithdrawCreateRawTx_Ok->setEnabled(true);
                ui->btn_ManualWithdrawCreateRawTx_Ok->setText(strBtnTextBak);
                return ;
            }
            //继续创建
        }




        QString strTmpSrcAddr = "请正确填写人工提币源地址";
        if (0 == strCoinType.compare("btc", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("bch", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
            )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("12rNKX2cJu3eihw2TM25xonwxpeUhGE3gC") : ("mxSK56qKAtt74ZhNN6iA9X6rKXiFqR9DP4");
        }
        else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("LPdKMrLKr45MDaA6qs5RHny1eqCYxUyMg6") : ("mn9ft9QiM36uZTFJkpU5PV6c1VDoGNCtp3");
        }
        else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("Xhyu7sh5FZg6FcPmd4NuDa454U1h6536to") : ("yiBiGAUe24kbifH23rXV2h5fsyRJ6kTqcV");
        }
        else if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("1BeAAKzm3geEs3Px44fFXoydHLdTmmS8NT") : ("mpreew4dYwpZ9hsJLFzzQVzstgZobGjy4K");
        }
        else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
                || 0 == strCoinType.compare("etc", Qt::CaseInsensitive)
                )
        {
            //如果ERC20有指定的地址,则使用指定的, 否则, 使用ETH的地址
//            if(0 == strCoinType.compare("bjc", Qt::CaseInsensitive))
//            {
//                strTmpSrcAddr = (g_bBlockChainMainnet) ? (/*主*/"0x9b54ce514391a0d960ec3113dde5097af4e3a8b7") : (/*测*/"0xD7A4a381Fca4be6b8A645d858f1bDc3107Ac3f5D");
//            }
//            else
//            {
                strTmpSrcAddr = (g_bBlockChainMainnet) ? ("0x3d761e1037885420ab736b4c0723b7333993a25a") : ("0xD7A4a381Fca4be6b8A645d858f1bDc3107Ac3f5D");
//            }
        }
        else if(0 == strCoinType.compare("htdf", Qt::CaseInsensitive) || utils::Is_HRC20_Token(strCoinType) ) //2020-04-16  增加  HRC20支持
        {
            //strTmpSrcAddr = (g_bBlockChainMainnet) ? ("htdf1nclhq92gfnk9zqxgp5jpsqxx8vfdqexwz4sd2k") : ("htdf1ll9vc32wggxmh7enl2vggry0e4u3f5knmvq2d6");
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("htdf1l6mdhhg6n95e57stxa75ed6zv3wy95jhlz08g4") : ("htdf1ll9vc32wggxmh7enl2vggry0e4u3f5knmvq2d6"); //2020-08-04 更新
        }
        else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("usdp1wl8n3yvuha6qjt3zq8pmgy57jfrwga29nww2cs") : ("usdp1slqqph8wqwekksa7lpjjf778tdhpp58clxgdgv");
        }
        else if(0 == strCoinType.compare("het", Qt::CaseInsensitive) )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("0x19fxsen9kg8z0yzzzehdjeffwppu6y4896care0") : ("0x1duandu0tr7qzdj9x4f4e2ehmpvfzcc2kae3tx7");
        }
        else if(0 == strCoinType.compare("xrp", Qt::CaseInsensitive) )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("rBhpeMdtZZ1Prf6ErvcLz6uv8DACDZDatr"/*主网生产环境*/) : ("rH2mvacU9XSSsZ9X7sfuokZ2QPcGRcVpLN");
        }
        else if(0 == strCoinType.compare("eos", Qt::CaseInsensitive) )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("hetbimcpgqnd"/*主网生产环境*/) : ("hetbitraeqhj");
        }
        else if(0 == strCoinType.compare("xlm", Qt::CaseInsensitive) )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("GC2QRLQCNCIK3FEIPEO7KP64PBOTFREGNCLMUG64QYOQFVQVARQCNPTV"/*主网生产环境*/) : ("GDLRAYITZ2RIC2FLYHEXNOUYGJY3LX4STDYDIVUAZ26YFTPCCOGLYRKI"/*测试网*/);
        }
        else if(0 == strCoinType.compare("trx", Qt::CaseInsensitive) )
        {
            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("TLzTUkXqyZXHcbB5BVUK1Nh8tXs9gVBRrX"/*主网生产环境*/) : ("TLhkcC6ohwmDJv6hETaS1TmVMrENCgifxV"/*测试网*/);
        }
        else if(0 == strCoinType.compare("xmr", Qt::CaseInsensitive) )
        {
            /*
                XMR人工出币主地址: 41mvVx2Fzzzg8vzu8aaT6PAK8M1ANMmJ7AQqzccdVGUbBstaf5RSngJcrzWLgFCJgHFFrN1n4wLVp7L6Jn5FX2xPPW9ovA8
                XMR人工出币主地址观察私钥:
                dcb2ac500a3bbf944ccf69e283e017680adfaaafd9000890ce997f9bed69190a
                XMR  自动出币主地址:
                42TEyR1TDqkHcwXy1ysH1iHtf5RV5Tk1i9hkh3P9fatUVcBN7Hp3QG54ycjtTC1QosCwZ5NN23ZWb4qG1Fop7RdnTnAj1cB
                XMR  自动出币主地址观察私钥:
                486312a77e2476e4269ef86642ab8bf3c60fe704e287bcf9c3c31ae53767ff02
             */

            strTmpSrcAddr = (g_bBlockChainMainnet) ? ("41mvVx2Fzzzg8vzu8aaT6PAK8M1ANMmJ7AQqzccdVGUbBstaf5RSngJcrzWLgFCJgHFFrN1n4wLVp7L6Jn5FX2xPPW9ovA8"/*主网生产环境*/)
                                                    : ("5797UqL9dVKd6qSjTt48c1D9dCxrL6R2xJho81v4VkGuGRt96QZBafWVxPoRN9276t6ctDK5bwg1RPYxiHjqyQdJDWAoWdz"/*测试网*/);
        }

        std::shared_ptr<CCollectionConfirmDialog> pConfirmDlg( new CCollectionConfirmDialog(strTotalAmount, uiAddrCount,
                                                                strTxFee, strCoinType, strTmpSrcAddr, QString("出币地址:"), true));
        if(QDialog::Rejected == pConfirmDlg->exec()) //取消
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "取消创建";
            throw runtime_error("已经取消创建");
        }

        strSrcAddr = pConfirmDlg->m_strAddr.trimmed();

        __m_pWithdraw->CreateUnsignedRawTx(strCoinType, vctRows, strSrcAddr); //源地址由管理自行输入

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建成功";
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("创建成功"));
        MESSAGE_BOX_OK;
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
        strErrMsg = QString("创建失败: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    //恢复按钮
    ui->btn_ManualWithdrawCreateRawTx_Ok->setEnabled(true);
    ui->btn_ManualWithdrawCreateRawTx_Ok->setText(strBtnTextBak);

    return;
}


/*TODO:
 * 2019-04-29 yqq
 * 1.优化"创建" 和 "导出" (保证"导出"的币种  与 预期的是一样的)
 * 2.一次只处理同一币种创建与导出, 如果需要处理多币种, 需要优化策略方案
 * 3.以上两点都必须以方便用户操作为前提
 */
void CMainWindow::page_ManualWithdrawCreateRawTx_OnExport()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("创建未签名提币交易界面导出已创建的未签名交易按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "创建未签名提币交易界面导出已创建的未签名交易按钮操作插入操作日志表失败";
    }


    //获取当前选择的币种(注意: 如果combox中用的是中文, 注意优化一下币种判断)
    QString strCurSelCoinType  = ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentText();
    if(0 == ui->cbx_ManualWithdrawCreateRawTx_CoinType->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType));
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType);
        MESSAGE_BOX_OK;
        return;
    }

    if(0 == __m_pWithdraw->GetExportDataCount(strCurSelCoinType) )
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("未检测到任何交易, 请先创建交易.").arg(strCurSelCoinType));
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("未检测到任何交易, 请先创建交易.").arg(strCurSelCoinType);
        MESSAGE_BOX_OK;
        return;
    }

    if( !(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType) //ERC20支持
        || 0 == strCurSelCoinType.compare("ETC", Qt::CaseInsensitive) //ETC
        || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
        || utils::Is_HRC20_Token(strCurSelCoinType)  // HRC20 支持
        || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("XRP", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("EOS", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("TRX", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("XMR", Qt::CaseInsensitive)
        ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("当前筛选的币种是\"%1\", 暂不支持.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    //导出
    QString strRetExportFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + strCurSelCoinType + "_withdraw_" + "_export_unsigned.json";
    strRetExportFilePath = strRetExportFilePath.toLower(); //转为小写

    try
    {
        __m_pWithdraw->ExportRawTxFile(strCurSelCoinType,  strRetExportFilePath);


        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("%1 导出成功").arg(strRetExportFilePath));
        MESSAGE_BOX_OK;

        QDesktopServices::openUrl(QUrl::fromLocalFile( QCoreApplication::applicationDirPath() ));
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("导出错误: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("导出错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    return;
}


//广播页面-导入
void CMainWindow::page_ManualWithdrawBroadcast_OnImport()
{

    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("人工提币交易广播界面导入待广播文件按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工提币交易广播界面导入待广播文件按钮操作插入操作日志表失败";
    }


    //获取当前选择的币种(注意: 如果combox中用的是中文, 注意优化一下币种判断)
    QString strCurSelCoinType  = ui->cbx_ManualWithdrawBroadcast_CoinType->currentText();
    if(0 == ui->cbx_ManualWithdrawBroadcast_CoinType->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType));
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType);
        MESSAGE_BOX_OK;
        return;
    }

    if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("ETC", Qt::CaseInsensitive) //ETC
           || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
           || utils::Is_HRC20_Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("EOS", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("TRX", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("XMR", Qt::CaseInsensitive)
           ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的广播功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    QString strFilePath = QFileDialog::getOpenFileName(this, tr("选择交易文件"), tr("."), tr("josn文件(*.json);;所有文件(*.*)"));
    if(strFilePath.trimmed().isEmpty())
        return;


    try
    {
        QFile fileTmpImport(strFilePath);
        if(!fileTmpImport.exists())
        {
            strErrMsg = QString("operation error: import file: %1 do not exists, please check it.").arg(strFilePath);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_pWithdraw->ImportRawTxFile(strCurSelCoinType, strFilePath);

        //通过引用方式获取导入数据
        void const * const pData = __m_pWithdraw->GetImportData(strCurSelCoinType);
        if(NULL == pData)
        {
           strErrMsg = QString("internal error: get import data failed.");
           qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
           throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
            )
        {
#if defined(BTC_SERIES_EX)
            rawtx::btc::BTC_ImportRawTxParam const * const pBtcImportData =  reinterpret_cast<rawtx::btc::BTC_ImportRawTxParam const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
            if(NULL == pBtcImportData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }


            if(0 == pBtcImportData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }


            QString strSigState = tr("已签名");
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pBtcImportData->size());
            for(uint i = 0; i < pBtcImportData->size(); i++)
            {
                int iCol = 0;

                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pBtcImportData->m_strCoinType );
                if(false == (*pBtcImportData)[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setText( pBtcImportData->m_strCoinType );
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pBtcImportData)[i].strOrderID ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pBtcImportData)[i].vctDstAddr[0] ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pBtcImportData)[i].vctSrcAddr[0] ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pBtcImportData)[i].strAmount));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pBtcImportData)[i].strTxFee ));

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
#else
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


            QString strSigState = tr("已签名");
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pBtcImportData->vctExportItems.size());
            for(uint i = 0; i < pBtcImportData->vctExportItems.size(); i++)
            {
                int iCol = 0;

                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pBtcImportData->strCoinType );
                if(false == pBtcImportData->vctExportItems[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setText( pBtcImportData->strCoinType );
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].strOrderID ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].vctDstAddr[0] ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].vctSrcAddr[0] ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].strAmount));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pBtcImportData->vctExportItems[i].strTxFee ));

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
#endif
        }
        else if(0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            USDTImportData const * const pUsdtImportData =  reinterpret_cast<BTCImportData const * const>(pData);//__m_pWithdraw->GetImportData(strCurSelCoinType);
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

            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pUsdtImportData->vctExportItems.size());
            for(uint i = 0; i < pUsdtImportData->vctExportItems.size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText(pUsdtImportData->strCoinType);
                if(false == pUsdtImportData->vctExportItems[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].strOrderID ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].vctDstAddr[0] ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].vctSrcAddr[0] ));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].strTokenAmount));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( pUsdtImportData->vctExportItems[i].strTxFee ));

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
        }
        else if( 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) ||  utils::IsERC20Token(strCurSelCoinType)
            || 0 == strCurSelCoinType.compare("ETC", Qt::CaseInsensitive) )
        {
            rawtx::ETH_ImportRawTxParam const* const pEthImportData =  reinterpret_cast<rawtx::ETH_ImportRawTxParam const* const>(pData);
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

            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount(pEthImportData->size());

            for(size_t i = 0; i < pEthImportData->size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);

                QString strShowSymbol = strCurSelCoinType;
                if( utils::IsERC20Token(strCurSelCoinType) && !strCurSelCoinType.contains("ERC20", Qt::CaseInsensitive)) strShowSymbol = "ERC20-" + strShowSymbol;
                strShowSymbol = strShowSymbol.toUpper();
                checkBox->setText(strShowSymbol);//币种,

                if(false == (*pEthImportData)[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pEthImportData)[i].strOrderId) );

                //目的地址
                QString strShowTo = (utils::IsERC20Token(strCurSelCoinType)) ? ( (*pEthImportData)[i].strERC20Recipient ) : ((*pEthImportData)[i].strAddrTo);
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowTo ));

                //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pEthImportData)[i].strAddrFrom) );

                //金额
                QString strShowValue =  (utils::IsERC20Token(strCurSelCoinType)) ? (  (*pEthImportData)[i].strERC20TokenValue ) : ( (*pEthImportData)[i].strValue);
                strShowValue += strCurSelCoinType;
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowValue ));

                //手续费
                QString strShowFee = (utils::IsERC20Token(strCurSelCoinType)) ? ("0.002ETH") : ("0.000105ETH");
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowFee ) );

                //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState));
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
        }
        else if(0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
                || utils::Is_HRC20_Token(strCurSelCoinType)
                )
        {
            //指针强制转换(只读指针)
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
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount( pCosmosImportData->size() );

            for(size_t i = 0; i < pCosmosImportData->size(); i++)
            {
                int iCol = 0;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pCosmosImportData->m_strCoinType.toUpper()  );
                if(false == (*pCosmosImportData)[i].bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                //pTbw->setItem(i, iCol++, new QTableWidgetItem( pCosmosImportData->m_strCoinType.toUpper() )  ); //币种,
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strOrderId) );       //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strTo) );            //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strFrom) );          //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strValue) );         //金额

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
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }

        }
        else if( 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)  )
        {
            //1.获取导入交易数据
            rawtx::Xrp_ImportRawTxParam const * const pXrpImptData = static_cast<rawtx::Xrp_ImportRawTxParam const* const>(pData);
            if(NULL == pXrpImptData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pXrpImptData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount( pXrpImptData->size() );


            for(size_t i = 0; i < pXrpImptData->size(); i++)
            {
                int iCol = 0;
                rawtx::Xrp_ImportRawTxItem  const &refImptItem = (*pXrpImptData)[i];
                QString strDstTag = QString::asprintf("%llu", refImptItem.uDestinationTag);

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pXrpImptData->m_strCoinType.toUpper()  );
                if(false == refImptItem.bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );       //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAddr + QString("_") + strDstTag) );            //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAddr) );          //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", refImptItem.dAmountInXRP)) );         //金额
                pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", (double)(refImptItem.uFeeInDrop * 1.0 / 1000000))));


                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }

        }
        else if(  0 == strCurSelCoinType.compare("EOS", Qt::CaseInsensitive) )
        {
            //1.获取导入交易数据
            rawtx::Eos_ImportRawTxParam const * const pEosImptData = static_cast<rawtx::Eos_ImportRawTxParam const* const>(pData);
            if(NULL == pEosImptData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pEosImptData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount( pEosImptData->size() );


            for(size_t i = 0; i < pEosImptData->size(); i++)
            {
                int iCol = 0;
                rawtx::Eos_ImportRawTxItem  const &refImptItem = (*pEosImptData)[i];
                QString strDstTag =  refImptItem.strMemo;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pEosImptData->m_strCoinType.toUpper()  );
                if(false == refImptItem.bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );       //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAcct + QString("_") + strDstTag) );            //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAcct ) );          //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));         //金额
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" )); //写不写都可以


                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
        }
        else if(  0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive) )
        {
            //1.获取导入交易数据
            rawtx::Xlm_ImportRawTxParam const * const pXlmImptData = static_cast<rawtx::Xlm_ImportRawTxParam const* const>(pData);
            if(NULL == pXlmImptData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pXlmImptData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file: %1.").arg(fileTmpImport.fileName());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount( pXlmImptData->size() );


            for(size_t i = 0; i < pXlmImptData->size(); i++)
            {
                int iCol = 0;
                rawtx::Xlm_ImportRawTxItem  const &refImptItem = (*pXlmImptData)[i];
                QString strDstTag =  refImptItem.strMemo;

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pXlmImptData->m_strCoinType.toUpper()  );
                if(false == refImptItem.bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );       //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAddr + QString("_") + strDstTag) );            //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAddr ) );          //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));         //金额
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" )); //写不写都可以


                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
        }
        else if(  0 == strCurSelCoinType.compare("TRX", Qt::CaseInsensitive) )
        {
            //1.获取导入交易数据
            rawtx::Trx_ImportRawTxParam const * const pTrxImptData = static_cast<rawtx::Trx_ImportRawTxParam const* const>(pData);
            if(NULL == pTrxImptData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pTrxImptData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file.") + fileTmpImport.fileName();
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount( pTrxImptData->size() );


            for(size_t i = 0; i < pTrxImptData->size(); i++)
            {
                int iCol = 0;
                rawtx::Trx_ImportRawTxItem  const &refImptItem = (*pTrxImptData)[i];

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pTrxImptData->m_strCoinType.toUpper()  );
                if(false == refImptItem.bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );       //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAcct ) );            //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAcct ) );          //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));         //金额
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" )); //写不写都可以

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
        }
        else if(  0 == strCurSelCoinType.compare("XMR", Qt::CaseInsensitive) )
        {
            //1.获取导入交易数据
            rawtx::XMR_ImportRawTxParam const * const pXmrImptData = static_cast<rawtx::XMR_ImportRawTxParam const* const>(pData);
            if(NULL == pXmrImptData)
            {
                strErrMsg = QString("internal error: reinterpret_cast failed .");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(0 == pXmrImptData->size())
            {
                strErrMsg = QString("external error: imported data is empty, please check file.") + fileTmpImport.fileName();
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //显示导入的数据
            QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
            pTbw->clearContents(); //只清除内容, 不清除列头
            pTbw->setRowCount( pXmrImptData->size() );


            for(size_t i = 0; i < pXmrImptData->size(); i++)
            {
                int iCol = 0;
                rawtx::XMR_ImportRawTxItem  const &refImptItem = (*pXmrImptData)[i];

                QString strSigState = tr("已签名");
                QTableWidgetItem *checkBox = new QTableWidgetItem();
                checkBox->setCheckState(Qt::Unchecked);
                checkBox->setText( pXmrImptData->m_strCoinType.toUpper()  );
                if(false == refImptItem.bComplete)
                {
                    strSigState = tr("未签名");
                    checkBox->setFlags((checkBox->flags() | Qt::ItemIsUserCheckable) ^ (Qt::ItemIsUserCheckable) );
                }
                pTbw->setItem(i, iCol++, checkBox);

                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );       //订单号
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAddr ) );            //目的地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAddr ) );          //源地址
                pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));         //金额
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" )); //写不写都可以

                pTbw->setItem(i, iCol++, new QTableWidgetItem( strSigState)); //签名状态
                pTbw->setItem(i, iCol++, new QTableWidgetItem( "----"));
            }
        }
        else
        {
            strErrMsg = QString("operation error: not support %1 in current version.").arg(strCurSelCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }
    catch(std::exception &e)
    {
        strErrMsg =  QString("导入错误, 错误信息:%1").arg(e.what());
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg =  QString("导入错误: 未知错误");
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    return;
}


//广播页面-全选
void CMainWindow::page_ManualWithdrawBroadcast_OnSelAll()
{
    //如果当前是选中  则取消全选
    Qt::CheckState checkState = ( ui->checkBox_ManualWithdrawBroadcast_SelAll->isChecked() ) ? (Qt::Checked) : (Qt::Unchecked) ;


    QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
    for(int iRow = 0; iRow < pTbw->rowCount(); iRow++ )
    {
        QTableWidgetItem  *pTbwItem =  pTbw->item(iRow, 0);
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

    return;
}

//广播页面-搜索
void CMainWindow::page_ManualWithdrawBroadcast_OnSearch()
{
    QMessageBox msgbox(QMessageBox::Information, tr("提示"), "暂不支持搜索");
    MESSAGE_BOX_OK;
}

//人工提币, 广播已签名交易
void CMainWindow::page_ManualWithdrawBroadcast_OnBroadcast()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("人工提币交易广播界面确认广播选中按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工提币交易广播界面确认广播选中按钮操作插入操作日志表失败";
    }

    ui->btn_ManualWithdrawBroadcast_Ok->setEnabled(false);
    ui->btn_ManualWithdrawBroadcast_Ok->setText("广播交易中...");

    //获取当前选择的币种(注意: 如果combox中用的是中文, 注意优化一下币种判断)
    QString strCurSelCoinType  = ui->cbx_ManualWithdrawBroadcast_CoinType->currentText().trimmed();
    if(0 == ui->cbx_ManualWithdrawBroadcast_CoinType->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType));
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType);
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdrawBroadcast_Ok->setEnabled(true);
        ui->btn_ManualWithdrawBroadcast_Ok->setText("确认广播选中");
        return;
    }

    if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
        || 0 == strCurSelCoinType.compare("ETC", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
        || utils::Is_HRC20_Token(strCurSelCoinType)
        || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("EOS", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("TRX", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("XMR", Qt::CaseInsensitive)
        ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("当前筛选的币种是\"%1\", 暂不支持.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的广播功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdrawBroadcast_Ok->setEnabled(true);
        ui->btn_ManualWithdrawBroadcast_Ok->setText("确认广播选中");
        return;
    }

    try
    {
        QTableWidget *pTbw = ui->tbw_ManualWithdrawBroadcast;
        vector<int> vctRows;
        for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
        {
            QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
            if(nullptr == pTbwItem)
            {
                strErrMsg = "internal error: get checkbox returned nullptr.";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(Qt::Checked == pTbwItem->checkState())
            {
                //保证一次广播的交易, 都是同一个币种, 不要混乱!!
                auto pTmp = pTbw->item(iRow, 0 /*币种, 如果币种不是第0列, 务必修改此值*/);
                if(NULL != pTmp)
                {
                    if(! pTmp->text().contains(strCurSelCoinType, Qt::CaseInsensitive))
                    {
                        strErrMsg = QString("operation error: the coin type '%1' you selected is different with you imported '%2' file, please check.")
                                .arg(pTmp->text()).arg(strCurSelCoinType);
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                        throw runtime_error(strErrMsg.toStdString());
                    }
                }

                vctRows.push_back(iRow);
            }
        }

        //获取所有选择范围
        if(vctRows.empty())
        {
            strErrMsg  = QString("请先选择已签名交易, 然后再进行广播.");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //ETH 和  USDP 和 HTDF 的批量提币有点特殊
        //ETH有 nonce机制(虽然较大的(大于当前的)nonce可以广播出去, 但是会在交易池中, 需等到前面的nonce被填充之后, 才会被打包
        //USDP和HTDF有 sequence机制, sequence必须是链上最新的sequence, 不能大更不能小
        //综上, 必须保证 nonce 或 sequence 逐一自增, 不能跳跃
        //所以, 为了简化操作, ETH和USDP和HTDF 必须全选
        if(0 == strCurSelCoinType.compare("HTDF", Qt::CaseInsensitive)
                || utils::Is_HRC20_Token(strCurSelCoinType)
                || 0 == strCurSelCoinType.compare("USDP", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("ETH", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
                || 0 == strCurSelCoinType.compare("ETC", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("XRP", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("EOS", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("TRX", Qt::CaseInsensitive)
                )
        {
            //检查是否全选了
            if(vctRows.size() != ui->tbw_ManualWithdrawBroadcast->rowCount())
            {
                // 检查是否存在没有签名的(选不了)的交易,
                for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
                {
                    QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0);
                    if(nullptr == pTbwItem)
                    {
                        strErrMsg = "internal error: get checkbox returned nullptr.";
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                        throw runtime_error(strErrMsg.toStdString());
                    }

                    //checkbox 是否可选择(未签名的是不可选择的)
                    if(!(pTbwItem->flags() & Qt::ItemIsUserCheckable))
                    {
                        strErrMsg = QString("operation error: row %1 is unsigned item, It couldn't be selected all."
                                            " NOTE:XLM, EOS ,XRP, TRX,ETH, USDP, HTDF, HRC20-tokens must be selected all to broadcast."
                                            "(存在未签名的交易,导致无法全选,注意: XLM, EOS, XRP, TRX, ETH,ERC20,USDP,HTDF, HRC20代币, 必须要全部选中才能广播)").arg(iRow+1);
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                        throw runtime_error(strErrMsg.toStdString());
                    }
                }

                strErrMsg = QString("operation error: NOTE: XLM,  EOS,XRP, TRX, ETH, USDP, HTDF,  HRC20-tokens, must be selected all to broadcast."
                                    "(注意: XLM, EOS, XRP, TRX,ETH,ERC20,USDP,HTDF,  HRC20代币,必须要全部选中才能广播)");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

        }


        __m_pWithdraw->BroadcastRawTx(strCurSelCoinType, vctRows);

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("广播完成"));
        MESSAGE_BOX_OK;
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("广播失败, 错误信息: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("广播失败: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }

    ui->btn_ManualWithdrawBroadcast_Ok->setEnabled(true);
    ui->btn_ManualWithdrawBroadcast_Ok->setText("确认广播选中");
    return;
}

//提币列表-重置
void CMainWindow::page_ManualWithdrawList_OnReset()
{
    ui->le_ManualWithdrawList_Txid->clear();
    ui->cbx_ManualWithdrawList_CoinType->setCurrentIndex(0);
    ui->le_ManualWithdrawList_UserID->clear();
    ui->le_ManualWithdrawList_UserTel->clear();
    ui->le_ManualWithdrawList_DstAddr->clear();
    ui->cbx_ManualWithdrawList_Status->setCurrentIndex(0);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("重置查询条件");
}

//提币列表-搜索
void CMainWindow::page_ManualWithdrawList_OnSearch()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("人工提币列表界面搜索按钮");
    if (NO_ERROR != __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工提币列表界面搜索按钮操作插入操作日志表失败";
    }


    ui->btn_ManualWithdrawList_Search->setEnabled(false);
    ui->btn_ManualWithdrawList_Search->setText("搜索中...");

    //使用 ChargeList 命名空间中的 结构体
    NS_Withdraw::SEARCHCOND  searchCond;   //搜索条件
    vector<NS_Withdraw::WITHDRAWINFO> vctWithdrawInfo; //搜索结果

    //查询所有人工审核提币交易清单
    if(0 == ui->cbx_ManualWithdrawList_CoinType->currentIndex())
        searchCond.strCoinType = "";
    else
        searchCond.strCoinType = ui->cbx_ManualWithdrawList_CoinType->currentText().trimmed();

    searchCond.strUserID = ui->le_ManualWithdrawList_UserID->text().trimmed();
    searchCond.strUserTel = ui->le_ManualWithdrawList_UserTel->text().trimmed();
    searchCond.strAddr = ui->le_ManualWithdrawList_DstAddr->text().trimmed();
    if ("HTDF" == ui->cbx_ManualWithdrawList_CoinType->currentText() || "USDP" == ui->cbx_ManualWithdrawList_CoinType->currentText()
            || "HET" == ui->cbx_ManualWithdrawList_CoinType->currentText())
        searchCond.strTxID = ui->le_ManualWithdrawList_Txid->text().trimmed().toUpper();
    else
        searchCond.strTxID = ui->le_ManualWithdrawList_Txid->text().trimmed();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询‘%1’交易").arg(ui->cbx_ManualWithdrawList_Status->currentText());
    if("待初审" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_PENDING;
    }
    else if("已初审" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FIRST_PASS;
    }
    else if("已复审" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_SUCCESS;
    }
    else if("已提现" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_SUCCESS;
        searchCond.uiAuditStatus = SEARCHALL;
    }
    else if("已拒绝" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FAILED;
    }
    else
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = SEARCHALL;
    }

    searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;

    try
    {
        //查询数据库
        __m_pWithdraw->ConditionSearch(searchCond, vctWithdrawInfo);


        vector<QStringList> vctWdItems;
        for(vector<NS_Withdraw::WITHDRAWINFO>::iterator it = vctWithdrawInfo.begin(); it != vctWithdrawInfo.end(); ++it)
        {
            QStringList listWdItem;

            listWdItem<< it->strOrderID << it->strTime << it->strCompleteTime << it->strUserID << it->strUserTel << it->strUserName \
                        << it->strCoinType << it->strAmount << it->strTranFee << it->strTxFee << it->strAuditStatus \
                        << it->strRemarks<< it->strAuditor << it->strTxID << it->strDstAddr ;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << listWdItem;
            vctWdItems.push_back(listWdItem);


        }

        if(true == g_bIsTipsNeeded)
        {
            strErrMsg = QString("查询数据成功, 共%1条记录.").arg(vctWdItems.size());
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), strErrMsg);
            MESSAGE_BOX_OK;

        }


        //清除上次查询显示的内容
        ui->tbw_WithdrawList->clearContents();
        ui->tbw_WithdrawList->setRowCount(0);
        if(vctWdItems.size() > 0)
        {
            SetTableWidgetFillData(ui->tbw_WithdrawList, vctWdItems);
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }


    ui->btn_ManualWithdrawList_Search->setEnabled(true);
    ui->btn_ManualWithdrawList_Search->setText("搜索");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询-人工提币表成功";
}

//人工提币列表-驳回
void CMainWindow::page_ManualWithdrawList_OnReject()
{
    QString strErrMsg;

    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("人工提币列表界面驳回按钮");
    if (NO_ERROR !=  __m_pOperateLog->InsertOperateLog(LoginLog))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工提币列表界面驳回按钮操作插入操作日志表失败";
    }

    //先判断是否选中有效行
    QTableWidgetItem *pOrderItem = ui->tbw_WithdrawList->item(ui->tbw_WithdrawList->currentRow(), UINT_POS_ORDERID);
    if(NULL == pOrderItem )
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "选择无效";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("选择无效"));
        MESSAGE_BOX_OK;
        return;
    }

    //先判断是否为待初审状态
    QTableWidgetItem *pAuditItem =  ui->tbw_WithdrawList->item(ui->tbw_WithdrawList->currentRow(), UNIT_AUTO_AND_MANUAL_AUDITSTATUS);
    if(0 != pAuditItem->text().trimmed().compare("已复审"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("该订单当前状态为\"%1\",不能进行驳回操作").arg(pAuditItem->text().trimmed());
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("该订单当前状态为\"%1\",不能进行驳回操作").arg(pAuditItem->text().trimmed()));
        MESSAGE_BOX_OK;
        return;
    }

    //弹出对话框，验证谷歌验证码和添加备注
    std::shared_ptr<CManualConfirmDialog> pConfirmDlg(new CManualConfirmDialog(QString("审核驳回")));
    if(QDialog::Rejected == pConfirmDlg->exec())
        return;

    try
    {
        //__m_pWithdraw->VerifyAuditStatus(strOderID, CAutoWithdraw::AUDIT_SUCCESS);

        QString strOderID = pOrderItem->text();
        QString strAuditor = CUserInfo::loginUser().loginUser().LoginName();
        __m_pWithdraw->UpdateAuditStatus(strOderID, strAuditor, pConfirmDlg->m_strRemarks, CAutoWithdraw::AUDIT_FAILED);

        g_bIsTipsNeeded = false;
        page_ManualWithdrawList_OnSearch();
        g_bIsTipsNeeded = true;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("复审驳回成功");
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("驳回失败, 错误信息: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("导出错误: 未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
}

//处理 HTDF/USDP 异常订单
void CMainWindow::page_ManualWithdrawList_Release()
{
    int iSelectedRow = ui->tbw_WithdrawList->currentRow();
    if (0 == ui->tbw_WithdrawList->item(iSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_WithdrawList->setCurrentItem(NULL);
        return;
    }

    if ((0 != ui->tbw_WithdrawList->item(iSelectedRow, 6)->text().compare("htdf", Qt::CaseInsensitive))
            && (0 != ui->tbw_WithdrawList->item(iSelectedRow, 6)->text().compare("usdp", Qt::CaseInsensitive))
            && (0 != ui->tbw_WithdrawList->item(iSelectedRow, 6)->text().compare("het", Qt::CaseInsensitive)))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("失败"), tr("注意:只能选择TXID为空且提现状态为\"已复审\"的HTDF或USDP或HET的订单"));
        MESSAGE_BOX_OK;
        ui->tbw_WithdrawList->setCurrentItem(NULL);
        return;
    }

    if (!ui->tbw_WithdrawList->item(iSelectedRow, 12)->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("失败"), tr("注意:只能选择TXID为空且提现状态为\"已复审\"的HTDF或USDP或HET的订单"));
        MESSAGE_BOX_OK;
        ui->tbw_WithdrawList->setCurrentItem(NULL);
        return;
    }

    if (!("已复审"  == ui->tbw_WithdrawList->item(iSelectedRow, 10)->text() ))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("失败"), tr("注意:只能选择TXID为空且提现状态为\"已复审\"的HTDF或USDP或HET的订单"));
        MESSAGE_BOX_OK;
        ui->tbw_WithdrawList->setCurrentItem(NULL);
        return;
    }


    QString strOrderId = ui->tbw_WithdrawList->item(iSelectedRow, 0)->text();
    QString strCoinType = ui->tbw_WithdrawList->item(iSelectedRow, 6)->text();

    QDialog *releaseDlg = new QDialog(this);
    releaseDlg->setGeometry(this->width() / 2, this->height() / 2, 400, 300);

    QLabel *lblTitle = new QLabel("处理异常订单\n仅支持USDT或HTDF或HET", releaseDlg);
    lblTitle->setGeometry(releaseDlg->width() / 2 - 80, 10, 160, 60);
    lblTitle->setAlignment(Qt::AlignCenter);

    QLabel *lblOrderId = new QLabel("订单ID:", releaseDlg);
    lblOrderId->setGeometry(10, 100, 60, 30);
    lblOrderId->setAlignment(Qt::AlignCenter);

    QLabel *lblTxId = new QLabel("交易ID:", releaseDlg);
    lblTxId->setGeometry(10, 150, 60, 30);
    lblTxId->setAlignment(Qt::AlignCenter);

    QLabel *lblSrcAddr = new QLabel("源地址:", releaseDlg);
    lblSrcAddr->setGeometry(10, 200, 60, 30);
    lblSrcAddr->setAlignment(Qt::AlignCenter);

    QLineEdit *ledOrderId = new QLineEdit(releaseDlg);
    ledOrderId->setGeometry(80, 100 ,310 ,30);
    ledOrderId->setEnabled(false);
    ledOrderId->setText(strOrderId);

    QLineEdit *ledTxId = new QLineEdit(releaseDlg);
    ledTxId->setGeometry(80, 150 ,310 ,30);

    QLineEdit *ledSraAddr = new QLineEdit(releaseDlg);
    ledSraAddr->setGeometry(80, 200 ,310 ,30);

    QPushButton *btnOk = new QPushButton("确定", releaseDlg);
    btnOk->setGeometry(60, 250, 100, 30);

    QPushButton *btnCancel = new QPushButton("取消", releaseDlg);
    btnCancel->setGeometry(240, 250, 100, 30);

    connect(btnCancel, &QPushButton::clicked, releaseDlg, [=](){
        ledOrderId->clear();
        ledTxId->clear();
        ledSraAddr->clear();
        releaseDlg->close();
    });

    connect(btnOk, &QPushButton::clicked, releaseDlg, [=](){
        //判读界面上是否为空
        if (ledSraAddr->text().isEmpty() || ledTxId->text().isEmpty())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("失败"), tr("注意:请输入完整的信息!!"));
            MESSAGE_BOX_OK;
            return;
        }
        //判断地址是否有效
        if (!utils::IsValidAddr(strCoinType, ledSraAddr->text()))
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("输入的地址无效!"));
            MESSAGE_BOX_OK;
            ledSraAddr->clear();
            return;
        }
        //判断TXID是否有效
        if (!boost::all(ledTxId->text().toStdString(), boost::is_xdigit()))
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("输入的TXID无效!"));
            MESSAGE_BOX_OK;
            ledTxId->clear();
            return;
        }


        QSqlDatabase m_db = QSqlDatabase::addDatabase("QMYSQL", "detailErrData");
        if (g_bIsLAN)
        {
            m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
            m_db.setHostName("192.168.10.81");
            m_db.setPort(3306);
            m_db.setUserName("root");
            m_db.setPassword("MyNewPass4!");
            m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
        }
        else
        {
            m_db.setDatabaseName("assertdb");
            m_db.setHostName("rm-wz991r2rjs3wmd9t1io.mysql.rds.aliyuncs.com");
            m_db.setPort(3306);
            m_db.setUserName("accertdb");
            m_db.setPassword("CG1R47JxIfBofG5uIofHcUKW0Ay1f8");
            m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
        }
        if (!m_db.open())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "连接(打开)数据库失败, 错误信息:" << m_db.lastError();
            return;
        }

        QSqlQuery query(m_db);
        QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4,"
                                 " src_addr='%5', complete_time=%6 WHERE order_id='%7';")
                .arg(ledTxId->text()).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/)
                .arg(ledSraAddr->text()).arg(QDateTime::currentDateTime().toTime_t()).arg(strOrderId);

        if (!query.exec(strSql))
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("失败"), tr("注意:更新数据库失败!!"));
            MESSAGE_BOX_OK;
            return;
        }
        ledOrderId->clear();
        ledTxId->clear();
        ledSraAddr->clear();
        releaseDlg->close();
    });

    releaseDlg->exec();
}

void CMainWindow::page_ManualWithdrawList_OnExportExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("人工提币列表界面导出EXCEL按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工提币列表界面导出EXCEL按钮操作插入操作日志表失败";
    }

    NS_Withdraw::SEARCHCOND  searchCond;   //搜索条件
    vector<NS_Withdraw::WITHDRAWINFO> vctWithdrawInfo; //搜索结果

    uint uStartDatetime = QDateTime::fromString(ui->date_WithdrawList_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    uint uEndDatetime = QDateTime::fromString(ui->date_WithdrawList_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    if (UINT_TIMESTAMP == uEndDatetime)
        ui->date_WithdrawList_End->setDateTime(QDateTime::currentDateTime());

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_ManualWithdrawList_Search->setEnabled(true);
        ui->btn_ManualWithdrawList_Search->setText("搜索");
        return;
    }

    searchCond.uDatetimeStart = QDateTime::fromString(ui->date_WithdrawList_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    searchCond.uDatetimeEnd = QDateTime::fromString(ui->date_WithdrawList_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    if(0 == ui->cbx_ManualWithdrawList_CoinType->currentIndex())
        searchCond.strCoinType = "";
    else
        searchCond.strCoinType = ui->cbx_ManualWithdrawList_CoinType->currentText().trimmed();

    searchCond.strUserID = ui->le_ManualWithdrawList_UserID->text().trimmed();
    searchCond.strUserTel = ui->le_ManualWithdrawList_UserTel->text().trimmed();
    searchCond.strAddr = ui->le_ManualWithdrawList_DstAddr->text().trimmed();
    if ("HTDF" == ui->cbx_ManualWithdrawList_CoinType->currentText() || "USDP" == ui->cbx_ManualWithdrawList_CoinType->currentText()
            || "HET" == ui->cbx_ManualWithdrawList_CoinType->currentText())
        searchCond.strTxID = ui->le_ManualWithdrawList_Txid->text().trimmed().toUpper();
    else
        searchCond.strTxID = ui->le_ManualWithdrawList_Txid->text().trimmed();

    qDebug() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << QString("查询‘%1’交易").arg(ui->cbx_ManualWithdrawList_Status->currentText());
    if("待初审" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_PENDING;
    }
    else if("已初审" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_1;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FIRST_PASS;
    }
    else if("已复审" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_PENDING;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_SUCCESS;
    }
    else if("已提现" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = CAutoWithdraw::ORDER_SUCCESS;
        searchCond.uiAuditStatus = SEARCHALL;
    }
    else if("已拒绝" == ui->cbx_ManualWithdrawList_Status->currentText().trimmed())
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = CAutoWithdraw::AUDIT_FAILED;
    }
    else
    {
        searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;
        searchCond.uiWithdrawStatus = SEARCHALL;
        searchCond.uiAuditStatus = SEARCHALL;
    }

    searchCond.uiQueryType = CPageManualWithdraw::MANUALWITHDRAW_ALL;

    //查询数据库
    __m_pWithdraw->ConditionSearch(searchCond, vctWithdrawInfo);
    if (vctWithdrawInfo.size() <= 0)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("查询相应的数据失败!"));
        MESSAGE_BOX_OK;
        return;
    }

    vector<QStringList> vctWdItems;
    for(vector<NS_Withdraw::WITHDRAWINFO>::iterator it = vctWithdrawInfo.begin(); it != vctWithdrawInfo.end(); ++it)
    {
        QStringList listWdItem;

        listWdItem<< it->strOrderID << it->strTime << it->strCompleteTime << it->strUserID << it->strUserTel << it->strUserName \
                    << it->strCoinType << it->strAmount << it->strTranFee << it->strTxFee << it->strAuditStatus \
                    << it->strAuditor << it->strTxID << it->strDstAddr << it->strRemarks;
//            qDebug() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << listWdItem;
        vctWdItems.push_back(listWdItem);
    }

    //清除上次查询显示的内容
    ui->tbw_WithdrawList->clearContents();
    ui->tbw_WithdrawList->setRowCount(0);

    if(vctWdItems.size() > 0)
    {
        SetTableWidgetFillData(ui->tbw_WithdrawList, vctWdItems);
    }

    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("数据查询成功,\n点击确定进行导出操作"));
    MESSAGE_BOX_OK_CANCEL;
    if (QMessageBox::Ok != msgbox.exec())
    {
        return;
    }

    if (0 == ui->tbw_WithdrawList->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有人工提币数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中不存在数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_WithdrawList);
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "人工提币数据导出EXCEL文件失败";
        QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("EXCEL文件导出失败!"));
        MESSAGE_BOX_OK;
        return;
    }

    {
        QMessageBox msgbox(QMessageBox::Information, tr("成功"), tr("导出EXCEL成功!"));
        MESSAGE_BOX_OK;
    }
}

// XMR提币比较特殊, 操作流程较多
int CMainWindow::XMR_PreworkHandler()
{
    QString strErrMsg = "";

    QMessageBox msgbox(QMessageBox::Question, tr("XMR操作类型选择"), tr("由于XMR的特殊,A2和B1操作在WM端进行,请选择操作类型:\r\n  (A1): 导出交易输出(tx_outputs)\r\n  (B2): 导入密钥镜像(key_images)\r\n  (C1): 此轮操作中已完成A1,A2和B1,B2四个操作,继续创建交易"));
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgbox.setButtonText(QMessageBox::Yes, "A1");//导出交易输出
    msgbox.setButtonText(QMessageBox::No, "B2");//导入key-images
    msgbox.setButtonText(QMessageBox::Cancel, "C1"); //继续创建
    msgbox.setWindowFlags(Qt::FramelessWindowHint);

    int iRet = msgbox.exec();

    if( QMessageBox::Cancel == iRet)
        return iRet;

    //导出交易输出 export txoutputs
    if( QMessageBox::Yes == iRet)
    {
        QString strFilePathRet = "";
        __m_pWithdraw->XMR_ExportTxOutputs( strFilePathRet );

        //
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("%1 导出成功").arg(strFilePathRet));
        MESSAGE_BOX_OK;

        QDesktopServices::openUrl(QUrl::fromLocalFile( QCoreApplication::applicationDirPath() ));

    }

    //导入 key-images
    if( QMessageBox::No == iRet )
    {
        QString strFilePath = QFileDialog::getOpenFileName(this, tr("选择交易文件"), tr("."), tr("josn文件(*.json);;所有文件(*.*)"));
        if(strFilePath.trimmed().isEmpty())
        {
            strErrMsg = QString("文件路径为空").arg(strFilePath);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_pWithdraw->XMR_ImportKeyImages( strFilePath );

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("key-images文件:%1 导入成功").arg(strFilePath));
        MESSAGE_BOX_OK;
    }


    return iRet;
}
