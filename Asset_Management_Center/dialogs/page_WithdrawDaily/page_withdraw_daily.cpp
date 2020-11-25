/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      提现日报界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_withdraw_daily.h"

//CPageWithdrawDaily::CPageWithdrawDaily(QObject *parent) : QObject(parent)
//{

//}

CPageWithdrawDaily::CPageWithdrawDaily(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageWithdrawDaily::~CPageWithdrawDaily()
{

}

int CPageWithdrawDaily::Init()
{
    return CAMMain::Init();
}

int CPageWithdrawDaily::WithdrawDailySearch(const vector<QString> &vctCondition, vector<QStringList> &vctWithdrawDaily)
{
    if (3 != vctCondition.size())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    QSqlQuery   queryResult;
    QString     strSql1 = QString("select coin_type,amount,complete_time from tb_auto_withdraw where order_status = 1 ");//自动提币
    QString     strSql2 = QString("select coin_type,coin_count,complete_time from tb_manual_audit where order_status = 1 ");//手动提币
    uint64_t    uTimestamp;
    uint64_t    uStartTimestamp;
    uint64_t    uEndTimestamp;
    uint64_t    uCurrentDatetime;

    if (!vctCondition[1].isEmpty())
    {
        //将string转为时间QDateTime
        QDateTime   startDatetime(QDateTime::fromString(vctCondition[1], "yyyy-MM-dd hh:mm:ss"));
        //将QDateTime转为时间戳
        uTimestamp = startDatetime.toTime_t();
        uStartTimestamp = uTimestamp;
        if (!vctCondition[2].isEmpty())
        {
            QDateTime   endDatetime(QDateTime::fromString(vctCondition[2], "yyyy-MM-dd hh:mm:ss"));
            uTimestamp = endDatetime.toTime_t();
            uEndTimestamp = uTimestamp;
        }
        else
        {
            QDateTime   currentDatetime = QDateTime::currentDateTime();
            uCurrentDatetime = currentDatetime.toTime_t();
            uEndTimestamp = uCurrentDatetime;
        }
    }
    else
    {
        if (!vctCondition[2].isEmpty())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "有结束时间,但没有开始时间";
            return PARAM_ERR;
        }
    }

    if (tr("所有币种") != vctCondition[0])
    {
        strSql1 += QString("and coin_type = \'%1\' ").arg(vctCondition[0]);
        strSql2 += QString("and coin_type = \'%1\' ").arg(vctCondition[0]);
    }
    if (!vctCondition[1].isEmpty())
    {
        strSql1 += QString("and complete_time <= \'%1\' and complete_time >= \'%2\' ").arg(uEndTimestamp).arg(uStartTimestamp);
    }

    strSql1 += QString("order by complete_time desc ");
    strSql2 += QString("order by complete_time desc ");
//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询提现日报自动提币表sql语句: " << strSql1;
//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询提现日报手动提币sql语句: " << strSql2;

    queryResult = _m_db.exec(strSql1);
    QStringList strlist;
    while (queryResult.next())
    {
        if (queryResult.value(0).toString().isEmpty())
        {
            return DB_ERR;
        }
        strlist.clear();
        strlist << queryResult.value(0).toString().trimmed() << QString("提币")
                << queryResult.value(1).toString().trimmed();
        QDateTime datetime(QDateTime::fromTime_t(queryResult.value(2).toUInt()));
        QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strDatetime;
        vctWithdrawDaily.push_back(strlist);
    }

    queryResult.clear();
    queryResult = _m_db.exec(strSql2);
    while (queryResult.next())
    {
        if (queryResult.value(0).toString().isEmpty())
        {
            return DB_ERR;
        }
        strlist.clear();
        strlist << queryResult.value(0).toString().trimmed() << QString("提币")
                << queryResult.value(1).toString().trimmed();
        QDateTime datetime(QDateTime::fromTime_t(queryResult.value(2).toUInt()));
        QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strDatetime;
        vctWithdrawDaily.push_back(strlist);
    }

    return NO_ERROR;
}
