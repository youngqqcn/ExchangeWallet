/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      充值列表界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_charge_list.h"

//CPageChargeList::CPageChargeList(QObject *parent) : QObject(parent)
//{
//}

CPageChargeList::CPageChargeList(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageChargeList::~CPageChargeList()
{
    if (NULL != __m_pEncryptionModule)
    {
        delete __m_pEncryptionModule;
        __m_pEncryptionModule = NULL;
    }
}

int CPageChargeList::Init()
{
    //如果没有其他初始化, 直接使用父类的Init函数即可
    //如果有其他初始化, 请在此函数中实现初始化
    __m_strStartUrl = STR_GET_ORDER_ID_URL;
    __m_strChargeFinishUrl = STR_COIN_CHARGE_SUCCESS_URL;
    __m_pEncryptionModule = NULL;
    __m_pEncryptionModule = new CEncryptionModule();

    return CAMMain::Init();
}



int CPageChargeList::ConditionSearch(const ChargeList::SEARCHCOND &searchCond,
                                     vector<QStringList> &vctChargeInfo, uint &uMaxPage)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ConditionSearch 进入函数";

    QSqlQuery queryResult(_m_db);
    QString strSql_1 = QString("select recharge_time,order_id,coin_type,recharge_count,recharge_status,upload_status,audit_status,auditor,"
                             "audit_remark,txid,src_addr,receive_addr from tb_recharge where recharge_time > %1 and recharge_time < %2 ")
            .arg(searchCond.uDatetimeStart).arg(searchCond.uDatetimeEnd);
    QString strSql_2 = QString("select count(*) from tb_recharge; ");

    QStringList strlist;

    if (!searchCond.strTxid.isEmpty())
        strSql_1 += QString("and txid = \'%1\' ").arg(searchCond.strTxid);
    if (!searchCond.strDstAddr.isEmpty())
        strSql_1 += QString("and receive_addr = \'%1\' ").arg(searchCond.strDstAddr);
    if (tr("所有币种") != searchCond.strCoinType.trimmed())
        strSql_1 += QString("and coin_type = \'%1\' ").arg(searchCond.strCoinType);
    if (tr("待入账") == searchCond.strChargeStatus.trimmed())
        strSql_1 += QString("and recharge_status = 0 ");
    if (tr("已入账") == searchCond.strChargeStatus.trimmed())
        strSql_1 += QString("and recharge_status = 1 ");
    if (tr("待上传") == searchCond.strUploadStatus.trimmed())
        strSql_1 += QString("and upload_status = 1 ");
    if (tr("上传成功") == searchCond.strUploadStatus.trimmed())
        strSql_1 += QString("and upload_status = 0 ");
    if (tr("上传失败") == searchCond.strUploadStatus.trimmed())
        strSql_1 += QString("and upload_status = 2 ");
    if (tr("需审核") == searchCond.strAuditStatus.trimmed())
        strSql_1 += QString("and audit_status = 2 ");
    if (tr("审核通过") == searchCond.strAuditStatus.trimmed())
        strSql_1 += QString("and audit_status = 1 ");
    if (tr("自动充币") == searchCond.strAuditStatus.trimmed())
        strSql_1 += QString("and audit_status = -1 ");

//    strSql_1 += QString("order by upload_status desc, recharge_time desc, audit_status desc limit %1,500; ").arg((searchCond.uCurrentPage - 1) * 500);
//    strSql_1 += QString("order by upload_status desc, recharge_time desc, audit_status desc limit %1,500; ").arg((searchCond.uCurrentPage - 1) * 50000);
    strSql_1 += QString("order by upload_status desc, recharge_time desc, audit_status  ; ");

    if (!queryResult.exec(strSql_1))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询充币信息失败" << queryResult.lastError();
        return DB_ERR;
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "SQL: " << strSql_1;

    while (queryResult.next())
    {
        strlist.clear();
        QDateTime chargeDatetime = QDateTime::fromTime_t(queryResult.value(0).toUInt());
        QString strChargeDatetime = chargeDatetime.toString("yyyy-MM-dd hh:mm:ss");
        strlist << strChargeDatetime << queryResult.value(1).toString().trimmed()
                << queryResult.value(2).toString().trimmed() << queryResult.value(3).toString().trimmed();
        if (0 == queryResult.value(4).toInt())
            strlist << QString("待入账");
        else if (1 == queryResult.value(4).toInt())
            strlist << QString("已入账");
        if (1 == queryResult.value(5).toInt())
            strlist <<  QString("待上传");
        else if (0 == queryResult.value(5).toInt())
            strlist <<  QString("上传成功");
        else if (2 == queryResult.value(5).toInt())
            strlist <<  QString("上传失败");
        if (1 == queryResult.value(6).toInt())
            strlist << QString("审核通过");
        else if (2 == queryResult.value(6).toInt())
            strlist << QString("需审核");
        else if (-1 == queryResult.value(6).toInt())
            strlist << QString("自动充币");
        strlist << queryResult.value(7).toString();
        strlist << queryResult.value(8).toString();
        strlist << queryResult.value(9).toString();
        strlist << queryResult.value(10).toString();
        strlist << queryResult.value(11).toString();
        vctChargeInfo.push_back(strlist);
    }

    queryResult.clear();
    if (!queryResult.exec(strSql_2))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询充币表最大数据量失败!" << queryResult.lastError();
        return DB_ERR;
    }
    else if (queryResult.next())
        uMaxPage = queryResult.value(0).toUInt();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ConditionSearch 成功返回";
    return NO_ERROR;
}

int CPageChargeList::ReUploadFailedOrder(const int &iRet, const QString &strDatetime, const QString &strTXID, const QString &strCoinType)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ReUploadFailedOrder 进入函数";

    QDateTime datetime = QDateTime::fromString(strDatetime, "yyyy-MM-dd HH:mm:ss");
    uint uTimestamp = datetime.toTime_t();
    uint uBlockNumber = 0;
    QSqlQuery query(_m_db);
    QString strSql1 = QString("select block_number from tb_recharge where recharge_status = 0 order by block_number; ");
    QString strSql2 = QString("select recharge_time from tb_recharge where recharge_status = 0 order by recharge_time; ");
    QString strSql = QString("update tb_recharge set upload_status = 1 where txid = \'%1\' and coin_type = \'%2\';").arg(strTXID).arg(strCoinType);

    if(1024 == iRet)
    {
        qDebug() << "确定";
        QSqlQuery queryResult(_m_db);
        if (!queryResult.exec(strSql))
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库中的上传状态失败" << queryResult.lastError();
            return DB_ERR;
        }
        else
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库中的上传状态成功";
        }
    }
    else
    {
        qDebug() << "取消";
        return INVALID_DATA_ERR;
    }


    if (!query.exec(strSql1))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库中待入账数据的最小区块高度失败" << query.lastError();
    }
    if (query.next())
    {
        uBlockNumber = query.value(0).toUInt();
    }
    query.clear();
    if (!query.exec(strSql2))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询数据库中待入账数据的最小时间戳失败" << query.lastError();
    }
    if (query.next())
    {
        uTimestamp = query.value(0).toUInt();
    }

    __m_CoinChargeMng.sendTimeStamp(uTimestamp, uBlockNumber, strCoinType);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "由于释放了一个充币信息,故更新币种: " << strCoinType << "时间戳为: " << uTimestamp
             << " 更新区块高度为: " << uBlockNumber;

    g_iReleaseFlag = 1;//释放充值信息之后,将标志置为1,防止新的充币信息完成时把请求区块监测数据的时间戳或区块高度重置

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ReUploadFailedOrder 成功返回";
    return NO_ERROR;
}

int CPageChargeList::ShortMsgSetting(const vector<QString> &vctStrTel, const QString &strGoogleCode)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ShortMsgSetting 进入函数";

    if (5 != vctStrTel.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    if (vctStrTel.at(0).isEmpty() && vctStrTel.at(1).isEmpty() && vctStrTel.at(2).isEmpty()
            && vctStrTel.at(3).isEmpty() && vctStrTel.at(4).isEmpty())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "短信设置时,没有输入至少一个手机号";
        return ARGS_ERR;
    }
    if (strGoogleCode.isEmpty())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "短信设置时,未输入谷歌验证码";
        return ARGS_ERR;
    }

    uint uMaxNo = __QueryMaxNum();

    QSqlQuery queryReasult(_m_db);
    QString strSql = QString("insert into tb_sms_num_set values(%1,\'%2\',\'%3\',\'%4\',\'%5\',\'%6\');")
            .arg(uMaxNo).arg(vctStrTel.at(0)).arg(vctStrTel.at(1)).arg(vctStrTel.at(2)).arg(vctStrTel.at(3)).arg(vctStrTel.at(4));

    if (queryReasult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "短信设置手机号信息插入数据库成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "短信设置手机号信息插入数据库失败";
        return DB_ERR;
    }

    //......其他操作 (包括更新数据,  请求谷歌验证码等操作)

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ShortMsgSetting 成功返回";
    return NO_ERROR;
}

int CPageChargeList::CheckAllCoinType(QStringList &strlist)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select distinct coin_nick from tb_coin_type order by coin_nick; ");

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败:请检查数据库 " << query.lastError();
        return DB_ERR;
    }

    while (query.next())
    {
        strlist << query.value(0).toString();
    }

    return NO_ERROR;
}

void CPageChargeList::coinChargeFinishUpdateDB(const ChargeList::CHARGEINFO &selectedChargeInfo)
{
    QSqlQuery queryResult(_m_db);
    QString strSql = QString("update tb_recharge set audit_status = 1,"
                             "auditor = \'%1\', audit_remark = \'%2\' where txid = \'%3\' and receive_addr = \'%4\' and coin_type = \'%5\';")
            .arg(selectedChargeInfo.strAuditor).arg(selectedChargeInfo.strAuditRemark).arg(selectedChargeInfo.strTxid)
            .arg(selectedChargeInfo.strDstAddr).arg(selectedChargeInfo.strCoinType);

    if (queryResult.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币审核:充币完成更新数据库中的充币状态和充币时间成功";
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币审核:充币完成更新数据库中的充币状态和充币时间失败";
    }
}

int CPageChargeList::__QueryMaxNum()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryMaxNum START";

    QString strSql = QString("select max(no) from tb_sms_num_set;");

    QSqlQuery queryResult = _m_db.exec(strSql);

    while (queryResult.next())
    {
        if (!queryResult.value(0).toString().isEmpty())
        {
            return queryResult.value(0).toUInt() + 1;
        }
        else
        {
            return 0;
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CCoinChargeMng::queryMaxNum END";
    return 0;
}

int CPageChargeList::parseData(QByteArray &byteArrayData, orderId &outOrderId)
{
    //json解析错误
    QJsonParseError jsonParseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(byteArrayData, &jsonParseErr);
    if (jsonParseErr.error != QJsonParseError::NoError)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "解析json数据出错:" << jsonParseErr.errorString();
        return 1;
    }

    //json解析正确,获取数据
    QJsonObject jsonObj = jsonDoc.object();                     //将JsonDocument转为JsonObject
    QJsonValue jsonValue = jsonObj.value("data");    //获取data ,


    int errCode = jsonObj.value("errCode").toInt();             //获取code,即交易ID
    if (10832 == errCode)
    {
        outOrderId.strOrderId = QString("third_party_coin");
        outOrderId.strTXID = jsonValue.toObject().value("code").toString();
        return 0;
    }
    if (0 == errCode)
    {
        outOrderId.strOrderId = jsonValue.toObject().value("orderId").toString();
        outOrderId.strTXID = jsonValue.toObject().value("code").toString();
        return 0;
    }
    else
    {
        return 1;
    }
    return 0;
}

int CPageChargeList::requestData(const QString strUrl, QMap<QString, QVariant> mapObj, QByteArray &byteArrayReply)
{
    //下面是调用加密接口
    QJsonDocument requestJsonDoc = QJsonDocument::fromVariant(mapObj);
    //接口请求需要的参数
    QByteArray requestJson = requestJsonDoc.toJson(QJsonDocument::Compact/*不加空白字符*/);

    int iRet = -1;
    try
    {
        iRet = __m_pEncryptionModule->postEx(strUrl, requestJson, byteArrayReply);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请求Java充币数据失败";
            return iRet;
        }
    }
    catch(std::exception &e)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请求Java充币数据失败:" << QString(e.what());
        return HTTP_ERR;
    }
    catch(...)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请求Java充币数据失败:未知错误.";
        return HTTP_ERR;
    }

    if (AUTH_FAILED == iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "注意:签名验证失败!!!";
        return iRet;
    }
    return 0;
}

