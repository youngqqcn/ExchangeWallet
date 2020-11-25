/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      充值日报界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_charge_daily.h"
#include <QDateTime>

//CPageChargeDaily::CPageChargeDaily(QObject *parent) : QObject(parent)
//{

//}

CPageChargeDaily::CPageChargeDaily(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageChargeDaily::~CPageChargeDaily()
{

}

int CPageChargeDaily::Init()
{
    return CAMMain::Init();
}

int CPageChargeDaily::ChargeDailySearch(const vector<QString> &vctCondition, vector<QStringList> &vctChargeDaily)
{
    if (3 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }
    QSqlQuery queryResult;
    QString strSql = QString("select * from tb_recharge where coin_type is not null ");
    uint        uTimestamp;
    uint        uStartTimestamp;
    uint        uEndTimestamp;
    uint        uCurrentDatetime;

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

    if (tr("所有币种") != vctCondition[0])
    {
        strSql += QString("and coin_type = \'%1\' ").arg(vctCondition[0].trimmed());
    }
    if (!vctCondition[1].isEmpty())
    {
        strSql += QString("and recharge_time <= \'%1\' and recharge_time >= \'%2\' ").arg(uEndTimestamp).arg(uStartTimestamp);
    }

    strSql += QString("order by recharge_time desc; ");

//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询充值日报sql语句: " << strSql;

    queryResult = _m_db.exec(strSql);
    QStringList strlist;
    while (queryResult.next())
    {
        if (queryResult.value(0).toString().isEmpty())
        {
            return INVALID_DATA_ERR;
        }

        strlist.clear();
        strlist << queryResult.value(3).toString().trimmed() << QString("充币")
                << queryResult.value(4).toString().trimmed();
        QDateTime datetimeTmp(QDateTime::fromTime_t(queryResult.value(10).toUInt()));
        QString strDatetime = datetimeTmp.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strDatetime;
        vctChargeDaily.push_back(strlist);
    }
    return NO_ERROR;
}
