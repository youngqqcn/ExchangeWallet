/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员类型业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_add_admin_type.h"

//CPageAddAdminType::CPageAddAdminType(QObject *parent) : QObject(parent)
//{

//}

CPageAddAdminType::CPageAddAdminType(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageAddAdminType::~CPageAddAdminType()
{

}

int CPageAddAdminType::Init()
{
    return CAMMain::Init();
}

int CPageAddAdminType::AddAdminType(const vector<QString> &vctCondition, uint &uAdminTypeID)
{
    if (3 != vctCondition.size()/* || 26 != vctValue.size()*/)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "传入参数有问题";
        return PARAM_ERR;
    }
    if (vctCondition[0].isEmpty() || vctCondition[1].isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    int iRet1 = __InsertAdminType(vctCondition, uAdminTypeID);
    if (NO_ERROR != iRet1)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入管理员类型表失败";
        return iRet1;
    }

    return NO_ERROR;
}

int CPageAddAdminType::UpdateAdminType(const uint &uFlag, const vector<QString> &vctCondition/*, const vector<bool> vctValue*/)
{
    if (3 != vctCondition.size()/* || 26 != vctValue.size()*/)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "传入参数有问题";
        return PARAM_ERR;
    }
    if (vctCondition[0].isEmpty() || vctCondition[1].isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    int iRet1 = __UpdateAdminType(uFlag, vctCondition);
    if (NO_ERROR != iRet1)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入管理员类型表失败";
        return iRet1;
    }

    return NO_ERROR;
}

int CPageAddAdminType::CheckAdminType(const QString &strAdminTypeName)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select * from tb_admin_type where type_name = \'%1\'; ").arg(strAdminTypeName);
    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型失败, 请检查数据库" << query.lastError();
        return DB_ERR;
    }
    if (query.next())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库中已存在这种管理员类型";
        return INVALID_DATA_ERR;
    }
    return NO_ERROR;
}

uint CPageAddAdminType::__QueryMaxAdminTypeId()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CPageAddAdminType::__QueryMaxAdminTypeId START";
    QSqlQuery queryResult;
    QString strSql = QString("select max(admin_type_id) from tb_admin_type; ");

    queryResult = _m_db.exec(strSql);
    while (queryResult.next())
    {
        if (queryResult.value(0).toInt())
        {
            return queryResult.value(0).toUInt() + 1;
        }
        else
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型列表中的最大ID失败" << queryResult.lastError();
            return 1;
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CPageAddAdminType::__QueryMaxAdminTypeId END";
    return 1;
}

int CPageAddAdminType::__InsertAdminType(const vector<QString> &vctCondition, uint &uAdminTypeID)
{
    QSqlQuery query(_m_db);
    QDateTime  currentDatetime = QDateTime::currentDateTime();
    uint uCurrentDatetime = currentDatetime.toTime_t();
    uAdminTypeId = __QueryMaxAdminTypeId();
    uAdminTypeID = uAdminTypeId;

    uint    uStatus = 1;

    QString strSql = QString("insert into tb_admin_type(admin_type_id,type_name,status,departement,create_time,remark) "
                             "values(%1,\'%2\',%3,\'%4\',%5,\'%6\');").arg(uAdminTypeId).arg(vctCondition[0]).arg(uStatus)
            .arg(vctCondition[1]).arg(uCurrentDatetime).arg(vctCondition[2]);

    if (query.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入数据成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入数据失败";
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAddAdminType::__UpdateAdminType(const uint uFlag, const vector<QString> &vctCondition)
{
    QSqlQuery query(_m_db);

    uint    uStatus = 1;

    QString strSql1 = QString("update tb_admin_type set type_name = \'%1\', status = \'%2\', "
                              "departement = \'%3\', remark = \'%4\' where admin_type_id = \'%5\'; ")
            .arg(vctCondition[0]).arg(uStatus).arg(vctCondition[1]).arg(vctCondition[2]).arg(uFlag);

    query.clear();
    if (query.exec(strSql1))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新管理员类型表成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新管理员类型表失败";
        return DB_ERR;
    }
    return NO_ERROR;
}
