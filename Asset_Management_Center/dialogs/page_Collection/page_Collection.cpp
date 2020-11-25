/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      归集列表界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_Collection.h"
#include "user_info.h"
#include "utils.h"

CPageCollection::CPageCollection(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageCollection::~CPageCollection()
{

}

int CPageCollection::Init()
{
    //如果没有其他初始化, 直接使用父类的Init函数即可
    //如果有其他初始化, 请在此函数中实现初始化
    //初始化归集状态列表
    __m_mapCollectionStatus.insert(make_pair(0, tr("待归集")));
    __m_mapCollectionStatus.insert(make_pair(1, tr("已归集")));
    __m_mapCollectionStatus.insert(make_pair(2, tr("归集失败")));
    return CAMMain::Init();
}


//归集查询
int CPageCollection::CollectionQuery(const QString &strCoinType, vector<CollectionQueryRsp> &vctColRsp) noexcept(false)
{
    QString strErrMsg;

    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
        QByteArray bytesRsp;
        if(0 == strCoinType.compare("btc", Qt::CaseInsensitive) )
        {
            SyncGetData(g_qstr_WalletHttpIpPort + STR_btc_collectionquery, bytesRsp);
        }
        else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive) )
        {
            SyncGetData(g_qstr_WalletHttpIpPort + STR_ltc_collectionquery, bytesRsp);
        }
        else if(0 == strCoinType.compare("bch", Qt::CaseInsensitive) )
        {
            SyncGetData(g_qstr_WalletHttpIpPort + STR_bch_collectionquery, bytesRsp);
        }
        else if(0 == strCoinType.compare("bsv", Qt::CaseInsensitive) )
        {
            SyncGetData(g_qstr_WalletHttpIpPort + STR_bsv_collectionquery, bytesRsp);
        }
        else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive) )
        {
            SyncGetData(g_qstr_WalletHttpIpPort + STR_dash_collectionquery, bytesRsp);
        }


        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            strErrMsg = "json parse error:" + error.errorString();
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!jsonRsp.isObject())
        {
            strErrMsg = "external error: server response json is invalid." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jsonRsp.object();
        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            strErrMsg = "external error: server response json mssing `success` or `result`." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            strErrMsg = QString("server returns error: %1").arg(joRes.value("result").toString());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joResult = joRes.value("result").toObject();
        for(auto it = joResult.begin(); it != joResult.end(); it++)
        {
            CollectionQueryRsp  colRsp;
            colRsp.strCoinType = strCoinType;
            colRsp.strAddr = it.key();
            colRsp.strAmount = it.value().toString();

            vctColRsp.push_back(colRsp);
        }
    }
    else if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        QByteArray bytesRsp;
        SyncGetData(g_qstr_WalletHttpIpPort + STR_usdt_collectionquery, bytesRsp);


        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            strErrMsg = "json parse error:" + error.errorString();
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!jsonRsp.isObject())
        {
            strErrMsg = "external error: server response json is invalid." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jsonRsp.object();
        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            strErrMsg = "external error: server response json mssing `success` or `result`." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            strErrMsg = QString("server returns error: %1").arg(joRes.value("result").toString());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joResult = joRes.value("result").toObject();
        for(auto it = joResult.begin(); it != joResult.end(); it++)
        {
            CollectionQueryRsp  colRsp;
            colRsp.strCoinType = strCoinType;
            colRsp.strAddr = it.key();
            colRsp.strAmount = it.value().toString();

            vctColRsp.push_back(colRsp);
        }
    }
    else if(0 == strCoinType.compare("ETH", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("ETC", Qt::CaseInsensitive)
        )
    {
        QByteArray bytesRsp;
        QString strReqURL = g_qstr_WalletHttpIpPort + STR_eth_collectionquery;
        if(0 == strCoinType.compare("ETC", Qt::CaseInsensitive))
            strReqURL =  g_qstr_WalletHttpIpPort + STR_etc_collectionquery;

        strReqURL += "?symbol=" + QString(strCoinType).toLower();
        SyncGetData(strReqURL, bytesRsp);


        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            strErrMsg = "json parse error:" + error.errorString();
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!jsonRsp.isObject())
        {
            strErrMsg = "external error: server response json is invalid." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jsonRsp.object();
        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            strErrMsg = "external error: server response json mssing `success` or `result`." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            strErrMsg = QString("server returns error: %1").arg(joRes.value("result").toString());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonArray jaResult = joRes.value("result").toArray();
        if(jaResult.isEmpty())
        {
            strErrMsg = QString("infomation: %1").arg("There're no addresses could be collectioned.");
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_vctEthCollectionQueryRsp.clear(); //清空历史数据
        for(int i = 0;  i < jaResult.size(); i++)
        {
            QJsonObject joItem = jaResult.at(i).toObject();
            if(!(joItem.contains("nonce") && joItem.contains("balances") && joItem.contains("address")))
            {
                strErrMsg = QString("external error: server response json missing `nonce` or `balance` or `address`.");
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            ETHCollectionQeuryRsp ethClcRsp;
            ethClcRsp.strCoinType = strCoinType;
            ethClcRsp.strAddr = joItem.value("address").toString().trimmed();

            QJsonObject joBalances = joItem.value("balances").toObject();
            if(joBalances.isEmpty())
            {
                strErrMsg = QString("external error: server response json `balances` is empty.");
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QString strMainType = "ETH";
            if( 0 == strCoinType.compare("ETC", Qt::CaseInsensitive))
                strMainType = "ETC";

            if(  !(joBalances.contains(strMainType) && joBalances.contains(strCoinType)))
            {
                strErrMsg = QString("external error: server response json `balances` do not contains `%1` or `%2`").arg(strMainType).arg(strCoinType);
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            ethClcRsp.strAmount = joBalances.value(strCoinType).toString();
            ethClcRsp.strETHBalance = joBalances.value(strMainType).toString();
            ethClcRsp.uNonce = joItem.value("nonce").toInt(INT_MAX);

            __m_vctEthCollectionQueryRsp.push_back(ethClcRsp);
            vctColRsp.push_back( (CollectionQueryRsp)ethClcRsp ); //直接截取, 仅用于显示
        }
    }
    else if(0 == strCoinType.compare("USDP", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HTDF", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive))
    {
        QByteArray bytesRsp;
        if(0 == strCoinType.compare("USDP", Qt::CaseInsensitive))
            SyncGetData(g_qstr_WalletHttpIpPort + STR_usdp_collectionquery, bytesRsp);
        else if(0 == strCoinType.compare("HTDF", Qt::CaseInsensitive))
            SyncGetData(g_qstr_WalletHttpIpPort + STR_htdf_collectionquery, bytesRsp);
        else if(0 == strCoinType.compare("HET", Qt::CaseInsensitive))
            SyncGetData(g_qstr_WalletHttpIpPort + STR_het_collectionquery, bytesRsp);


        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            strErrMsg = "json parse error:" + error.errorString();
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!jsonRsp.isObject())
        {
            strErrMsg = "external error: server response json is invalid." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jsonRsp.object();
        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            strErrMsg = "external error: server response json mssing `success` or `result`." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            strErrMsg = QString("server returns error: %1").arg(joRes.value("result").toString());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonArray jaResult = joRes.value("result").toArray();
        if(jaResult.isEmpty())
        {
            strErrMsg = QString("infomation: %1").arg("There're no addresses could be collectioned.");
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_vctCosmosCollectionQueryRsp.clear(); //清空历史数据
        for(int i = 0; i < jaResult.size(); i++)
        {
            QJsonObject joItem = jaResult.at(i).toObject();
            if(!(joItem.contains("account_number") && joItem.contains("sequence") && joItem.contains("balance") && joItem.contains("address")))
            {
                strErrMsg = QString("external error: server response json missing `account_number` or `sequence` or `balance` or `address`.");
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            CosmosCollectionQueryRsp cosmosClcRsp;
            cosmosClcRsp.strCoinType = strCoinType.trimmed().toUpper();
            cosmosClcRsp.strAmount = joItem.value("balance").toString().trimmed();
            cosmosClcRsp.strAddr = joItem.value("address").toString().trimmed();

            std::string strTmpSequence = joItem.value("sequence").toString().toStdString(); //json返回的就是str
            std::string strTmpAccountNumber = joItem.value("account_number").toString().toStdString();
            try{
                cosmosClcRsp.uAccountNumber = boost::lexical_cast<quint32>(strTmpAccountNumber);
                cosmosClcRsp.uSequence = boost::lexical_cast<quint32>(strTmpSequence);
            }catch(boost::bad_lexical_cast &e){
                strErrMsg = QString("server response json `sequence` or `account_number` is invalid. errMsg:%1").arg(QString(e.what()));
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            __m_vctCosmosCollectionQueryRsp.push_back(cosmosClcRsp);
            vctColRsp.push_back((CollectionQueryRsp)cosmosClcRsp); //直接截取, 仅用于显示
        }


    }

    else if( utils::Is_HRC20_Token(strCoinType) ) //HRC20 代币归集  2020-04-20
    {
        QByteArray bytesRsp;
        QString strReqUrl = g_qstr_WalletHttpIpPort + STR_htdf_collectionquery + "";
        strReqUrl += "?symbol=" + strCoinType;
        CAMMain::SyncGetData(strReqUrl , bytesRsp);

        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            strErrMsg = "json parse error:" + error.errorString();
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!jsonRsp.isObject())
        {
            strErrMsg = "external error: server response json is invalid." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jsonRsp.object();
        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            strErrMsg = "external error: server response json mssing `success` or `result`." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            strErrMsg = QString("server returns error: %1").arg(joRes.value("result").toString());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonArray jaResult = joRes.value("result").toArray();
        if(jaResult.isEmpty())
        {
            strErrMsg = QString("infomation: %1").arg("There're no addresses could be collectioned.(没有待归集地址)");
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_vctHrc20CollectionQueryRsp.clear(); //清空历史数据
        for(int i = 0; i < jaResult.size(); i++)
        {
            QJsonObject joItem = jaResult.at(i).toObject();
            if(!(joItem.contains("account_number") && joItem.contains("sequence")
                 && joItem.contains("balances") && joItem.contains("address"))) //注意 是 balances 有 's'
            {
                strErrMsg = QString("external error: server response json missing `account_number` or `sequence` or `balances` or `address`.");
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            Hrc20TokensCollectionQueryRsp tmpClcRsp;
            tmpClcRsp.strCoinType = strCoinType.trimmed().toUpper();
            //tmpClcRsp.strAmount = joItem.value("balance").toString().trimmed();
            tmpClcRsp.strAddr = joItem.value("address").toString().trimmed();

            std::string strTmpSequence = joItem.value("sequence").toString().toStdString(); //json返回的就是str
            std::string strTmpAccountNumber = joItem.value("account_number").toString().toStdString();
            try{
                tmpClcRsp.uAccountNumber = boost::lexical_cast<quint32>(strTmpAccountNumber);
                tmpClcRsp.uSequence = boost::lexical_cast<quint32>(strTmpSequence);
            }catch(boost::bad_lexical_cast &e){
                strErrMsg = QString("server response json `sequence` or `account_number` is invalid. errMsg:%1").arg(QString(e.what()));
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QJsonObject joBalances = joItem.value("balances").toObject();
            if(joBalances.isEmpty())
            {
                strErrMsg = QString("external error: server response json `balances` is empty.");
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            QString strMainType = "HTDF" ;
            if(  !(joBalances.contains(strMainType) && joBalances.contains(strCoinType)))
            {
                strErrMsg = QString("external error: server response json `balances` do not contains `%1` or `%2`").arg(strMainType).arg(strCoinType);
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            tmpClcRsp.strAmount =  joBalances.value(strCoinType).toString(); //HRC20代币余额
            tmpClcRsp.strHTDFBalance = joBalances.value(strMainType).toString();

            __m_vctHrc20CollectionQueryRsp.push_back( tmpClcRsp );
            vctColRsp.push_back( (CollectionQueryRsp)tmpClcRsp  );  //直接截断

        }

    }
    else if( 0 == strCoinType.compare("TRX", Qt::CaseInsensitive) )
    {
        QByteArray bytesRsp;
        QString strReqURL = g_qstr_WalletHttpIpPort + STR_trx_collectionquery;

        //strReqURL += "?symbol=" + QString(strCoinType).toLower();
        SyncGetData(strReqURL, bytesRsp);


        QJsonParseError error;
        QJsonDocument jsonRsp = QJsonDocument::fromJson( bytesRsp, &error);
        if(error.error != QJsonParseError::NoError)
        {
            strErrMsg = "json parse error:" + error.errorString();
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!jsonRsp.isObject())
        {
            strErrMsg = "external error: server response json is invalid." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jsonRsp.object();
        if(!(joRes.contains("success") && joRes.contains("result")))
        {
            strErrMsg = "external error: server response json mssing `success` or `result`." ;
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( 0 != joRes.value("success").toString().compare("true", Qt::CaseInsensitive))
        {
            strErrMsg = QString("server returns error: %1").arg(joRes.value("result").toString());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonArray jaResult = joRes.value("result").toArray();
        if(jaResult.isEmpty())
        {
            strErrMsg = QString("infomation: %1").arg("There're no addresses could be collectioned.");
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_vctTrxCollectionQueryRsp.clear(); //清空历史数据
        for(int i = 0;  i < jaResult.size(); i++)
        {
            QJsonObject joItem = jaResult.at(i).toObject();
            if(!(joItem.contains("symbol") && joItem.contains("balance") && joItem.contains("address")))
            {
                strErrMsg = QString("external error: server response json missing `symbol` or `balance` or `address`.");
                qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            CollectionQueryRsp tmpClcRsp;
            tmpClcRsp.strCoinType = strCoinType;
            tmpClcRsp.strAddr = joItem.value("address").toString().trimmed();
            tmpClcRsp.strAmount = joItem.value("balance").toString().trimmed();

            __m_vctTrxCollectionQueryRsp.push_back(tmpClcRsp);

            vctColRsp.push_back( tmpClcRsp ); //直接截取, 仅用于显示
        }

    }
    else
    {
        strErrMsg = QString("operation error: not support %1 in current version.").arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    return NO_ERROR;
}



int CPageCollection::ConditionSearch_Collection_Log(Collection::SEARCHCOND &searchCond,
                                                    vector<Collection::COLLECTIONINFO> &vctCollectionInfo) noexcept(false)
{
    QString strErrMsg;

    //构造查询语句
    QString strQuery;
    strQuery = QString("select coin_type,order_id,src_addr,dst_addr,amount,token_amount,family_name,tx_fee, tb_collection.status,"
                       "complete_time,txid from tb_collection,tb_admin where tb_collection.admin_id=tb_admin.admin_id and "
                       "complete_time > %1 and complete_time < %2 ")
            .arg(searchCond.uDatetimeStart).arg(searchCond.uDatetimeEnd);

    if (!searchCond.strCoinType.isEmpty())
        strQuery += QString("and coin_type = \'%1\' ").arg(searchCond.strCoinType);
    if (!searchCond.strAdminName.isEmpty())
        strQuery += QString("and family_name = \'%1\' ").arg(searchCond.strAdminName);
    strQuery += QString("order by complete_time DESC;");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " collection_Log-SQL: " << strQuery;

    QSqlQuery query(_m_db);
    if(!query.exec(strQuery))
    {
        strErrMsg = QString("sql error: %1").arg(query.lastError().text());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    __m_vctCollectionCreateItem.clear();
    while(query.next())
    {
        Collection::COLLECTIONINFO clcItem;

        clcItem.strCoinType = query.value(0).toString();
        clcItem.strOrderID = query.value(1).toString();
        clcItem.strSrcAddr = query.value(2).toString();
        clcItem.strDstAddr = query.value(3).toString();
        clcItem.strAmount = query.value(4).toString();
        clcItem.strTokenAmount = query.value(5).toString();
        clcItem.strAdminName = query.value(6).toString();
        clcItem.strTxFee = query.value(7).toString();
        clcItem.strStatus = "已完成";
        clcItem.strTime = QDateTime::fromTime_t(query.value(9).toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
        clcItem.strTxid = query.value(10).toString();

        __m_vctCollectionCreateItem.push_back(clcItem);
        vctCollectionInfo.push_back(clcItem);
//        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "collection_id = " << clcItem.strOrderID;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询归集数据成功";
    return NO_ERROR;
}



int CPageCollection::CreateAndExportUnsignedRawTx(
        const QString &strCoinType,
        const vector<int> &vctSelRows,
        const vector<QString> &vctSrcAddrs,
        const QString& strDstAddr,
        QString &strRetExportFilePath)noexcept(false)
{
    QString strErrMsg;
    if(0 == vctSrcAddrs.size())
    {
        strErrMsg = "args error: vctAddrs is empty, please select rows first.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(0 == vctSelRows.size())
    {
        strErrMsg = "args error: vctSelRows is empty, please select rows first.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if( !utils::IsValidAddr(strCoinType, strDstAddr)  )
    {
        strErrMsg = QString("operation error: destination address %1 is illegal %2 address, please check again.").arg(strDstAddr).arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    strRetExportFilePath = "";
    strRetExportFilePath += QString(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    strRetExportFilePath += QString("_%1_collection_unsigned_RawTx.json").arg(strCoinType.toLower());

    if(0 == strCoinType.compare("BTC", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
        __m_btcExportData = BTCExportData(); //清空

        BTCExportItem  btcExportItem;
        btcExportItem.vctSrcAddr = vctSrcAddrs;
        btcExportItem.strOrderID = utils::GenOrderID();

        btcExportItem.vctDstAddr.push_back(strDstAddr);

        double dAmount = 0.0;
        double dTxFee = 0.0;
        if(0 == strCoinType.compare("BTC", Qt::CaseInsensitive))
        {
            CBTCRawTxUtil().CreateRawTransactionEx_Collection(
                    g_qstr_WalletHttpIpPort + STR_btc_createrawtransactionex_collection,
                    btcExportItem.vctSrcAddr, btcExportItem.vctDstAddr,  btcExportItem.strTxHex,
                    btcExportItem.mapTxOut, btcExportItem.vctUTXOs, dTxFee , dAmount);
        }
        else if(0 == strCoinType.compare("LTC", Qt::CaseInsensitive))
        {
            CLTCRawTxUtil().CreateRawTransactionEx_Collection(
                    g_qstr_WalletHttpIpPort + STR_ltc_createrawtransactionex_collection,
                    btcExportItem.vctSrcAddr, btcExportItem.vctDstAddr,  btcExportItem.strTxHex,
                    btcExportItem.mapTxOut, btcExportItem.vctUTXOs, dTxFee , dAmount);
        }
        else if(0 == strCoinType.compare("BCH", Qt::CaseInsensitive))
        {
            CBCHRawTxUtil().CreateRawTransactionEx_Collection(
                    g_qstr_WalletHttpIpPort + STR_bch_createrawtransactionex_collection,
                    btcExportItem.vctSrcAddr, btcExportItem.vctDstAddr,  btcExportItem.strTxHex,
                    btcExportItem.mapTxOut, btcExportItem.vctUTXOs, dTxFee , dAmount);
        }
         else if(0 == strCoinType.compare("BSV", Qt::CaseInsensitive))
        {
            CBSVRawTxUtil().CreateRawTransactionEx_Collection(
                    g_qstr_WalletHttpIpPort + STR_bsv_createrawtransactionex_collection,
                    btcExportItem.vctSrcAddr, btcExportItem.vctDstAddr,  btcExportItem.strTxHex,
                    btcExportItem.mapTxOut, btcExportItem.vctUTXOs, dTxFee , dAmount);
        }
         else if(0 == strCoinType.compare("DASH", Qt::CaseInsensitive))
        {
            CDASHRawTxUtil().CreateRawTransactionEx_Collection(
                    g_qstr_WalletHttpIpPort + STR_dash_createrawtransactionex_collection,
                    btcExportItem.vctSrcAddr, btcExportItem.vctDstAddr,  btcExportItem.strTxHex,
                    btcExportItem.mapTxOut, btcExportItem.vctUTXOs, dTxFee , dAmount);
        }

        btcExportItem.strTxFee = QString::asprintf("%.8f", dTxFee);
        btcExportItem.strAmount = QString::asprintf("%.8f", dAmount);

        __m_btcExportData.vctExportItems.push_back(btcExportItem);
        __m_btcExportData.bIsCollection = true;
        __m_btcExportData.strCoinType = strCoinType.trimmed();
        __m_btcExportData.uTimestamp = QDateTime::currentSecsSinceEpoch();


        if(__m_btcExportData.vctExportItems.empty())
        {
            strErrMsg = QString("There were not any %1 transactions be found, maybe errors occurs in create transactions.").arg(strCoinType);
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString() );
        }

        //导出已签名的BTC交易数据
        if(0 == strCoinType.compare("BTC", Qt::CaseInsensitive))
        {
            CBTCRawTxUtil().ExportRawTxToJsonFile(strRetExportFilePath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("LTC", Qt::CaseInsensitive))
        {
            CLTCRawTxUtil().ExportRawTxToJsonFile(strRetExportFilePath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("BCH", Qt::CaseInsensitive))
        {
            CBCHRawTxUtil().ExportRawTxToJsonFile(strRetExportFilePath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("BSV", Qt::CaseInsensitive))
        {
            CBSVRawTxUtil().ExportRawTxToJsonFile(strRetExportFilePath, __m_btcExportData);
        }
        else if(0 == strCoinType.compare("DASH", Qt::CaseInsensitive))
        {
            CDASHRawTxUtil().ExportRawTxToJsonFile(strRetExportFilePath, __m_btcExportData);
        }

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strRetExportFilePath << "导出成功";
    }
    else if(0 == strCoinType.compare("USDT", Qt::CaseInsensitive))
    {
        __m_usdtExportData = USDTExportData(); //清空

        QString strURL = g_qstr_WalletHttpIpPort + STR_usdt_createrawtransactionex_collection;
        __m_usdtRawTxUtil.CreateRawTransactionEx_Collection(strURL, vctSrcAddrs, strDstAddr, __m_usdtExportData);

        if(__m_usdtExportData.vctExportItems.empty() )
        {
            strErrMsg = QString("There were not any %1 transactions be found, maybe errors occurs in create transactions.").arg(strCoinType);
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString() );
        }

        //导出未签名USDT交易
        __m_usdtRawTxUtil.ExportRawTxToJsonFile(strRetExportFilePath, __m_usdtExportData);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strRetExportFilePath << "导出成功";
    }
    else if(0 == strCoinType.compare("ETH", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("ETC", Qt::CaseInsensitive) )
    {
        std::vector<ETHCollectionQeuryRsp> const &vctEthRsp = __m_vctEthCollectionQueryRsp;
        if(vctSelRows.size() > vctEthRsp.size())
        {
            strErrMsg = "internal error: vctSelRows's size is greater than vctEthRsp";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_ethExportRawTxParams.clear();

        double dClcEthFee  = utils::GetETH_ERC20_CollectionFee(strCoinType);

        if(utils::IsERC20Token(strCoinType))
        {
            for(auto iRow : vctSelRows)
            {
                ETHCollectionQeuryRsp const &rspItem = vctEthRsp[iRow];

                //计算  归集金额 = 余额 - 手续费(FLOAT_ERC20_NEED_ETH_FEE  ETH)
                QString strRealTxAmount = "";
                try{
                    double dTxAmount =  boost::lexical_cast<double>(rspItem.strAmount.toStdString());
                    //dTxAmount = dTxAmount - FLOAT_ERC20_NEED_ETH_FEE;
                    dTxAmount = dTxAmount - dClcEthFee;
                    if (dTxAmount <=  0)
                    {
                        strErrMsg = QString("internal error: source balance(%1) is too small to be collectioned.").arg(rspItem.strAmount);
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                        throw runtime_error(strErrMsg.toStdString());
                    }

                    strRealTxAmount = QString::asprintf("%.8f", dTxAmount);
                }catch(boost::bad_lexical_cast &e){
                    strErrMsg = QString("internal error: strAmount cast failed , error: %1").arg(e.what());
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                rawtx::ETH_ExportRawTxItem expItem;
                expItem.uChainId        = (g_bBlockChainMainnet) ? (eth::ETHChainID::Mainnet) : ( eth::ETHChainID::Rinkeby);
                expItem.strAddrFrom     = rspItem.strAddr;     //源地址(被归集的地址)
                expItem.strERC20Recipient = strDstAddr;         //ERC20 Token的接收地址(归集的目的地址)
                expItem.strAddrTo       = utils::GetERC20ContractAddr(strCoinType); //代币合约地址
                expItem.strValue        = "0";     //归集金额 = 余额 - 手续费
                expItem.strERC20TokenValue = rspItem.strAmount; //ERC20 Token的金额
                expItem.strGasPrice     = STR_COLLECTION_GASPRICE_WEI;//gasprice,
                expItem.strGasStart     = utils::GetGasLimit(strCoinType); //gasstart, 也称 gaslimit
                expItem.strOrderId      = utils::GenOrderID();   //订单编号
                expItem.strNonce        = QString::asprintf("%u", rspItem.uNonce);
                expItem.strSymbol       = strCoinType.toUpper();

                __m_ethExportRawTxParams.push_back(expItem);
            }
        }
        else //ETH  & ETC
        {
            for(auto iRow : vctSelRows)
            {
                ETHCollectionQeuryRsp const &rspItem = vctEthRsp[iRow];

                //计算  归集金额 = 余额 - 手续费( FLOAT_NORMAL_ETH_TX_FEE ETH)
                QString strRealTxAmount = "";
                try{
                    double dTxAmount =  boost::lexical_cast<double>(rspItem.strAmount.toStdString());
                    dTxAmount = dTxAmount - dClcEthFee;//FLOAT_NORMAL_ETH_TX_FEE;
                    if (dTxAmount <=  0)
                    {
                        strErrMsg = QString("internal error: source balance(%1) is too small to be collectioned.").arg(rspItem.strAmount);
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                        throw runtime_error(strErrMsg.toStdString());
                    }

                    strRealTxAmount = QString::asprintf("%.8f", dTxAmount);
                }catch(boost::bad_lexical_cast &e){
                    strErrMsg = QString("internal error: strAmount cast failed , error: %1").arg(e.what());
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                rawtx::ETH_ExportRawTxItem expItem;
                expItem.uChainId        = (g_bBlockChainMainnet) ? (eth::ETHChainID::Mainnet) : ( eth::ETHChainID::Rinkeby);
                if(g_bBlockChainMainnet  && (0 == strCoinType.compare("ETC", Qt::CaseInsensitive)))
                    expItem.uChainId        = (eth::ETHChainID::EthereumClassic);

                expItem.strAddrFrom     = rspItem.strAddr;     //源地址(被归集的地址)
                expItem.strAddrTo       = strDstAddr;          //归集目的地址
                expItem.strValue        = strRealTxAmount;     //归集金额 = 余额 - 手续费
                expItem.strGasPrice     = STR_COLLECTION_GASPRICE_WEI; //gasprice,
                expItem.strGasStart     = STR_NORMAL_ETH_TX_GASLIMIT; //gasstart, 也称 gaslimit
                expItem.strOrderId      = utils::GenOrderID();   //订单编号
                expItem.strNonce        = QString::asprintf("%u", rspItem.uNonce);
                expItem.strSymbol       = strCoinType.toUpper();

                __m_ethExportRawTxParams.push_back(expItem);
            }
        }

        __m_ethExportRawTxParams.m_bIsCollection = true;
        __m_ethExportRawTxParams.m_strCoinType = strCoinType.toUpper().trimmed();
        __m_ethExportRawTxParams.m_strFilePath = strRetExportFilePath;

        __m_ethRawTxUtils.ExportRawTx(&__m_ethExportRawTxParams);
    }
    else if(0 == strCoinType.compare("USDP", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HTDF", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("HET", Qt::CaseInsensitive))
    {
        std::vector<CosmosCollectionQueryRsp> const &vctCosmosRsp = __m_vctCosmosCollectionQueryRsp;
        if(vctSelRows.size() > vctCosmosRsp.size())
        {
            strErrMsg = "internal error: vctSelRows's size is greater than vctCosmosRsp";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_cosmosExportRawTxParams.clear(); //清空历史记录

        for(auto iRow : vctSelRows)
        {
            CosmosCollectionQueryRsp const &rspItem = vctCosmosRsp[iRow];

            bool isHTDF = (0 == strCoinType.compare(STR_HTDF, Qt::CaseInsensitive));

            //计算  归集金额 = 余额 - 手续费
            QString strRealTxAmount = "";
            try{
                double dTxAmount =  boost::lexical_cast<double>(rspItem.strAmount.toStdString());

                if(0 == strCoinType.compare(STR_HTDF, Qt::CaseInsensitive))
                    dTxAmount = dTxAmount - 0.03000000;  //因为feeAmount 是 100 satoshi, gaswanted(gaslimit) 30000,   100 * 30000 satoshi = 0.03 HTDF
                else
                    dTxAmount = dTxAmount - 0.00000020;

                if (dTxAmount <=  0)
                {
                    strErrMsg = QString("internal error: source balance(%1) is too small to be collectioned.").arg(rspItem.strAmount);
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                strRealTxAmount = QString::asprintf("%.8f", dTxAmount);
            }catch(boost::bad_lexical_cast &e){
                strErrMsg = QString("internal error: strAmount cast failed , error: %1").arg(e.what());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            rawtx::Cosmos_ExportRawTxItem expItem;
            expItem.strOrderId      = utils::GenOrderID(); //随机生成订单号
            expItem.strCoinType     = strCoinType.toLower().trimmed();
            expItem.strChainId      = (g_bBlockChainMainnet) ? (STR_MAINCHAIN) : (STR_TESTCHAIN);
            expItem.strFrom         = rspItem.strAddr.trimmed(); //源地址
            expItem.strTo           = strDstAddr.trimmed(); //归集的目的地址
            expItem.strValue        = strRealTxAmount;
            expItem.strFeeAmount    = (isHTDF) ? ("100") : ("20"); //satoshi
            expItem.strFeeGas       = (isHTDF) ? ("30000"): ("200000") ;  //satoshi   TODO:可配置
            expItem.strMemo         = expItem.strOrderId;  //默认使用订单号作为memo
            expItem.strSequence = QString::asprintf("%u", rspItem.uSequence);
            expItem.strAccountNumber = QString::asprintf("%u", rspItem.uAccountNumber);

            __m_cosmosExportRawTxParams.push_back(expItem);
        }

        __m_cosmosExportRawTxParams.m_bIsCollection = true;
        __m_cosmosExportRawTxParams.m_strCoinType =  strCoinType.toLower().trimmed();
        __m_cosmosExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("%1_collection_export_unsigned_RawTx.json").arg(strCoinType.toLower().trimmed());


        __m_cosmosRawTxUtils.ExportRawTx(&__m_cosmosExportRawTxParams);

    }
    else if( utils::Is_HRC20_Token( strCoinType )  )
    {
        typedef Hrc20TokensCollectionQueryRsp Hrc20Rsp;
        const std::vector<Hrc20Rsp> &vctHrc20Rsp = __m_vctHrc20CollectionQueryRsp;
        if(vctSelRows.size() > vctHrc20Rsp.size())
        {
            strErrMsg = "internal error: vctSelRows's size is greater than vctHrc20Rsp";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        __m_cosmosExportRawTxParams.clear();

        //计算  归集金额(HRC20代币金额) = 余额
        for(auto iRow : vctSelRows)
        {
            const Hrc20Rsp &crefRsp = vctHrc20Rsp[iRow];

            rawtx::Cosmos_ExportRawTxItem expItem;
            expItem.strOrderId      = utils::GenOrderID(); //随机生成订单号
            expItem.strCoinType     = strCoinType.toLower().trimmed();
            expItem.strChainId      = (g_bBlockChainMainnet) ? (STR_MAINCHAIN) : (STR_TESTCHAIN);
            expItem.strFrom         = crefRsp.strAddr.trimmed(); //源地址,
            expItem.strTo           = strDstAddr.trimmed(); //归集的目的地址 ,注意: 离线端再替换为合约地址
            expItem.strValue        = crefRsp.strAmount;  //金额,  注意: 离线端再替换为 0,  可以参考人工提币
            expItem.strFeeAmount    = "100"; //satoshi
            expItem.strFeeGas       = "200000";  //satoshi
            expItem.strMemo         = expItem.strOrderId;  //默认使用订单号作为memo
            expItem.strSequence = QString::asprintf("%u", crefRsp.uSequence);
            expItem.strAccountNumber = QString::asprintf("%u", crefRsp.uAccountNumber);

            __m_cosmosExportRawTxParams.push_back(expItem);
        }

        __m_cosmosExportRawTxParams.m_bIsCollection = true;
        __m_cosmosExportRawTxParams.m_strCoinType =  strCoinType.toLower().trimmed();
        __m_cosmosExportRawTxParams.m_strFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + \
                QString("HRC20_%1_collection_export_unsigned_RawTx.json").arg(strCoinType.toUpper().trimmed());


        __m_cosmosRawTxUtils.ExportRawTx(&__m_cosmosExportRawTxParams);

    }
    else if( 0 == strCoinType.compare("TRX", Qt::CaseInsensitive))
    {

        std::vector<CollectionQueryRsp> const &vctTrxRsp = __m_vctTrxCollectionQueryRsp;
        if(vctSelRows.size() > vctTrxRsp.size())
        {
            strErrMsg = "internal error: vctSelRows's size is greater than vctEthRsp";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //清空历史记录
        __m_trxExportRawTxParams.clear();


        for(auto iRow : vctSelRows)
        {
            CollectionQueryRsp const &rspItem = vctTrxRsp[iRow];

            //计算  归集金额 = 余额 - 手续费
            QString strRealTxAmount = "";
            try{
                double dTxAmount =  boost::lexical_cast<double>(rspItem.strAmount.toStdString());
                dTxAmount = dTxAmount - 0.1; //TODO: 手续费待精确
                if (dTxAmount <=  0)
                {
                    strErrMsg = QString("internal error: source balance(%1) is too small to be collectioned.").arg(rspItem.strAmount);
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                strRealTxAmount = QString::asprintf("%.8f", dTxAmount);
            }catch(boost::bad_lexical_cast &e){
                strErrMsg = QString("internal error: strAmount cast failed , error: %1").arg(e.what());
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }


             //2.1 构造请求参数
            rawtx::Trx_CreateRawTxParam  param;
            param.bSigned           = false;
            param.strCoinType       = rspItem.strCoinType;
            param.strSrcAcct        = rspItem.strAddr;
            param.strTxid           = "";  //交易摘要,  注意: 此处的txid 是 raw_data_hex 的hash值, 即  sha256(raw_data_hex)
            param.strSignedRawTx    = "";
            param.dAmount           = strRealTxAmount.toDouble();
            param.strAmount         = strRealTxAmount;
            param.strDstAcct        = strDstAddr;

            __m_trxRawTxUtils.CreateRawTx( &param );


            //2.2 放入导出缓冲区
            rawtx::Trx_ExportRawTxItem expItem;

            expItem.bComplete       = false;
            expItem.strSrcAcct      = rspItem.strAddr;     //源地址(被归集的地址)
            expItem.strDstAcct      = strDstAddr;          //归集目的地址
            expItem.strAmount       = strRealTxAmount;     //归集金额 = 余额 - 手续费
            expItem.strOrderId      = utils::GenOrderID();   //订单编号
            expItem.strCoinType     = strCoinType.toUpper();
            expItem.dAmount         =  param.dAmount;  //因为  要扣除手续费 0.1 TRX

            expItem.strSignedRawTx  = param.strSignedRawTx;
            expItem.strTxid         = param.strTxid;

            __m_trxExportRawTxParams.push_back(expItem);
        }

        __m_trxExportRawTxParams.m_bIsCollection = true;
        __m_trxExportRawTxParams.m_strCoinType = strCoinType.toUpper().trimmed();
        __m_trxExportRawTxParams.m_strFilePath = strRetExportFilePath;

        __m_trxRawTxUtils.ExportRawTx(&__m_trxExportRawTxParams);

    }
    else
    {
        strErrMsg = QString("operation error: not support %1 in current version.").arg(strCoinType);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("创建未签名归集交易成功");
    return NO_ERROR;
}

int CPageCollection::ImportRawTxFile(const QString& strCoinType, const QString &strImportFilePath)
{
    QString strErrMsg;

    QString strFilePath = strImportFilePath.trimmed();
    if(strFilePath.trimmed().isEmpty())
    {
        strErrMsg = "args error: import file path is empty.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QFile fileTmp(strFilePath);
    if(!fileTmp.exists())
    {
        strErrMsg = "args error: import file is not exists.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if(0 == strCoinType.compare("btc",Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
        //清空历史信息
        __m_btcImportData = BTCImportData();

        if( 0 == strCoinType.compare("btc",Qt::CaseInsensitive))
        {
            CBTCRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if( 0 == strCoinType.compare("ltc",Qt::CaseInsensitive) )
        {
            CLTCRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if( 0 == strCoinType.compare("bch",Qt::CaseInsensitive) )
        {
            CBCHRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if( 0 == strCoinType.compare("bsv",Qt::CaseInsensitive) )
        {
            CBSVRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }
        else if( 0 == strCoinType.compare("dash",Qt::CaseInsensitive) )
        {
            CDASHRawTxUtil().ImportRawTxFromJsonFile(strImportFilePath, __m_btcImportData);
        }



        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(strImportFilePath);
    }
    else if( 0 == strCoinType.compare("usdt", Qt::CaseInsensitive ))
    {
        //清空历史信息
        __m_usdtImportData = BTCImportData();

        __m_usdtRawTxUtil.ImportRawTxFromJsonFile(strImportFilePath, __m_usdtImportData);

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(strImportFilePath);
    }
    else if(0 == strCoinType.compare("eth",Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("etc",Qt::CaseInsensitive) )
    {
        //清空历史数据
        __m_ethImportRawTxParams = rawtx::ETH_ImportRawTxParam();

        //设置导入文件路径
        __m_ethImportRawTxParams.m_strFilePath = strImportFilePath;
        __m_ethImportRawTxParams.m_strCoinType = strCoinType.toUpper().trimmed();


        //导入数据
        __m_ethRawTxUtils.ImportRawTx( &__m_ethImportRawTxParams );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(strImportFilePath);
    }
    else if(0 == strCoinType.compare("usdp",Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf",Qt::CaseInsensitive)
            || 0 == strCoinType.compare("het",Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType)
            )
    {
        //清空历史数据
        __m_cosmosImportRawTxParams = rawtx::Cosmos_ImportRawTxParam();

        //导入文件文件路径
        __m_cosmosImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_cosmosImportRawTxParams.m_strFilePath = strFilePath;
        __m_cosmosRawTxUtils.ImportRawTx( &__m_cosmosImportRawTxParams );
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(strFilePath);
    }

    else if ( 0 == strCoinType.compare("trx",Qt::CaseInsensitive) )
    {
         //清空历史数据
        __m_trxImportRawTxParams = rawtx::Trx_ImportRawTxParam();


        //设置导入文件路径
        __m_trxImportRawTxParams.m_strCoinType = strCoinType.toLower().trimmed();
        __m_trxImportRawTxParams.m_strFilePath = strImportFilePath;

        //导入数据
        __m_trxRawTxUtils.ImportRawTx( &__m_trxImportRawTxParams );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("import signed tx file: \"%1\" successed.").arg(strImportFilePath);
    }
    else
    {
        strErrMsg = QString("operation error: not support %1 in current version.").arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("导入已签名交易文件成功");
    return NO_ERROR;
}

int CPageCollection::BroadcastRawTx(const QString& strCoinType, const vector<int> &vctRowsSel,  vector<QString> &vctStrTxid) noexcept(false)
{
    QString strErrMsg;

    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
     )
    {
        _BTC_BroadcastRawTx(vctRowsSel,  vctStrTxid);
    }
    else if(0 == strCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        _USDT_BroadcastRawTx(vctRowsSel, vctStrTxid);
    }
    else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
        || 0 == strCoinType.compare("etc", Qt::CaseInsensitive)
        )
    {
        _ETH_BroadcastRawTx(vctRowsSel, strCoinType);
    }
    else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("het", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCoinType)
             )
    {
        _Cosmos_BroadcastRawTx(vctRowsSel);
    }
    else if(0 == strCoinType.compare("trx", Qt::CaseInsensitive))
    {
        _TRX_BroadcastRawTx(vctRowsSel, strCoinType);
    }
    else
    {
        strErrMsg = QString("operation error: not support %1 in current version.").arg(__m_btcImportData.strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("%1广播交易成功").arg(__m_btcImportData.strCoinType);
    return NO_ERROR;
}

int CPageCollection::_BTC_BroadcastRawTx(const vector<int> &vctRowsSel,  vector<QString> &vctStrTxid) noexcept(false)
{
    QString strErrMsg;

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(auto iRow : vctRowsSel)
    {
        if((uint)iRow >= __m_btcImportData.vctExportItems.size())
        {
            strErrMsg = QString("selected count greater than imported count");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        BTCImportItem importItem = __m_btcImportData.vctExportItems[iRow];

        QString strTxidRet = "";

        //TODO: 批量广播时, 处理有成功有失败 的情况
        if( 0 == __m_btcImportData.strCoinType.compare("btc",Qt::CaseInsensitive))
        {
            CBTCRawTxUtil().BroadcastRawTx(g_qstr_WalletAPI_btc_sendrawtransaction,  importItem.strTxHex, strTxidRet);
        }
        else if( 0 == __m_btcImportData.strCoinType.compare("ltc",Qt::CaseInsensitive))
        {
            CLTCRawTxUtil().BroadcastRawTx(g_qstr_WalletAPI_ltc_sendrawtransaction,  importItem.strTxHex, strTxidRet);
        }
        else if( 0 == __m_btcImportData.strCoinType.compare("bch",Qt::CaseInsensitive))
        {
            CBCHRawTxUtil().BroadcastRawTx(g_qstr_WalletAPI_bch_sendrawtransaction,  importItem.strTxHex, strTxidRet);
        }
        else if( 0 == __m_btcImportData.strCoinType.compare("bsv",Qt::CaseInsensitive))
        {
            CBSVRawTxUtil().BroadcastRawTx(g_qstr_WalletAPI_bsv_sendrawtransaction,  importItem.strTxHex, strTxidRet);
        }
        else if( 0 == __m_btcImportData.strCoinType.compare("dash",Qt::CaseInsensitive))
        {
            CDASHRawTxUtil().BroadcastRawTx(g_qstr_WalletAPI_dash_sendrawtransaction,  importItem.strTxHex, strTxidRet);
        }



        vctStrTxid.push_back(strTxidRet);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << __m_btcImportData.vctExportItems[iRow].strTxHex << QString("交易广播成功, txid=") << strTxidRet;

        QString strTmpSrcAddr;
        for(auto strAddr : __m_btcImportData.vctExportItems[iRow].vctSrcAddr)
        {
            if(!strTmpSrcAddr.isEmpty()) strTmpSrcAddr += ",";
            strTmpSrcAddr += strAddr.trimmed() ;
        }

        QString strTmpDstAddr;
        for(auto strAddr : __m_btcImportData.vctExportItems[iRow].vctDstAddr)
        {
            if(!strTmpDstAddr.isEmpty()) strTmpDstAddr += ",";
            strTmpDstAddr += strAddr.trimmed() ;
        }


        //更新数据库, 将交易id写入数据库
        QString strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')").arg(importItem.strOrderID)
                .arg(uAdminId).arg(__m_btcImportData.strCoinType).arg(strTmpSrcAddr).arg(strTmpDstAddr).arg(importItem.strAmount).arg(""/*importItem.strTokenAmount*/)
                .arg(importItem.strTxFee).arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("交易广播完成后,开始插入数据库 sql = ")<< strSql;

        QSqlQuery  query(_m_db);
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
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("交易广播完成后,更新数据库成功, sql = ")<< strSql;
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << __m_btcImportData.strCoinType  << QString("交易广播广播成功");
    return NO_ERROR;
}

int CPageCollection::_USDT_BroadcastRawTx(const vector<int> &vctRowsSel,  vector<QString> &vctStrTxid) noexcept(false)
{
    QString strErrMsg;

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(auto iRow : vctRowsSel)
    {
        if((uint)iRow >= __m_usdtImportData.vctExportItems.size())
        {
            strErrMsg = QString("selected count greater than imported count");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        BTCImportItem importItem = __m_usdtImportData.vctExportItems[iRow];

        QString strTxidRet = "";
        // fix bug: 2020-03-30 yqq
        //如果是 USDT 则使用 USDT 自己的节点广播, 不然会导致 交易池的差异,
        //导致双花问题, 因为创建的时候使用USDT节点, 而广播的使用使用的是 USDT节点,
        //__m_usdtRawTxUtil.BroadcastRawTx(g_qstr_WalletAPI_btc_sendrawtransaction, importItem.strTxHex, strTxidRet);
        __m_usdtRawTxUtil.BroadcastRawTx(g_qstr_WalletHttpIpPort + STR_usdt_sendrawtransaction, importItem.strTxHex, strTxidRet);

        vctStrTxid.push_back(strTxidRet);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << __m_usdtImportData.vctExportItems[iRow].strTxHex << QString("usdt交易广播成功, txid=") << strTxidRet;

        QString strTmpSrcAddr;
        for(auto strAddr : __m_usdtImportData.vctExportItems[iRow].vctSrcAddr)
        {
            if(!strTmpSrcAddr.isEmpty()) strTmpSrcAddr += ",";
            strTmpSrcAddr += strAddr.trimmed() ;
        }

        QString strTmpDstAddr;
        for(auto strAddr : __m_usdtImportData.vctExportItems[iRow].vctDstAddr)
        {
            if(!strTmpDstAddr.isEmpty()) strTmpDstAddr += ",";
            strTmpDstAddr += strAddr.trimmed() ;
        }


        //更新数据库, 将交易id写入数据库
        QSqlQuery query(_m_db);


        QString strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(importItem.strOrderID).arg(uAdminId).arg(__m_usdtImportData.strCoinType).arg(strTmpSrcAddr)
                .arg(strTmpDstAddr).arg(importItem.strAmount).arg(importItem.strTokenAmount).arg(importItem.strTxFee)
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);

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
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("交易广播完成后,更新数据库成功, sql = ")<< strSql;
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("usdt广播成功");
    return NO_ERROR;
}

int CPageCollection::_ETH_BroadcastRawTx(const vector<int> &vctRowsSel, QString strCoinType/*="ETH"*/) noexcept(false)
{
    QString strErrMsg;

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //注意: vctRowSel 中的顺序要 与 __m_ethImportRawTxParams中数据顺序一致, 否则会出错
    for(auto iRow : vctRowsSel)
    {
        if((size_t)iRow >= __m_ethImportRawTxParams.size())
        {
            strErrMsg = QString("selected count greater than imported count");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        rawtx::ETH_BroadcastRawTxParam  bcastParam;
        bcastParam.strCoinType          = strCoinType;
        bcastParam.strAddrFrom          = __m_ethImportRawTxParams[iRow].strAddrFrom;  //源地址
        bcastParam.strNonce             = __m_ethImportRawTxParams[iRow].strNonce;     //nonce
        bcastParam.uChainId             = __m_ethImportRawTxParams[iRow].uChainId;     //uChainId
        bcastParam.strSignedRawTxHex    = __m_ethImportRawTxParams[iRow].strSignedRawTxHex; //签名交易
        bcastParam.strOrderId           = __m_ethImportRawTxParams[iRow].strOrderId;    //订单号

        if(true)
        {
            QSqlQuery query(_m_db);
            if(false == query.exec(QString("select * from tb_collection where order_id='%1';").arg(bcastParam.strOrderId)))
            {
                strErrMsg =  QString("DB error: query database error..");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            if(0 < query.size() )
            {
                strErrMsg =  QString("debug info: order_id:%1 had  successfuly broadcated . so skip it now.");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                continue;
            }
        }

        __m_ethRawTxUtils.BroadcastRawTx( &bcastParam );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "broadcast successed.";
        QString strTxidRet = bcastParam.strTxid;

        //更新数据库, 将交易id写入数据库
        QThread::msleep(10);
        QSqlQuery query(_m_db);



        rawtx::ETH_ImportRawTxItem const &ethItem = __m_ethImportRawTxParams[iRow];
        QString strCoinType  = ethItem.strSymbol;
        strCoinType = strCoinType.toUpper();

        QString strSql = "";
        if(utils::IsERC20Token(strCoinType))
        {
            strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(ethItem.strOrderId).arg(uAdminId).arg( QString(strCoinType/*如果是ERC20,再修改*/))
                .arg(ethItem.strAddrFrom).arg(ethItem.strERC20Recipient).arg(ethItem.strERC20TokenValue).arg("---").arg("0.006")
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);
        }
        else
        {
            strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(ethItem.strOrderId).arg(uAdminId).arg( QString(strCoinType/*如果是ERC20,再修改*/))
                .arg(ethItem.strAddrFrom).arg(ethItem.strAddrTo).arg(ethItem.strValue).arg("---").arg("0.006")
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);
        }

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql :" << strSql;

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
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid:%1,成功").arg(strTxidRet);
    }
    return NO_ERROR;
}


int CPageCollection::_Cosmos_BroadcastRawTx(const vector<int> &vctRowsSel) noexcept(false)
{
    QString strErrMsg;

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(auto iRow : vctRowsSel)
    {
        if((uint)iRow >= __m_cosmosImportRawTxParams.size())
        {
            strErrMsg = QString("selected count greater than imported count");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //JudgeBroadcastable(__m_cosmosImportRawTxParams[iRow].strOrderId);



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


        if(true)
        {
            QSqlQuery query(_m_db);
            if(false == query.exec(QString("select * from tb_collection where order_id='%1';").arg(param.strOrderId)))
            {
                strErrMsg =  QString("DB error: query database error.");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            if(0 < query.size() )
            {
                strErrMsg =  QString("debug info: order_id:%1 had  successfuly broadcated . so skip it now.");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                continue;
            }
        }


        __m_cosmosRawTxUtils.BroadcastRawTx( &param );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                    "  __m_cosmosRawTxUtils.BroadcastRawTx(): broadcast successed. orderId:"<< param.strOrderId << " txid:"  << param.strTxid ;


        QString strTxidRet = param.strTxid;
        //更新数据库, 将交易id写入数据库
        QThread::msleep(10);
        QSqlQuery query(_m_db);

        rawtx::Cosmos_ImportRawTxItem const &cosmosItem = __m_cosmosImportRawTxParams[iRow];
        QString strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(cosmosItem.strOrderId).arg(uAdminId).arg(cosmosItem.strCoinType.toUpper())
                .arg(cosmosItem.strFrom).arg(cosmosItem.strTo).arg(cosmosItem.strValue).arg("---").arg("0.0000002")
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);

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

    return NO_ERROR;
}


int CPageCollection::ERC20SupplyTxFee( const vector<QString> &vctAddrs, const QString& strERC20TokenName  ) noexcept(false)
{
    QString strErrMsg = "";


    QString strCoinType = "ETH";
    QSqlQuery  sql(_m_db);
    QString strSql = QString("select withdraw_src_addr,encrypted_privkey from tb_coin_type where coin_nick='ERC20FEE';");
    sql.prepare(strSql);
    if(!sql.exec())
    {
        //数据库错误
        strErrMsg = "sql.exec() error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(!sql.next())
    {
        strErrMsg = "sql result is empty";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QString strSrcAddr = sql.value("withdraw_src_addr").toString();
    QString strSrcPrivKey = QByteArray::fromBase64( sql.value("encrypted_privkey").toString().toUtf8() , \
                                                     QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);; //TODO:获取私钥

    //先判断余额
    rawtx::CETHRawTxUtilsImpl  ethUtil;
    std::map<QString, double> mapBalances;
    ethUtil.GetBalance(strSrcAddr, mapBalances, strCoinType);
    double dBalance = mapBalances.at("ETH");

    double dClcEthFee_ERC20  = utils::GetETH_ERC20_CollectionFee(strERC20TokenName);
    double dEthFee  = utils::GetETH_ERC20_CollectionFee("ETH");

    double dSumNeed = dClcEthFee_ERC20 * vctAddrs.size() + dEthFee * vctAddrs.size() + 0.01;
    if(dBalance <  dSumNeed  )
    {
        strErrMsg = QString( "ERC20SupplyTxFee: src-address balance is not enough. %1ETH < %2ETH").arg(dBalance).arg(dSumNeed);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(auto strAddr  : vctAddrs)
    {
        QString strDstAddr = strAddr; //TODO:目的地址
        double  dETHAmount = dClcEthFee_ERC20 + 0.00001;

        //1.准备 DLL 接口调用参数
        rawtx::ETH_CreateRawTxParam param;
        param.uChainId          = (g_bBlockChainMainnet) ? (eth::ETHChainID::Mainnet) : (eth::ETHChainID::Rinkeby);//TODO:仅Rinkeby测试网络, 迁移到主网时需进行切换
        param.strAddrFrom       = strSrcAddr.toStdString();       //from地址 , TODO:从数据库中获取
        param.strPrivKey        =  strSrcPrivKey.toStdString(); //from地址的私钥,此私钥从数据库获取并解密

        //ETH转账
        param.float100Value     = dETHAmount; // float100(STR_ERC20_SUPPLY_TXFEE);        //金额, 以 ether为单位  1 ether = 10^18 wei
        param.strAddrTo         = strDstAddr.toStdString(); //目的地址, 以"0x"开头,共42字符
        //param.szData             = NULL;                  //附加数据, 为空即可
        param.uDataLen          = 0;                        //附加数据的长度, 为0即可
        param.ubigintGasPrice   = ubigint(STR_COLLECTION_GASPRICE_WEI);   // gasprice, 单位是wei, 默认值是1GWei , 其中1 GWei = 10^9 wei
        param.ubigintGasStart   = ubigint(STR_NORMAL_ETH_TX_GASLIMIT);//21000;            //gasStart, 单位是 wei, 默认值是 21000 wei

        //TODO: 添加余额判断   2019-09-03 yqq
        ethUtil.GetNonce(QString(param.strAddrFrom.c_str()),  param.uNonce, strCoinType);

        //2.检查参数
        if(false == param.ParamsCheck(&strErrMsg))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
            throw runtime_error(strErrMsg.toStdString() );
        }

        //3.调用 DLL 接口, 创建签名交易
        ethUtil.CreateRawTx(&param);


        //4.组装参数, 调用广播接口进行广播
        rawtx::ETH_BroadcastRawTxParam  bcastParam;
        bcastParam.strCoinType          = strCoinType;
        bcastParam.strAddrFrom          = QString(param.strAddrFrom.c_str());
        bcastParam.strNonce             = QString::asprintf("%u", param.uNonce);
        bcastParam.uChainId             = param.uChainId;
        bcastParam.strSignedRawTxHex    = QString( rawtx::Bin2HexString(param.szRetBuf, param.uRetDataLen).c_str() );
        bcastParam.strOrderId           = utils::GenOrderID(); //TODO:设置订单号

        ethUtil.BroadcastRawTx( &bcastParam );


        //5.返回广播结果
        QString strTxidRet  = bcastParam.strTxid;

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ") << "txid: " << bcastParam.strTxid
                        << " , nonce:" << bcastParam.strNonce << " , chainId:" << QString::asprintf("%d", bcastParam.uChainId)
                         << "  ,  from:" << bcastParam.strAddrFrom  << " to: " << QString( param.strAddrTo.c_str() )
                         << " , rawtx: " <<  bcastParam.strSignedRawTxHex;

        //TODO:存数据库...
        if(true)
        {
            QSqlQuery query(_m_db);
            QString strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(bcastParam.strOrderId).arg(uAdminId).arg("ETH" /*如果是ERC20,再修改*/)
                .arg(strSrcAddr).arg(strDstAddr).arg( QString::asprintf("%.08f", dETHAmount)).arg("---")
                .arg(  QString::asprintf("%.08f", dEthFee))
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);

            for(auto iTry = 0; iTry < 5; iTry++)
            {
                if(true == query.exec(strSql) )
                    break;
            }
        }
    }



    return NO_ERROR;
}


int CPageCollection::_TRX_BroadcastRawTx(const vector<int> &vctRowsSel,  QString strCoinType) noexcept(false)
{
    QString strErrMsg;

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(auto iRow : vctRowsSel)
    {
        if((size_t)iRow >= __m_trxImportRawTxParams.size())
        {
            strErrMsg = QString("selected count greater than imported count");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        rawtx::Trx_BroadcastRawTxParam  bcastParam;
        bcastParam.strCoinType          = strCoinType;
        bcastParam.strSrcAcct           = __m_trxImportRawTxParams[iRow].strSrcAcct;  //源地址
        bcastParam.strSignedRawTx       = __m_trxImportRawTxParams[iRow].strSignedRawTx; //签名交易
        bcastParam.strOrderId           = __m_trxImportRawTxParams[iRow].strOrderId;    //订单号
        bcastParam.strDstAcct           = __m_trxImportRawTxParams[iRow].strDstAcct;
        bcastParam.strTxid              = __m_trxImportRawTxParams[iRow].strTxid;
        bcastParam.strSuccessedTxid     = ""; //这个是广播成功后的txid, 注意和  strTxid 区分
        bcastParam.dAmount              = __m_trxImportRawTxParams[iRow].dAmount;
        bcastParam.strErrMsg            = "";

        if(true)
        {
            QSqlQuery query(_m_db);
            if(false == query.exec(QString("select * from tb_collection where order_id='%1';").arg(bcastParam.strOrderId)))
            {
                strErrMsg =  QString("DB error: query database error..");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            if(0 < query.size() )
            {
                strErrMsg =  QString("debug info: order_id:%1 had  successfuly broadcated . so skip it now.");
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                continue;
            }
        }

        __m_trxRawTxUtils.BroadcastRawTx( &bcastParam );

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "broadcast successed.";
        QString strTxidRet = bcastParam.strSuccessedTxid;

        //更新数据库, 将交易id写入数据库
        QThread::msleep(10);
        QSqlQuery query(_m_db);



        rawtx::Trx_ImportRawTxItem const &trxItem = __m_trxImportRawTxParams[iRow];
        QString strCoinType  = trxItem.strCoinType;
        strCoinType = strCoinType.toUpper();

        QString strSql = "";

        strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(trxItem.strOrderId).arg(uAdminId).arg( QString(strCoinType/*如果是ERC20,再修改*/))
                .arg(trxItem.strSrcAcct).arg(trxItem.strDstAcct).arg(trxItem.strAmount).arg("---").arg("0.1")
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql :" << strSql;

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
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("广播完成后,更新数据库txid:%1,成功").arg(strTxidRet);
    }

    return NO_ERROR;
}




int CPageCollection::HRC20_SupplyTxFee( const vector<QString> &vctAddrs ) noexcept(false)
{
    QString strErrMsg = "";
    QString strCoinType = "HTDF";

    QSqlQuery  sql(_m_db);
    QString strSql = QString("select withdraw_src_addr,encrypted_privkey from tb_coin_type where coin_nick='HRC20FEE';");
    sql.prepare(strSql);
    if(!sql.exec())
    {
        //数据库错误
        strErrMsg = "sql.exec() error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(!sql.next())
    {
        strErrMsg = "sql result is empty";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QString strSrcAddr = sql.value("withdraw_src_addr").toString();
    QString strSrcPrivKey = QByteArray::fromBase64( sql.value("encrypted_privkey").toString().toUtf8() , \
                                                     QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals); //TODO:获取私钥


    //获取 HTDF余额
    rawtx::CCosmosRawTxUtilsImpl  cosUtil;
    rawtx::CosmosAccountInfo cosAccountInfo;
    cosAccountInfo.strCoinType = strCoinType;
    cosAccountInfo.strAddress = strSrcAddr;
    cosUtil.GetAccountInfo( cosAccountInfo );


    double dSumFee = FLOAT_HRC20_SUPPLY_FEE * ( vctAddrs.size() + 1 );
    double dBalance = cosAccountInfo.dBalance;
    if( dBalance < dSumFee )
    {
        strErrMsg = QString( "HRC20_SupplyTxFee: src-address balance is not enough. %1HTDF < %2HTDF").arg(dBalance).arg(dSumFee);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    //获取当前管理员id
    bool bOk = false;
    uint32_t uAdminId = CUserInfo::loginUser().AdminID().toInt(&bOk);
    if(!bOk)
    {
        strErrMsg = QString("Failed to got login user's ID, please login in first, and try again.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    uint64_t uSequence = cosAccountInfo.uSequence;

    for(auto strAddr  : vctAddrs)
    {
        QString strDstAddr = strAddr;

        cosmos::CsRawTx  csRawTx;

        QString  strOrderId = utils::GenOrderID();

        strcpy_s(csRawTx.szChainId, (g_bBlockChainMainnet)?(STR_MAINCHAIN):(STR_TESTCHAIN));
        strcpy_s(csRawTx.szFeeDenom, STR_SATOSHI);
        strcpy_s(csRawTx.szMemo, strOrderId.toStdString().c_str());
        strcpy_s(csRawTx.szMsgDenom, STR_SATOSHI);
        strcpy_s(csRawTx.szMsgFrom, strSrcAddr.toStdString().c_str());
        strcpy_s(csRawTx.szMsgTo, strDstAddr.toStdString().c_str());
        csRawTx.uMsgAmount          = (uint64_t)(FLOAT_HRC20_SUPPLY_FEE * 100000000);  //注意整型溢出(经过计算, dAmount的上限远远大于总发行量)
        csRawTx.uGas                = 30000;// 200000;
        csRawTx.uFeeAmount          = 100;//; //使用 HTDF_2020版
        csRawTx.uSequence      =  uSequence++; //cosAccountInfo.uSequence;
        csRawTx.uAccountNumber =  cosAccountInfo.uAccountNumber;


        rawtx::Cosmos_CreateRawTxParam param;
        param.csRawTx = csRawTx;
        param.strPrivKey = strSrcPrivKey; //手续费地址私钥


        //2.检查参数
        if(false == param.ParamsCheck( &strErrMsg ))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //3.调用接口创建
        cosUtil.CreateRawTx( &param );


        //4.组装参数, 调用广播接口进行广播
        rawtx::Cosmos_BroadcastRawTxParam  bcastParam;
        bcastParam.strOrderId = strOrderId;
        bcastParam.strChainId   = param.csRawTx.szChainId;
        bcastParam.strSequence  = QString::asprintf("%u", param.csRawTx.uSequence);
        bcastParam.strCoinType = strCoinType;
        bcastParam.strFrom = param.csRawTx.szMsgFrom;
        bcastParam.strTo = param.csRawTx.szMsgTo;
        bcastParam.strValue = QString::asprintf("%llu", FLOAT_HRC20_SUPPLY_FEE * 100000000);//转为satoshi
        bcastParam.strSignedRawTxHex = param.strHexData; //广播数据

        if(false == bcastParam.ParamsCheck( &strErrMsg ))
        {
//            orderData.iOrderStatus =  OrderStatus::ORDER_CLIENT_ERR;
//            orderData.strRemarks = strErrMsg;

            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"bcastParam.ParamsCheck() error:" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        cosUtil.BroadcastRawTx( &bcastParam );

        //5.返回广播结果
        QString strTxidRet  = bcastParam.strTxid;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ")
                            << "txid: " << strTxidRet  << " , nonce:" << bcastParam.strSequence << " , chainId:" << bcastParam.strChainId
                             << "  ,  from:" << bcastParam.strFrom  << " to: " << bcastParam.strTo
                             << " , rawtx: " <<  bcastParam.strSignedRawTxHex;

        if(true)
        {
            QSqlQuery query(_m_db);
            QString strSql = QString("insert into tb_collection values('%1',%2,'%3','%4','%5','%6','%7','%8',%9,'%10','%11')")
                .arg(bcastParam.strOrderId).arg(uAdminId).arg(strCoinType)
                .arg(strSrcAddr).arg(strDstAddr).arg(FLOAT_HRC20_SUPPLY_FEE).arg("---").arg(0.01)
                .arg(1 /*全部成功,失败的不存数据库*/).arg(QDateTime::currentSecsSinceEpoch()).arg(strTxidRet);

            for(auto iTry = 0; iTry < 5; iTry++)
            {
                if(true == query.exec(strSql) )
                    break;
            }
        }


    }




    return NO_ERROR;
}






