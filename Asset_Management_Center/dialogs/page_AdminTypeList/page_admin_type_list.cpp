/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员类型对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_admin_type_list.h"

//CPageAdminTypeList::CPageAdminTypeList(QObject *parent) : QObject(parent)
//{

//}

CPageAdminTypeList::CPageAdminTypeList(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageAdminTypeList::~CPageAdminTypeList()
{

}

int CPageAdminTypeList::Init()
{
    return CAMMain::Init();
}

int CPageAdminTypeList::ConditionSearch(vector<QStringList> &vctAdminType)
{
    QSqlQuery queryResult(_m_db);
    QString strSql = QString("select * from tb_admin_type; ");

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败,数据库中没有管理员类型数据" << queryResult.lastError();
        return DB_ERR;
    }
    QStringList strlist;
    QString strDatetime;

    while (queryResult.next())
    {
        strlist.clear();
        strlist << queryResult.value(0).toString().trimmed() << queryResult.value(1).toString().trimmed()
                << queryResult.value(3).toString().trimmed() << queryResult.value(5).toString().trimmed();

        if (0 == queryResult.value(2).toInt())
        {
            strlist << QString("冻结");
        }
        if (1 == queryResult.value(2).toInt())
        {
            strlist << QString("正常");
        }

        QDateTime datetime(QDateTime::fromTime_t(queryResult.value(4).toUInt()));
        strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strDatetime;
        vctAdminType.push_back(strlist);
    }
    if (0 == vctAdminType.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中没有数据";
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminTypeList::FreezeAdminType(const uint &uAdminTypeId)
{
    QSqlQuery query(_m_db);
    QString strSql1 = QString("update tb_admin_type set status = 0 where admin_type_id = %1; ").arg(uAdminTypeId);
    QString strSql2 = QString("update tb_admin set status = 0 where admin_type_id = %1; ").arg(uAdminTypeId);

    if (query.exec(strSql1))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库失败" << query.lastError();
        return DB_ERR;
    }

    query.clear();
    if (query.exec(strSql2))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminTypeList::UnfreezeAdminType(const uint &uAdminTypeId)
{
    QSqlQuery query(_m_db);
    QString strSql1 = QString("update tb_admin_type set status = 1 where admin_type_id = %1; ").arg(uAdminTypeId);
    QString strSql2 = QString("update tb_admin set status = 1 where admin_type_id = %1; ").arg(uAdminTypeId);

    if (query.exec(strSql1))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库失败" << query.lastError();
        return DB_ERR;
    }

    query.clear();
    if (query.exec(strSql2))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminTypeList::DeleteAdminType(const uint uAdminTypeId)
{
    QSqlQuery query(_m_db);
    QString strSql1 = QString("delete from tb_admin where admin_type_id = %1;").arg(uAdminTypeId);
    QString strSql2 = QString("delete from tb_admin_type where admin_type_id = %1;").arg(uAdminTypeId);
    QString strSql3 = QString("delete from tb_auth_flag where admin_type_id = %1;").arg(uAdminTypeId);

    if (query.exec(strSql1))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员表中的数据成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员表中的数据失败" << query.lastError();
        return DB_ERR;
    }

    query.clear();
    if (query.exec(strSql2))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员类型表中的数据成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员类型表中的数据失败" << query.lastError();
        return DB_ERR;
    }

    query.clear();
    if (query.exec(strSql3))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除权限表中的数据成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除权限表中的数据失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}
