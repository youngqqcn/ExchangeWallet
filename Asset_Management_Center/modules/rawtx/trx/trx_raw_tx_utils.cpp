#include "trx/trx_raw_tx_utils.h"
#include "utils.h"
#include "config.h"
#include "libtrxsig/TrxSig.h"  //TRX 签名,  即  secp256k1

#define   STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort
#define  STR_strUrlCreateRawTransaction_TRX  ( STR_WALLET_API_HOST_PORT + STR_trx_createrawtransaction )
#define  STR_strUrlSendRawTransaction_TRX  ( STR_WALLET_API_HOST_PORT + STR_trx_sendrawtransaction )
#define  STR_strUrlGetAccountInfo_TRX ( STR_WALLET_API_HOST_PORT  + STR_trx_getaccountinfo)


int rawtx::CTrxRawTxUtilsImpl::CreateRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    //1.参数检查
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    Trx_CreateRawTxParam *pParam = dynamic_cast<Trx_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<Trx_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //2.构造请求参数
    QString strUrl = STR_strUrlCreateRawTransaction_TRX;
    strUrl += "?src_acct=" + pParam->strSrcAcct;
    strUrl += "&dst_acct=" + pParam->strDstAcct;
    strUrl += "&amount=" + pParam->strAmount;

    //3.调用服务端接口创建交易
    QByteArray  bytesReply;
    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求数据: " <<  strUrl ;
    IRawTxUtils::_SyncPostByURL(strUrl, bytesReply);

    //4.解析响应值
    QJsonParseError parseJsonErr;
    QJsonDocument jdoc = QJsonDocument::fromJson(bytesReply, &parseJsonErr);
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

    QJsonObject  joResult = joReply.value("result").toObject();
    if( !joResult.contains("raw_trx_json_str") || !joResult.contains("digest"))
    {
        strErrMsg  = "`result` missing some fileds";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "错误信息:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QString strRawTxJsonStr = joResult.value("raw_trx_json_str").toString();
    QString strDigest = joResult.value("digest").toString();

    //对返回值的合法性做校验
    if( true )
    {
        parseJsonErr = QJsonParseError();
        QJsonDocument jdoc = QJsonDocument::fromJson(strRawTxJsonStr.toLocal8Bit(), &parseJsonErr);
        if( QJsonParseError::NoError != parseJsonErr.error )
        {
            strErrMsg = " response `raw_trx_json_str` parse failed: "  + parseJsonErr.errorString();
            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() )  ;
        }

        //TODO:对json中的  ownder_address  , to_address, amount 做校验
        // 其中 amount = dAmount * 1000000
        // 比如:  0.1 TRX  ,   json中的amount 为    100000

        if( ! boost::all( strDigest.toStdString(),  boost::is_xdigit() ) )
        {
            strErrMsg = " response `digest` is invalid hexstring " ;
            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() )  ;
        }
    }

    //必须存在 签名 占位符,  方便后面直接替换(将签名嵌入)
    const QString strPlaceHolder =  "this_is_placeholder_for_signature";
    if(!strRawTxJsonStr.contains(strPlaceHolder, Qt::CaseSensitive))
    {
        strErrMsg = " response json does not contains  \"this_is_placeholder_for_signature\" " ;
        qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() )  ;
    }

    pParam->strSignedRawTx = strRawTxJsonStr;
    pParam->strTxid = strDigest;

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CTrxRawTxUtilsImpl::ExportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    Trx_ExportRawTxParam *pParam = dynamic_cast<Trx_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Trx_ExportRawTxParam *> failed.");
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

    if(true)
    {
        QJsonArray jaItems;
        for(size_t i = 0; i < pParam->size(); i++)
        {
            Trx_ExportRawTxItem item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("coinType", item.strCoinType);
            joItem.insert("strAmount", item.strAmount);
            joItem.insert("dAmount", item.dAmount);
            joItem.insert("srcAccount", item.strSrcAcct);
            joItem.insert("dstAccount", item.strDstAcct);

            //资产管理导出时此字段为空,钱包管理导出时此字段不能为空
            joItem.insert("rawSignedTxHex", item.strSignedRawTx);
            joItem.insert("txid", item.strTxid);

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


    return IRawTxUtils::NO_ERROR;
}

int rawtx::CTrxRawTxUtilsImpl::ImportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
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

    Trx_ImportRawTxParam *pParam = dynamic_cast<Trx_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Trx_ImportRawTxParam *> failed.");
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

    if(!(0 == joInfo.value("coinType").toString().compare("TRX", Qt::CaseInsensitive)) )
    {
        strErrMsg = QString("`cointype` is `%1`, is not `%2` ").arg(joInfo.value("coinType").toString()).arg("TRX");
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
        || joInfo.value("timestamp").toString().toULong() > 1599213540 /*2020-09-04 17:59:00*/)
    {
        strErrMsg = QString("`timestamp` is illegal, too small or too large. timestamp  must between  2019-04-27 16:54:27 and 2020-04-27 16:52:35. \
                            if the current timestamp is larger than 2020-09-04 17:59:00 , please contact with developers.");
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
        Trx_ImportRawTxItem  importItem;
        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("orderId")&& joItem.contains("coinType")  \
             && joItem.contains("strAmount")  && joItem.contains("srcAccount")\
             && joItem.contains("dstAccount")  && joItem.contains("dAmount")\
             && joItem.contains("rawSignedTxHex") \
             && joItem.contains("complete") && joItem.contains("txid") ))
        {
            strErrMsg =  QString("`items` missing some necessary fields");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        importItem.bComplete    = joItem["complete"].toBool(false);
        importItem.strOrderId   = joItem["orderId"].toString();
        importItem.strCoinType  = joItem["coinType"].toString();
        importItem.strAmount = joItem["strAmount"].toString();
        importItem.dAmount = joItem["dAmount"].toDouble();
        importItem.strSrcAcct = joItem["srcAccount"].toString();
        importItem.strDstAcct   = joItem["dstAccount"].toString();
        importItem.strSignedRawTx    = joItem["rawSignedTxHex"].toString();
        importItem.strTxid = joItem["txid"].toString();

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

int rawtx::CTrxRawTxUtilsImpl::BroadcastRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    //1.参数检查
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    Trx_BroadcastRawTxParam *pParam = dynamic_cast<Trx_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<Trx_BroadcastRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<Trx_BroadcastRawTxParam *> failed.").toStdString() );
    }

    QString strErrMsg = "";
    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( QString( "internal error:" + strErrMsg ).toStdString()  );
    }

     //2.拼装请求参数
    QString strReqUrl( STR_strUrlSendRawTransaction_TRX );


    QJsonObject joParams;
    joParams.insert( "orderId", pParam->strOrderId );
    joParams.insert( "data", pParam->strSignedRawTx );

    QByteArray bytesRsp;
    IRawTxUtils::_SyncPostByJson(strReqUrl, joParams, bytesRsp);


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

    QJsonObject joRsp = jsonDoc.object();
    if(!joRsp.contains("result") || !joRsp.contains("success"))
    {
        pParam->strErrMsg = joRsp.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("json data missing `result` or `success` ");
        throw runtime_error(QString("json data missing `result` or `success` ").toStdString());
    }

    if(0 == joRsp.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        pParam->strErrMsg = joRsp.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << QString(joRsp.value("result").toString());
        throw runtime_error(QString(joRsp.value("result").toString()).toStdString());
    }

    QJsonObject joResult = joRsp.value("result").toObject();
    if(joResult.isEmpty())
    {
        pParam->strErrMsg = QString("error msg: result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: result is empty. ");
        throw runtime_error(QString("error msg: result is empty. ").toStdString());
    }

    //解析广播返回数据
    // 广播成功: {"success":true,  "result": { "result":true, "transaction":{"txID":"xxxxxx",....}}  }
    //广播失败:
    //    {"success":false, "result" : "{\"code\":\"DUP_TRANSACTION_ERROR\"}"  }
    //    {"success":false, "result" : "{\"code\":\"SIGERROR\", \"message\":\"validate signature error .....\"}"  }
    bool bTxResult = joResult.value("result").toBool();
    if(!bTxResult)
    {
        QString strErrMsg = joRsp.value("result").toString();
        pParam->strErrMsg = QString("error msg: tx result is 'false', tx failed. more details: %1").arg(strErrMsg);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(pParam->strErrMsg.toStdString());
    }


    QJsonObject joTransaction = joResult.value("transaction").toObject();
    QString strRetTxid = joTransaction.value("txID").toString();


    //目前, 不检查,  均以服务端返回的数据为准
    //如果返回的数据中的txid, 与本地计算的txid 不同, 可能是重复广播的订单

    pParam->strSuccessedTxid = strRetTxid;

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CTrxRawTxUtilsImpl::SignTxidWithPrivKey(const QString &strTxid, const QString &strPrivKey, QString &strOutSigResult)
{
    //1.参数检查
    if( ! ( boost::all( strTxid.toStdString(), boost::is_xdigit() ) ))
    {
	    QString strErrMsg = QString("strTxDigest: %1 is invalid ").arg(strTxid);
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    if(! (64 == strPrivKey.length()   && boost::all( strPrivKey.toStdString(), boost::is_xdigit() )))
    {
        //禁止打印私钥!!!
        QString strErrMsg = QString("strPrivKey is invalid ");
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //2.调用动态库中的 签名函数

    unsigned char uszOutBuf[1024] = {0};
    memset(uszOutBuf, 0, sizeof(uszOutBuf));
    unsigned int  uOutDataLen = 0;

    char szErrMsg[1024] = {0};
    memset(szErrMsg, 0, sizeof(szErrMsg));

    std::string cstrBinPrivKey = utils::HexToBin(strPrivKey.toStdString());
    std::string cstrBinTxid = utils::HexToBin(strTxid.toStdString());

    int iRet = trx::TrxUitls::ECDSA_Sign(
            (unsigned char *)cstrBinTxid.data(),
            cstrBinTxid.size(),
            (unsigned char *)cstrBinPrivKey.data(),
            cstrBinPrivKey.size(),
            uszOutBuf,
            sizeof(uszOutBuf),
            &uOutDataLen,
            (char *)szErrMsg
        );

    if(0 != iRet)
    {
        QString strErrMsg = QString(" trx::TrxUitls::ECDSA_Sign  failed, errMsg : %1").arg(QString(szErrMsg));
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //3.返回
    std::string strHexSig = utils::Bin2HexStr( uszOutBuf, uOutDataLen );
    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString::fromStdString(strHexSig);

    strOutSigResult = QString::fromStdString( strHexSig );

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CTrxRawTxUtilsImpl::GetAccountInfo(rawtx::TrxAccountInfo &accountInfo)
{
     QString strErrMsg = "";
    //1.参数检查
    if( !utils::IsValidAddr("TRX", accountInfo.strAddress) )
    {
        strErrMsg = QString("%1: args error. `%2` is invalid address.").arg(__FUNCTION__).arg(accountInfo.strAddress);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求参数
    QString  strReqUrl;
    strReqUrl = STR_strUrlGetAccountInfo_TRX;
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

    QJsonObject joRsp = jsonDoc.object();
    if(!joRsp.contains("result") || !joRsp.contains("success"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("reponse json missing `result` or `success` field.");
        throw runtime_error(QString("reponse json missing `result` or `success` field.").toStdString());
    }

    if(0 == joRsp.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = QString("source-address has no transaction history yet. It's meant source address has any available balance.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joResult = joRsp.value("result").toObject();
    if(joResult.isEmpty())
    {
        strErrMsg = QString("source-address has no transaction history yet. It's meant source address has any available balance.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //5.返回
    try
    {
        accountInfo.isActive = joResult.value("active").toBool(); //账户是否存在
        accountInfo.dBalance = boost::lexical_cast<double>(joResult.value("balance").toString().toStdString()); //json中本来是浮点数字符串
    }
    catch(boost::bad_lexical_cast &e)
    {
        strErrMsg = QString::asprintf("boost::lexical_cast<>() error: %s", e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    return IRawTxUtils::NO_ERROR;
}
