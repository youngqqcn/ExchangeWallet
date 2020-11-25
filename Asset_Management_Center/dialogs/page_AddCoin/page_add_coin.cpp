/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加币种业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_add_coin.h"
#include <list>
#include <QSqlQuery>
//using namespace std;

//CSysMngAddCoinType::CAddCoin(QObject *parent) : QObject(parent)
//{

//}

CPageAddCoin::CPageAddCoin(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageAddCoin::~CPageAddCoin()
{

}

int CPageAddCoin::Init()
{
    return CAMMain::Init();
}

int CPageAddCoin::AddCoin(const vector<QString> &vctCondition, const QString &strGoogleCode)
{
    if(strGoogleCode.isEmpty())
    {
#if 0
        //TODO:记得验证谷歌验证码是否正确   2019-04-24 yqq
        qCritical() << __FUNCDNAME__ << QString("谷歌验证码为空");
#else

#endif
    }

    if (10 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("参数错误: 10 != vctCondition.size() ");
        return PARAM_ERR;
    }
    QSqlQuery       query1(_m_db);
    QString         strSql2 = QString("select max(type_no) from tb_coin_type;");
    QString         strSql3 = QString("select count(addr) from tb_addr where coin_type = \'%1\';").arg(vctCondition[1]);

    uint            strTypeNo;
    uint            uAddrCount;

    if (!query1.exec(strSql2))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种表中最大序号失败:"  << query1.lastError();
        return DB_ERR;
    }
    if (query1.next())
    {
        strTypeNo = query1.value(0).toInt() + 1;
    }
    else
    {
        strTypeNo = 0;
    }
    qDebug() << "***************" << strTypeNo;

    query1.clear();
    if (!query1.exec(strSql3))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种表中最大序号失败:"  << query1.lastError();
        return DB_ERR;
    }
    if (query1.next())
    {
        uAddrCount = query1.value(0).toUInt();
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "地址数量查询失败" << query1.lastError();
        return DB_ERR;
    }

    //将提现地址私钥和解密密码转为base64编码,然后进行比较
    QByteArray privkeyBaseEncode = vctCondition[3].toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QByteArray decodePwdBaseEncode = vctCondition[4].toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QString strprivkeyBaseEncode = privkeyBaseEncode;
    QString strDecodePwdBaseEnCode = decodePwdBaseEncode;

    QString         strSql1 = QString("insert into tb_coin_type(type_no,coin_nick,withdraw_src_addr,addr_count,encrypted_privkey,decode_pwd,"
                                      "min_withdraw_count,semi_auto_windraw_amount,day_withdraw_count,tx_fee,charge_threshold) values(%1,\'%2\',\'%3\',%4,\'%5\',\'%6\',"
                                      "\'%7\',\'%8\',\'%9\', \'%10\',\'%11\');").arg(strTypeNo).arg(vctCondition[1]).arg(vctCondition[2]).arg(uAddrCount)
            .arg(strprivkeyBaseEncode).arg(strDecodePwdBaseEnCode).arg(vctCondition[5]/*全自动*/).arg(vctCondition[6]/*半自动*/).arg(vctCondition[7]).arg(vctCondition[8]).arg(vctCondition[9]);

//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_+_+_+_+_+_+" << strSql1;

    query1.clear();
    if (query1.exec(strSql1))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加币种完成";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加币种失败" << query1.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAddCoin::DeleteCoin(const QString &strWithdrawAddr, const QString &strCoinType, const QString &strGoogleCode)
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "谷歌验证码为:" << strGoogleCode;
    //TODO:判断谷歌验证码是否有效

    //谷歌验证码有效,删除数据库中数据
    QSqlQuery query(_m_db);
    QString strSql = QString("delete from tb_coin_type where withdraw_src_addr = \'%1\' and coin_nick = \'%2\';")
            .arg(strWithdrawAddr).arg(strCoinType);

    if (query.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除币种成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除币种失败";
        return DB_ERR;
    }

    return NO_ERROR;
}

int CPageAddCoin::Search(const vector<QString> &vctCondition, vector<QStringList> &vctAddCoin)
{
    QSqlQuery query(_m_db);
    QSqlQuery queryResult;
    QString strSql1 = QString("select * from tb_coin_type where tx_fee is not null ");

    if (tr("所有币种") != vctCondition[0])
    {
        strSql1 += QString("and coin_nick = \'%1\'; ").arg(vctCondition[0]);
    }
    if (!vctCondition[1].isEmpty())
    {
        strSql1 += QString("and withdraw_src_addr = \'%1\'; ").arg(vctCondition[1]);
    }

    queryResult = _m_db.exec(strSql1);
    if (query.exec(strSql1))
    {
        QStringList strlistTmp;
        while (queryResult.next())
        {
            strlistTmp.clear();
            strlistTmp << queryResult.value(1).toString().trimmed() << queryResult.value(0).toString().trimmed()
                       << queryResult.value(3).toString().trimmed() << queryResult.value(2).toString().trimmed()
                        << queryResult.value(6).toString().trimmed() /*全自动上限*/
                        << queryResult.value(10).toString().trimmed()/*半自动上限*/
                       << queryResult.value(7).toString().trimmed()
                       << queryResult.value(9).toString().trimmed() << queryResult.value(8).toString().trimmed();
            vctAddCoin.push_back(strlistTmp);
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种信息成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种信息失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAddCoin::EditCoin(const vector<QString> &vctCondition)
{
    QSqlQuery query(_m_db);

    //将现地址私钥和解密密码转为base64编码,然后进行比较
    QByteArray privkeyBaseEncode = vctCondition[3].toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QByteArray decodePwdBaseEncode = vctCondition[4].toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    QString strprivkeyBaseEncode = privkeyBaseEncode;
    QString strDecodePwdBaseEnCode = decodePwdBaseEncode;

    QString strSql = QString("update tb_coin_type set coin_nick = \'%1\', "
                             "withdraw_src_addr =\'%2\', encrypted_privkey = \'%3\', min_withdraw_count = \'%4\', semi_auto_windraw_amount=\'%5\', "
                             "day_withdraw_count = \'%6\', decode_pwd = \'%7\', tx_fee = \'%8\', charge_threshold = \'%9\' where type_no = %10; ")
            .arg(vctCondition[1]).arg(vctCondition[2]).arg(strprivkeyBaseEncode).arg(vctCondition[5]).arg(vctCondition[6]/*半自动*/).arg(vctCondition[7])
            .arg(strDecodePwdBaseEnCode).arg(vctCondition[8]).arg(vctCondition[9]).arg(vctCondition[0]);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql: " << strSql;

    if (query.exec(strSql))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "编辑币种信息成功";
        return NO_ERROR;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "编辑币种信息失败" << query.lastError();
        return DB_ERR;
    }
    return NO_ERROR;
}

int CPageAddCoin::queryAllCoinType(QMap<QString, QString> &mapCoinAndBlockBrowser)
{
    QSqlQuery query(_m_db);
    QString strSql = QString("select distinct coin_nick, tx_fee from tb_coin_type;");

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种表失败,检查数据库是否正常!" << query.lastError();
        return DB_ERR;
    }

    while (query.next())
    {
        mapCoinAndBlockBrowser.insert(query.value(0).toString(), query.value(1).toString());
    }
    return NO_ERROR;
}

int  CPageAddCoin::UploadAllConfigToExAmdin() noexcept(false)
{
    QString strErrMsg;

    QSqlQuery query(_m_db);
    //QSqlQuery queryResult;
    QString strSql1 = QString("select * from tb_coin_type where tx_fee is not null ");
    //queryResult = _m_db.exec(strSql1);

    //币种, 人工审核额度, 单日单币种累计额度
    std::vector< std::tuple<QString, double, double> >  vctCoinInfo;

    const int  N_IDX_COIN_NAME = 1;
    const int  N_IDX_MINWITHDRAW_VALUE = 6;
    const int  N_IDX_DAYWITHDRAW_VALUE = 7;

    if (!query.exec(strSql1))
    {
        strErrMsg = "查询数据库错误";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种信息失败" << query.lastError();
        throw std::runtime_error( strErrMsg.toStdString() );
    }
        
    QStringList strlistTmp;
    while (query.next())
    {
        strlistTmp.clear();

        QString strCoinType =  "";
        double  dMinwithdrawValue = 0.0;
        double dDaywithdrawValue = 0.0;

        if(true)
        {
            //获取币种名称
            strCoinType = query.value( N_IDX_COIN_NAME   ).toString();

            if( 0 ==  strCoinType.compare("ERC20FEE", Qt::CaseInsensitive) || 0 ==  strCoinType.compare("HRC20FEE", Qt::CaseInsensitive) )
                continue;

            //获取人工审核额度
            bool ok = false;
            QString strMinwithdrawValue = query.value( N_IDX_MINWITHDRAW_VALUE  ).toString();
            dMinwithdrawValue = strMinwithdrawValue.toDouble( &ok );
            if(false == ok)
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "strMinwithdrawValue.toDouble failed";
                continue;
            }

            //获取单日单币种 额度
            QString strDaywithdrawValue = query.value( N_IDX_DAYWITHDRAW_VALUE ).toString();
            dDaywithdrawValue = strDaywithdrawValue.toDouble( &ok );
            if(false == ok)
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "strDaywithdrawValue.toDouble failed";
                continue;
            }
        }

        vctCoinInfo.push_back( std::make_tuple( strCoinType,  dMinwithdrawValue, dDaywithdrawValue )  );
    }

    std::vector<QString> vctFailed;

    //开始上传
    for(const std::tuple<QString, double, double> &tpCoinInfo: vctCoinInfo  )
    {
        QJsonObject  joParams;
        joParams.insert("coinCode",  std::get<0>(tpCoinInfo) );
        joParams.insert("auditMinWithdraw", QString::asprintf( "%.8f", std::get<1>(tpCoinInfo))  );
        joParams.insert("dayAuditWithdraw", QString::asprintf( "%.8f", std::get<2>(tpCoinInfo))  );

        QByteArray  bytesRsp;

        QString strReqUrl = g_qstr_JavaHttpIpPort + "/web/manage/coin/changeCoinInfo";
        CAMMain::SyncPostByJson(strReqUrl, joParams, bytesRsp );


        QJsonParseError error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRsp, &error));
        if(QJsonParseError::NoError != error.error  )
        {
            strErrMsg = QString("上传错误:json parse error: %1").arg(error.errorString());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw std::runtime_error( strErrMsg.toStdString() );
        }
        QJsonObject rootObj = jsonDoc.object();

        //判断返回状态是否OK
        if(!(rootObj.contains("errCode") && 0 == rootObj.value("errCode").toInt()))
        {
            strErrMsg = QString("上传错误: java returns error:%1, %2").arg(rootObj.value("errCode").toInt() ).arg(rootObj.value("errMsg").toString());
            qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            //throw runtime_error(strErrMsg.toStdString());

            vctFailed.push_back( std::get<0>(tpCoinInfo)  );
            continue;
        }

    }

    if( vctFailed.size() > 0 )
    {
        strErrMsg = "上传失败的币种:  ";
        std::for_each( vctFailed.begin(), vctFailed.end(), [&strErrMsg](QString & item){
            strErrMsg += item + ",";
        });
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw std::runtime_error( strErrMsg.toStdString() );
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "上传提币额度配置成功!";

    return NO_ERROR;
}
