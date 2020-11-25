#include "xrp/xrp_raw_tx_utils.h"
#include "utils.h"
using namespace rawtx;


#define   STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort
#define STR_strUrlGetAccountInfo_XRP (STR_WALLET_API_HOST_PORT + STR_xrp_getaccountinfo_address)
#define STR_strUrlSendRawTransaction_XRP (STR_WALLET_API_HOST_PORT + STR_xrp_sendrawtransaction_data)

#define UINT_TXID_CHAR_LEN 64
#define UINT_XRP_ADDR_LEN  34

int rawtx::CXrpRawTxUtilsImpl::CreateRawTx(rawtx::IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    //TODO:
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    Xrp_CreateRawTxParam *pParam = dynamic_cast<Xrp_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<Xrp_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //2.组装参数进行创建
    //TODO: 目前直接使用 rippled的  sign接口进行创建(同时签名交易), 大大简化了 xrp的对接复杂度
    // 如果后期使用cpp代码实现 ripple的对象序列化, 并自己完成签名,
    // 彼时 切换到其他 代理函数即可
    __SignProxyByRippled(pParam);


    return IRawTxUtils::NO_ERROR;
}


int CXrpRawTxUtilsImpl::__SignProxyByRippled(Xrp_CreateRawTxParam *_pParam)
{
    //跳过参数检查
    QString strErrMsg = "";

    //1.拼装请求参数

    QByteArray  bytesRet;
    QString strContentType = "application/json";
    QString  strReqUrl;
    strReqUrl = "http://127.0.0.1:5005";

    QJsonObject  joReqParams;
    joReqParams.insert("method", "sign");
    QJsonArray jaParams;
    QJsonObject joParamsItem;
    joParamsItem.insert("secret", _pParam->strPrivKey );
    joParamsItem.insert("offline", true);
    QJsonObject joTxJson;
    joTxJson.insert("TransactionType", "Payment"); //只支持Payment, 其他交易类型不考虑
    joTxJson.insert("Account", _pParam->strSrcAddr);
    joTxJson.insert("Destination", _pParam->strDstAddr);
    joTxJson.insert("Amount", _pParam->strAmountInDrop);
    joTxJson.insert("Sequence",  QJsonValue::fromVariant( QVariant(_pParam->uSequence)));
    joTxJson.insert("SigningPubKey", ""); //由 rippled 填充
    joTxJson.insert("Fee",  QJsonValue::fromVariant(QVariant( _pParam->uFeeInDrop)));
    joTxJson.insert("DestinationTag",  QJsonValue::fromVariant(QVariant( _pParam->uDestinationTag)));

    //生成一个随机, 防止交易完全相同(因为sequence更新有延迟,如果出现目的地址_标签,金额完全相同的情况, 就会出现交易内容完全一样)
    //广播交易的时候出现问题
    joTxJson.insert("SourceTag", QJsonValue::fromVariant(QVariant( utils::GenRandomUInt() ) )  );

    joParamsItem.insert("tx_json", joTxJson);
    jaParams.append(joParamsItem);
    joReqParams.insert("params", jaParams );


    //2.请求数据
    IRawTxUtils::_SyncPostByJson(strReqUrl, joReqParams, bytesRet, strContentType);


    //3.解析返回数据
    QJsonParseError jError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &jError));
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") )
    {
        strErrMsg = QString("json data missing `result` ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joResult =  joRes.value("result").toObject();
    if(!joResult.contains("status") )
    {
        strErrMsg = QString("error msg: server response missing `status`. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QString strReqStatus = joResult.value("status").toString();

    if(! (0 == strReqStatus.compare("success", Qt::CaseInsensitive)))
    {
        QString strError = joResult.value("error").toString();
        int  iErrorCode = joResult.value("error_code").toInt(-1);
        QString strErrorMessage = joResult.value("error_message").toString();
        strErrMsg = QString("error msg:  rippled return error:%1 error_code:%2 error_msg:%3. ")
                    .arg(strError).arg(iErrorCode).arg(strErrorMessage);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!(joResult.contains("tx_blob") && joResult.contains("tx_json")))
    {
        strErrMsg = QString("error msg: server response missing `tx_blob` or `tx_json`. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //4.返回结果
    QString strTxBlob = joResult.value("tx_blob").toString();
    _pParam->strSignedRawTx = strTxBlob;
    _pParam->bSigned = true;

    return IRawTxUtils::NO_ERROR;
}



int rawtx::CXrpRawTxUtilsImpl::ExportRawTx(rawtx::IRawTxUtilsParamBase *_pParam) noexcept(false)
{
//1.参数检查
    //int iRet = -1;
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    Xrp_ExportRawTxParam *pParam = dynamic_cast<Xrp_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Xrp_ExportRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //2.组成json对象
    QJsonObject  joExp;

    //info
    if(true)
    {
        QJsonObject joInfo;
        joInfo.insert("coinType", pParam->m_strCoinType.trimmed());
        joInfo.insert("timestamp", QString::asprintf("%u", QDateTime::currentDateTime().toTime_t()) );
        joInfo.insert("isClcSign", pParam->m_bIsCollection);

        joExp.insert("info", joInfo);
    }

    //items
    if(true)
    {
        QJsonArray jaItems;
        for(size_t i = 0; i < pParam->size(); i++)
        {
            Xrp_ExportRawTxItem item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("dstTag",  QJsonValue::fromVariant(QVariant(  item.uDestinationTag)));
            joItem.insert("coinType", item.strCoinType);
            joItem.insert("chainNet", item.strChainNet);
            joItem.insert("strAmountInDrop", item.strAmountInDrop);
            joItem.insert("dAmountInXRP", item.dAmountInXRP);
            joItem.insert("sequence",  QJsonValue::fromVariant(QVariant( item.uSequence)));
            joItem.insert("srcAddr", item.strSrcAddr);
            joItem.insert("dstAddr", item.strDstAddr);
            joItem.insert("feeInDrop",  QJsonValue::fromVariant(QVariant( item.uFeeInDrop)));

            //资产管理导出时此字段为空,钱包管理导出时此字段不能为空
            joItem.insert("rawSignedTxHex", item.strSignedRawTx);

            jaItems.append(joItem);
        }

        joExp.insert("items", jaItems);
    }

    //4.导出到文件
    QFile fileOutput(pParam->m_strFilePath);
    if(false == fileOutput.open(QIODevice::WriteOnly))
    {
        strErrMsg =  QString("export json file open() failed!  filePath:\"%1\"").arg(pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QJsonDocument  jdocExport;
    jdocExport.setObject(joExp);
    fileOutput.write(jdocExport.toJson());
    fileOutput.close();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("export to json file success. filePath:\"%1\"").arg(pParam->m_strFilePath);
    return IRawTxUtils::NO_ERROR;
}

int rawtx::CXrpRawTxUtilsImpl::ImportRawTx(rawtx::IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    //1.参数检查
    //int iRet = -1;
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    Xrp_ImportRawTxParam *pParam = dynamic_cast<Xrp_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Cosmos_ImportRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        strErrMsg = QString("internal error:") + strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.打开文件
    QFile fileImport(pParam->m_strFilePath);
    if(false == fileImport.exists())
    {
        strErrMsg = QString("import json file not exists! FILE PATH:\"%1\"").arg(pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == fileImport.open(QIODevice::ReadOnly))
    {
        strErrMsg = QString("import json file open failed! FILE PATH:\"%1\"").arg(pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //3.读取文件内容,解析成jsondocument
    QByteArray bytesImport = fileImport.readAll();
    fileImport.close();

    QJsonParseError jError;
    QJsonDocument jdocImport(QJsonDocument::fromJson(bytesImport, &jError));
    if(QJsonParseError::NoError != jError.error )
    {
        strErrMsg = QString("json parse error! errorMsg:%1").arg(jError.error);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //4.读取jsondocument中的数据,存入 __m_vctItems
    QJsonObject joImport = jdocImport.object();
    if(!joImport.contains("info") || !joImport.contains("items"))
    {
        strErrMsg = "json file missing `info` or `items` fields";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joInfo = joImport.value("info").toObject();
    if(!(joInfo.contains("coinType") && joInfo.contains("timestamp") && joInfo.contains("isClcSign")))
    {
        strErrMsg = "json file `info` missing some fields.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    pParam->m_bIsCollection = joInfo["isClcSign"].toBool();

    if(!(0 == joInfo.value("coinType").toString().compare("XRP", Qt::CaseInsensitive)) )
    {
        strErrMsg = QString("`cointype` is `%1`, is not `%2` ").arg(joInfo.value("coinType").toString()).arg("XRP");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(joInfo.value("timestamp").toString().isEmpty())
    {
        strErrMsg = QString("`timestamp` is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(joInfo.value("timestamp").toString().toULong() < 1556355267/*2019-04-27 16:54:27*/
        || joInfo.value("timestamp").toString().toULong() > 1599636430 /*2020-09-09 15:27:10*/)
    {
        strErrMsg = QString("`timestamp` is illegal, too small or too large. timestamp  must between 2020-09-09 15:27:10 and2020-09-09 15:27:10. \
                            if the current timestamp is larger than 2020-09-09 15:27:10 , please contact with developers.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //5.获取数据
    QJsonArray  jaItems = joImport.value("items").toArray();
    if(jaItems.isEmpty())
    {
        strErrMsg = QString("`items` is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(int i = 0; i < jaItems.size(); i++)
    {
        Xrp_ImportRawTxItem  importItem;
        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("orderId")&& joItem.contains("coinType") && joItem.contains("chainNet") \
             && joItem.contains("strAmountInDrop") && joItem.contains("sequence") && joItem.contains("srcAddr")\
             && joItem.contains("dstAddr") && joItem.contains("feeInDrop") && joItem.contains("dAmountInXRP")\
             && joItem.contains("rawSignedTxHex") && joItem.contains("dstTag")\
             && joItem.contains("complete") ))
        {
            strErrMsg =  QString("`items` missing some neccessary fields");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        importItem.bComplete    = joItem["complete"].toBool(false);
        importItem.strOrderId   = joItem["orderId"].toString();
        importItem.uDestinationTag = joItem["dstTag"].toVariant().toUInt();
        importItem.strCoinType  = joItem["coinType"].toString();
        importItem.strChainNet  = joItem["chainNet"].toString();
        importItem.strAmountInDrop    = joItem["strAmountInDrop"].toString();
        importItem.dAmountInXRP      = joItem["dAmountInXRP"].toDouble();
        importItem.uSequence  = joItem["sequence"].toVariant().toUInt();
        importItem.strSrcAddr   = joItem["srcAddr"].toString();
        importItem.strDstAddr   = joItem["dstAddr"].toString();
        importItem.uFeeInDrop         = joItem["feeInDrop"].toVariant().toUInt();
        importItem.strSignedRawTx    = joItem["rawSignedTxHex"].toString();

        if(false == importItem.ParamsCheck( &strErrMsg ))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "importItem.ParamsCheck(): " << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
        pParam->push_back(importItem);
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("ImportRawTx() success.");


    return IRawTxUtils::NO_ERROR;
}

int rawtx::CXrpRawTxUtilsImpl::BroadcastRawTx(rawtx::IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    //1.参数检查
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    Xrp_BroadcastRawTxParam *pParam = dynamic_cast<Xrp_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<Cosmos_ExportRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<Xrp_BroadcastRawTxParam *> failed.").toStdString() );
    }

    QString strErrMsg = "";
    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( QString( "internal error:" + strErrMsg ).toStdString()  );
    }


    //2.拼装请求参数
    QString strReqUrl ;
    strReqUrl = STR_strUrlSendRawTransaction_XRP;
    strReqUrl += pParam->strSignedRawTx;
    strReqUrl += "&orderId=";
    strReqUrl += pParam->strOrderId.trimmed();
    strReqUrl = strReqUrl.trimmed();

    QByteArray bytesRsp;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRsp);


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL() called, success return.";


    //3.解析返回接口
    QJsonParseError jError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRsp, &jError));
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        pParam->strErrMsg  = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        pParam->strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("json data missing `result` or `success` ");
        throw runtime_error(QString("json data missing `result` or `success` ").toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        pParam->strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << QString(joRes.value("result").toString());
        throw runtime_error(QString(joRes.value("result").toString()).toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        pParam->strErrMsg = QString("error msg: result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: result is empty. ");
        throw runtime_error(QString("error msg: result is empty. ").toStdString());
    }

    if(!(joResult.contains("order_id") && joResult.contains("txid")
         && joResult.contains("tx_success") && joResult.contains("req_status")
         && joResult.contains("code") && joResult.contains("msg")
         && joResult.contains("tx_json")
         ))
    {
        pParam->strErrMsg = QString("error msg: response json miss `order_id` or `txid` or `tx_success` or `req_status` or `code` or `msg` or `tx_json`. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(  pParam->strErrMsg.toStdString() );
    }

    QString strOrderId = joResult.value("order_id").toString().trimmed();
    if(0 != strOrderId.compare(pParam->strOrderId.trimmed(), Qt::CaseInsensitive))
    {
        strErrMsg = QString("error msg: orderId is dismatched. req-orderId:%1, but rsp-orderId:%2").arg(pParam->strOrderId).arg(strOrderId);
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QString strTxid = joResult.value("txid").toString();
    bool bReqStatus = joResult.value("req_status").toBool(false);
    bool bTxSuccess = joResult.value("tx_success").toBool(false);
    int iCode = joResult.value("code").toInt(-1);
    QString strSubmitMsg = joResult.value("msg").toString();
    QJsonObject joTxJson = joResult.value("tx_json").toObject();
    QString strTxJson =  QJsonDocument(joTxJson).toJson();

    if( !bReqStatus || !bTxSuccess || 0 != iCode)
    {
        strErrMsg = QString("error msg: request failed. status:%1 code: %2 , msg:%3, tx_json:%4")
                            .arg(bReqStatus).arg(iCode).arg(strSubmitMsg).arg(strTxJson);
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    if(UINT_TXID_CHAR_LEN != strTxid.length())
    {
        strErrMsg = QString("error msg: txid's length is illegal. the correct length is  %1.").arg(UINT_TXID_CHAR_LEN);
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " BroadcastRawTx() success. txid:"
            << strTxid  << ".  sequence=" << pParam->uSequence << "." << "tx_json:" << strTxJson;


    //4.返回txid
    pParam->strTxid = strTxid;
    pParam->strErrMsg = "";

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CXrpRawTxUtilsImpl::GetAccountInfo(rawtx::XrpAccountInfo &accountInfo) noexcept(false)
{
    QString strErrMsg = "";
    //1.参数检查
    if( accountInfo.strAddress.isEmpty()
        || !accountInfo.strAddress.startsWith('r')
        || accountInfo.strAddress.length() != UINT_XRP_ADDR_LEN)
    {
        strErrMsg = QString("%1: args error. `%2` is invalid address.").arg(__FUNCTION__).arg(accountInfo.strAddress);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求参数
    QString  strReqUrl;
    strReqUrl = STR_strUrlGetAccountInfo_XRP;
    strReqUrl += accountInfo.strAddress;

    //3.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRet);


    //4.解析返回数据
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &error));
    if(error.error != QJsonParseError::NoError)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("json parse error! errorCode = ") << error.error;
        throw runtime_error("json parse error");
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("reponse json missing `result` or `success` field.");
        throw runtime_error(QString("reponse json missing `result` or `success` field.").toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = QString("source-address has no transaction history yet. It's meant source address has any available balance.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        strErrMsg = QString("source-address has no transaction history yet. It's meant source address has any available balance.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!(joResult.contains("balance") && joResult.contains("sequence")
        && joResult.contains("account")))
    {
        strErrMsg = QString("reponse json missing `balance` or `sequence` or `account` field.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //5.返回
    try
    {
        //QString strAddr = joResult.value("account").toString();
        //accountInfo.uSequqnce =  boost::lexical_cast<uint64_t>(joResult.value("sequence").toString().toStdString());
        accountInfo.uSequqnce =  joResult.value("sequence").toVariant().toUInt();
        accountInfo.dBalanceInXRP = boost::lexical_cast<double>(joResult.value("balance").toString().toStdString()); //json中本来是浮点数字符串
    }
    catch(boost::bad_lexical_cast &e)
    {
        strErrMsg = QString::asprintf("boost::lexical_cast<>() error: %s", e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    return IRawTxUtils::NO_ERROR;
}



QString rawtx::CXrpRawTxUtilsImpl::GetAddrFrom_Addr_Tag(const QString &str_Addr_Tag) noexcept(true)
{
    //如果后面跟了标签, 需要截取 '_' 前面的作为地址
    if(str_Addr_Tag.contains('_') )
    {
        QStringList strTmpList = str_Addr_Tag.split('_');
        QString strTmpXrpAddr = strTmpList[0];
        return strTmpXrpAddr;
    }

    //如果不存在 '_' 直接返回即可
    return str_Addr_Tag;
}


uint32_t rawtx::CXrpRawTxUtilsImpl::GetTagFrom_Addr_Tag(const QString &str_Addr_Tag) noexcept(false)
{
    QString strErrMsg;

    //如果后面跟了标签, 需要截取 '_' 前面的作为地址
    if(!str_Addr_Tag.contains('_') )
    {
        strErrMsg = QString("%1 地址_标签格式错误! 请检查!").arg(str_Addr_Tag);
        throw std::runtime_error(strErrMsg.toStdString());
    }

    QString strTag = str_Addr_Tag.mid( str_Addr_Tag.indexOf('_') + 1);


    uint32_t uRetTag  = 0;
    try
    {
        uRetTag = boost::lexical_cast<uint32_t>(strTag.toStdString());
    }
    catch(boost::bad_lexical_cast &e)
    {
        strErrMsg = QString("%1 标签格式错误! 请检查! %2").arg(str_Addr_Tag).arg(e.what());
        throw std::runtime_error(strErrMsg.toStdString());
    }

    return  uRetTag;
}
