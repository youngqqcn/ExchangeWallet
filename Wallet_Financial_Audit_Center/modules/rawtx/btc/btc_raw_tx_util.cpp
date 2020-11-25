#include "btc_raw_tx_util.h"
#include "utils.h"
#include  <QNetworkRequest>


CBTCRawTxUtil::CBTCRawTxUtil(QObject *parent):QObject(parent)
{
    m_pNetAccMgr = NULL;//201905231742增加
    m_pNetAccMgr = new QNetworkAccessManager(this);

    m_strCoinType = "BTC";
}

CBTCRawTxUtil::~CBTCRawTxUtil()
{
    if(NULL !=m_pNetAccMgr)
    {
        m_pNetAccMgr->destroyed();
        m_pNetAccMgr = NULL;
    }

}





//调用createrawtransactionex创建交易
//strRawTxHex:返回的交易的十六进制字符串
 //vctUTXOs: 创建交易所引用到的UTXOs
int CBTCRawTxUtil::CreateRawTransactionEx_Collection(
        const QString &strURL,
        vector<QString> &vctstrSrcAddr,
        vector<QString> &vctstrDstAddr,
        QString &strRawTxHex,
        map<QString, double> &mapTxout,
        vector<UTXO>  &vctRefUTXOs,
        double &dTxFee,
        double &dAmount)noexcept(false)
{
    //int iRet = -1;
    //参数检查
    QString strErrMsg = "";
    if(strURL.isEmpty())
    {
        strErrMsg = QString("strUrl为空");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(0 == vctstrSrcAddr.size() || 0== vctstrDstAddr.size() )
    {
        strErrMsg  = QString("源地址或目的地址或归集金额为空");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QJsonObject joParams;

    QJsonArray jaParamDst;
    QJsonArray jaParams;

    for(uint i = 0; i < vctstrSrcAddr.size(); i++) //待归集地址
    {
        if(false == utils::IsValidAddr(m_strCoinType, vctstrSrcAddr[i]))
        {
            strErrMsg = QString("源地址%1 不合法").arg(vctstrSrcAddr[i]);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }
        jaParams.append(vctstrSrcAddr[i]);
    }
    joParams.insert("src", jaParams);


    for(uint j=0; j < vctstrDstAddr.size(); j++)
    {
        if(false == utils::IsValidAddr(m_strCoinType, vctstrDstAddr[j]))
        {
            strErrMsg = QString("目的地址%1 不合法").arg(vctstrDstAddr[j]);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }
        jaParamDst.append(vctstrDstAddr[j]);
    }
    joParams.insert("dest", jaParamDst);

    //发送创建未签名归集交易请求
    QByteArray  byteArrayReplay;
    QNetworkRequest req;
    req.setUrl(QUrl(strURL));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求数据: " << QUrl(strURL) << strURL <<  joParams ;

    m_pNetAccMgr->disconnect();
    m_pReply = m_pNetAccMgr->post( req, QJsonDocument(joParams).toJson(QJsonDocument::Compact));

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(m_pReply);

    byteArrayReplay = m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArrayReplay;

    m_pReply->close();

    QJsonParseError parseJsonErr;
    QJsonDocument jdoc = QJsonDocument::fromJson(byteArrayReplay, &parseJsonErr);
    if(QJsonParseError::NoError != parseJsonErr.error)
    {
        strErrMsg = parseJsonErr.errorString();
        qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QJsonObject joReply = jdoc.object();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " id :" << joReply["id"].toString();

    if(!joReply.contains("result") || !joReply.contains("success"))
    {
        strErrMsg =  "missing `result` or `success` fields.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }



    //判断返回状态success是否true
    if(0 == joReply.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg  = joReply.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "错误信息:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    QJsonObject subObjData = joReply.value("result").toObject();

    //获取txout
    if(!subObjData.contains("txout"))
    {
        strErrMsg = "response json missing `txout` ";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QJsonObject joTxout = subObjData.value("txout").toObject();
    auto mapTmpTxout = joTxout.toVariantMap().toStdMap();

    double dSumAmount = 0.0;
    for(auto it = mapTmpTxout.begin(); it != mapTmpTxout.end(); it++)
    {
        double dTmp = QVariant(it->second).toDouble();
        if(dTmp > 0) dSumAmount += dTmp;

        mapTxout.insert(make_pair(it->first, QVariant(it->second).toDouble() ));
    }

    //获取转账总金额
    dAmount = dSumAmount;

    //获取手续费
    dTxFee = subObjData.value("txFee").toDouble(0);


    if(mapTxout.empty())
    {
        strErrMsg = "response json `txout` is empty.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //获取创建的rawData信息
    if(!subObjData.contains("hex"))
    {
        strErrMsg = "response json missing `hex`.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }
    strRawTxHex = subObjData.value("hex").toArray().at(0).toString();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "raw data: " << strRawTxHex;

    //获取utxos
    if(!subObjData.contains("utxos"))
    {
        strErrMsg =  "response json missing `utxos`.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }
    QJsonArray jaUTXOs = subObjData.value("utxos").toArray();

    if(jaUTXOs.isEmpty())
    {
        strErrMsg = "`utxos` is empty";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    for(int i = 0; i < jaUTXOs.size(); i++)
    {
        UTXO tempUtxo;

        QJsonObject joUtxo = jaUTXOs.at(i).toObject();
        tempUtxo.address = joUtxo.value("address").toString();
        tempUtxo.amount = joUtxo.value("amount").toDouble();
        tempUtxo.scriptPubKey = joUtxo.value("scriptPubKey").toString();
        tempUtxo.txid = joUtxo.value("txid").toString();
        tempUtxo.vout = joUtxo.value("vout").toInt();
        //tempUtxo.redeemScript = joUtxo.value("redeemScript").toString();

        vctRefUTXOs.push_back(tempUtxo);

        qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("UTXO内容:") << i << tempUtxo.address << tempUtxo.amount << tempUtxo.scriptPubKey\
                    <<tempUtxo.txid << tempUtxo.vout << tempUtxo.redeemScript ;
    }

    return NO_ERROR;
}



int CBTCRawTxUtil::CreateRawTransaction(
        const QString &strURL,
        const QString &strSrcAddr,
        const QString &strDstAddr,
        const double &dAmount,
        QString &strRawTxHex,
        map<QString, double> &mapTxout,
        vector<UTXO> &vctRefUTXOs) noexcept(false)
{
    QString strErrMsg = "";


    //参数检查
    if(dAmount < 0.000001  || dAmount >= 2100*10000)  //检查金额
    {
        strErrMsg = QString("args error: dAmount is too small or too large, dAmuont is %1").arg(dAmount);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(false == utils::IsValidAddr(m_strCoinType, strDstAddr)) //检查地址的有效性
    {
        strErrMsg = QString("destination address '%1' is invalid.").arg(strDstAddr);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(false == utils::IsValidAddr(m_strCoinType, strSrcAddr)) //检查地址的有效性
    {
        strErrMsg = QString("source address '%1' is invalid.").arg(strSrcAddr);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "源地址: " << strSrcAddr << "目的地址:" << strDstAddr;
    QByteArray byteArrayRet;

    QString strReqUrl = strURL + "from=" + strSrcAddr + "&to=" + strDstAddr + "&amount=" + QString::number(dAmount, 'f', 8);
    QJsonObject joObject;
    SyncPostData(strReqUrl, joObject, byteArrayRet);

    ParseRawData(byteArrayRet, strRawTxHex, mapTxout, vctRefUTXOs);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "成功返回";
    return NO_ERROR; //返回错误码
}



int CBTCRawTxUtil::ExportRawTxToJsonFile(const QString &strJsonFilePath, const BTCExportData &btcExportData)noexcept(false)
{
    QString strErrMsg;

    //判断参数
    if(strJsonFilePath.isEmpty())
    {
        strErrMsg = QString::asprintf("args error: export-file path is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

   //转换为json数据
    QJsonObject joRoot;
    QJsonObject joInfo;
    joInfo.insert("isClcSign",btcExportData.bIsCollection);
    joInfo.insert("coinType", btcExportData.strCoinType);
    joInfo.insert("timestamp",  QString::asprintf("%llu", btcExportData.uTimestamp));
    joRoot.insert("info", joInfo);

    QJsonArray jaItems;
    QJsonObject joItem;
    for(uint i = 0; i < btcExportData.vctExportItems.size(); i++)
    {
        joItem.insert("tokenId", QJsonValue::fromVariant(QVariant( btcExportData.vctExportItems.at(i).uTokenId)) );
        joItem.insert("tokenAmount", btcExportData.vctExportItems.at(i).strTokenAmount);
        joItem.insert("orderID", btcExportData.vctExportItems.at(i).strOrderID);
        joItem.insert("amount", btcExportData.vctExportItems.at(i).strAmount);
        joItem.insert("txFee", btcExportData.vctExportItems.at(i).strTxFee);
        joItem.insert("txHex", btcExportData.vctExportItems.at(i).strTxHex);

        QJsonObject joTxout;
        for(auto txout : btcExportData.vctExportItems.at(i).mapTxOut)
        {
            joTxout.insert(txout.first, txout.second );
        }
        joItem.insert("txout", joTxout);

        //完成状态
        joItem.insert("complete", btcExportData.vctExportItems[i].bComplete);

        //源地址列表
        QJsonArray jaSrcAddr;
        vector<QString> vctSrcAddr = btcExportData.vctExportItems.at(i).vctSrcAddr;
        for(uint j = 0; j < vctSrcAddr.size(); j++)
        {
            jaSrcAddr.push_back(vctSrcAddr.at(j));
        }
        joItem.insert("srcAddr", jaSrcAddr);

        //目的地址列表
        QJsonArray jaDstAddr;
        vector<QString> vctDstAddr = btcExportData.vctExportItems.at(i).vctDstAddr;
        for(uint k = 0; k < vctDstAddr.size(); k++)
        {
            jaDstAddr.push_back(vctDstAddr.at(k));
        }
        joItem.insert("dstAddr", jaDstAddr);

        //utxos
        QJsonArray jaUtxos;
        vector<UTXO> vctUTXO = btcExportData.vctExportItems.at(i).vctUTXOs;
        for(uint m = 0; m < vctUTXO.size(); m++)
        {
            QJsonObject joUtxo;
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "m = " << m;
            joUtxo.insert("txid", vctUTXO.at(m).txid);
            joUtxo.insert("amount", vctUTXO.at(m).amount);
            joUtxo.insert("scriptPubKey", vctUTXO.at(m).scriptPubKey);
            joUtxo.insert("vout", (int)vctUTXO.at(m).vout);
            if(!vctUTXO.at(m).redeemScript.isEmpty())
            {
                joUtxo.insert("redeemScript", vctUTXO.at(m).redeemScript);
            }

            jaUtxos.push_back(joUtxo);
        }
        joItem.insert("utxos", jaUtxos);


        jaItems.push_back(joItem);
     }
    joRoot.insert("items", jaItems);

    //写入Json文件
    QFile outFile(strJsonFilePath);
    if(!outFile.open(QIODevice::WriteOnly))
    {
        strErrMsg = QString::asprintf("%s open failed.", strJsonFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonDocument jsonDoc;
    jsonDoc.setObject(joRoot);
    outFile.write(jsonDoc.toJson());
    outFile.close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ExportToJsonFile成功返回";
    return NO_ERROR;
}

int CBTCRawTxUtil::ImportRawTxFromJsonFile(const QString &strJsonFilePath, BTCImportData &btcImportData)noexcept(false)
{
    QString strErrMsg = "";

    //判断参数
    if(strJsonFilePath.isEmpty())
    {
        strErrMsg = QString::asprintf("args error: export-file path is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QFile  inFile(strJsonFilePath);
    if(!inFile.exists())
    {
        strErrMsg = QString::asprintf("args error: import-file %s is not exists.", strJsonFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        strErrMsg = QString::asprintf("%s open failed.", strJsonFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QByteArray byteArray = inFile.readAll();
    inFile.close();

    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byteArray, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = QString::asprintf("parse json error:%s", error.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("info") || !rootObj.contains("items"))
    {
        strErrMsg = QString("json missing `info` or `items` .");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joInfo = rootObj.value("info").toObject();
    if(!(joInfo.contains("coinType") && joInfo.contains("timestamp") && joInfo.contains("isClcSign")))
    {
        strErrMsg = QString("`info` missing `coinType` or `timestamp` or `isClcSign` .");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    btcImportData.strCoinType = joInfo.value("coinType").toString();
    btcImportData.uTimestamp = joInfo.value("timestamp").toString().toULongLong();
    btcImportData.bIsCollection = joInfo.value("isClcSign").toBool();

    if(!(0 == joInfo.value("coinType").toString().compare(m_strCoinType) || 0 == joInfo.value("coinType").toString().compare("USDT")))
    {
        strErrMsg = QString("you selected cointype is different with imported. imported data is %1 .").arg(joInfo.value("coinType").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonArray jaItems = rootObj.value("items").toArray();
    if(jaItems.isEmpty())
    {
        strErrMsg = "`items` is empty.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(int i = 0; i < jaItems.size(); i++)
    {
        BTCImportItem importItem;
        QJsonObject joItem = jaItems.at(i).toObject();
        importItem.strTokenAmount = joItem.value("tokenAmount").toString();
        importItem.uTokenId = joItem.value("tokenId").toInt();
        importItem.bComplete = joItem.value("complete").toBool(false /*默认是false*/ );
        importItem.strOrderID = joItem.value("orderID").toString();
        importItem.strAmount = joItem.value("amount").toString();
        importItem.strTxFee = joItem.value("txFee").toString();
        importItem.strTxHex = joItem.value("txHex").toString();

        auto mapTxout = joItem.value("txout").toObject().toVariantMap().toStdMap();
        for(auto it = mapTxout.begin(); it != mapTxout.end(); it++)
        {
            importItem.mapTxOut.insert(make_pair(it->first, QVariant(it->second).toDouble() ));
        }


        //源地址
        QJsonArray jaSrcAddrs = joItem.value("srcAddr").toArray();
        for(int j = 0; j < jaSrcAddrs.size(); j++)
        {
            importItem.vctSrcAddr.push_back(jaSrcAddrs.at(j).toString());
        }

        //目的地址
        QJsonArray jaDstAddrs = joItem.value("dstAddr").toArray();
        for(int k = 0; k < jaDstAddrs.size(); k++)
        {
            importItem.vctDstAddr.push_back(jaDstAddrs.at(k).toString());
        }

        //UTXOs
        QJsonArray jaUtxos = joItem.value("utxos").toArray();
        for(int m = 0; m < jaUtxos.size(); m++)
        {
            UTXO utxo;

            QJsonObject joUTXO = jaUtxos[m].toObject();
            if(!(joUTXO.contains("txid") && joUTXO.contains("amount")\
                 && joUTXO.contains("scriptPubKey") && joUTXO.contains("vout")))
            {
                strErrMsg = "`utxo` missing some necessary fields. maybe `txid` or `amount` or `scriptPubKey` or `vout`";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            utxo.txid = joUTXO.value("txid").toString();
            //utxo.amount = joUTXO.value("amount").toString().toDouble(); //BUG!! 不要转为String再转double  2019-05-6 yqq
            utxo.amount = joUTXO.value("amount").toDouble();
            utxo.scriptPubKey = joUTXO.value("scriptPubKey").toString();
            utxo.vout = joUTXO.value("vout").toInt();
            if(joUTXO.contains("redeemScript"))
            {
                utxo.redeemScript = joUTXO.value("redeemScript").toString();
            }

            importItem.vctUTXOs.push_back(utxo);
        }

        btcImportData.vctExportItems.push_back(importItem);
    }


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ImportSignedTxJsonFile 成功返回";
    return NO_ERROR;
}

int CBTCRawTxUtil::BroadcastRawTx(const QString &strURL, const QString &strSignedRawTxHex, QString &strTxid)noexcept(false)
{
    QString strErrMsg = "";
    //参数检查
    if(strSignedRawTxHex.isEmpty()) //Hex文件是否OK
    {
        strErrMsg = "signedRawTxHex is empty";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "strURL: " << strURL;

    //请求http数据
    QByteArray byteArrayRec;
    QString strDetailUrl = strURL + "rawdata=" + strSignedRawTxHex;
    QJsonObject joObject;
    SyncPostData(strDetailUrl, joObject, byteArrayRec);



    //获取交易ID
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byteArrayRec, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = "json error: " + error.errorString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject rootObj = jsonDoc.object();

    if(!rootObj.contains("success") || !rootObj.contains("result"))
    {
        strErrMsg = " response json missing `success` or `result`. ";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if("false" == rootObj.value("success").toString())
    {
        strErrMsg = rootObj.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "调用广播接口,返回错误:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QJsonArray jaTxid =  rootObj.value("result").toArray();
    if(0 == jaTxid.size())
    {
        strErrMsg = "`result` is empty.";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    strTxid = jaTxid[0].toString();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "BroadcastRawTx 成功返回";
    return NO_ERROR;
}


int CBTCRawTxUtil::ParseRawData(
        const QByteArray &arrayJsonData,
        QString &strRawTxHex,
        map<QString, double> &mapTxout,
        vector<UTXO> &vctRefUTXOs) noexcept(false)
{

    QString strErrMsg = "";

    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(arrayJsonData, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error:") + error.errorString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "json解析错误" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QJsonObject joReply = jsonDoc.object();

    if(!(joReply.contains("result") && joReply.contains("success") && joReply.contains("sysTime") ))
    {
        strErrMsg = "reponse json missing `result` or `success` or `sysTime`";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //判断返回状态success是否true
    if(0 == joReply.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg  = joReply.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "错误信息:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    QJsonObject subObjData = joReply.value("result").toObject();

    //获取txout
    if(!subObjData.contains("txout"))
    {
        strErrMsg = "response json missing `txout` ";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //获取txout
    QJsonObject joTxout = subObjData.value("txout").toObject();
    auto mapTmpTxout = joTxout.toVariantMap().toStdMap();
    for(auto it = mapTmpTxout.begin(); it != mapTmpTxout.end(); it++)
    {
        mapTxout.insert(make_pair(it->first, QVariant(it->second).toDouble() ));
    }

    if(mapTxout.empty())
    {
        strErrMsg = "response json `txout` is empty.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //获取创建的rawData信息
    strRawTxHex = subObjData.value("hex").toArray().at(0).toString();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "raw data: " << strRawTxHex;

    //获取utxos
    if(!subObjData.contains("utxos"))
    {
        strErrMsg =  "response json missing `utxos`.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }
    QJsonArray jaUTXOs = subObjData.value("utxos").toArray();

    if(jaUTXOs.isEmpty())
    {
        strErrMsg =  "response json missing `utxos`.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    for(int i = 0; i < jaUTXOs.size(); i++)
    {
        UTXO tempUtxo;

        QJsonObject joUtxo = jaUTXOs.at(i).toObject();
        tempUtxo.address = joUtxo.value("address").toString();
        tempUtxo.amount = joUtxo.value("amount").toDouble();
        tempUtxo.scriptPubKey = joUtxo.value("scriptPubKey").toString();
        tempUtxo.txid = joUtxo.value("txid").toString();
        tempUtxo.vout = joUtxo.value("vout").toInt();
        tempUtxo.redeemScript = joUtxo.value("redeemScript").toString();

        vctRefUTXOs.push_back(tempUtxo);

        qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("UTXO内容:") << i << tempUtxo.address << tempUtxo.amount << tempUtxo.scriptPubKey\
                    <<tempUtxo.txid << tempUtxo.vout << tempUtxo.redeemScript ;
    }

    return NO_ERROR;
}



//根据需要修改!!!!!
int CBTCRawTxUtil::SyncPostData(const QString &strInUrl, const QJsonObject &joPostData, QByteArray &byteArray) noexcept(false)
{
    QString strErrMsg = "";

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始http请求";
    QUrl url = strInUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求链接: " << strInUrl;
    m_pNetAccMgr->disconnect();
    m_pReply = m_pNetAccMgr->post( req, QJsonDocument(joPostData).toJson());

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(m_pReply);


    byteArray = m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArray;


    m_pReply->close();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求成功";
    return NO_ERROR;
}


int CBTCRawTxUtil::SyncGetData(const QString &strInUrl, QByteArray &byteArray) noexcept(false)
{
    QString strErrMsg = "";
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始http get请求";

    QUrl url = strInUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求链接: " << strInUrl;
    m_pNetAccMgr->disconnect();
    m_pReply = m_pNetAccMgr->get(req);

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(m_pReply);

    byteArray = m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArray;

    m_pReply->close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求成功";
    return NO_ERROR;
}


int CBTCRawTxUtil::GetBalance(const QString &strURL, const QString &strSrcAddr, double &dBalance) noexcept(false)
{
    QString strErrMsg = "";
    //参数检查
    if(strURL.isEmpty() || strSrcAddr.isEmpty())
    {
        strErrMsg = QString("internal error: strURL or strSrcAddr is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //请求http数据
    QByteArray byteArrayRec;
    QString strDetailUrl = strURL + "address=" + strSrcAddr;
    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "GetBalance with strURL: " << strDetailUrl;

    SyncGetData(strDetailUrl,byteArrayRec);

    //获取交易ID
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byteArrayRec, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = "json parse error:" + error.errorString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("success") || !rootObj.contains("result") )
    {
        strErrMsg = "response json missing `success` or `result` ";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if("false" == rootObj.value("success").toString())
    {
        strErrMsg = rootObj.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    dBalance =  rootObj.value("result").toDouble();
    return NO_ERROR;
}

//禁止将私钥打印到日志文件, 如有发现, 重罚!
int CBTCRawTxUtil::ImportPrivKey(const QString& strFilePath, map<QString, QString>& mapAddrPrivKey) noexcept(false)
{
    QString strErrMsg = "";

    mapAddrPrivKey.clear();
    QFile file(strFilePath);
    if(!file.open(QFile::ReadOnly))
    {
        strErrMsg = QString("open file %1 error : %2").arg(strFilePath).arg(file.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QTextStream stream(&file);
    for(; !stream.atEnd() ;)
    {
        QString strLine = stream.readLine();
        strLine = strLine.trimmed();
        if(strLine.isEmpty()) continue;

        QStringList strlstSplit  = strLine.split("\t");

        //  币种  地址   私钥  公钥(v1.1版本有, v1.2版本及以后没有公钥)
        if(strlstSplit.size() < 3)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("私钥文件格式不正确, 至少需要3个字段");
            throw std::runtime_error("私钥文件格式不正确, 至少需要3个字段. 字段以制表符分隔. 例如: 币种[制表符]地址[制表符]私钥 ");
        }

        /*if(3 == strlstSplit.size()) //v1.2版本及以上
        {
        }
        else if(4 == strlstSplit.size()) //v1.1 版本
        {
        }*/

        QString strCoinType = strlstSplit[0];
        if(!(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("usdt", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("bch", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << QString("BTC 私钥文件包含其他币种 %1的私钥").arg(strCoinType);
            continue;
        }

        QString strAddr = strlstSplit[1];
        QString strPrivKey = strlstSplit[2];//禁止将私钥打印到日志文件, 如有发现, 重罚!

        mapAddrPrivKey.insert(std::make_pair(strAddr.trimmed(), strPrivKey.trimmed()));
    }


    file.close();
    return NO_ERROR;
}














