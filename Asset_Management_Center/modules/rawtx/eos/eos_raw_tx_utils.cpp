#include "eos/eos_raw_tx_utils.h"
#include "libeossig/eos_tx_sign.h"  //EOS签名封装函数
#include "utils.h"
#include "config.h"

#define   STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort
#define  STR_strUrlCreateRawTransaction_EOS  ( STR_WALLET_API_HOST_PORT + STR_eos_createrawtransaction )
#define  STR_strUrlSendRawTransaction_EOS  ( STR_WALLET_API_HOST_PORT + STR_eos_sendrawtransaction )
#define  STR_strUrlGetAccountInfo_EOS ( STR_WALLET_API_HOST_PORT  + STR_eos_getaccountinfo)

int rawtx::CEosRawTxUtilsImpl::CreateRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
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

    Eos_CreateRawTxParam *pParam = dynamic_cast<Eos_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<Eos_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //2.构造请求参数
    QString strUrl = STR_strUrlCreateRawTransaction_EOS;
    strUrl += "?src_acct=" + pParam->strSrcAcct;
    strUrl += "&dst_acct=" + pParam->strDstAcct;
    strUrl += "&amount=" + pParam->strAmount;
    strUrl += "&memo=" + pParam->strMemo;

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

        if( ! boost::all( strDigest.toStdString(),  boost::is_xdigit() ) )
        {
            strErrMsg = " response `digest` is invalid: "  + parseJsonErr.errorString();
            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() )  ;
        }
    }

    //5.返回
    pParam->strSignedRawTx = strRawTxJsonStr;
    pParam->strTxDigestHex = strDigest;

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CEosRawTxUtilsImpl::ExportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    //TODO:实现
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    Eos_ExportRawTxParam *pParam = dynamic_cast<Eos_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Eos_ExportRawTxParam *> failed.");
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
            Eos_ExportRawTxItem item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("memo",  item.strMemo);
            joItem.insert("coinType", item.strCoinType);
            joItem.insert("chainNet", item.strChainNet);
            joItem.insert("strAmount", item.strAmount);
            joItem.insert("dAmount", item.dAmount);
            joItem.insert("srcAccount", item.strSrcAcct);
            joItem.insert("dstAccount", item.strDstAcct);

            //资产管理导出时此字段为空,钱包管理导出时此字段不能为空
            joItem.insert("rawSignedTxHex", item.strSignedRawTx);
            joItem.insert("txDigestHex", item.strTxDigestHex);

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

int rawtx::CEosRawTxUtilsImpl::ImportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
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

    Eos_ImportRawTxParam *pParam = dynamic_cast<Eos_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Eos_ImportRawTxParam *> failed.");
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

    if(!(0 == joInfo.value("coinType").toString().compare("EOS", Qt::CaseInsensitive)) )
    {
        strErrMsg = QString("`cointype` is `%1`, is not `%2` ").arg(joInfo.value("coinType").toString()).arg("EOS");
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
        Eos_ImportRawTxItem  importItem;
        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("orderId")&& joItem.contains("coinType") && joItem.contains("chainNet") \
             && joItem.contains("strAmount")  && joItem.contains("srcAccount")\
             && joItem.contains("dstAccount")  && joItem.contains("dAmount")\
             && joItem.contains("rawSignedTxHex") && joItem.contains("memo")\
             && joItem.contains("complete") && joItem.contains("txDigestHex") ))
        {
            strErrMsg =  QString("`items` missing some necessary fields");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        importItem.bComplete    = joItem["complete"].toBool(false);
        importItem.strOrderId   = joItem["orderId"].toString();
        importItem.strMemo = joItem["memo"].toString();
        importItem.strCoinType  = joItem["coinType"].toString();
        importItem.strChainNet  = joItem["chainNet"].toString();
        importItem.strAmount = joItem["strAmount"].toString();
        importItem.dAmount = joItem["dAmount"].toDouble();
        importItem.strSrcAcct = joItem["srcAccount"].toString();
        importItem.strDstAcct   = joItem["dstAccount"].toString();
        importItem.strSignedRawTx    = joItem["rawSignedTxHex"].toString();
        importItem.strTxDigestHex = joItem["txDigestHex"].toString();

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

int rawtx::CEosRawTxUtilsImpl::BroadcastRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    //1.参数检查
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    Eos_BroadcastRawTxParam *pParam = dynamic_cast<Eos_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<Eos_BroadcastRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<Eos_BroadcastRawTxParam *> failed.").toStdString() );
    }

    QString strErrMsg = "";
    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( QString( "internal error:" + strErrMsg ).toStdString()  );
    }

     //2.拼装请求参数
    QString strReqUrl( STR_strUrlSendRawTransaction_EOS );


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


    //判断是否包含必要字段
    if( !( joResult.contains("transaction_id")  && joResult.contains("processed") ) )
    {
        pParam->strErrMsg = QString("error msg: `result` missed `transaction_id` or `processed`.  " );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(  pParam->strErrMsg.toStdString() );
    }

    QJsonObject joProcessed = joResult.value("processed").toObject();
    if( !( joProcessed.contains("error_code")  && joProcessed.contains("except")  && joProcessed.contains("receipt")) )
    {
        pParam->strErrMsg = QString("error msg: `processed` missed `error_code` or `except` or `receipt`.  " );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(  pParam->strErrMsg.toStdString() );
    }

    //检查  error_code , except  是否为 null
    if(! (joProcessed.value("error_code").isNull()  && joProcessed.value("except").isNull() ) )
    {
        pParam->strErrMsg = QString("error msg: `error_code`: %1,  except: %2" )
                            .arg( joProcessed.value("error_code").toString() ).arg(joProcessed.value("except").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(  pParam->strErrMsg.toStdString() );
    }

    QJsonObject joReceipt = joProcessed.value("receipt").toObject();
    if( ! (joReceipt.contains("status") ) )
    {
        pParam->strErrMsg = QString("error msg: `receipt` missed `status`  " );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(  pParam->strErrMsg.toStdString() );
    }

    QString strTxStatus = joReceipt.value("status").toString();
    if(0 != strTxStatus.compare("executed", Qt::CaseSensitive))
    {
        pParam->strErrMsg = QString("error msg: tx'receipt status is not 'executed', this tx had  been failed. " );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(  pParam->strErrMsg.toStdString() );
    }


    //广播成功
    pParam->strTxid = joResult.value("transaction_id").toString();


    return IRawTxUtils::NO_ERROR;
}

int rawtx::CEosRawTxUtilsImpl::SignTxDigestWithPrivKey(const QString &strTxDigest, const QString &strPrivKey, QString &strOutSigResult)
{
    /// 禁止打印任何形式的私钥!!


    //参数检查
    if( ! ( boost::all( strTxDigest.toStdString(), boost::is_xdigit() ) ))
    {
	    QString strErrMsg = QString("strTxDigest: %1 is invalid ").arg(strTxDigest );
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }


    if( ! (strPrivKey.startsWith('5') && strPrivKey.length() == 51) )
    {
	    QString strErrMsg = QString("strPrivKey is invalid");
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }



    std::string strHexDigest = strTxDigest.toStdString() ;
	std::string strBinHexDigest  = utils::HexToBin(strHexDigest);

	if(32 != strBinHexDigest.size() )
	{
	    QString strErrMsg = QString("strBinHexDigest's length :%1 is not 32 bytes").arg(strBinHexDigest.size());
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
	}

    std::string  cstrPrivKey  = strPrivKey.toStdString();
	const char *pszWIFKey = cstrPrivKey.c_str();

	char szOutData[1024] = { 0 };
	memset(szOutData, 0, sizeof(szOutData));
	unsigned int uOutDataLen = 0;
	int iRet = eos::EosTxSignWithWIFKey(pszWIFKey, (unsigned char *)strBinHexDigest.data(), strBinHexDigest.size(), szOutData, &uOutDataLen);
	if (0 != iRet)
	{
		QString strErrMsg = QString("eos::EosTxSignWithWIFKey  failed,  iRet : %1 ").arg(iRet);
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
	}

	QString strSig(szOutData);
	if(!strSig.startsWith("SIG_K1_", Qt::CaseSensitive))
	{
	    QString strErrMsg = QString("eos::EosTxSignWithWIFKey  failed,  iRet : %1 ").arg(iRet);
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
	}

	//返回签名结果
	strOutSigResult = strSig;

    return IRawTxUtils::NO_ERROR;
}


int rawtx::CEosRawTxUtilsImpl::GetAccountInfo(rawtx::EosAccountInfo &accountInfo)
{
    QString strErrMsg = "";
    //1.参数检查
    if( !utils::IsValidAddr("EOS", accountInfo.strAccountName) )
    {
        strErrMsg = QString("%1: args error. `%2` is invalid address.").arg(__FUNCTION__).arg(accountInfo.strAccountName);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求参数
    QString  strReqUrl;
    strReqUrl = STR_strUrlGetAccountInfo_EOS;
    strReqUrl += accountInfo.strAccountName;

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

    //解析
    /*
    core_liquid_balance   //EOS余额
    ram_quota    // RAM最大量
    ram_usage   // RAM已使用量
    cpu_limit:  //CPU 指标
    {
        used : 已使用量
        available : 剩余量
        max : 最大可用量
    }
    net_limit   //NET指标
    {
        used
        available
        max
    },
    */


    if( !(joResult.contains("core_liquid_balance") && joResult.contains("ram_quota")
            && joResult.contains("ram_usage") && joResult.contains("cpu_limit")
            && joResult.contains("net_limit")
     ) )
     {
        strErrMsg = QString(" response missing some fields");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
     }


    QString strBalance = joResult.value("core_liquid_balance").toString();
    strBalance = strBalance.replace("EOS", "");
    strBalance = strBalance.trimmed();

    bool ok = false;
    accountInfo.dEOSBalance = strBalance.toDouble( &ok );

    accountInfo.uRAM_Used = joResult.value("ram_usage").toInt();
    accountInfo.uRAM_Total = joResult.value("ram_quota").toInt();


    QJsonObject joCPU = joResult.value("cpu_limit").toObject();
    accountInfo.uCPU_Used = joCPU.value("used").toInt();
    accountInfo.uCPU_Total = joCPU.value("max").toInt();
    accountInfo.uCPU_Avail = joCPU.value("available").toInt();


    QJsonObject joNET = joResult.value("net_limit").toObject();
    accountInfo.uNET_Used = joNET.value("used").toInt();
    accountInfo.uNET_Total = joNET.value("max").toInt();
    accountInfo.uNET_Avail = joNET.value("available").toInt();


    return IRawTxUtils::NO_ERROR;
}
