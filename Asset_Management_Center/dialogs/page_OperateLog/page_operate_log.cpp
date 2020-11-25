/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      操作日志界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_operate_log.h"

//CPageOperateLog::CPageOperateLog(QObject *parent) : QObject(parent)
//{

//}

CPageOperateLog::CPageOperateLog(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageOperateLog::~CPageOperateLog()
{

}

int CPageOperateLog::Init()
{
    return CAMMain::Init();
}

int CPageOperateLog::SearchOperateLog(const OperateLogSpace::Condition &condition, vector<QStringList> &vctOperteLog)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select * from tb_operation_log where log_no is not null ");
    QStringList strlist;

    if (!condition.strFamilyName.isEmpty())
    {
        strSql += QString("and admin_family_name = \'%1\' ").arg(condition.strFamilyName);
    }
    if (!condition.strTel.isEmpty())
    {
        strSql += QString("and tel = \'%1\' ").arg(condition.strTel);
    }
    if (tr("全部类型") != condition.strOperateType.trimmed())
    {
        strSql += QString("and operation_type = \'%1\' ").arg(condition.strOperateType);
    }
    if (!condition.strAdminId.isEmpty())
    {
        strSql += QString("and admin_id = \'%1\' ").arg(condition.strAdminId);
    }

    strSql += QString("order by operation_time desc ");

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库操作日志失败" << query.lastError();
        return DB_ERR;
    }

    while (query.next())
    {
        strlist.clear();
        QDateTime operateDatetime = QDateTime::fromTime_t(query.value(6).toUInt());
        QString strOperateDatetime = operateDatetime.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strOperateDatetime << query.value(1).toString().trimmed()
                << query.value(3).toString().trimmed() << query.value(4).toString().trimmed()
                << query.value(5).toString().trimmed();
        vctOperteLog.push_back(strlist);
    }
    return NO_ERROR;
}

int CPageOperateLog::InsertOperateLog(OperateLogSpace::OperateLog &insertData)
{
    QSqlQuery query(_m_db);

    uint uLogNo;
    if (NO_ERROR != __QueryMaxLogNo(uLogNo))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库中操作日志序号失败";
        return DB_ERR;
    }

    QString strSql = QString("insert into tb_operation_log values(%1,%2,\'%3\',\'%4\',\'%5\',\'%6\',%7)")
            .arg(uLogNo).arg(insertData.strAdminID.toUInt()).arg(insertData.strAdminLoginName).arg(insertData.strAdminFamilyName)
            .arg(insertData.strAdminTel).arg(insertData.strOperateType).arg(insertData.strDatetime.toUInt());

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入操作日志失败,操作日志类型为: " << insertData.strOperateType << "错误为: " << query.lastError().text();
        return DB_ERR;
    }

    return NO_ERROR;
}

int CPageOperateLog::__QueryMaxLogNo(uint &uLogNo)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select max(log_no) from tb_operation_log");
    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败,请检查数据库" << query.lastError().text();
        return DB_ERR;
    }

    while (query.next())
    {
        uLogNo = query.value(0).toUInt() + 1;
        return NO_ERROR;
    }
    uLogNo = 1;
    return NO_ERROR;
}
