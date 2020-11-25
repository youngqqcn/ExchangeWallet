#include "page_manual_withdraw.h"

#include "utils.h"
#include <QDateTime>


//WITHDRAW::WITHDRAW(QObject *parent) : QObject(parent)
//{
//}

CPageManualWithdraw::CPageManualWithdraw(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageManualWithdraw::~CPageManualWithdraw()
{

}

int CPageManualWithdraw::Init()
{
    //如果没有其他初始化, 直接使用父类的Init函数即可
    //如果有其他初始化, 请在此函数中实现初始化
    //---交易类型

    __m_mapTradeType.insert("0", QString("交易"));
    __m_mapTradeType.insert("1", QString("交易"));
    __m_mapTradeType.insert("2", QString("手续费-交易"));
    __m_mapTradeType.insert("3", QString("充币"));
    __m_mapTradeType.insert("4", QString("提币"));
    __m_mapTradeType.insert("5", QString("手续费-提币"));
    __m_mapTradeType.insert("6", QString("划转-转出"));
    __m_mapTradeType.insert("7", QString("划转-转入"));
    __m_mapTradeType.insert("8", QString("注册赠送-邀请"));
    __m_mapTradeType.insert("9", QString("交易返佣"));
    __m_mapTradeType.insert("10", QString("实名赠送-邀请"));
    __m_mapTradeType.insert("11", QString("注册赠送"));
    __m_mapTradeType.insert("12", QString("实名赠送"));

    //---审核状态
    __m_mapAuditStatus.insert("0", QString("待初审"));
    __m_mapAuditStatus.insert("1", QString("已复审"));
    __m_mapAuditStatus.insert("2", QString("已拒绝"));
    __m_mapAuditStatus.insert("3", QString("已初审"));

    //---订单状态
    __m_mapOrderStatus.insert(CAutoWithdraw::ORDER_PENDING, QString("待提现"));
    __m_mapOrderStatus.insert(CAutoWithdraw::ORDER_SUCCESS, QString("已提现"));
    __m_mapOrderStatus.insert(CAutoWithdraw::ORDER_FAILED, QString("已拒绝"));
    __m_mapOrderStatus.insert(CAutoWithdraw::ORDER_CLIENT_ERR, QString("已拒绝"));
    __m_mapOrderStatus.insert(CAutoWithdraw::ORDER_NET_ERR, QString("已拒绝"));
    __m_mapOrderStatus.insert(CAutoWithdraw::ORDER_SERVER_ERR, QString("已拒绝"));

    return CAMMain::Init();
}


int CPageManualWithdraw::ConditionSearch_AutoWithDraw(const NS_Withdraw::SEARCHCOND &searchCond,  vector<NS_Withdraw::WITHDRAWINFO> &vctWithdrawInfo) noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("ConditionSearch 进入函数");

    //构造查询语句
    //2019.04.23  查询时增加排序 by liling
    QString strCond = "";
    if(!searchCond.strUserID.isEmpty())
        strCond.append(QString(" and tb_user.user_id = '%1'").arg(searchCond.strUserID));
    if(!searchCond.strUserTel.isEmpty())
        strCond.append(QString(" and (tel = '%1' or email = '%2')").arg(searchCond.strUserTel).arg(searchCond.strUserTel));
    if(!searchCond.strTxID.isEmpty())
        strCond.append(QString(" and txid = '%1'").arg(searchCond.strTxID));
    if(!searchCond.strAddr.isEmpty())
        strCond.append(QString(" and dst_addr = '%1'").arg(searchCond.strAddr));
    if(!searchCond.strCoinType.isEmpty())
        strCond.append(QString(" and coin_type = '%1'").arg(searchCond.strCoinType));
    if(SEARCHALL != searchCond.uiWithdrawStatus)
    {
        if(3 == searchCond.uiWithdrawStatus) //待提现
        {
            strCond.append(QString(" and (order_status = 0 or order_status > 2) ").arg(searchCond.uiWithdrawStatus));
        }
        else
        {
            strCond.append(QString(" and order_status = %1 ").arg(searchCond.uiWithdrawStatus));
        }
    }

    if(SEARCHALL != searchCond.uiAuditStatus)
        strCond.append(QString(" and audit_status = %1 ").arg(searchCond.uiAuditStatus));


    QString strQuery;

    //TODO:有些可能获取不到用户
    if (0 == searchCond.uDatetimeStart && 0 == searchCond.uDatetimeEnd)
    {
        strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, "
                           "dst_addr,order_status,tel, email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw,"
                           "tb_user where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way = %1 %2 order by create_time desc")
                .arg(CAutoWithdraw::WAY_AUTO).arg(strCond);
    }
    else
    {
        strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, "
                           "dst_addr,order_status,tel, email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw,"
                           "tb_user where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way = %1 %2 and complete_time > %3 "
                           "and complete_time < %4 order by complete_time desc")
                .arg(CAutoWithdraw::WAY_AUTO).arg(strCond).arg(searchCond.uDatetimeStart).arg(searchCond.uDatetimeEnd);
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("Query autowithdraw data with sql = ") << strQuery;

    QSqlQuery query(_m_db);
    if(!query.exec(strQuery))
    {
        strErrMsg = QString("sql error: %1").arg( query.lastError().text() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg ;
        throw runtime_error(strErrMsg.toStdString());
    }

    //用户信息由从java后台获取，改为从数据库获取 20190223 by liling
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp1;
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp2;
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp3;

    while(query.next())
    {
        NS_Withdraw::WITHDRAWINFO wdItem;
        wdItem.strOrderID = query.value(0).toString();
        wdItem.strTime = QDateTime::fromTime_t(query.value(1).toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
        wdItem.strUserID = query.value(2).toString();
        wdItem.strCoinType = query.value(3).toString();
        wdItem.strAmount = query.value(4).toString();
        wdItem.strTxFee = query.value(5).toString();
        wdItem.strAuditStatus = __m_mapAuditStatus[QString::number(query.value(6).toInt())];
        wdItem.strTxID = query.value(7).toString();
        wdItem.strDstAddr = query.value(8).toString();
        wdItem.strOrderStatus = __m_mapOrderStatus[query.value(9).toInt()];
        QString strTel = query.value(10).toString();
        QString strMail = query.value(11).toString();
        wdItem.strUserTel = strTel.isEmpty()?strMail:strTel;
        wdItem.strUserName = query.value(12).toString();
        wdItem.strRemarks = query.value(13).toString();
        wdItem.strAuditor = query.value(14).toString();
        wdItem.strAuditTime = QDateTime::fromTime_t(query.value(15).toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
        wdItem.strTranFee = query.value(16).toString();
        if (0 == query.value(17).toUInt())
            wdItem.strCompleteTime = QString("--");
        else
            wdItem.strCompleteTime = QDateTime::fromTime_t(query.value(17).toUInt()).toString("yyyy-MM-dd hh:mm:ss");
        if(0==wdItem.strOrderStatus.compare(QString("已提现")))
        {
            wdItem.strAuditStatus = wdItem.strOrderStatus;
        }
        else
        {
            if(0!=wdItem.strAuditStatus.compare(QString("已拒绝")))
            {
                wdItem.strAuditStatus = QString("待提现");
            }

        }

        if(0==wdItem.strAuditStatus.compare(QString("待提现")))
        {
            vctTemp1.push_back(wdItem);
        }
        else if(0==wdItem.strAuditStatus.compare(QString("已提现")))
        {
            vctTemp2.push_back(wdItem);
        }
        else if(0==wdItem.strAuditStatus.compare(QString("已拒绝")))
        {
            vctTemp3.push_back(wdItem);
        }

    }


    if((SEARCHALL == searchCond.uiWithdrawStatus && SEARCHALL == searchCond.uiAuditStatus) ||
            (SEARCHALL == searchCond.uiAuditStatus && CAutoWithdraw::ORDER_PENDING == searchCond.uiWithdrawStatus))  //待提现或全选
    {
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp1.begin(), vctTemp1.end());
    }

    if((SEARCHALL == searchCond.uiWithdrawStatus && SEARCHALL == searchCond.uiAuditStatus) ||
            (SEARCHALL == searchCond.uiAuditStatus && CAutoWithdraw::ORDER_SUCCESS == searchCond.uiWithdrawStatus))  //已提现或全选
    {
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp2.begin(), vctTemp2.end());
    }

    if((SEARCHALL == searchCond.uiWithdrawStatus && SEARCHALL == searchCond.uiAuditStatus) ||
            (CAutoWithdraw::AUDIT_FAILED == searchCond.uiAuditStatus&&SEARCHALL ==searchCond.uiWithdrawStatus)) //已拒绝或全选
    {
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp3.begin(), vctTemp3.end());
    }


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("ConditionSearch  result rows: %1" ).arg( vctTemp1.size() + vctTemp2.size() + vctTemp3.size() );
    return NO_ERROR;
}

int CPageManualWithdraw::ConditionSearch(const NS_Withdraw::SEARCHCOND &searchCond,  vector<NS_Withdraw::WITHDRAWINFO> &vctWithdrawInfo)noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("ConditionSearch 进入函数");

    __m_vctWithdrawData.clear();

    //1.检查参数
    if(/*searchCond.uiQueryType < 0 ||*/ searchCond.uiQueryType >3)
    {
        strErrMsg = "internal error: query type is invalid.";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //构造查询语句
    //2019.04.23  查询时增加排序 by liling
    QString strCond = "";
    if(!searchCond.strUserID.isEmpty())
        strCond.append(QString(" and tb_user.user_id = '%1'").arg(searchCond.strUserID));
    if(!searchCond.strUserTel.isEmpty())
        strCond.append(QString(" and (tel = '%1' or email = '%2')").arg(searchCond.strUserTel).arg(searchCond.strUserTel));
    if(!searchCond.strTxID.isEmpty())
        strCond.append(QString(" and txid = '%1'").arg(searchCond.strTxID));
    if(!searchCond.strAddr.isEmpty())
        strCond.append(QString(" and dst_addr = '%1'").arg(searchCond.strAddr));
    if(!searchCond.strCoinType.isEmpty())
        strCond.append(QString(" and coin_type = '%1'").arg(searchCond.strCoinType));
    if(SEARCHALL != searchCond.uiWithdrawStatus)
    {
        if(3 == searchCond.uiWithdrawStatus) //待提现
            strCond.append(QString(" and (order_status = 0 or order_status > 2) ").arg(searchCond.uiWithdrawStatus));
        else
            strCond.append(QString(" and order_status = %1 ").arg(searchCond.uiWithdrawStatus));
    }

    if(SEARCHALL != searchCond.uiAuditStatus)
        strCond.append(QString(" and audit_status = %1 ").arg(searchCond.uiAuditStatus));
    QString strQuery;
    if (0 == searchCond.uDatetimeEnd && 0 == searchCond.uDatetimeStart)
    {
        if(AUTOWITHDRAW == searchCond.uiQueryType) //查询自动提现表
        {
            strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, "
                               "dst_addr,order_status,tel, email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from "
                               "tb_auto_withdraw,tb_user where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way = %1 %2 "
                               "order by create_time desc").arg(CAutoWithdraw::WAY_AUTO).arg(strCond);
        }
        else if(MANUALWITHDRAW_1 == searchCond.uiQueryType)   //查询人工提币初审表
        {
             //暂时人工提币表和自动提币表共用一个表
             /*
            strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, dst_addr,"
                               "order_status,tel,email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw,"
                               "tb_user where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way=%1 and audit_status!=%2 and "
                               "order_status!=%3 %4 ORDER BY create_time desc")
                    .arg(CAutoWithdraw::WAY_MANUAL).arg(CAutoWithdraw::AUDIT_SUCCESS).arg(CAutoWithdraw::ORDER_SUCCESS).arg(strCond);
            */

            //增加  `半自动`待审核  即:  (withdraw_way=0 and audit_status=0)   2020-04-24  yqq
            strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, dst_addr,"
                               "order_status,tel,email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw,"
                               "tb_user where tb_auto_withdraw.user_id = tb_user.user_id and (withdraw_way=%1 or (withdraw_way=0 and audit_status=0) ) and audit_status!=%2 and "
                               "order_status!=%3 %4 ORDER BY create_time desc")
                    .arg(CAutoWithdraw::WAY_MANUAL).arg(CAutoWithdraw::AUDIT_SUCCESS).arg(CAutoWithdraw::ORDER_SUCCESS).arg(strCond);
        }
        else if(MANUALWITHDRAW_2 == searchCond.uiQueryType) //查询人工提币复审表
        {
             //暂时人工提币表和自动提币表共用一个表
            /*
            strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, dst_addr,"
                               "order_status,tel,email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw, tb_user "
                               "where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way=%1 and audit_status!=%2 and order_status!=%3 %4 "
                               "order by create_time desc")
                    .arg(CAutoWithdraw::WAY_MANUAL).arg(CAutoWithdraw::AUDIT_PENDING).arg(CAutoWithdraw::ORDER_SUCCESS).arg(strCond);
            */

            //增加  `半自动`已初审  (withdraw_way=0 and audit_status=3)   2020-04-24  yqq
            strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, dst_addr,"
                               "order_status,tel,email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw, tb_user "
                               "where tb_auto_withdraw.user_id = tb_user.user_id and (withdraw_way=%1 or (withdraw_way=0 and audit_status=3)) and audit_status!=%2 and order_status!=%3 %4 "
                               "order by create_time desc")
                    .arg(CAutoWithdraw::WAY_MANUAL).arg(CAutoWithdraw::AUDIT_PENDING).arg(CAutoWithdraw::ORDER_SUCCESS).arg(strCond);
        }
        else if(MANUALWITHDRAW_ALL == searchCond.uiQueryType) //查询所有人工提币记录
        {
            //暂时人工提币表和自动提币表共用一个表
            strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, dst_addr,"
                               "order_status ,tel,email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from "
                               "tb_auto_withdraw,tb_user where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way=%1 %2 "
                               "order by create_time desc")
                    .arg(CAutoWithdraw::WAY_MANUAL).arg(strCond);
        }
    }
    else
    {
        strQuery = QString("select order_id,create_time,tb_auto_withdraw.user_id,coin_type,amount,tx_fee,audit_status,txid, dst_addr,"
                           "order_status ,tel,email,user_family_name,remarks,auditor,audit_time,trade_fee,complete_time from tb_auto_withdraw,tb_user "
                           "where tb_auto_withdraw.user_id = tb_user.user_id and withdraw_way=%1 %2 and complete_time > %3 and complete_time < %4 "
                           "order by complete_time desc").arg(CAutoWithdraw::WAY_MANUAL).arg(strCond).arg(searchCond.uDatetimeStart)
                .arg(searchCond.uDatetimeEnd);
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("Query withdraw data with sql = ") << strQuery;

    QSqlQuery query(_m_db);
    if(!query.exec(strQuery))
    {
        strErrMsg = QString("sql error: %1").arg(query.lastError().text());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //用户信息由从java后台获取，改为从数据库获取 20190223 by liling
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp1;
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp2;
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp3;
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp4;
    vector<NS_Withdraw::WITHDRAWINFO> vctTemp5;

    while(query.next())
    {
        NS_Withdraw::WITHDRAWINFO wdItem;
        wdItem.strOrderID = query.value(0).toString();
        wdItem.strTime = QDateTime::fromTime_t(query.value(1).toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
        wdItem.strUserID = query.value(2).toString();
        wdItem.strCoinType = query.value(3).toString();
        wdItem.strAmount = query.value(4).toString();
        wdItem.strTxFee = query.value(5).toString();
        wdItem.strAuditStatus = __m_mapAuditStatus[QString::number(query.value(6).toInt())];
        wdItem.strTxID = query.value(7).toString();
        wdItem.strDstAddr = query.value(8).toString();
        wdItem.strOrderStatus = __m_mapOrderStatus[query.value(9).toInt()];
        QString strTel = query.value(10).toString();
        QString strMail = query.value(11).toString();
        wdItem.strUserTel = strTel.isEmpty()?strMail:strTel;
        wdItem.strUserName = query.value(12).toString();
        wdItem.strRemarks = query.value(13).toString();
        wdItem.strAuditor = query.value(14).toString();
        wdItem.strAuditTime = QDateTime::fromTime_t(query.value(15).toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
        wdItem.strTranFee = query.value(16).toString();
        if (0 == query.value(17).toUInt())
            wdItem.strCompleteTime = QString("--");
        else
            wdItem.strCompleteTime = QDateTime::fromTime_t(query.value(17).toUInt()).toString("yyyy-MM-dd hh:mm:ss");

        if(0 == wdItem.strOrderStatus.compare(QString("已提现")))
            wdItem.strAuditStatus = wdItem.strOrderStatus;



        if(0 == wdItem.strAuditStatus.compare(QString("待初审")))
            vctTemp1.push_back(wdItem);
        else if(0 ==wdItem.strAuditStatus.compare(QString("已初审")))
            vctTemp2.push_back(wdItem);
        else if(0 == wdItem.strAuditStatus.compare(QString("已复审")))
            vctTemp3.push_back(wdItem);
        else if(0 == wdItem.strAuditStatus.compare(QString("已拒绝")))
            vctTemp4.push_back(wdItem);
        else if(0 == wdItem.strAuditStatus.compare(QString("已提现")))
            vctTemp5.push_back(wdItem);

    }

    if(MANUALWITHDRAW_1 == searchCond.uiQueryType)   //查询人工提币初审表
    {
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp1.begin(), vctTemp1.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp2.begin(), vctTemp2.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp4.begin(), vctTemp4.end());
    }
    else if(MANUALWITHDRAW_2 == searchCond.uiQueryType) //查询人工提币复审表
    {
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp2.begin(), vctTemp2.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp3.begin(), vctTemp3.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp4.begin(), vctTemp4.end());
    }
    else if(MANUALWITHDRAW_ALL == searchCond.uiQueryType) //查询所有人工提币记录
    {
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp1.begin(), vctTemp1.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp2.begin(), vctTemp2.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp3.begin(), vctTemp3.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp5.begin(), vctTemp5.end());
        vctWithdrawInfo.insert(vctWithdrawInfo.end(), vctTemp4.begin(), vctTemp4.end());
    }
    __m_vctWithdrawData.insert(__m_vctWithdrawData.begin(), vctWithdrawInfo.begin(), vctWithdrawInfo.end());
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("ConditionSearch 成功返回");
    return NO_ERROR;
}

int  CPageManualWithdraw::UpdateAuditStatus(const QString& strOrderID, const QString& strAuditor, const QString& strRemark, int iStatus)noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("UpdateAuditStatus 进入函数");
    if(strOrderID.isEmpty() || strAuditor.isEmpty())
    {
        strErrMsg = QString("订单号或审核人为空");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    if(iStatus < 0 || iStatus > 3 )
    {
        strErrMsg =  QString("审核状态参数错误, iStatus=%1 ").arg( iStatus);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QSqlQuery query(CAMMain::_m_db);
    QString strQuery = QString("update tb_auto_withdraw set audit_status = %1,auditor = '%2', remarks='%3',audit_time=%4 where order_id = \"%5\"").\
            arg(iStatus).arg(strAuditor).arg(strRemark).arg(QDateTime::currentSecsSinceEpoch()).arg(strOrderID);

    if(!query.exec(strQuery))
    {
        strErrMsg = QString("更新审核状态错误, errMsg:%1").arg( query.lastError().text() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("更新审核状态成功");
    return NO_ERROR;
}



int CPageManualWithdraw::__ParseUserAssetData(const QByteArray &arrayJsonData) noexcept(false)
{
    QString strErrMsg;

    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(arrayJsonData, &error));
    if(QJsonParseError::NoError != error.error  )
    {
        strErrMsg = QString("json parse error: %1").arg(error.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject rootObj = jsonDoc.object();



    //判断返回状态是否OK
    if(!(rootObj.contains("errCode") && 0 == rootObj.value("errCode").toInt()))
    {
        strErrMsg = QString("java returns error:%1, %2").arg(rootObj.value("errCode").toInt() ).arg(rootObj.value("errMsg").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if(!rootObj.contains("data"))
    {
        strErrMsg = QString("java response json missing `data`.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QJsonObject joData = rootObj.value("data").toObject();
    if(joData.isEmpty() || 0 == joData.size())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "data is null";
        return NO_ERROR;
    }

    //获取用户基本信息
    QJsonObject joUserInfo = joData.value("userInfo").toObject();
    __m_userAssetData.userBasicInfo.strUserId = joUserInfo.value("userId").toString();
    __m_userAssetData.userBasicInfo.strUserName = joUserInfo.value("realName").toString();
    __m_userAssetData.userBasicInfo.strTelNo = joUserInfo.value("phone").toString();
    __m_userAssetData.userBasicInfo.strUserEmail = joUserInfo.value("email").toString();

    //获取资产数据
    QJsonArray arrayUserData = joData.value("detail").toArray();
    NS_Withdraw::AssetData tempData;
    for(int i = 0; i < arrayUserData.size(); i++)
    {
        QJsonObject subObj = arrayUserData.at(i).toObject();
        tempData.strCoinName = subObj.value("coin").toString();                                         //币种
        tempData.strAvailVol = QString::number(subObj.value("availVol").toDouble(), 'f', 8);            //可用资产
        tempData.strTotalAssets = QString::number(subObj.value("totalAssets").toDouble(), 'f', 8);      //总资产
        tempData.strTotalInCoin = QString::number(subObj.value("totalInCoin").toDouble(), 'f', 8);      //充值总额
        tempData.strTotalOutCoin = QString::number(subObj.value("totalOutCoin").toDouble(), 'f', 8);    //提币总额
        tempData.strFreezeVol = QString::number(subObj.value("freezeVol").toDouble(), 'f', 8);          //冻结金额
        tempData.strLockVol = QString::number(subObj.value("lockVol").toDouble(), 'f', 8);              //锁定金额
        tempData.strWithdrawFreeze = QString::number(subObj.value("outCoinFrozen").toDouble(), 'f', 8); //提币冻结
        tempData.strDelegateVol = "---";                                                               //委托数量
        tempData.strStatus = "---";
        tempData.strWithdrawAmount = "---";
        __m_userAssetData.lstAssetData.push_back(tempData);
    }

    return NO_ERROR;
}

int CPageManualWithdraw::__ParseUserFlowData(const QByteArray &arrayJsonData, int *poutPageCnt)  noexcept(false)
{
    QString strErrMsg;

    QJsonParseError jerror;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(arrayJsonData, &jerror));
    if( QJsonParseError::NoError != jerror.error )
    {
        strErrMsg = QString("json parse error:%1").arg(jerror.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject rootObj = jsonDoc.object();

    //26错误码, C++后台不存在这张表(按月分表)
    if(rootObj.contains("errCode") && 26 == rootObj.value("errCode").toInt())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "errCode: 26 , not data.";
        return NO_ERROR;
    }

    //判断返回状态是否OK
    if(!(rootObj.contains("errCode") && 0 == rootObj.value("errCode").toInt()))
    {
        strErrMsg = QString("java returns error:%1, %2").arg(rootObj.value("errCode").toInt() ).arg(rootObj.value("errMsg").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if(!rootObj.contains("data"))
    {
        strErrMsg = QString("java response json missing `data`.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joData = rootObj.value("data").toObject();
    if(joData.isEmpty() || 0 == joData.size())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "data is null";
        return NO_ERROR;
    }


    //获取页码总数
    *poutPageCnt = joData.value("pageCnt").toInt();

    //获取用户基本信息
    QJsonObject objUserInfo = joData.value("userInfo").toObject();
    __m_userFlowData.userBasicInfo.strUserId = objUserInfo.value("userId").toString();
    __m_userFlowData.userBasicInfo.strUserName = objUserInfo.value("realName").toString();
    __m_userFlowData.userBasicInfo.strTelNo = objUserInfo.value("phone").toString();
    __m_userFlowData.userBasicInfo.strUserEmail = objUserInfo.value("email").toString();

    //获取流水数据
    QJsonArray jaUserData = joData.value("detail").toArray();
    NS_Withdraw::FlowData tempData;
    for(int i = 0; i < jaUserData.size(); i++)
    {
        QJsonObject joDetailItem = jaUserData.at(i).toObject();
        tempData.strCoinName = joDetailItem.value("coin").toString();
        tempData.strTxType = __m_mapTradeType[joDetailItem.value("tradeType").toVariant().toString()];
        tempData.strCount = joDetailItem.value("changeVol").toString();
        tempData.strBalance = QString::number(joDetailItem.value("balance").toString().toDouble(), 'f', 8);

        //4919 初审列表页面选中一条记录点击查看用户流水，用户流水页面显示时间不正确 by liling
        tempData.strTime = QDateTime::fromTime_t((joDetailItem.value("nTime").toVariant().toULongLong()/1000)).toString("yyyy-MM-dd hh:mm:ss");
        __m_userFlowData.lstFlowData.push_front(tempData);

    }

    return NO_ERROR;
}

int CPageManualWithdraw::PullUserAssetDataFromServer(const QString &strInUrl, const QString &strInUserID) noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "strInUrl = " << strInUrl;
    if(strInUrl.isEmpty())
    {
        strErrMsg = QString("internal error: url is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    __m_userAssetData.lstAssetData.clear();

    QByteArray byteArrayReply;
    QJsonObject joParams;
    joParams.insert("userId", strInUserID);
    //joParams.insert("pageIndex", pageIndex);
     //__RequestData(strInUrl, strInUserID, 0, byteArrayReply);
    CAMMain::SyncPostByJson(strInUrl, joParams, byteArrayReply);


    qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "Return data of RequestData: " << byteArrayReply;
    __ParseUserAssetData(byteArrayReply);


    NS_Withdraw::UserAsset  userAsset;
    for(auto itWithdraw = __m_vctWithdrawData.begin(); itWithdraw != __m_vctWithdrawData.end(); itWithdraw++)
    {
        if(itWithdraw->strUserID == strInUserID &&(itWithdraw->strAuditStatus == "待初审" || itWithdraw->strAuditStatus == "已初审"))
        {
            for(auto itUserAsset = __m_userAssetData.lstAssetData.begin(); itUserAsset != __m_userAssetData.lstAssetData.end(); itUserAsset++)
            {
                if(0 == itUserAsset->strCoinName.compare( itWithdraw->strCoinType, Qt::CaseInsensitive)
                || ( 0 == itUserAsset->strCoinName.compare("USDT", Qt::CaseInsensitive) && 0 == itWithdraw->strCoinType.compare("ERC20-USDT", Qt::CaseInsensitive) ))
                {
                    NS_Withdraw::AssetData  tmpData;
                    tmpData.strAvailVol = itUserAsset->strAvailVol;
                    tmpData.strCoinName = itUserAsset->strCoinName;
                    tmpData.strTotalAssets = itUserAsset->strTotalAssets;
                    tmpData.strTotalInCoin = itUserAsset->strTotalInCoin;
                    tmpData.strTotalOutCoin = itUserAsset->strTotalOutCoin;
                    tmpData.strFreezeVol = itUserAsset->strFreezeVol;
                    tmpData.strLockVol = itUserAsset->strLockVol;
                    tmpData.strWithdrawFreeze = itUserAsset->strWithdrawFreeze;
                    tmpData.strStatus = itWithdraw->strAuditStatus;
                    tmpData.strWithdrawAmount = itWithdraw->strAmount;
                    tmpData.strOrderId = itWithdraw->strOrderID;
                    tmpData.strPlatformFee = itWithdraw->strTranFee;
                    userAsset.lstAssetData.push_back(tmpData);
                    qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tmpData.strCoinName << tmpData.strStatus<<tmpData.strWithdrawAmount;
                }
            }
        }
    }


    __m_userAssetData.lstAssetData.insert(__m_userAssetData.lstAssetData.begin(), userAsset.lstAssetData.begin(), userAsset.lstAssetData.end());

    return NO_ERROR;
}


/**
 * 获取用户资产的流水
 *   截止目前(2019-12-17) , 因后台分库分表, 只能按照月份查询一个用户的流水
 *   所以, 从  2019-06(交易所上线)  一个月接一个月地查询用户流水
 */
int CPageManualWithdraw::PullUserFlowFromServer(const QString &strInUrl, const QString &strInUserID)noexcept(false)
{
    QString strErrMsg;

    if(strInUrl.isEmpty())
    {
        strErrMsg = QString("internal error: url is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strInUrl;
        throw runtime_error(strErrMsg.toStdString());
    }

    __m_userFlowData.lstFlowData.clear();

    QDateTime dateStart;
    dateStart.setDate(QDate(2019, 6, 1) ); //2019-06-01  Hetbi正式上线
    QDateTime dateCurrent = QDateTime::currentDateTime();

    QDateTime dateCursor = dateStart;
    for( ; dateCursor < dateCurrent ;  dateCursor = dateCursor.addMonths(1) )
    {
        int nPageCnt = 1;
        int  nPageIndex = 1;

        QString strQueryMonth = dateCursor.toString("yyyy-MM"); // 格式"2019-09"


        for(; nPageIndex <= nPageCnt; nPageIndex++ )
        {
            QByteArray byteArrayReply;
            QJsonObject joParams;
            joParams.insert("userId", strInUserID);
            joParams.insert("pageIndex", nPageIndex);
            joParams.insert("pageSize", 100);
            joParams.insert("queryMonth", strQueryMonth);
            CAMMain::SyncPostByJson(strInUrl, joParams, byteArrayReply); //调用父类方法进行post请求
            __ParseUserFlowData(byteArrayReply,  &nPageCnt);
        }

    }
    return NO_ERROR;
}



int CPageManualWithdraw::SearchManualWithdrawOrder(const QString &strCoinType, const QString &strDstAddr, vector<NS_Withdraw::WithdrawData> &vctManualWithdrawData) noexcept(false)
{
    QString strErrMsg;
    (void)strDstAddr;

    //1.判断查询条件

    if(strCoinType.isEmpty() ||
        (!boost::all( strCoinType.toStdString(), boost::is_alpha())
            && !utils::IsERC20Token(strCoinType)
            && !utils::Is_HRC20_Token(strCoinType)
            )
        )
    {
        strErrMsg = QString("coin type is empty or name contains illegal characters .");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //2.组装sql语句
    QString strSql;
    strSql  = QString("select coin_type, order_id, dst_addr, src_addr, amount, tx_fee from tb_auto_withdraw where coin_type='%1' "
                      "and withdraw_way=1 and audit_status=1 and tx_status=0;").arg(strCoinType);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询sql语句: ") << strSql;

    //3.查询数据库
    QSqlQuery  query(_m_db);
    if(!query.exec(strSql))
    {
        strErrMsg = QString("sql error: %1.").arg(query.lastError().text());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //5.组装返回数据
    __m_vctManualWithdrawData.clear();
    while(query.next())
    {
        int iCol = 0;
        NS_Withdraw::WithdrawData  withdrawData;
        withdrawData.strCoinType = query.value(iCol++).toString();
        withdrawData.strOrderID = query.value(iCol++).toString();
        withdrawData.strDstAddr = query.value(iCol++).toString();
        withdrawData.strSrcAddr = query.value(iCol++).toString();
        withdrawData.strAmount = query.value(iCol++).toString();
        withdrawData.strTxFee = query.value(iCol++).toString();

        vctManualWithdrawData.push_back(withdrawData);
        __m_vctManualWithdrawData.push_back(withdrawData);
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询人工审核提币通过数据成功";
    return NO_ERROR;
}

int CPageManualWithdraw::CreateUnsignedRawTx(const QString &strCoinType, const vector<int> &vctRowsSel, const QString &strSrcAddr) noexcept(false)
{
    QString strErrMsg;

    //BTC
    __m_btcExportData.bIsCollection = false;
    __m_btcExportData.strCoinType = strCoinType.trimmed().toUpper();
    __m_btcExportData.uTimestamp = QDateTime::currentMSecsSinceEpoch();
    __m_btcExportData.vctExportItems.clear(); //清空, 每次都清空

#if defined(BTC_SERIES_EX)
    __m_btcExportRawTxParams.clear();
    __m_btcExportRawTxParams.m_bIsCollection = false;
    __m_btcExportRawTxParams.m_strFilePath = "";
    __m_btcExportRawTxParams.m_strCoinType = "BTC";
#endif


    //USDT
    __m_usdtExportData.bIsCollection = false;
    __m_usdtExportData.strCoinType = strCoinType.trimmed().toUpper();
    __m_usdtExportData.uTimestamp = QDateTime::currentMSecsSinceEpoch();
    __m_usdtExportData.vctExportItems.clear(); //清空, 每次都清空

    //ETH  & ETC
    __m_ethExportRawTxParams.clear();  //清空

    //USDP/HTDF & HRC20
    __m_cosmosExportRawTxParams.clear();//清空

    //XRP
    __m_xrpExportRawTxParams.clear();


    //EOS
    __m_eosExportRawTxParams.clear();

    //XLM
    __m_xlmExportRawTxParams.clear();

    //TRX
    __m_trxExportRawTxParams.clear();

    //XMR
    __m_xmrExportRawTxParams.clear();



    if(0 == strCoinType.compare("BTC", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
#if defined(BTC_SERIES_EX)
        if(vctRowsSel.size() > 1)
        {
            strErrMsg = QString("BTC暂不支持批量提币.");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        for(auto iRow : vctRowsSel)
        {
            rawtx::btc::BTC_ExportRawTxItem expItem;
            expItem.strCoinType = strCoinType;
            expItem.strAmount = __m_vctManualWithdrawData[iRow].strAmount;
            expItem.strOrderID = __m_vctManualWithdrawData[iRow].strOrderID;
            expItem.strTxFee = __m_vctManualWithdrawData[iRow].strTxFee;
            expItem.strRawTxHex = "";
            expItem.vctDstAddr.push_back( __m_vctManualWithdrawData[iRow].strDstAddr );
            expItem.vctSrcAddr.push_back( strSrcAddr ); //源地址使用一个
            expItem.strTokenAmount = "";
            expItem.uTokenId = 0;
            expItem.bComplete = false;


            rawtx::btc::BTC_CreateRawTxParam  createParam;
            createParam.strCoinType = strCoinType;
            createParam.strURL = g_qstr_WalletAPI_btc_createrawtransaction;
            createParam.vctSrcAddr = expItem.vctSrcAddr;
            createParam.vctDstAddr = expItem.vctDstAddr;
            createParam.dAmount = expItem.strAmount.toDouble();
            createParam.mapTransfer.insert(std::make_pair( __m_vctManualWithdrawData[iRow].strDstAddr, createParam.dAmount));

            __m_btcRawTxUtilEx.CreateRawTx( &createParam );

            //创建成功, 则导出数据
            expItem.strRawTxHex = createParam.strRawTxHex;
            expItem.mapTxOut = createParam.mapTxOut;
            expItem.vctRefUTXOs = createParam.vctRefUTXOs;

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("CreateRawTx successed.");

            __m_btcExportRawTxParams.push_back( expItem);
        }
#else

        for(auto iRow : vctRowsSel)
        {
            BTCExportItem btcExportItem;
            btcExportItem.strAmount = __m_vctManualWithdrawData[iRow].strAmount;
            btcExportItem.strOrderID = __m_vctManualWithdrawData[iRow].strOrderID;
            btcExportItem.strTxFee = __m_vctManualWithdrawData[iRow].strTxFee;
            btcExportItem.strTxHex = "";
            btcExportItem.vctDstAddr.push_back(__m_vctManualWithdrawData[iRow].strDstAddr);
            btcExportItem.vctSrcAddr.push_back(strSrcAddr); //2019-05-22 by yqq  源地址使用一个


            QString strURL = "";
            QString strSrcAddr = btcExportItem.vctSrcAddr[0];
            QString strDstAddr = btcExportItem.vctDstAddr[0];
            double dAmount = btcExportItem.strAmount.toDouble();

            if(0 == strCoinType.compare("BTC", Qt::CaseInsensitive) )
            {
                strURL =  g_qstr_WalletAPI_btc_createrawtransaction;
                CBTCRawTxUtil().CreateRawTransaction(strURL, strSrcAddr, strDstAddr, dAmount, btcExportItem.strTxHex, btcExportItem.mapTxOut, btcExportItem.vctUTXOs);
            }
            else if(0 == strCoinType.compare("LTC", Qt::CaseInsensitive))
            {
                strURL =  g_qstr_WalletAPI_ltc_createrawtransaction;
                CLTCRawTxUtil().CreateRawTransaction(strURL, strSrcAddr, strDstAddr, dAmount, btcExportItem.strTxHex, btcExportItem.mapTxOut, btcExportItem.vctUTXOs);
            }
            else if(0 == strCoinType.compare("BCH", Qt::CaseInsensitive) )
            {
                strURL =  g_qstr_WalletAPI_bch_createrawtransaction;
                CBCHRawTxUtil().CreateRawTransaction(strURL, strSrcAddr, strDstAddr, dAmount, btcExportItem.strTxHex, btcExportItem.mapTxOut, btcExportItem.vctUTXOs);
            }
            else if(0 == strCoinType.compare("BSV", Qt::CaseInsensitive) )
            {
                strURL =  g_qstr_WalletAPI_bsv_createrawtransaction;
                CBSVRawTxUtil().CreateRawTransaction(strURL, strSrcAddr, strDstAddr, dAmount, btcExportItem.strTxHex, btcExportItem.mapTxOut, btcExportItem.vctUTXOs);
            }
            else if(0 == strCoinType.compare("DASH", Qt::CaseInsensitive) )
            {
                strURL =  g_qstr_WalletAPI_dash_createrawtransaction;
                CDASHRawTxUtil().CreateRawTransaction(strURL, strSrcAddr, strDstAddr, dAmount, btcExportItem.strTxHex, btcExportItem.mapTxOut, btcExportItem.vctUTXOs);
            }



            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("创建成功");
            __m_btcExportData.vctExportItems.push_back(btcExportItem);
        }
#endif
    }
    else if(0 == strCoinType.compare("USDT", Qt::CaseInsensitive))
    {
        __m_usdtExportData = USDTExportData(); //清空历史信息


        __m_usdtExportData.bIsCollection = false;
        __m_usdtExportData.strCoinType = "USDT";
        __m_usdtExportData.uTimestamp = QDateTime::currentSecsSinceEpoch();
        for( auto iSel : vctRowsSel )
        {
            USDTExportItem  item;
            item.bComplete = false;
            item.strOrderID = __m_vctManualWithdrawData[iSel].strOrderID;
            item.strTokenAmount = __m_vctManualWithdrawData[iSel].strAmount; //注意是  tokenAmount 代表的是 USDT
            item.vctDstAddr.push_back(__m_vctManualWithdrawData[iSel].strDstAddr);
            item.vctSrcAddr.push_back(strSrcAddr.trimmed() );

            __m_usdtExportData.vctExportItems.push_back(item);
        }

        __m_usdtRawTxUtil.CreateRawTransaction(g_qstr_WalletHttpIpPort + STR_usdt_createrawtransaction, strSrcAddr,  __m_usdtExportData);

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("创建成功");
    }
    else if(0 == strCoinType.compare("ETH", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
       || 0 == strCoinType.compare("ETC", Qt::CaseInsensitive)
        )
    {
        std::map<QString, double> mapBalances;
        __m_ethRawTxUtils.GetBalance(strSrcAddr, mapBalances, strCoinType);
        const double dMinEth = 0.05;

        double dBalance = mapBalances.at(strCoinType);

        double dSumAmount = 0.0;
        for(auto iRow : vctRowsSel)
        {
            bool bOk = false;
            double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.toDouble( &bOk );
            if(!bOk)
            {
                strErrMsg = QString("internal error: amount is invalid double, please check it.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            dSumAmount += ( utils::IsERC20Token(strCoinType) ? dTmpAmount : ( dTmpAmount + dMinEth ) );
        }

        if(dBalance < dSumAmount)
        {
            strErrMsg = QString::asprintf("operation error: The balance of source-address is not enough.\
                                          You selected %d addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                          vctRowsSel.size(), dSumAmount, dBalance);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //TODO: 判断 ETH的余额是否足够手续费 2019-09-04
        //如果是ERC20代币, 需要判断出币地址的ETH 是否足以支付手续费
        if( utils::IsERC20Token(strCoinType) )
        {
            double dETHBalance = mapBalances.at("ETH");
            if(dETHBalance < dMinEth * vctRowsSel.size())
            {
                strErrMsg = QString::asprintf("operation error: The ETH balance of source-address is not enough to pay tx-fee.\
                                          You selected %d addresses to make ERC20 transaction. It totally need %.8fETH to pay transaction fee , \
                                            but ETH balance is %.8f ETH. Please get some ETH first.",
                                          vctRowsSel.size(), dMinEth * vctRowsSel.size(), dETHBalance);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }

        QString strGasPrice = __m_ethRawTxUtils.GetGasPrice(); //获取实时的gasPrice
        //std::string cstrGasPrice = strGasPrice.toStdString();


        uint64_t uNonceStart = ULONG_MAX;
        __m_ethRawTxUtils.GetNonce(strSrcAddr, uNonceStart, strCoinType);  //在线获取nonce


        for(auto iRow : vctRowsSel)
        {
            //2019-04-29 yqq  添加以太坊人工审核提币
            rawtx::ETH_ExportRawTxItem expItem;
            expItem.uChainId            =  (g_bBlockChainMainnet) ? (eth::ETHChainID::Mainnet) : ( eth::ETHChainID::Rinkeby);
            if(g_bBlockChainMainnet  && (0 == strCoinType.compare("ETC", Qt::CaseInsensitive)))
                expItem.uChainId            =  (eth::ETHChainID::EthereumClassic);


            expItem.strAddrFrom         =  strSrcAddr;                                  //源地址
            expItem.strSymbol           = strCoinType.toUpper();                        //symbol, ETH为ETH, ERC20为其代币简称
            if(utils::IsERC20Token(strCoinType))
            {
                QString strGasLimit = utils::GetGasLimit(strCoinType);

                expItem.strAddrTo       = utils::GetERC20ContractAddr(strCoinType);     //ERC20合约的地址, 注意:是合约的地址
                expItem.strERC20Recipient = __m_vctManualWithdrawData[iRow].strDstAddr; //ERC20代币的接收方地址, 注意: 不是 to地址
                expItem.strValue        = "0";                                          //金额 , ERC20代币此值为 "0"
                expItem.strERC20TokenValue = __m_vctManualWithdrawData[iRow].strAmount; //ERC20 代币金额
                expItem.strGasPrice     = strGasPrice;                                //gasprice, ERC20为 4Gwei
                expItem.strGasStart     = strGasLimit;                                     //gasstart, ERC20 为 15万
                expItem.strOrderId      = __m_vctManualWithdrawData[iRow].strOrderID;   //订单编号
            }
            else //ETH  ETC
            {
                expItem.strAddrFrom     =  strSrcAddr;                                  //源地址
                expItem.strAddrTo       = __m_vctManualWithdrawData[iRow].strDstAddr;   //目的地址
                expItem.strValue        = __m_vctManualWithdrawData[iRow].strAmount;    //金额
                expItem.strERC20TokenValue = "";                                        //ETH 不用填此字段
                expItem.strERC20Recipient = "";                                         //ETH 不用填此字段
                expItem.strGasPrice     = strGasPrice;                                 //gasprice
                expItem.strGasStart     = STR_NORMAL_ETH_TX_GASLIMIT;                                      //gasstart
                expItem.strOrderId      = __m_vctManualWithdrawData[iRow].strOrderID;   //订单编号
            }


            /*TODO:
            *获取nonce值, 目前, 暂时支持一次处理一笔提币订单,
            *后期优化的时候, 增加一个nonce管理模块
            *专门负责以太坊的nonce管理, 彼时, 就支持一次处理多笔提币订单数据了
            *
            * 关于nonce管理模块的设计:
            *   1.根据地址进行nonce管理
            *   2.nonce不能再由其他模块自行在线获取nonce, 只能通过nonce管理模块提供的接口获取
            *   3.nonce需谨慎小心, 防止nonce过大, 或过小(过小会导致交易广播时发生错误)
            */

            /*uint64_t uNonce = ULONG_MAX;
            __m_ethRawTxUtils.GetNonce(expItem.strAddrFrom, uNonce);*/  //在线获取nonce

            expItem.strNonce = QString::asprintf("%llu", uNonceStart++);//nonce值, TODO:优化nonce管理

            __m_ethExportRawTxParams.push_back(expItem);
            __m_ethExportRawTxParams.m_bIsCollection = false;
            __m_ethExportRawTxParams.m_strCoinType = strCoinType.toUpper();

            QString strTmpCoinType = (0 == strCoinType.compare("ETC", Qt::CaseInsensitive)) ? ("ETC") : ("ETH");
            QString strTmpStr = "";
            if(utils::IsERC20Token(strCoinType))
            {
                if(strCoinType.contains("ERC20", Qt::CaseInsensitive)) strTmpStr = strCoinType;
                else strTmpStr = "ERC20-" + strCoinType;
            }
            strTmpStr = strTmpCoinType + "_" + strTmpStr + "_withdraw_";

            __m_ethExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_")
                                    + strTmpStr  + "_export_unsigned_Rawtx.json";
        }
    }
    else if(0 == strCoinType.compare("USDP", Qt::CaseInsensitive) || 0 == strCoinType.compare("HTDF", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive)    )
    {

        //usdp 和 htdf 完全一样处理, 至于, 混乱的问题, 解决方案是:
        //  添加限制, 一次处理, 只能处理一个币种

        //获取账户信息  sequence , account_number
        rawtx::CosmosAccountInfo cosAccountInfo;
        cosAccountInfo.strCoinType = strCoinType.trimmed();
        cosAccountInfo.strAddress = strSrcAddr.trimmed();
        __m_cosmosRawTxUtils.GetAccountInfo( cosAccountInfo );

        //支持批量转账-计算转账总金额
        double dSumAmount = 0.0;
        for(auto iRow : vctRowsSel)
        {
            bool bOk = false;
            double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.trimmed().toDouble( &bOk );
            if(!bOk)
            {
                strErrMsg = QString("internal error: amount is invalid double, please check it.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            dSumAmount += dTmpAmount + 0.03/*手续费*/;
        }

        if(dSumAmount  > cosAccountInfo.dBalance)
        {
            strErrMsg = QString::asprintf("operation error: The balance of source-address is not enough.\
                                          You selected %d addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                          vctRowsSel.size(), dSumAmount, cosAccountInfo.dBalance);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        //seq起始值
        uint64_t uSeqeuenceStart = cosAccountInfo.uSequence;

        for(auto iRow : vctRowsSel)
        {

            bool isHTDF =  (0 == strCoinType.compare(STR_HTDF, Qt::CaseInsensitive));

            //usdp 和 htdf 完全一样处理, 至于, 混乱的问题, 解决方案是:
            //  添加限制, 一次处理, 只能处理一个币种
            rawtx::Cosmos_ExportRawTxItem expItem;
            expItem.strOrderId      = __m_vctManualWithdrawData[iRow].strOrderID.trimmed();
            expItem.strCoinType     = __m_vctManualWithdrawData[iRow].strCoinType.toLower().trimmed();
            expItem.strChainId      = (g_bBlockChainMainnet) ? (STR_MAINCHAIN) : (STR_TESTCHAIN); //TODO:可配置
            expItem.strFrom         = strSrcAddr.trimmed();//2019-05-22 by yqq  源地址使用一个
            expItem.strTo           = __m_vctManualWithdrawData[iRow].strDstAddr.trimmed();
            expItem.strValue        = __m_vctManualWithdrawData[iRow].strAmount.trimmed();
            expItem.strFeeAmount    = (isHTDF) ? ("100") : ("20") ; //satoshi
            expItem.strFeeGas       = (isHTDF) ? ("30000") : ("200000");  //satoshi   TODO:可配置
            expItem.strMemo         = __m_vctManualWithdrawData[iRow].strOrderID.trimmed();  //默认使用订单号作为memo



            /*
            //获取账户信息  sequence , account_number
            rawtx::CosmosAccountInfo cosAccountInfo;
            cosAccountInfo.strCoinType = expItem.strCoinType.toLower().trimmed();
            cosAccountInfo.strAddress = expItem.strFrom.trimmed();
            __m_cosmosRawTxUtils.GetAccountInfo( cosAccountInfo );

            //判断余额 , 必须满足   余额 > 转账金额 + 手续费    //使用 usdp作为单位
            double dTmpValue = expItem.strValue.toDouble() + expItem.strFeeAmount.toDouble() / 100000000.0 ;
            if(!(cosAccountInfo.dBalance > dTmpValue )) //使用 usdp作为单位
            {
                QString strErrMsg = QString("source-address's balance is not enough. current balance is %1 , less than %2").arg(cosAccountInfo.dBalance).arg(dTmpValue);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            expItem.strSequence = QString::asprintf("%llu", cosAccountInfo.uSequence) ;
            expItem.strAccountNumber = QString::asprintf("%llu", cosAccountInfo.uAccountNumber);
            */

            expItem.strSequence = QString::asprintf("%u", uSeqeuenceStart++ ) ; //sequence自增 .  需要确保sequence小的值先广播
            expItem.strAccountNumber = QString::asprintf("%u", cosAccountInfo.uAccountNumber);

            __m_cosmosExportRawTxParams.m_bIsCollection = false;
            __m_cosmosExportRawTxParams.m_strCoinType =  expItem.strCoinType.toLower().trimmed();
            __m_cosmosExportRawTxParams.push_back(expItem);
            __m_cosmosExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                    QString("%1_withdraw_export_unsigned_RawTx.json").arg(expItem.strCoinType.toLower().trimmed());
        }
    }
    else if(utils::Is_HRC20_Token(strCoinType))
    {
        //检查  HRC20 打币余额是否足够
        rawtx::CCosmosRawTxUtilsImpl  cosUtil;
        std::map<QString , double> mapBalancesOut;
        cosUtil.GetBalance_HRC20(  strSrcAddr.trimmed(), mapBalancesOut );
        double  dBalance = mapBalancesOut.at( strCoinType  ); //如果不存在,抛异  常

        //支持批量转账-计算转账总金额
        double dSumAmount = 0.0;
        for(auto iRow : vctRowsSel)
        {
            bool bOk = false;
            double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.trimmed().toDouble( &bOk );
            if(!bOk)
            {
                strErrMsg = QString("internal error: amount is invalid double, please check it.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            dSumAmount += dTmpAmount + 0.1;
        }

        if(dSumAmount  > dBalance )
        {
            strErrMsg = QString::asprintf("operation error: The HRC20 Token %s balance of source-address is not enough.\
                                          You selected %d addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                          strCoinType.toStdString().c_str(), vctRowsSel.size(), dSumAmount, dBalance);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //判断 HTDF(支付手续费) 余额是否足够
        //获取账户信息  sequence , account_number
        rawtx::CosmosAccountInfo cosAccountInfo;
        cosAccountInfo.strCoinType = "HTDF" ; // 这里获取HTDF账户信息
        cosAccountInfo.strAddress = strSrcAddr.trimmed();
        __m_cosmosRawTxUtils.GetAccountInfo( cosAccountInfo );
        double dSumFee = vctRowsSel.size() * (0.20 + 0.01);
        if( cosAccountInfo.dBalance < dSumFee )
        {
            strErrMsg = QString::asprintf("operation error: The HTDF balance of source-address is not enough.\
                                          You selected %d addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                          vctRowsSel.size(), dSumFee, cosAccountInfo.dBalance);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //seq起始值
        uint64_t uSeqeuenceStart = cosAccountInfo.uSequence;
        for(auto iRow : vctRowsSel)
        {
            //usdp 和 htdf 完全一样处理, 至于, 混乱的问题, 解决方案是:
            //  添加限制, 一次处理, 只能处理一个币种
            rawtx::Cosmos_ExportRawTxItem expItem;
            expItem.strOrderId      = __m_vctManualWithdrawData[iRow].strOrderID.trimmed();
            expItem.strCoinType     = __m_vctManualWithdrawData[iRow].strCoinType.toLower().trimmed();
            expItem.strChainId      = (g_bBlockChainMainnet) ? (STR_MAINCHAIN) : (STR_TESTCHAIN);
            expItem.strFrom         = strSrcAddr.trimmed();//2019-05-22 by yqq  源地址使用一个

            //特别注意:  这里是 token_recipient 即代币接收方 ,在离线端创建交易时再使用
            // 合约地址作为 to,  此字段做 token_recipient   2020-04-16  yqq
            expItem.strTo           = __m_vctManualWithdrawData[iRow].strDstAddr.trimmed();

            //注意: 同上, 这里是  token_amount, 代币转账金额,   在离线端创建交易时再进行替换
            expItem.strValue        = __m_vctManualWithdrawData[iRow].strAmount.trimmed();

            expItem.strFeeAmount    = "100"; //satoshi
            expItem.strFeeGas       = "200000";  //satoshi
            expItem.strMemo         = __m_vctManualWithdrawData[iRow].strOrderID.trimmed();  //默认使用订单号作为memo

            expItem.strSequence = QString::asprintf("%u", uSeqeuenceStart++ ) ; //sequence自增 .  需要确保sequence小的值先广播
            expItem.strAccountNumber = QString::asprintf("%u", cosAccountInfo.uAccountNumber);

            //注意:  此处并没有  Data 字段,  在离线端创建交易是进行构造

            __m_cosmosExportRawTxParams.m_bIsCollection = false;
            __m_cosmosExportRawTxParams.m_strCoinType =  expItem.strCoinType.toLower().trimmed();
            __m_cosmosExportRawTxParams.push_back(expItem);
            __m_cosmosExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                    QString("HRC20_%1_withdraw_export_unsigned_RawTx.json").arg(expItem.strCoinType.toLower().trimmed());
        }

    }
    else if(0 == strCoinType.compare("XRP", Qt::CaseInsensitive))
    {
        //1.检查账户余额是否足够
        //获取sequence 和 balance
        rawtx::XrpAccountInfo  accInfo;
        accInfo.strAddress = strSrcAddr; //源地址
        __m_xrpRawTxUtils.GetAccountInfo(accInfo);

        //支持批量转账-计算转账总金额
        double dSumAmount = 0.0;
        for(auto iRow : vctRowsSel)
        {
            bool bOk = false;
            double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.trimmed().toDouble( &bOk );
            if(!bOk)
            {
                strErrMsg = QString("internal error: amount is invalid double, please check it.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            dSumAmount += dTmpAmount +0.000100;/*手续费*/
        }

        if(dSumAmount  > accInfo.dBalanceInXRP)
        {
            strErrMsg = QString::asprintf("operation error: The balance of source-address is not enough.\
                                          You selected %d addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                          vctRowsSel.size(), dSumAmount, accInfo.dBalanceInXRP);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //2.构造导出参数,存入缓冲区
        //seq起始值
        uint64_t uSeqeuenceStart = accInfo.uSequqnce;

        for(auto iRow : vctRowsSel)
        {
            NS_Withdraw::WithdrawData const &refWithdrawItem =  __m_vctManualWithdrawData[iRow];

            rawtx::Xrp_ExportRawTxItem  expItem;
            expItem.bComplete = false;
            expItem.dAmountInXRP = refWithdrawItem.strAmount.toDouble();
            expItem.strAmountInDrop =  QString::asprintf("%llu",  (uint64_t)(expItem.dAmountInXRP * 1000000)); // 以drop为单位,   1XRP  = 1000000 drop
            expItem.strChainNet = STR_XRP_CHAINNET;
            expItem.strCoinType = refWithdrawItem.strCoinType;
            expItem.strOrderId = refWithdrawItem.strOrderID;
            expItem.strSignedRawTx = "";
            expItem.strSrcAddr = strSrcAddr; //不是 refWithdrawItem.strSrcAddr
            expItem.uFeeInDrop = UINT_XRP_DEFAULT_FEE_IN_DROP;//(uint64_t)(refWithdrawItem.strTxFee.toDouble() * 1000000);
            expItem.uSequence =  uSeqeuenceStart++;
            expItem.strDstAddr =  rawtx::CXrpRawTxUtilsImpl::GetAddrFrom_Addr_Tag( refWithdrawItem.strDstAddr); //需要处理标签
            expItem.uDestinationTag =  rawtx::CXrpRawTxUtilsImpl::GetTagFrom_Addr_Tag( refWithdrawItem.strDstAddr );

            __m_xrpExportRawTxParams.push_back( expItem );
        }

        //3.设置导出文件信息
        __m_xrpExportRawTxParams.m_bIsCollection = false;
        __m_xrpExportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_xrpExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("%1_withdraw_export_unsigned_RawTx.json").arg(__m_xrpExportRawTxParams.m_strCoinType);

    }
    else if(0 == strCoinType.compare("EOS", Qt::CaseInsensitive))
    {
        //1.先判断余额, 和 资源是否足够
        if(true)
        {
            //支持批量转账-计算转账总金额
            double dSumAmount = 0.0;
            for(auto iRow : vctRowsSel)
            {
                bool bOk = false;
                double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.trimmed().toDouble( &bOk );
                if(!bOk)
                {
                    strErrMsg = QString("internal error: amount is invalid double, please check it.");
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
                dSumAmount += dTmpAmount;
            }


            //检查余额和资源是否足够
            //rawtx::CEosRawTxUtilsImpl eosUtil;
            rawtx::EosAccountInfo  accInfo;
            accInfo.strAccountName  = strSrcAddr.trimmed();
            __m_eosRawTxUtils.GetAccountInfo( accInfo );

            if(  accInfo.dEOSBalance < dSumAmount )
            {
                strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                               You selected 1 addresses to make transaction. It totally need %.4f , but balance is %.4f.",
                                               dSumAmount , accInfo.dEOSBalance);

                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString() );
            }

            if(  accInfo.uCPU_Avail < 1000 * vctRowsSel.size() )
            {
                strErrMsg = QString::asprintf("CPU NOT ENOUGH: The CPU time of source-address is not enough. CPU_total: %1, \
                                                CPU_used: %2 , CPU_avail:%3", accInfo.uCPU_Total, accInfo.uCPU_Used , accInfo.uCPU_Avail);

                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString() );
            }

            if( accInfo.uNET_Avail < 1000 * vctRowsSel.size())
            {
                strErrMsg = QString::asprintf("CPU NOT ENOUGH: The NET  of source-address is not enough. NET_total: %1, \
                                                NET_used: %2 , NET_avail:%3", accInfo.uNET_Total, accInfo.uNET_Used , accInfo.uNET_Avail);

                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString() );
            }

            if( accInfo.uRAM_Total - accInfo.uRAM_Used  < 4000)
            {
                strErrMsg = QString::asprintf("RAM NOT ENOUGH: The RAM of source-address is not enough. RAM_total: %1, \
                                                RAM_used: %2 , RAM_avail:%3", accInfo.uRAM_Total, accInfo.uRAM_Used , accInfo.uRAM_Total - accInfo.uRAM_Used);

                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString() );
            }

        }

        //2.调用服务端接口创建未签名交易

        for( auto iRow : vctRowsSel )
        {
            NS_Withdraw::WithdrawData const &refWithdrawItem =  __m_vctManualWithdrawData[iRow];


            //2.1 构造请求参数
            rawtx::Eos_CreateRawTxParam  param;
            param.bSigned           = false;
            param.strCoinType       = refWithdrawItem.strCoinType;
            param.strChainNet       = STR_EOS_CHAINNET;
            param.strSrcAcct        = strSrcAddr.trimmed();
            param.strTxDigestHex    = "";
            param.strSignedRawTx    = "";
            param.dAmount           = refWithdrawItem.strAmount.toDouble();
            param.strAmount         = refWithdrawItem.strAmount;

            //地址和 memo
            param.strDstAcct        = utils::GetXrpAddrFrom_Addr_Tag( refWithdrawItem.strDstAddr);
            param.strMemo           = utils::GetTagFrom_Addr_Tag( refWithdrawItem.strDstAddr );


            __m_eosRawTxUtils.CreateRawTx( &param );


            //2.2 放入导出缓冲区

            rawtx::Eos_ExportRawTxItem  expItem;
            expItem.bComplete = false;
            expItem.dAmount = refWithdrawItem.strAmount.toDouble();
            expItem.strAmount =  refWithdrawItem.strAmount;
            expItem.strChainNet = STR_EOS_CHAINNET;
            expItem.strCoinType = refWithdrawItem.strCoinType;
            expItem.strSrcAcct = strSrcAddr.trimmed();
            expItem.strDstAcct = param.strDstAcct;
            expItem.strMemo = param.strMemo;
            expItem.strOrderId = refWithdrawItem.strOrderID;

            expItem.strSignedRawTx = param.strSignedRawTx; //未签名交易json字符串
            expItem.strTxDigestHex = param.strTxDigestHex; //交易摘要

            __m_eosExportRawTxParams.push_back(expItem);

        }

        __m_eosExportRawTxParams.m_bIsCollection = false;
        __m_eosExportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_eosExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("%1_withdraw_export_unsigned_RawTx.json").arg(__m_eosExportRawTxParams.m_strCoinType);

    }
    else if( 0 == strCoinType.compare("XLM", Qt::CaseInsensitive))
    {
        //1.检查账户余额是否足够
        //获取sequence 和 balance
        rawtx::XlmAccountInfo  accInfo;
        accInfo.strAddress = strSrcAddr; //源地址
        __m_xlmRawTxUtils.GetAccountInfo(accInfo);

         //支持批量转账-计算转账总金额
        double dSumAmount = 0.0;
        for(auto iRow : vctRowsSel)
        {
            bool bOk = false;
            double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.trimmed().toDouble( &bOk );
            if(!bOk)
            {
                strErrMsg = QString("internal error: amount is invalid double, please check it.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            dSumAmount += dTmpAmount +0.000100;/*手续费*/
        }

        if(dSumAmount  > accInfo.dBalance)
        {
            strErrMsg = QString::asprintf("operation error: The balance of source-address is not enough.\
                                          You selected %d addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                          vctRowsSel.size(), dSumAmount, accInfo.dBalance);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //2.构造导出参数,存入缓冲区
        //seq起始值
        uint64_t uSeqeuenceStart = accInfo.uSequence;

        //注意:  如果一个尚不存在的地址, 有多笔人工提币
        //那么, 第一笔提币要使用 createoperation, 之后的提币交易使用 paymentoperation
        std::map<QString, bool>  mapDstAccountFlag;

        for(auto iRow : vctRowsSel)
        {
            NS_Withdraw::WithdrawData const &refWithdrawItem =  __m_vctManualWithdrawData[iRow];


            rawtx::Xlm_ExportRawTxItem  expItem;
            expItem.bComplete = false;
            expItem.dAmount = refWithdrawItem.strAmount.toDouble();
            expItem.strAmount =  QString::asprintf("%.7f",  expItem.dAmount); //
            expItem.strChainNet = STR_XLM_CHAINNET;
            expItem.strCoinType = refWithdrawItem.strCoinType;
            expItem.strOrderId = refWithdrawItem.strOrderID;
            expItem.strSignedRawTx = "";
            expItem.strSrcAddr = strSrcAddr; //不是 refWithdrawItem.strSrcAddr
            expItem.uBaseFee= 100 ;
            expItem.uSequence =  uSeqeuenceStart++;
            expItem.strDstAddr =  utils::GetXrpAddrFrom_Addr_Tag( refWithdrawItem.strDstAddr); //需要处理标签
            expItem.strMemo =  utils::GetTagFrom_Addr_Tag( refWithdrawItem.strDstAddr );

            rawtx::XlmAccountInfo  dstAccInfo;
            dstAccInfo.strAddress = expItem.strDstAddr; //目的地址
            __m_xlmRawTxUtils.GetAccountInfo(dstAccInfo);
            if(!dstAccInfo.bFound) //目的账户不存在
            {
                if(mapDstAccountFlag.end() == mapDstAccountFlag.find(expItem.strDstAddr))
                {
                    mapDstAccountFlag.insert( std::make_pair( expItem.strDstAddr, false) );
                    expItem.bDstAccountExists = false; //使用 createaccountoperation创建账户
                }
                else
                {
                    expItem.bDstAccountExists = true;  //如果之前已经有过  createaccountoperation, 之后则默认账户已经存在, 故而, 使用paymentopera
                }
            }
            else //目的账户存在
            {
                expItem.bDstAccountExists = true;
            }

            __m_xlmExportRawTxParams.push_back( expItem );
        }

         __m_xlmExportRawTxParams.m_bIsCollection = false;
        __m_xlmExportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_xlmExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("%1_withdraw_export_unsigned_RawTx.json").arg(__m_xlmExportRawTxParams.m_strCoinType);


    }
    else if(0 == strCoinType.compare("TRX", Qt::CaseInsensitive))
    {
        //1. 余额判断
        if(true)
        {
            //支持批量转账-计算转账总金额
            double dSumAmount = 0.0;
            for(auto iRow : vctRowsSel)
            {
                bool bOk = false;
                double dTmpAmount = __m_vctManualWithdrawData[iRow].strAmount.trimmed().toDouble( &bOk );
                if(!bOk)
                {
                    strErrMsg = QString("internal error: amount is invalid double, please check it.");
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
                dSumAmount += dTmpAmount;
            }

            rawtx::TrxAccountInfo accInfo;
            accInfo.strAddress = strSrcAddr.trimmed();
            __m_trxRawTxUtils.GetAccountInfo( accInfo );

            if(  accInfo.dBalance < dSumAmount )
            {
                strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                               You selected 1 addresses to make transaction. It totally need %.6f , but balance is %.6f.",
                                               dSumAmount , accInfo.dBalance);

                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString() );
            }
        }

        //2. 调用服务端接口创建交易(如: createrawtransaction?src_acct=xxxx&dst_acct=xxxxx&amount=0.123456 )
        for( auto iRow : vctRowsSel )
        {
            NS_Withdraw::WithdrawData const &refWithdrawItem =  __m_vctManualWithdrawData[iRow];


            //2.1 构造请求参数
            rawtx::Trx_CreateRawTxParam  param;
            param.bSigned           = false;
            param.strCoinType       = refWithdrawItem.strCoinType;
            param.strSrcAcct        = strSrcAddr.trimmed();
            param.strTxid           = "";  //交易摘要,  注意: 此处的txid 是 raw_data_hex 的hash值, 即  sha256(raw_data_hex)
            param.strSignedRawTx    = "";
            param.dAmount           = refWithdrawItem.strAmount.toDouble();
            param.strAmount         = refWithdrawItem.strAmount;
            param.strDstAcct        =  refWithdrawItem.strDstAddr;

            __m_trxRawTxUtils.CreateRawTx( &param );


            //2.2 放入导出缓冲区

            rawtx::Trx_ExportRawTxItem  expItem;
            expItem.bComplete = false;
            expItem.dAmount = refWithdrawItem.strAmount.toDouble();
            expItem.strAmount =  refWithdrawItem.strAmount;
            expItem.strCoinType = refWithdrawItem.strCoinType;
            expItem.strSrcAcct = strSrcAddr.trimmed();
            expItem.strDstAcct = param.strDstAcct;
            expItem.strOrderId = refWithdrawItem.strOrderID;

            expItem.strSignedRawTx = param.strSignedRawTx; //未签名交易json字符串
            expItem.strTxid = param.strTxid; //交易摘要

            __m_trxExportRawTxParams.push_back(expItem);

        }

        __m_trxExportRawTxParams.m_bIsCollection = false;
        __m_trxExportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_trxExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("%1_withdraw_export_unsigned_RawTx.json").arg(__m_trxExportRawTxParams.m_strCoinType);


    }
    else if(0 == strCoinType.compare("XMR", Qt::CaseInsensitive))
    {
        //获取余额?  可以直接创建, 如果能创建说明余额"应该"(前提是已经导入signed-key-images)
        //创建交易,如果创建失败说明余额不足
        //暂时只考虑   1对1 , 不考虑  1对n
        for( auto iRow : vctRowsSel   )
        {
            NS_Withdraw::WithdrawData const &refWithdrawItem =  __m_vctManualWithdrawData[iRow];

            //创建交易
            rawtx::XMR_CreateRawTxParam  param;
            param.bIsManualWithdraw = true; //是手动
            param.bSigned           = false;
            param.strCoinType       = refWithdrawItem.strCoinType;
            param.strSrcAddr        = strSrcAddr.trimmed();
            param.strDstAddr        =  refWithdrawItem.strDstAddr;
            param.dAmount           = refWithdrawItem.strAmount.toDouble();
            param.strAmount         = refWithdrawItem.strAmount;
            param.strSignedRawTx    = "";

            __m_xmrRawTxUtils.CreateRawTx( &param );

            //导出交易
            rawtx::XMR_ExportRawTxItem  expItem;
            expItem.bComplete = false;
            expItem.dAmount = refWithdrawItem.strAmount.toDouble();
            expItem.strAmount =  refWithdrawItem.strAmount;
            expItem.strCoinType = refWithdrawItem.strCoinType;
            expItem.strSrcAddr = strSrcAddr.trimmed();
            expItem.strDstAddr = param.strDstAddr;
            expItem.strOrderId = refWithdrawItem.strOrderID;
            expItem.strSignedRawTx = param.strSignedRawTx;

            __m_xmrExportRawTxParams.push_back(expItem);
        }

        __m_xmrExportRawTxParams.m_bIsCollection = false;
        __m_xmrExportRawTxParams.m_strCoinType =  strCoinType.toLower().trimmed();
        __m_xmrExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("%1_withdraw_export_unsigned_RawTx.json").arg(__m_xmrExportRawTxParams.m_strCoinType);


    }
    else //暂不支持
    {
        strErrMsg = QString("operation error:%1 is not be supported.").arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("创建未签名交易 成功");
    return  CPageManualWithdraw::NO_ERROR;
}

int CPageManualWithdraw::ExportRawTxFile(const QString &strCoinType, const QString &strExportFilePath) noexcept(false)
{
    QString strErrMsg;

    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
#if defined(BTC_SERIES_EX)
        if(0 == __m_btcExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_btcExportRawTxParams.m_bIsCollection = false;
        __m_btcExportRawTxParams.m_strCoinType = strCoinType;
        __m_btcExportRawTxParams.m_strFilePath = strExportFilePath.toLower();

        __m_btcRawTxUtilEx.ExportRawTx( &__m_btcExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("export %1 successed.").arg( strExportFilePath.toLower());
#else
        if(__m_btcExportData.vctExportItems.empty())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //导出已签名的BTC交易数据
        QString strPath = strExportFilePath;
        strPath = strPath.toLower();
        if(0 == strCoinType.compare("btc", Qt::CaseInsensitive))
        {
            CBTCRawTxUtil().ExportRawTxToJsonFile(strPath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
        {
            CLTCRawTxUtil().ExportRawTxToJsonFile(strPath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("bch", Qt::CaseInsensitive))
        {
            CBCHRawTxUtil().ExportRawTxToJsonFile(strPath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
        {
            CBSVRawTxUtil().ExportRawTxToJsonFile(strPath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
        {
            CDASHRawTxUtil().ExportRawTxToJsonFile(strPath, __m_btcExportData);
        }

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("%1 导出成功").arg(strPath);
#endif
    }
    else if ( 0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        //导出已签名USDT交易
        if(__m_usdtExportData.vctExportItems.empty())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //导出已签名的USDT交易数据
        QString strPath = strExportFilePath;
        strPath = strPath.toLower();
        __m_usdtRawTxUtil.ExportRawTxToJsonFile(strPath, __m_usdtExportData);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(strPath);
    }
    else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("etc", Qt::CaseInsensitive)
        )
    {
        if(0 == __m_ethExportRawTxParams.size() )
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_ethRawTxUtils.ExportRawTx( &__m_ethExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_ethExportRawTxParams.m_strFilePath);
    }
    else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType)
            )
    {
        if(0 == __m_cosmosExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //TODO: 对__m_cosmosExportRawTxParams中的数据进行检查, 确保其中只有一个币种
        __m_cosmosRawTxUtils.ExportRawTx( &__m_cosmosExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_cosmosExportRawTxParams.m_strFilePath);
    }
    else if(  0 == strCoinType.compare("XRP", Qt::CaseInsensitive) )
    {
        if(0 == __m_xrpExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_xrpRawTxUtils.ExportRawTx( &__m_xrpExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_xrpExportRawTxParams.m_strFilePath);

    }
    else if(  0 == strCoinType.compare("EOS", Qt::CaseInsensitive) )
    {
        if(0 == __m_eosExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_eosRawTxUtils.ExportRawTx( &__m_eosExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_eosExportRawTxParams.m_strFilePath);

    }
    else if(  0 == strCoinType.compare("XLM", Qt::CaseInsensitive) )
    {
        if(0 == __m_xlmExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_xlmRawTxUtils.ExportRawTx( &__m_xlmExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_xlmExportRawTxParams.m_strFilePath);
    }
    else if(  0 == strCoinType.compare("TRX", Qt::CaseInsensitive) )
    {
        if(0 == __m_trxExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_trxRawTxUtils.ExportRawTx( &__m_trxExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_trxExportRawTxParams.m_strFilePath);

    }
    else if(  0 == strCoinType.compare("XMR", Qt::CaseInsensitive) )
    {
        if(0 == __m_xmrExportRawTxParams.size())
        {
            strErrMsg = QString( "operation error: not found any %1 transactions, please create transaction first.").arg(strCoinType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_xmrRawTxUtils.ExportRawTx( &__m_xmrExportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("%1 导出成功").arg(__m_xmrExportRawTxParams.m_strFilePath);
    }
    else
    {
        strErrMsg = QString( "tips: do not support %1 in current version.").arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    return  CPageManualWithdraw::NO_ERROR;
}

int CPageManualWithdraw::ImportRawTxFile(const QString &strCoinType, const QString &strImportFilePath) noexcept(false)
{
    QString strErrMsg;

    QString strFilePath = strImportFilePath.trimmed();
    if(strFilePath.trimmed().isEmpty())
    {
        strErrMsg = "operation error: import file path is empty, please select json file to import fisrt.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QFile fileImport(strFilePath);
    if(!fileImport.exists())
    {
        strErrMsg = "operation error: import file is not exists, please confirm the json file is exists.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
#if defined(BTC_SERIES_EX)
        __m_btcImportRawTxParams = rawtx::btc::BTC_ImportRawTxParam();
        __m_btcImportRawTxParams.m_strCoinType = strCoinType;
        __m_btcImportRawTxParams.m_strFilePath = strImportFilePath;
        __m_btcRawTxUtilEx.ImportRawTx( &__m_btcImportRawTxParams );

#else
        //清空历史信息
        __m_btcImportData = BTCImportData();

        if(0 == strCoinType.compare("btc", Qt::CaseInsensitive))
        {
            CBTCRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
        {
            CLTCRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if(0 == strCoinType.compare("bch", Qt::CaseInsensitive))
        {
            CBCHRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if(0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
        {
            CBSVRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
        {
            CDASHRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }


        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed").arg(fileImport.fileName());
#endif
    }
    else if( 0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
    {
         //清空历史信息
         __m_usdtImportData = USDTImportData();

        __m_usdtRawTxUtil.ImportRawTxFromJsonFile(strImportFilePath, __m_usdtImportData);

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed").arg(fileImport.fileName());
    }
    else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        ||  0 == strCoinType.compare("etc", Qt::CaseInsensitive) )
    {
        //清空历史数据
        __m_ethImportRawTxParams = rawtx::ETH_ImportRawTxParam();

        //设置导入文件路径
        __m_ethImportRawTxParams.m_strFilePath = strFilePath;
        __m_ethImportRawTxParams.m_strCoinType  = strCoinType;

        //导入数据
        __m_ethRawTxUtils.ImportRawTx( &__m_ethImportRawTxParams );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType)
            )
    {
        //清空历史数据
        __m_cosmosImportRawTxParams = rawtx::Cosmos_ImportRawTxParam();

        //导入文件文件路径
        __m_cosmosImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_cosmosImportRawTxParams.m_strFilePath = strFilePath;
        __m_cosmosRawTxUtils.ImportRawTx( &__m_cosmosImportRawTxParams );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else if(  0 == strCoinType.compare("XRP", Qt::CaseInsensitive) )
    {
        __m_xrpImportRawTxParams = rawtx::Xrp_ImportRawTxParam();

        __m_xrpImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_xrpImportRawTxParams.m_strFilePath = strFilePath;
        __m_xrpRawTxUtils.ImportRawTx( &__m_xrpImportRawTxParams  );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else if(  0 == strCoinType.compare("EOS", Qt::CaseInsensitive) )
    {
        __m_eosImportRawTxParams = rawtx::Eos_ImportRawTxParam();

        __m_eosImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_eosImportRawTxParams.m_strFilePath = strFilePath;
        __m_eosRawTxUtils.ImportRawTx( &__m_eosImportRawTxParams  );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else if(  0 == strCoinType.compare("XLM", Qt::CaseInsensitive) )
    {
        __m_xlmImportRawTxParams = rawtx::Xlm_ImportRawTxParam();

        __m_xlmImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_xlmImportRawTxParams.m_strFilePath = strFilePath;
        __m_xlmRawTxUtils.ImportRawTx( &__m_xlmImportRawTxParams  );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else if(  0 == strCoinType.compare("TRX", Qt::CaseInsensitive) )
    {
        __m_trxImportRawTxParams = rawtx::Trx_ImportRawTxParam();

        __m_trxImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_trxImportRawTxParams.m_strFilePath = strFilePath;
        __m_trxRawTxUtils.ImportRawTx( &__m_trxImportRawTxParams  );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else if(  0 == strCoinType.compare("XMR", Qt::CaseInsensitive) )
    {
        __m_xmrImportRawTxParams = rawtx::XMR_ImportRawTxParam();

        __m_xmrImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_xmrImportRawTxParams.m_strFilePath = strFilePath;
        __m_xmrRawTxUtils.ImportRawTx( &__m_xmrImportRawTxParams  );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());
    }
    else //not support
    {
        strErrMsg = QString("tips: not support %1 in current version.").arg(strCoinType);
        //qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("not support cointype.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    return CPageManualWithdraw::NO_ERROR;
}


int WriteTxidIntoBakFile(const QString &strFilePath, const QString &strOrderId, const QString &strTxid)
{
    QMutex mutexTmp;
    mutexTmp.lock();

    QFile bakFile(strFilePath);
    if(!bakFile.open(QIODevice::Append))
    {
        bool bFlag = false;
        for(int i = 0; i < 100; i++)
        {
            QThread::usleep(10);
            if(bakFile.open(QIODevice::Append))
            {
                bFlag = true;
                break;
            }
        }

        if(!bFlag)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("打开备份txid文件失败");
            mutexTmp.unlock();
            return FILE_ERROR;
        }
    }

    QString strOutput = strOrderId + "\t" + strTxid + "\n";
    bakFile.write(strOutput.toLatin1());
    bakFile.close();

    mutexTmp.unlock();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("写入txid到备份文件成功");
    return NO_ERROR;
}

int ReadTxidFromBakFile(const QString &strFilePath, map<QString, QString> &mapOrderTxid)
{
    QFile bakFile(strFilePath);
    if(!bakFile.exists())
    {
        return NO_ERROR;
    }

    QMutex mutexTmp;
    mutexTmp.lock();
    if(!bakFile.open(QIODevice::ReadOnly))
    {
        bool bFlag = false;
        for(int i = 0; i < 100; i++)
        {
            QThread::usleep(10);
            if(bakFile.open(QIODevice::ReadOnly))
            {
                bFlag = true;
                break;
            }
        }

        if(!bFlag)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("打开备份txid文件失败");
            mutexTmp.unlock();
            return FILE_ERROR;
        }
    }

    while(!bakFile.atEnd())
    {
        QString strLine = bakFile.readLine();
        QStringList strLineSplit = strLine.split("\t");
        if(2 != strLineSplit.count())
            continue;

        QString strOrderId = QString(strLineSplit[0]).trimmed();
        QString strTxid = QString(strLineSplit[1]).trimmed();
        if(strOrderId.isEmpty() || strTxid.isEmpty())
            continue;

        mapOrderTxid.insert(make_pair(strOrderId, strTxid));
    }

    bakFile.close();

    mutexTmp.unlock();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("从备份文件中读取txid成功");
    return NO_ERROR;
}


/*TODO:一次仅广播一个币种的交易
 * 即需要使 vctor<int> vctRowSel 所选的的订单那数据,必须是同一币种!! 不能混!!!
 */
int CPageManualWithdraw::BroadcastRawTx(const QString &strCoinType,  const vector<int> &vctRowsSel) noexcept(false)
{
    QString strErrMsg;

    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
     )
    {
#if defined(BTC_SERIES_EX)
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_btcImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //BTCExportItem exportItem = __m_btcImportData.vctExportItems[iRow];
            rawtx::btc::BTC_ImportRawTxItem &impItem = __m_btcImportRawTxParams[iRow];
            JudgeBroadcastable(impItem.strOrderID);


            //预先判断防止数组越界
            if(0 == impItem.vctSrcAddr.size())
            {
                strErrMsg = QString("args error: orderId: %1  source address is empty").arg(impItem.strOrderID);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            rawtx::btc::BTC_BroadcastRawTxParam  bcastParam;
            bcastParam.strCoinType = impItem.strCoinType;
            bcastParam.strSignedRawTxHex = impItem.strRawTxHex;
            bcastParam.strURL = g_qstr_WalletAPI_btc_sendrawtransaction;

            __m_btcRawTxUtilEx.BroadcastRawTx( &bcastParam );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "orderId:"
                << impItem.strOrderID << QString(" txid=") << bcastParam.strTxid;

            //更新数据库, 将交易id写入数据库,    //预先判断防止数vctExportItems QSqlQuery query(_m_db);
            QSqlQuery query(_m_db);
            QString strSql = QString("UPDATE tb_auto_withdraw SET txid='%1',tx_status=%2,order_status=%3,upload_status=%4,src_addr='%5' WHERE order_id='%6';")\
                    .arg( bcastParam.strTxid).arg(1).arg(1).arg(1).arg(impItem.vctSrcAddr[0].trimmed()).arg(impItem.strOrderID);

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql : " << strSql;

            if(!query.exec(strSql))
            {
                strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                        .arg(strSql).arg(bcastParam.strTxid).arg(query.lastError().text());
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("broadcast, update db successed.");
        }
#else
        QString strURL = "";
        if(0 == strCoinType.compare("btc", Qt::CaseInsensitive))
        {
            strURL = g_qstr_WalletAPI_btc_sendrawtransaction ;
        }
        else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
        {
            strURL = g_qstr_WalletAPI_ltc_sendrawtransaction ;
        }
        else if(0 == strCoinType.compare("bch", Qt::CaseInsensitive))
        {
            strURL = g_qstr_WalletAPI_bch_sendrawtransaction ;
        }
        else if(0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
        {
            strURL = g_qstr_WalletAPI_bsv_sendrawtransaction ;
        }
        else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
        {
            strURL = g_qstr_WalletAPI_dash_sendrawtransaction ;
        }


        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_btcImportData.vctExportItems.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            BTCExportItem exportItem = __m_btcImportData.vctExportItems[iRow];
            JudgeBroadcastable(exportItem.strOrderID);


            //预先判断防止数组越界
            if(0 == __m_btcImportData.vctExportItems[iRow].vctSrcAddr.size())
            {
                strErrMsg = QString("args error: orderId: %1  source address is empty").arg(exportItem.strOrderID);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QString strTxidRet = "";

            if(0 == strCoinType.compare("btc", Qt::CaseInsensitive))
            {
                CBTCRawTxUtil().BroadcastRawTx(strURL,  exportItem.strTxHex, strTxidRet);
            }
            else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
            {
                CLTCRawTxUtil().BroadcastRawTx(strURL,  exportItem.strTxHex, strTxidRet);
            }
            else if(0 == strCoinType.compare("bch", Qt::CaseInsensitive))
            {
                CBCHRawTxUtil().BroadcastRawTx(strURL,  exportItem.strTxHex, strTxidRet);
            }
            else if(0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
            {
                CBSVRawTxUtil().BroadcastRawTx(strURL,  exportItem.strTxHex, strTxidRet);
            }
            else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
            {
                CDASHRawTxUtil().BroadcastRawTx(strURL,  exportItem.strTxHex, strTxidRet);
            }



            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<   exportItem.strTxHex << "orderId:" << exportItem.strOrderID << QString("  txid=") << strTxidRet;

            //更新数据库, 将交易id写入数据库,    //预先判断防止数vctExportItems QSqlQuery query(_m_db);
            QSqlQuery query(_m_db);
            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4,src_addr='%5' WHERE order_id='%6';")\
                    .arg(strTxidRet).arg(1).arg(1).arg(1).arg(__m_btcImportData.vctExportItems[iRow].vctSrcAddr[0].trimmed())\
                    .arg(__m_btcImportData.vctExportItems[iRow].strOrderID);

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql : " << strSql;

            if(!query.exec(strSql))
            {
                strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                        .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");
        }
#endif
    }
    else if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive) )
    {
        //3.广播交易
        // fix bug: 2020-03-30 yqq
        //fix_bug: 2020-03-30 yqq
        //如果是 USDT 则使用 USDT 自己的节点广播, 不然会导致 交易池的差异,
        //导致双花问题, 因为创建的时候使用USDT节点, 而广播的使用使用的是 USDT节点,
        //QString strURL = g_qstr_WalletAPI_btc_sendrawtransaction ;
        QString strURL = g_qstr_WalletHttpIpPort + STR_usdt_sendrawtransaction;
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_usdtImportData.vctExportItems.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            USDTExportItem exportItem = __m_usdtImportData.vctExportItems[iRow];
            JudgeBroadcastable(exportItem.strOrderID);

            //预先判断防止数组越界
            if(0 == __m_usdtImportData.vctExportItems[iRow].vctSrcAddr.size())
            {
                strErrMsg = QString("args error: orderId: %1  source address is empty").arg(exportItem.strOrderID);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QString strTxidRet = "";
            __m_usdtRawTxUtil.BroadcastRawTx(strURL, exportItem.strTxHex, strTxidRet);


            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << __m_usdtImportData.vctExportItems[iRow].strTxHex << QString("交易广播成功, txid=") << strTxidRet;



            //更新数据库, 将交易id写入数据库,    //预先判断防止数vctExportItems QSqlQuery query(_m_db);
            QSqlQuery query(_m_db);
            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4,src_addr='%5' WHERE order_id='%6';")\
                    .arg(strTxidRet).arg(1).arg(1).arg(1).arg(__m_usdtImportData.vctExportItems[iRow].vctSrcAddr[0].trimmed())\
                    .arg(__m_usdtImportData.vctExportItems[iRow].strOrderID);

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql : " << strSql;

            if(!query.exec(strSql))
            {
                strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                        .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");
        }
    }
    else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("etc", Qt::CaseInsensitive) )
    {
        //注意: vctRowSel 中的顺序要 与 __m_ethImportRawTxParams中数据顺序一致, 否则会出错
        for(auto iRow : vctRowsSel)
        {
            if((size_t)iRow >= __m_ethImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //JudgeBroadcastable(__m_ethImportRawTxParams[iRow].strOrderId);

            if(true)
            {
                QSqlQuery query(_m_db);
                if(false == query.exec(QString("select txid from tb_auto_withdraw where order_id='%1' and length(txid)=66;").arg(__m_ethImportRawTxParams[iRow].strOrderId)))
                {
                    strErrMsg =  QString("DB error: query database error. haven't broadcast yet.");
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                if(0 < query.size()/* && 66 == query.value(0).toString().length()*/)
                {
                    strErrMsg =  QString("debug info: order_id:%1 had  successfuly broadcated . so skip it now.");
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    continue;
                }
            }

            rawtx::ETH_BroadcastRawTxParam  bcastParam;
            bcastParam.strCoinType          = strCoinType;
            bcastParam.strAddrFrom          = __m_ethImportRawTxParams[iRow].strAddrFrom;  //源地址
            bcastParam.strNonce             = __m_ethImportRawTxParams[iRow].strNonce;     //nonce
            bcastParam.uChainId             = __m_ethImportRawTxParams[iRow].uChainId;     //uChainId
            bcastParam.strSignedRawTxHex    = __m_ethImportRawTxParams[iRow].strSignedRawTxHex; //签名交易
            bcastParam.strOrderId           = __m_ethImportRawTxParams[iRow].strOrderId;    //订单号

            __m_ethRawTxUtils.BroadcastRawTx( &bcastParam );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "broadcast successed.";

            //更新数据库, 将交易id写入数据库
            QThread::msleep(10);
            QSqlQuery query(_m_db);
            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5' WHERE order_id='%6';")\
                    .arg(bcastParam.strTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/).arg(bcastParam.strAddrFrom.trimmed()).arg(__m_ethImportRawTxParams[iRow].strOrderId);

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql :" << strSql;
            QString strTxidRet = bcastParam.strTxid;

            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");

            //将nonce值存入数据库 tb_eth_tx_nonce 表
            query.clear();
            QString strSqlInsertNonce = QString("insert into tb_eth_tx_nonce values('%1', '%2', '%3', %4);")
                                       .arg(__m_ethImportRawTxParams[iRow].strOrderId).arg(bcastParam.strAddrFrom)
                                       .arg(bcastParam.strTxid).arg(bcastParam.strNonce);

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "start insert `nonce` into db, sql:" << strSqlInsertNonce;

            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSqlInsertNonce)) break;
                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg = QString("sql error: insert  nonce into db failed. nonce=%1. db error:%2").arg(bcastParam.strNonce).arg(query.lastError().text());
                    query.clear();
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error( strErrMsg.toStdString() );
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "insert nonce into db successed.";
        }

    }
    else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType)
            )
    {
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_cosmosImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            JudgeBroadcastable(__m_cosmosImportRawTxParams[iRow].strOrderId);

            rawtx::Cosmos_BroadcastRawTxParam  param;

            //随便填写真实参数即可
            param.strOrderId        = __m_cosmosImportRawTxParams[iRow].strOrderId; //防止重复转账, 服务端会根据订单号判断交易是否已经发过
            param.strChainId        = __m_cosmosImportRawTxParams[iRow].strChainId;
            param.strSequence       = __m_cosmosImportRawTxParams[iRow].strSequence;
            param.strCoinType       = __m_cosmosImportRawTxParams[iRow].strCoinType;
            param.strFrom           = __m_cosmosImportRawTxParams[iRow].strFrom;
            param.strTo             = __m_cosmosImportRawTxParams[iRow].strTo;
            param.strValue          = __m_cosmosImportRawTxParams[iRow].strValue; //以usdp 或 htdf 为单位
            param.strSignedRawTxHex = __m_cosmosImportRawTxParams[iRow].strSignedRawTxHex; //已签名交易数据


            if(false == param.ParamsCheck( &strErrMsg ))
            {
                strErrMsg = "args error:" +  strErrMsg;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_cosmosRawTxUtils.BroadcastRawTx( &param );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        "  __m_cosmosRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"<< param.strOrderId << " txid:"  << param.strTxid ;


            //更新数据库, 将交易id写入数据库
            QThread::msleep(10);
            QSqlQuery query(_m_db);

            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5',complete_time=%6 WHERE order_id='%7';")\
                    .arg(param.strTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/).arg(param.strFrom.trimmed())
                    .arg(QDateTime::currentDateTime().toTime_t()).arg(__m_cosmosImportRawTxParams[iRow].strOrderId);

            QString strTxidRet = param.strTxid;
            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");
        }
    }
    else if( 0 == strCoinType.compare("XRP", Qt::CaseInsensitive) )
    {
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_xrpImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            JudgeBroadcastable(__m_xrpImportRawTxParams[iRow].strOrderId);

            rawtx::Xrp_BroadcastRawTxParam  param;

            rawtx::Xrp_ImportRawTxItem const &refImportItem = __m_xrpImportRawTxParams[iRow];

            //随便填写真实参数即可
            param.strOrderId        = refImportItem.strOrderId; //防止重复转账, 服务端会根据订单号判断交易是否已经广播成功过
            param.strChainNet       = refImportItem.strChainNet;
            param.strCoinType       = refImportItem.strCoinType;
            param.strDstAddr        = refImportItem.strDstAddr;
            param.strSrcAddr        = refImportItem.strSrcAddr;
            param.dAmountInXRP           = refImportItem.dAmountInXRP;
            param.strAmountInDrop         = refImportItem.strAmountInDrop;
            param.strSignedRawTx    = refImportItem.strSignedRawTx; //已签名交易数据
            param.strTxid           = "";
            param.uSequence         = refImportItem.uSequence;
            param.uDestinationTag   = refImportItem.uDestinationTag;



            if(false == param.ParamsCheck( &strErrMsg ))
            {
                strErrMsg = "args error:" +  strErrMsg;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_xrpRawTxUtils.BroadcastRawTx( &param );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        "  __m_xrpRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"
                        << param.strOrderId << " txid:"  << param.strTxid << "sequence:" << param.uSequence ;


            //更新数据库, 将交易id写入数据库
            QThread::msleep(10);
            QSqlQuery query(_m_db);

            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5',complete_time=%6 WHERE order_id='%7';")\
                    .arg(param.strTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/).arg(param.strSrcAddr.trimmed())
                    .arg(QDateTime::currentDateTime().toTime_t()).arg(param.strOrderId);

            QString strTxidRet = param.strTxid;
            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");

        }
    }
    else if( 0 == strCoinType.compare("EOS", Qt::CaseInsensitive) )
    {
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_eosImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            JudgeBroadcastable(__m_eosImportRawTxParams[iRow].strOrderId);

            rawtx::Eos_BroadcastRawTxParam  param;

            rawtx::Eos_ImportRawTxItem const &refImportItem = __m_eosImportRawTxParams[iRow];

            //随便填写真实参数即可
            param.strOrderId        = refImportItem.strOrderId; //防止重复转账, 服务端会根据订单号判断交易是否已经广播成功过
            param.strChainNet       = refImportItem.strChainNet;
            param.strCoinType       = refImportItem.strCoinType;
            param.strDstAcct        = refImportItem.strDstAcct;
            param.strSrcAcct        = refImportItem.strSrcAcct;
            param.dAmount           = refImportItem.dAmount;
            param.strAmount         = refImportItem.strAmount;
            param.strSignedRawTx    = refImportItem.strSignedRawTx; //已签名交易数据
            param.strTxDigestHex    = refImportItem.strTxDigestHex; //已签名交易数据
            param.strTxid           = "";
            param.strMemo           = refImportItem.strMemo;



            if(false == param.ParamsCheck( &strErrMsg ))
            {
                strErrMsg = "args error:" +  strErrMsg;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_eosRawTxUtils.BroadcastRawTx( &param );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        "  __m_eosRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"
                        << param.strOrderId << " txid:"  << param.strTxid <<   "memo: " << param.strMemo;


            //更新数据库, 将交易id写入数据库
            QThread::msleep(500);
            QSqlQuery query(_m_db);

            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5',complete_time=%6 WHERE order_id='%7';")\
                    .arg(param.strTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/).arg(param.strSrcAcct.trimmed())
                    .arg(QDateTime::currentDateTime().toTime_t()).arg(param.strOrderId);

            QString strTxidRet = param.strTxid;
            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");

        }
    }
    else if( 0 == strCoinType.compare("XLM", Qt::CaseInsensitive) )
    {
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_xlmImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            JudgeBroadcastable(__m_xlmImportRawTxParams[iRow].strOrderId);

            rawtx::Xlm_BroadcastRawTxParam  param;

            rawtx::Xlm_ImportRawTxItem const &refImportItem = __m_xlmImportRawTxParams[iRow];

            //随便填写真实参数即可
            param.strOrderId        = refImportItem.strOrderId; //防止重复转账, 服务端会根据订单号判断交易是否已经广播成功过
            param.strChainNet       = refImportItem.strChainNet;
            param.strCoinType       = refImportItem.strCoinType;
            param.strDstAddr        = refImportItem.strDstAddr;
            param.strSrcAddr        = refImportItem.strSrcAddr;
            param.dAmount           = refImportItem.dAmount;
            param.strAmount         = refImportItem.strAmount;
            param.strSignedRawTx    = refImportItem.strSignedRawTx; //已签名交易数据
            param.uSequence         = refImportItem.uSequence;

            param.strTxid           = "";



            if(false == param.ParamsCheck( &strErrMsg ))
            {
                strErrMsg = "args error:" +  strErrMsg;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_xlmRawTxUtils.BroadcastRawTx( &param );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        "  __m_xlmRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"
                        << param.strOrderId << " txid:"  << param.strTxid <<   "memo: " << refImportItem.strMemo;


            //更新数据库, 将交易id写入数据库
            QThread::msleep(500);
            QSqlQuery query(_m_db);

            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5',complete_time=%6 WHERE order_id='%7';")\
                    .arg(param.strTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/).arg(param.strSrcAddr.trimmed())
                    .arg(QDateTime::currentDateTime().toTime_t()).arg(param.strOrderId);

            QString strTxidRet = param.strTxid;
            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");

        }
    }
    else if( 0 == strCoinType.compare("TRX", Qt::CaseInsensitive) )
    {
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_trxImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            JudgeBroadcastable(__m_trxImportRawTxParams[iRow].strOrderId);

            rawtx::Trx_BroadcastRawTxParam  param;

            rawtx::Trx_ImportRawTxItem const &refImportItem = __m_trxImportRawTxParams[iRow];

            //随便填写真实参数即可
            param.strOrderId        = refImportItem.strOrderId; //防止重复转账, 服务端会根据订单号判断交易是否已经广播成功过
            param.strCoinType       = refImportItem.strCoinType;
            param.strDstAcct        = refImportItem.strDstAcct;
            param.strSrcAcct        = refImportItem.strSrcAcct;
            param.dAmount           = refImportItem.dAmount;
            param.strAmount         = refImportItem.strAmount;
            param.strSignedRawTx    = refImportItem.strSignedRawTx; //已签名交易数据
            param.strTxid           = refImportItem.strTxid;  //交易摘要,
            param.strSuccessedTxid  = ""; //注意: 用来存放服务端返回到的 txid, 注意和上面的  strTxid 区分



            if(false == param.ParamsCheck( &strErrMsg ))
            {
                strErrMsg = "args error:" +  strErrMsg;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_trxRawTxUtils.BroadcastRawTx( &param );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        "  __m_trxRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"
                        << param.strOrderId << " txid:"  << param.strSuccessedTxid;


            //更新数据库, 将交易id写入数据库
            QThread::msleep(500);
            QSqlQuery query(_m_db);

            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5',complete_time=%6 WHERE order_id='%7';")\
                    .arg(param.strSuccessedTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(0/*成功*/).arg(param.strSrcAcct.trimmed())
                    .arg(QDateTime::currentDateTime().toTime_t()).arg(param.strOrderId);

            QString strTxidRet = param.strSuccessedTxid;
            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");

        }
    }
    else if( 0 == strCoinType.compare("XMR", Qt::CaseInsensitive) )
    {
        for(auto iRow : vctRowsSel)
        {
            if((uint)iRow >= __m_xmrImportRawTxParams.size())
            {
                strErrMsg = QString("selected count greater than imported count");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            JudgeBroadcastable(__m_xmrImportRawTxParams[iRow].strOrderId);

            rawtx::XMR_BroadcastRawTxParam  param;

            rawtx::XMR_ImportRawTxItem const &refImportItem = __m_xmrImportRawTxParams[iRow];

            //随便填写真实参数即可
            param.bIsManualWithdraw = true;
            param.strOrderId        = refImportItem.strOrderId; //防止重复转账, 服务端会根据订单号判断交易是否已经广播成功过
            param.strCoinType       = refImportItem.strCoinType;
            param.strDstAddr        = refImportItem.strDstAddr;
            param.strSrcAddr        = refImportItem.strSrcAddr;
            param.dAmount           = refImportItem.dAmount;
            param.strAmount         = refImportItem.strAmount;
            param.strSignedRawTx    = refImportItem.strSignedRawTx; //已签名交易数据
            param.strTxid           = "";



            if(false == param.ParamsCheck( &strErrMsg ))
            {
                strErrMsg = "args error:" +  strErrMsg;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_xmrRawTxUtils.BroadcastRawTx( &param );

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        "  __m_xmrRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"
                        << param.strOrderId << " txid:"  << param.strTxid;


            //更新数据库, 将交易id写入数据库
            QThread::msleep(500);
            QSqlQuery query(_m_db);

            QString strSql = QString("UPDATE tb_auto_withdraw set txid='%1',tx_status=%2,order_status=%3,upload_status=%4, src_addr='%5',complete_time=%6 WHERE order_id='%7';")\
                    .arg(param.strTxid).arg( 1 /*成功*/).arg(1/*成功*/).arg(1/*成功*/).arg(param.strSrcAddr.trimmed())
                    .arg(QDateTime::currentDateTime().toTime_t()).arg(param.strOrderId);

            QString strTxidRet = param.strTxid;
            const int ciTryCount = 5;
            for(int iTry = 0; iTry < ciTryCount; iTry++)
            {
                if(true == query.exec(strSql) ) break;

                QThread::msleep(100);

                if( ciTryCount - 1 == iTry ) //最后一次尝试都失败了
                {
                    strErrMsg =  QString("broadcasted successed, but insert database failed. orderId:%1 txid:%2. error msg:%3")
                            .arg(strSql).arg(strTxidRet).arg(query.lastError().text());

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }
            }
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid,成功");

        }


    }
    else //不支持的币种
    {
        strErrMsg = QString("operation error: not support %1 in current version.").arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("广播成功");
    return CPageManualWithdraw::NO_ERROR;
}


//获取导出数据的个数
int CPageManualWithdraw::GetExportDataCount(const QString &strCoinType)noexcept(true)
{
    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
#if defined(BTC_SERIES_EX)
        return __m_btcExportRawTxParams.size();
#else
        return __m_btcExportData.vctExportItems.size();
#endif
    }
    else if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
        return __m_usdtExportData.vctExportItems.size();
    else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("etc", Qt::CaseInsensitive) )
        return __m_ethExportRawTxParams.size();
    else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType) //HRC20
            )
    {
        return __m_cosmosExportRawTxParams.size();
    }
    else if(0 == strCoinType.compare("XRP"))
    {
        return __m_xrpExportRawTxParams.size();
    }
    else if(0 == strCoinType.compare("EOS"))
    {
        return __m_eosExportRawTxParams.size();
    }
    else if(0 == strCoinType.compare("XLM"))
    {
        return __m_xlmExportRawTxParams.size();
    }
    else if(0 == strCoinType.compare("TRX"))
    {
        return __m_trxExportRawTxParams.size();
    }
    else if(0 == strCoinType.compare("XMR"))
    {
        return __m_xmrExportRawTxParams.size();
    }
    else
        return 0;

    return 0;
}

//获取订单状态(
bool CPageManualWithdraw::JudgeBroadcastable(const QString &strOrderID) noexcept(false)
{
    QString strErrMsg;

    QSqlQuery query(_m_db);
    QString strQuery = QString("select order_status, txid, withdraw_way from tb_auto_withdraw where order_id = '%1'").arg(strOrderID);
    if(!query.exec(strQuery))
    {
        strErrMsg = QString("sql error: %1").arg(query.lastError().text());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    QString strTxid = "";
    int iwithdraw_way = -1;
    int iOrderStatus = -1;
    while (query.next())
    {
        iOrderStatus = query.value(0).toInt();
        strTxid = query.value(1).toString();
        iwithdraw_way = query.value(2).toInt();
    }

    if(CAutoWithdraw::WAY_MANUAL  != iwithdraw_way)
    {
        strErrMsg = QString("operation error:  order %1 is not manual withdraw order. It could not be broadcasted.").arg(strOrderID);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(CAutoWithdraw::ORDER_SUCCESS == iOrderStatus)
    {
        strErrMsg = QString("operation error:order %1 has been successed. It could not be broadcasted again.").arg(strOrderID);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!strTxid.isEmpty())
    {
        strErrMsg = QString("operation error: order %1 txid is not empty. It could not be broadcasted again.").arg(strOrderID);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    return true;
}

QString CPageManualWithdraw::CalculateTotalAmount(const vector<int> &vctRowsSel)noexcept(true)
{

    double dTotalAmount = 0;
    for(uint i=0; i<vctRowsSel.size(); i++)
    {
        dTotalAmount += __m_vctManualWithdrawData.at(vctRowsSel[i]).strAmount.toDouble();
    }
    return QString::number(dTotalAmount, 'f', 8);
}


QString CPageManualWithdraw::CalculateTxFee(const QString& strCoinType, const vector<int> &vctRowsSel) noexcept(true)
{
    if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || 0 == strCoinType.compare("etc", Qt::CaseInsensitive))
    {

        double dTotalAmount = 0;
        for(uint i=0; i<vctRowsSel.size(); i++)
        {
            dTotalAmount += __m_vctManualWithdrawData.at(vctRowsSel[i]).strTxFee.toDouble();
        }
        return QString::number(dTotalAmount, 'f', 8);
    }
    else
    {
        double dTotalAmount = 0;
        for(uint i=0; i<vctRowsSel.size(); i++)
        {
            dTotalAmount += __m_vctManualWithdrawData.at(vctRowsSel[i]).strTxFee.toDouble();
        }
        return QString::number(dTotalAmount, 'f', 8);
    }
    return QString("0");
}

bool CPageManualWithdraw::VerifyAuditStatus(const QString &strOrderId,  int iStatus) noexcept(false)
{
    QString strErrMsg;

    QSqlQuery queryResult(_m_db);
    QString strSql = QString("select audit_status from tb_auto_withdraw where order_id = \'%1\';").arg(strOrderId);

    if (!queryResult.exec(strSql))
    {
        strErrMsg = QString("sql error: %1").arg(queryResult.lastError().text());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!queryResult.next())
    {
        strErrMsg = QString("operation error: not found this order(%1) in database").arg(strOrderId);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    int iTmpStatus = queryResult.value(0).toInt();
    return (iStatus == iTmpStatus);
}

//XMR 特殊处理
int CPageManualWithdraw::XMR_ExportTxOutputs(QString &strExportFilePathRet) noexcept(false)
{
    QString strErrMsg = "";
    QString strTxOutputs = "";
    QString strReqUrl = g_qstr_WalletHttpIpPort + STR_xmr_manual_exporttxoutputs;
    __m_xmrRawTxUtils.ExportTxOutputs(strReqUrl, strTxOutputs);


    if(! boost::all( strTxOutputs.toStdString() , boost::is_xdigit() ) )
    {
        strErrMsg = " txoutputs is invalid hexstring " ;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg ;
        throw runtime_error(strErrMsg.toStdString());
    }



     //导出
    QString strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + + "XMR_txoutputs.json";

     //写入Json文件
    QFile outFile(strFilePath);
    if(!outFile.open(QIODevice::WriteOnly))
    {
        strErrMsg = QString::asprintf("%s open failed.", strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject  joOutputs;
    joOutputs.insert("outputs_data_hex", strTxOutputs);

    QJsonDocument jDoc;
    jDoc.setObject( joOutputs );

    outFile.write(jDoc.toJson());
    outFile.close();

    //返回导出文件的路径, 用于显示
    strExportFilePathRet = strFilePath;

    return 0;
}

//XMR 特殊处理
int CPageManualWithdraw::XMR_ImportKeyImages(const QString &strImportFilePath) noexcept(false)
{
    QString strErrMsg = "";
    if( strImportFilePath.isEmpty() )
    {
        strErrMsg = "file path is empty!";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QFile  inFile(strImportFilePath);
    if(!inFile.exists())
    {
        strErrMsg = QString::asprintf("args error: import-file %s is not exists.", strImportFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        strErrMsg = QString::asprintf("%s open failed.", strImportFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QByteArray byteArray = inFile.readAll();
    inFile.close();

    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "导入数据: " << byteArray;

    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byteArray, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = QString::asprintf("parse json error:%s", error.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QJsonObject joRoot  =  jsonDoc.object();
    if(!(!joRoot.isEmpty() && joRoot.contains("signed_key_images")  &&  !joRoot.value("signed_key_images").toArray().isEmpty()  ))
    {
        strErrMsg = " import file DOES NOT contains `signed_key_images` OR `signed_key_images` is EMPTY , please check it. ";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonArray jaSignedKeyImages =  joRoot.value("signed_key_images").toArray();
    QJsonDocument jDocTmp;
    jDocTmp.setArray(jaSignedKeyImages);

    QString  strImportKeyImagesArray =   jDocTmp.toJson();

    QString strReqUrl = g_qstr_WalletHttpIpPort + STR_xmr_manual_importkeyimages;
    double dBalance = 0;
    __m_xmrRawTxUtils.ImportKeyImages(strReqUrl, strImportKeyImagesArray, dBalance );


    return 0;
}

