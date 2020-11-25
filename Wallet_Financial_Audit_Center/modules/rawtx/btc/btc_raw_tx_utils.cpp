#include "btc_raw_tx_utils.h"
#include "utils.h"

using namespace rawtx ;
using namespace btc;


//实现创建交易接口
int CBTCRawTxUtilsImpl::CreateRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    BTC_CreateRawTxParam *pParam = dynamic_cast<BTC_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<BTC_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!pParam->ParamsCheck( &strErrMsg ) )
    {
        strErrMsg =  QString("internal error:") + strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QByteArray byteRet;

    //暂时仍 1对1
    QString strSrcAddr = pParam->vctSrcAddr.at(0);
    QString strDstAddr = pParam->vctDstAddr.at(0);
    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "源地址: " << strSrcAddr << "目的地址:" << strDstAddr;

    QString strReqUrl = pParam->strURL + "from=" + strSrcAddr + "&to=" + strDstAddr + "&amount=" + QString::number(pParam->dAmount, 'f', 8);

    _SyncPostByURL(strReqUrl, byteRet);

    //解析数据
    if(true)
    {
        QJsonParseError error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(byteRet, &error));
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
        if(mapTmpTxout.empty())
        {
            strErrMsg = "response json `txout` is empty.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }

        for(auto it = mapTmpTxout.begin(); it != mapTmpTxout.end(); it++)
        {
            pParam->mapTxOut.insert(make_pair(it->first, QVariant(it->second).toDouble() ));
        }


        //获取创建的rawData信息
        pParam->strRawTxHex = subObjData.value("hex").toArray().at(0).toString();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "raw data: " << pParam->strRawTxHex;

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

            //UTXOs
            pParam->vctRefUTXOs.push_back(tempUtxo);

            qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("UTXO内容:") << i << tempUtxo.address
                << tempUtxo.amount << tempUtxo.scriptPubKey <<tempUtxo.txid << tempUtxo.vout << tempUtxo.redeemScript ;
        }

    }


    return IRawTxUtils::NO_ERROR;
}

//实现,导出交易接口
int CBTCRawTxUtilsImpl::ExportRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    QString strErrMsg;

     //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    BTC_ExportRawTxParam *pParam = dynamic_cast<BTC_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<BTC_ExportRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!pParam->ParamsCheck( &strErrMsg ))
    {
        strErrMsg =  QString("internal error: ") + strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //转换为json数据
    QJsonObject joRoot;
    QJsonObject joInfo;
    joInfo.insert("isClcSign", true);
    joInfo.insert("coinType",  pParam->m_strCoinType);
    joInfo.insert("timestamp",  QDateTime::currentSecsSinceEpoch() );
    joRoot.insert("info", joInfo);

    QJsonArray jaItems;
    QJsonObject joItem;
    for(uint i = 0; i < pParam->size(); i++)
    {
        const BTC_ExportRawTxItem  &item  = (*pParam)[i];

        joItem.insert("tokenId", QJsonValue::fromVariant(QVariant(item.uTokenId)) );
        joItem.insert("tokenAmount", item.strTokenAmount);
        joItem.insert("orderID", item.strOrderID);
        joItem.insert("amount", item.strAmount);
        joItem.insert("txFee", item.strTxFee);
        joItem.insert("txHex", item.strRawTxHex);

        QJsonObject joTxout;
        for(auto txout : item.mapTxOut)
        {
            joTxout.insert(txout.first, txout.second );
        }
        joItem.insert("txout", joTxout);

        //完成状态
        joItem.insert("complete", item.bComplete);

        //源地址列表
        QJsonArray jaSrcAddr;
        for(auto strAddr : item.vctSrcAddr)
        {
            jaSrcAddr.push_back(strAddr);
        }
        joItem.insert("srcAddr", jaSrcAddr);

        //目的地址列表
        QJsonArray jaDstAddr;
        for(auto strAddr : item.vctDstAddr)
        {
            jaDstAddr.push_back(strAddr);
        }
        joItem.insert("dstAddr", jaDstAddr);

        //utxos
        QJsonArray jaUtxos;
        for(auto utxo : item.vctRefUTXOs)
        {
            QJsonObject joUtxo;
            joUtxo.insert("txid", utxo.txid);
            joUtxo.insert("amount", utxo.amount);
            joUtxo.insert("scriptPubKey", utxo.scriptPubKey);
            joUtxo.insert("vout", (int)utxo.vout);
            if(!utxo.redeemScript.isEmpty())
            {
                joUtxo.insert("redeemScript", utxo.redeemScript);
            }

            jaUtxos.push_back(joUtxo);
        }
        joItem.insert("utxos", jaUtxos);


        jaItems.push_back(joItem);
     }
    joRoot.insert("items", jaItems);

    //写入Json文件
    QFile outFile(pParam->m_strFilePath);
    if(!outFile.open(QIODevice::WriteOnly))
    {
        strErrMsg = QString::asprintf("%s open failed.", pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonDocument jsonDoc;
    jsonDoc.setObject(joRoot);
    outFile.write(jsonDoc.toJson());
    outFile.close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ExportToJsonFile成功返回";


    return IRawTxUtils::NO_ERROR;
}

//实现导入交易接口
int CBTCRawTxUtilsImpl::ImportRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    BTC_ImportRawTxParam *pParam = dynamic_cast<BTC_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<BTC_ExportRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!pParam->ParamsCheck( &strErrMsg ))
    {
        strErrMsg =  QString("internal error: ") + strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QFile  inFile(pParam->m_strFilePath);
    if(!inFile.exists())
    {
        strErrMsg = QString::asprintf("args error: import-file %s is not exists.", pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        strErrMsg = QString::asprintf("%s open failed.",  pParam->m_strFilePath);
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
    pParam->m_strCoinType = joInfo.value("coinType").toString();
    //pParam->  = joInfo.value("timestamp").toString().toULongLong();
    pParam->m_bIsCollection = joInfo.value("isClcSign").toBool();

    if(!(0 == joInfo.value("coinType").toString().compare("BTC", Qt::CaseInsensitive)
        || 0 == joInfo.value("coinType").toString().compare("USDT", Qt::CaseInsensitive)))
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
        BTC_ImportRawTxItem importItem;
        QJsonObject joItem = jaItems.at(i).toObject();
        importItem.strCoinType = pParam->m_strCoinType;
        importItem.strTokenAmount = joItem.value("tokenAmount").toString();
        importItem.uTokenId = joItem.value("tokenId").toInt();
        importItem.bComplete = joItem.value("complete").toBool(false /*默认是false*/ );
        importItem.strOrderID = joItem.value("orderID").toString();
        importItem.strAmount = joItem.value("amount").toString();
        importItem.strTxFee = joItem.value("txFee").toString();
        importItem.strRawTxHex = joItem.value("txHex").toString();

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
            if(joUTXO.contains("redeemScript") && !joUTXO.value("redeemScript").toString().isEmpty())
            {
                utxo.redeemScript = joUTXO.value("redeemScript").toString();
            }

            importItem.vctRefUTXOs.push_back(utxo);
        }

        pParam->push_back(importItem);
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ImportSignedTxJsonFile 成功返回";
    return IRawTxUtils::NO_ERROR;
}

//实现广播交易接口
int CBTCRawTxUtilsImpl::BroadcastRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    BTC_BroadcastRawTxParam *pParam = dynamic_cast<BTC_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<BTC_BroadcastRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!pParam->ParamsCheck( &strErrMsg ))
    {
        strErrMsg =  QString("internal error:") + strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "strURL: " << pParam->strURL;


    QByteArray byteRet;
    QString strReqUrl = pParam->strURL + "rawdata=" + pParam->strSignedRawTxHex;
    _SyncPostByURL(strReqUrl, byteRet);

    //获取交易ID
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byteRet, &error));
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
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "server's sendrawtransaction returns error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QJsonArray jaTxid =  rootObj.value("result").toArray();
    if(0 == jaTxid.size())
    {
        strErrMsg = "`result` is empty.";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //广播成功, 则返回txid
    pParam->strTxid = jaTxid[0].toString();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("sendrawtransaction successed, txid=%1")
            .arg(pParam->strTxid);

    return IRawTxUtils::NO_ERROR;
}


//签名交易
int CBTCRawTxUtilsImpl::SignRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    BTC_SignRawTxParam *pParam = dynamic_cast<BTC_SignRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<BTC_SignRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.交易检查(交易内容, 是否与 mapTxOutMap的信息对应)
    if(true)
    {
        //2.1 decoderawtransaction 解码裸交易
        QJsonObject joParams;

        QJsonArray jaParams;
        jaParams.append(pParam->strUnsignedRawTxHex);

        joParams.insert("jsonrpc", QString("1.0"));
        joParams.insert("id", QString("curltest"));
        joParams.insert("method", QString("decoderawtransaction"));
        joParams.insert("params", jaParams);

        QByteArray  byteArrayDecode;
        _SyncPostByJson(QString(STR_BTC_NODE_IP_PORT), joParams, byteArrayDecode, "text/plain", "btc:btc2018");


        qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "decoderawtransaction返回的数据:" << byteArrayDecode;


        //2.2 解析decoderawtransaction返回的数据
        QJsonParseError error;
        QJsonDocument jdoc = QJsonDocument::fromJson(byteArrayDecode, &error);
        if(QJsonParseError::NoError != error.error)
        {
            strErrMsg = "decoderawtransaction response json parse error: " + error.errorString();
            qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joDecode = jdoc.object();

        if(false == joDecode.value("error").toObject().isEmpty())
        {
            QJsonObject joError = joDecode.value("error").toObject();
            int iErrCode = joError["code"].toInt();
            QString strErrMsg = joError["message"].toString();
            if(0 != iErrCode)
            {
                strErrMsg = QString::asprintf("%d : %s",  iErrCode, strErrMsg);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }

        std::map<QString, double>  mapDecodeTxOutMap; //解析后的
        if(!joDecode.contains("result"))
        {
            strErrMsg = "missing `result`.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        QJsonObject joResult = joDecode.value("result").toObject();
        if(joResult.contains("vout"))
        {
            QJsonArray jaTxOut = joResult.value("vout").toArray();
            if(jaTxOut.isEmpty())
            {
                strErrMsg = "`vout` is empty";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            for(int u = 0; u < jaTxOut.size(); u++)
            {
                QJsonObject joTxOut = jaTxOut[u].toObject();

                if(!(joTxOut.contains("value") && joTxOut.contains("scriptPubKey")))
                {
                    strErrMsg = "missing `value` or `scriptPubKey`";
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                double dValue = joTxOut.value("value").toDouble();
                QJsonObject joScriptPubKey = joTxOut.value("scriptPubKey").toObject();
                //QString strAddr = joScriptPubKey.value("addresses").toArray()[0].toString(); //修改bugid:5027,数组越界导致程序崩溃   @yqq  2019-04-20
                if(!joScriptPubKey.contains("addresses"))
                {
                    strErrMsg =  "missing `addresses`";
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QJsonArray jaAddrs = joScriptPubKey.value("addresses").toArray();
                if(jaAddrs.isEmpty())
                {
                    strErrMsg = "`addresses` is empty.";
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QString strAddr = jaAddrs[0].toString();
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "addresses:" << strAddr << "  " << "value:" << dValue;

                mapDecodeTxOutMap.insert(std::make_pair(strAddr, dValue));
            }
        }



        if(pParam->bIsCollection)   //归集签名
        {
            if( mapDecodeTxOutMap.size() < 1)
            {
                strErrMsg = "safe check: mapDecodeTxOutMap is empty.";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }
        else  //提币签名
        {
            //必须要有找零地址, 防止巨额矿工费
            if( mapDecodeTxOutMap.size() < 2)
            {
                strErrMsg = "safe check: mapDecodeTxOutMap's size is less than 2, DANGEROUS: there was no pay back.";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }


        if(mapDecodeTxOutMap.size()  != pParam->mapTxout.size())
        {
            strErrMsg = "safe check: mapDecodeTxOutMap's size is not equals inputs args .";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        for(auto it = mapDecodeTxOutMap.begin(); it != mapDecodeTxOutMap.end(); it++)
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  it->first << "  " << it->second ;

            auto itTmp = pParam->mapTxout.find(it->first);
            if(itTmp == pParam->mapTxout.end())
            {
                strErrMsg = QString("safe check: mapTxOutMap not contains %1").arg(it->first);
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            if(itTmp->second != it->second) //比较金额
            {
                strErrMsg = QString("safe check: args's amount is not equals decoded amount.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }
    }//if



    //3.组装参数, 调用rpc接口的  signrawtransactionwithkey
    if(true)
    {
        QJsonObject joParams;

        QJsonArray jaParams;
        jaParams.append(pParam->strUnsignedRawTxHex); //未签名裸交易

        QJsonArray jaPrivKeys;
        for(auto strPrivKey : pParam->vctPrivKeys)  //添加私钥
        {
            jaPrivKeys.append(strPrivKey);
        }
        jaParams.append(jaPrivKeys);

        QJsonArray jaUTXOs;

        for(auto utxo : pParam->vctUTXOs)
        {
            QJsonObject joTmpUTXO;
            joTmpUTXO.insert("txid", utxo.txid);
            joTmpUTXO.insert("vout", QJsonValue((int)(utxo.vout)) );
            if(!utxo.redeemScript.trimmed().isEmpty())
            {
                joTmpUTXO.insert("redeemScript", utxo.redeemScript); //暂时不支持ps2h类型的地址, 如需添加, 后续放开即可
                qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"============警告:非测试模式去掉redeemScript";
            }

            joTmpUTXO.insert("scriptPubKey", utxo.scriptPubKey);
            joTmpUTXO.insert("amount", utxo.amount);

            jaUTXOs.append(joTmpUTXO);
        }
        jaParams.append(jaUTXOs);

        joParams.insert("jsonrpc", QString("1.0"));
        joParams.insert("id", QString("curltest"));
        joParams.insert("method", QString("signrawtransactionwithkey"));
        joParams.insert("params", jaParams);

        //=================== 警告: 此处禁止打印包含私钥的数据, 如发现, 重罚!  2019-08-27 by yqq============================

        //4.http请求
        QByteArray byteArrReply;
        _SyncPostByJson(QString(STR_BTC_NODE_IP_PORT), joParams, byteArrReply, "application/json", "btc:btc2018", false/*不打印请求数据*/);


        //5.解析http请求结果
        QJsonParseError jParseErr;
        QJsonDocument jDoc = QJsonDocument::fromJson(byteArrReply, &jParseErr);
        if(QJsonParseError::NoError != jParseErr.error)
        {
            strErrMsg = "json parse error:" + jParseErr.errorString() ;
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jDoc.object();
        if(!joRes.contains("error") || !joRes.contains("result"))
        {
            strErrMsg = "missing `error` or `result`";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(false == joRes.value("error").toObject().isEmpty())
        {
            int iErrCode = joRes.value("error").toObject().value("code").toInt();
            QString strTmp = joRes.value("error").toObject().value("message").toString();

            strErrMsg = QString("%1 : %2").arg(iErrCode).arg(strTmp);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joResult = joRes.value("result").toObject();
        if(joResult.isEmpty())
        {
            strErrMsg = " `result` is empty";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!joResult.contains("hex")  || !joResult.contains("complete"))
        {
            strErrMsg = "missing `hex` or `complete`";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QString strHexSignedRawTx = joResult.value("hex").toString();
        bool bComplete = joResult.value("complete").toBool(false);
        if(false == bComplete)
        {
            strErrMsg = "complete is false ";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //返回, 已签名交易
        pParam->strSignedRawTxHex = strHexSignedRawTx;
    }



    return IRawTxUtils::NO_ERROR;
}
