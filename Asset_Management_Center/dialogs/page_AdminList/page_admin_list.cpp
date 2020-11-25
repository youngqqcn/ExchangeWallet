/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加管理员数据库操作业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_admin_list.h"
#include <QDateTime>

//CPageAdminList::CPageAdminList()
//{

//}

CPageAdminList::CPageAdminList(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageAdminList::~CPageAdminList()
{

}

int CPageAdminList::Init()
{
    return CAMMain::Init();
}

int CPageAdminList::AddAdminAffirmBtn(const vector<QString> &vctCondition)
{
    if (5 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    QSqlQuery query(_m_db);

    //获取数据库中最大的管理员ID
    uint uMaxAdminID;
    if (NO_ERROR != __QueryMaxAdminID(uMaxAdminID))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库中最大管理员ID失败";
        return DB_ERR;
    }

    //获取数据库中的管理员类型ID和管理员类型的状态
    vector<QString> vctAdminTypeInfo;
    int iRet = __QueryAdminTypeID(vctCondition[0], vctAdminTypeInfo);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型ID和状态失败";
        return iRet;
    }
    //获取当前时间
    QDateTime currentDatatime = QDateTime::currentDateTime();
    uint uCurrentDatetime = currentDatatime.toTime_t();

    //将密码进行hash之后存入数据库
//    QString strPwdToHash = QCryptographicHash::hash(vctCondition[3].toUtf8(), QCryptographicHash::Sha256).toHex();

    //将密码转为base64编码
    QByteArray pwdBaseEncode = vctCondition[3].toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QString strPwdBaseEncode = pwdBaseEncode;

    QString strSql = QString("insert into tb_admin(admin_id,admin_type_id,login_name,password,family_name,tel,status,create_time,login_status) "
                             "values(%1,%2,\'%3\',\'%4\',\'%5\',\'%6\',%7,%8,%9);").arg(uMaxAdminID).arg(vctAdminTypeInfo[0])
            .arg(vctCondition[1]).arg(strPwdBaseEncode).arg(vctCondition[2]).arg(vctCondition[4])
            .arg(vctAdminTypeInfo[1].toInt()).arg(uCurrentDatetime).arg(0);

    if (query.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入管理员数据库成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入管理员数据失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::EditAdminAffirmBtn(const vector<QString> &vctCondition, const int &__m_iFlag)
{
    if (5 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    QSqlQuery query(_m_db);

    //获取数据库中的管理员类型ID和管理员类型状态
    vector<QString> vctAdminTypeInfo;
    int iRet = __QueryAdminTypeID(vctCondition[0], vctAdminTypeInfo);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型ID和状态失败";
        return iRet;
    }

    //将密码进行hash之后更新数据库
//    QString strPwdToHash = QCryptographicHash::hash(vctCondition[3].toUtf8(), QCryptographicHash::Sha256).toHex();

    //将密码转为base64编码
    QByteArray pwdBaseEncode = vctCondition[3].toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QString strPwdBaseEncode = pwdBaseEncode;

    QString strSql = QString("update tb_admin set admin_type_id = %1, login_name = \'%2\', password = \'%3\'"
                             ", family_name = \'%4\', tel = \'%5\', status = \'%6\' where admin_id = \'%7\'; ").arg(vctAdminTypeInfo[0].toInt())
            .arg(vctCondition[1]).arg(strPwdBaseEncode).arg(vctCondition[2]).arg(vctCondition[4]).arg(vctAdminTypeInfo[1].toInt()).arg(__m_iFlag);

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新管理员信息失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::ResetPwd(const QString &strCondition, const uint &uAdminId)
{
    QSqlQuery query(_m_db);

    //将密码进行hash之后更新数据库
//    QString strPwdToHash = QCryptographicHash::hash(strCondition.toUtf8(), QCryptographicHash::Sha256).toHex();

    //将密码转为base64编码
    QByteArray pwdBaseEncode = strCondition.toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QString strPwdBaseEncode = pwdBaseEncode;

    QString strSql = QString("update tb_admin set password = \'%1\' where admin_id = %2; ").arg(strPwdBaseEncode).arg(uAdminId);

    if (query.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "重置管理员密码成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "重置管理员密码失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::CheckOldPwd(const QString &strCondition, const uint &uAdminId)
{
    QSqlQuery query(_m_db);

    //将密码进行hash之后在进行比较
//    QString strPwdToHash = QCryptographicHash::hash(strCondition.toUtf8(), QCryptographicHash::Sha256).toHex();

    //将密码转为base64编码
    QByteArray pwdBaseEncode = strCondition.toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QString strPwdBaseEncode = pwdBaseEncode;

    QString strSql = QString("select password from tb_admin where admin_id = %1;").arg(uAdminId);
    QString strPwd;

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "根据管理员ID查询密码失败" << query.lastError();
        return DB_ERR;
    }
    while (query.next())
    {
        strPwd = query.value(0).toString().trimmed();
    }

    if (strPwdBaseEncode != strPwd)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "输入的密码跟数据库中的密码对不上";
        return PARAM_ERR;
    }

    return NO_ERROR;
}

int CPageAdminList::SearchAdminInfo(const vector<QString> &vctCondition, vector<QStringList> &vctAdminInfo)
{
    if (5 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    QSqlQuery queryResult(_m_db);
    QString strSql = QString("select * from tb_admin where admin_id is not null ");
    QStringList strlist;
    QString strAdminTypeName;

    uint uStartTimestamp;
    uint uEndTimestamp;

    if ("-- ::" != vctCondition[0])
    {
        QDateTime StartDatetime = QDateTime::fromString(vctCondition[0], "yyyy-MM-dd hh:mm:ss");
        uStartTimestamp = StartDatetime.toTime_t();
        strSql += QString("and create_time >= \'%1\' ").arg(uStartTimestamp);
    }
    if ("-- ::" != vctCondition[1])
    {
        QDateTime EndDatetime = QDateTime::fromString(vctCondition[1], "yyyy-MM-dd hh:mm:ss");
        uEndTimestamp = EndDatetime.toTime_t();
        strSql += QString("and create_time <= \'%1\' ").arg(uEndTimestamp);
    }
    if (!vctCondition[2].isEmpty())
    {
        strSql += QString("and family_name = \'%1\' ").arg(vctCondition[2]);
    }
    if (!vctCondition[3].isEmpty())
    {
        strSql += QString("and tel = \'%1\' ").arg(vctCondition[3]);
    }
    if (tr("正常") == vctCondition[4])
    {
        strSql += QString("and status = 1 ");
    }
    if (tr("冻结") == vctCondition[4])
    {
        strSql += QString("and status = 0 ");
    }

    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库失败";
        return DB_ERR;
    }
    while (queryResult.next())
    {
        strlist.clear();
        strAdminTypeName.clear();
        strlist << queryResult.value(0).toString().trimmed() << queryResult.value(2).toString().trimmed()
                << queryResult.value(4).toString().trimmed();
        if (NO_ERROR != __QueryAdminTypeName(queryResult.value(1).toUInt(), strAdminTypeName))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "根据管理员类型ID查询管理员类型名失败";
            return DB_ERR;
        }
        strlist << strAdminTypeName << queryResult.value(5).toString().trimmed();
        if (1 == queryResult.value(6).toInt())
        {
            strlist << QString("正常");
        }
        if (0 == queryResult.value(6).toInt())
        {
            strlist << QString("冻结");
        }
        QDateTime createDatetime = QDateTime::fromTime_t(queryResult.value(7).toUInt());
        QString strCreateDatetime = createDatetime.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strCreateDatetime;
        strCreateDatetime.clear();
        vctAdminInfo.push_back(strlist);
    }

    return NO_ERROR;
}

int CPageAdminList::DeleteAdminInfo(const uint &uAdminId)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("delete from tb_admin where admin_id = %1;").arg(uAdminId);
    if (query.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员数据成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员数据失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::FreezeAdmin(const uint &uAdminId)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("update tb_admin set status = 0 where admin_id = %1;").arg(uAdminId);

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新管理员状态失败" << query.lastError();
        return  DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::UnfreezeAdmin(const uint &uAdminId)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("update tb_admin set status = 1 where admin_id = %1;").arg(uAdminId);

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新管理员状态失败" << query.lastError();
        return  DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::SetAdminTypeCbx(QStringList &strlist)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select type_name from tb_admin_type; ");

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型名失败" << query.lastError().text();
        return DB_ERR;
    }
    while (query.next())
    {
        strlist << query.value(0).toString().trimmed();
    }
    return NO_ERROR;
}

int CPageAdminList::SearchAdminLoginName(const QString &strAdminLoginName)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select login_name from tb_admin where login_name = \'%1\'; ").arg(strAdminLoginName);

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员登录名失败: " << query.lastError();
        return DB_ERR;
    }
    if (query.next())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询成功,数据库中已存在该登录名";
        return 1;
    }
    return NO_ERROR;
}

int CPageAdminList::__QueryAdminTypeID(const QString &strAdminTypeName, vector<QString> &vctAdminInfo)
{
    QSqlQuery queryResult;
    QString strSql = QString("select admin_type_id, status from tb_admin_type where type_name = \'%1\';").arg(strAdminTypeName);

    queryResult = _m_db.exec(strSql);
    if (queryResult.next())
    {
        vctAdminInfo.push_back(queryResult.value(0).toString());
        vctAdminInfo.push_back(queryResult.value(1).toString());
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库出错" << queryResult.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAdminList::__QueryMaxAdminID(uint &uAdminId)
{
    QSqlQuery queryResult(_m_db);
    QString strSql = QString("select max(admin_id) from tb_admin; ");
    if (!queryResult.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库出错" << queryResult.lastError();
        return DB_ERR;
    }
    while (queryResult.next())
    {
        uAdminId = queryResult.value(0).toUInt() + 1;
        return NO_ERROR;
    }
    uAdminId = 0;
    return NO_ERROR;
}

int CPageAdminList::__QueryAdminTypeName(const uint &uAdminTypeId, QString &strAdminTypeName)
{
    QSqlQuery queryResult(_m_db);
    QString strSql = QString("select type_name from tb_admin_type where admin_type_id = %1;").arg(uAdminTypeId);
    if (!queryResult.exec(strSql))
    {
        return DB_ERR;
    }
    while (queryResult.next())
    {
        strAdminTypeName = queryResult.value(0).toString();
    }

    return NO_ERROR;
}
