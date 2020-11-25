#include "xmr/xmr_raw_tx_utils.h"
//#include "utils.h"
using namespace rawtx;

/*
#define   STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort
#define STR_strUrlGetAccountInfo_XRP (STR_WALLET_API_HOST_PORT + STR_xrp_getaccountinfo_address)
#define STR_strUrlSendRawTransaction_XRP (STR_WALLET_API_HOST_PORT + STR_xrp_sendrawtransaction_data)

#define UINT_TXID_CHAR_LEN 64
#define UINT_XRP_ADDR_LEN  34
*/

int CXmrRawTxUtilsImpl::ExportTxOutputs(const QString &strReqUrl , QString &strOutputsDataHex) noexcept(false)
{
    QString strErrMsg = "";

    QString  strUrl = strReqUrl;
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strUrl, bytesRet);

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
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("json data missing `result` or `success` ");
        throw runtime_error(QString("json data missing `result` or `success` ").toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << QString(joRes.value("result").toString());
        throw runtime_error(QString(joRes.value("result").toString()).toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        if(joRes.contains("message"))
            strErrMsg = joRes.value("message").toString();
        else
            strErrMsg = QString("error msg: result is empty. ");

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if( !( joResult.contains("outputs_data_hex") ) )
    {
        strErrMsg = QString("error msg: response json miss `outputs_data_hex` . ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<strErrMsg;
        throw runtime_error(  strErrMsg.toStdString() );
    }

    QString strTmpOutputsDataHex = joResult.value("outputs_data_hex").toString();
    if( !boost::all( strTmpOutputsDataHex.toStdString(), boost::is_xdigit() ) )
    {
        strErrMsg = QString(" `outputs_data_hex` is invalid hexstring ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //返回
    strOutputsDataHex = strTmpOutputsDataHex;

    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::ImportTxOutputs(const QString &strOutputsDataHex) noexcept(false)
{
    QString strErrMsg = "";

    QString strUrl = STR_XMR_LOCAL_DEAMON_IP_PORT;

    QJsonObject  joParams;

    QJsonObject joOutputDataHex;
    joOutputDataHex.insert("outputs_data_hex", strOutputsDataHex);
    joParams.insert("params", joOutputDataHex);
    joParams.insert("jsonrpc", "2.0");
    joParams.insert("method", "import_outputs");


    QByteArray bytesRsp;
    QString strContentType = "application/json";
    IRawTxUtils::_SyncPostByJson(strUrl, joParams, bytesRsp, strContentType);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL() called, success return.";

    //3.解析返回接口
    QJsonParseError jError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRsp, &jError));
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        strErrMsg  = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") )
    {
         if(joRes.contains("error"))
         {
            QJsonDocument joDoc(joRes.value("error").toObject());
            strErrMsg = joDoc.toJson();
        }
        else
        {
            strErrMsg = QString("json data missing `result` ");
        }

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(QString("json data missing `result` ").toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        strErrMsg = "`result` is empty";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if( !(joResult.contains("num_imported")) )
    {
        strErrMsg = QString("`num_imported` is empty ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(QString("error msg: `num_imported` is empty. ").toStdString());
    }

    //获取导入的个数
    int nNumImported =  joResult.value("num_imported").toInt(-1);

    if(! (0 < nNumImported  && nNumImported < 1000) )
    {
       strErrMsg = QString("`num_imported` is invalid");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(QString("error msg:`num_imported` is empty. ").toStdString());
    }

    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::ExportKeyImages( QString  &strSignedKeyImages) noexcept(false)
{
    QString strErrMsg = "";


    QString strUrl = STR_XMR_LOCAL_DEAMON_IP_PORT;

    QJsonObject  joParams;

    QJsonObject joOutputDataHex;
    joOutputDataHex.insert("all", true);
    joParams.insert("params", joOutputDataHex);
    joParams.insert("jsonrpc", "2.0");
    joParams.insert("method", "export_key_images");


    QByteArray bytesRsp;
    QString strContentType = "application/json";
    IRawTxUtils::_SyncPostByJson(strUrl, joParams, bytesRsp, strContentType);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL() called, success return.";

     //3.解析返回接口
    QJsonParseError jError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRsp, &jError));
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") )
    {
         if(joRes.contains("error"))
         {
            QJsonDocument joDoc(joRes.value("error").toObject());
            strErrMsg = joDoc.toJson();
        }
        else
        {
            strErrMsg = QString("json data missing `result` ");
        }

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        strErrMsg = QString("error msg: result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if( !( joResult.contains("offset")  && joResult.contains("signed_key_images")) )
    {
        strErrMsg = QString("error msg: response json miss `offset` or `signed_key_images`. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QJsonArray  jaSignedKeyImages = joResult.value("signed_key_images").toArray();
    if(jaSignedKeyImages.isEmpty())
    {
        strErrMsg = QString("error msg:  `signed_key_images` is empty array . ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //转为字符串返回
    QJsonDocument  joDoct;
    joDoct.setArray(joResult.value("signed_key_images").toArray());
    QString  strTmpSignedKeyImages =  joDoct.toJson();
    if(strTmpSignedKeyImages.isEmpty())
    {
        strErrMsg = QString("error msg: toString() failed, `strTmpSignedKeyImages` is empty  . ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(  strErrMsg.toStdString() );
    }

    //返回
    strSignedKeyImages = strTmpSignedKeyImages;

    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::ImportKeyImages(const QString &strReqUrl, const QString &strSignedKeyImagesArray,  double &dAvailBalance ) noexcept(false)
{
    QString strErrMsg = "";


    QString strUrl = strReqUrl;

    QJsonObject  joReqParams;

    QJsonParseError  jError;
    QJsonDocument joDoc = QJsonDocument::fromJson(strSignedKeyImagesArray.toLatin1(), &jError);
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonArray  jaSignedKeyImages = joDoc.array();
    if(jaSignedKeyImages.isEmpty())
    {
        strErrMsg = QString("`signed_key_images` is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    joReqParams.insert("signed_key_images", jaSignedKeyImages);


    //请求
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByJson(strUrl, joReqParams, bytesRet);


    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &jError));
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("json data missing `result` or `success` ");
        throw runtime_error(QString("json data missing `result` or `success` ").toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: ") << QString(joRes.value("result").toString());
        throw runtime_error(QString(joRes.value("result").toString()).toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        strErrMsg = QString("error msg: result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: result is empty. ");
        throw runtime_error(QString("error msg: result is empty. ").toStdString());
    }

    //dUnspent =  joResult.value("unspent").toDouble();
    //dSpent = joResult.value("spent").toDouble();
    //int   nHeight  = joResult.value("height").toInt();
    double dTotolBalance = joResult.value("balance").toDouble(0);
    double dUnlockedBalance = joResult.value("unlocked_balance").toDouble(0); //已解锁的余额(可用的)

    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  "totalBalance:" << dTotolBalance << "unlockedBalance : " << dUnlockedBalance;

    dAvailBalance = (0.0000001 > dUnlockedBalance) ? (0) : (dUnlockedBalance);
    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::CreateRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
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

    XMR_CreateRawTxParam *pParam = dynamic_cast<XMR_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<XMR_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //2.构造请求参数
    QJsonObject  joReqParams;
    QJsonArray  jaDestinations;
    //暂时只考虑    1对1 的转账,  不考虑一对多
    QJsonObject joDest;
    joDest.insert("address", pParam->strDstAddr);
    joDest.insert("amount", pParam->dAmount);
    jaDestinations.append(joDest);
    joReqParams.insert("destinations", jaDestinations );


    QByteArray  bytesRsp;

    QString strUrl = "";
    if( pParam->bIsManualWithdraw )
        strUrl = g_qstr_WalletHttpIpPort + STR_xmr_manual_createrawtransactions;
    else
        strUrl = g_qstr_WalletHttpIpPort + STR_xmr_auto_createrawtransactions;

    IRawTxUtils::_SyncPostByJson(strUrl, joReqParams, bytesRsp);


    //4.解析响应值
    QJsonParseError parseJsonErr;
    QJsonDocument jdoc = QJsonDocument::fromJson(bytesRsp, &parseJsonErr);
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

    if( !( joResult.contains("amount_list")
        && joResult.contains("fee_list")
        && joResult.contains("tx_hash_list")
        && joResult.contains("unsigned_txset")
        ))
    {
        strErrMsg  = "`result` missing `amount_list` or `fee_list` or `tx_hash_list` or `unsigned_txset`";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "错误信息:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QString strTmpUnsignedTxSet = joResult.value("unsigned_txset").toString();
    if( ! boost::all( strTmpUnsignedTxSet.toStdString(),  boost::is_xdigit() ) )
    {
        strErrMsg = " response `unsigned_txset` is invalid hexstring " ;
        qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() )  ;
    }

    //返回
    pParam->strSignedRawTx = strTmpUnsignedTxSet;


    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::ExportRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    XMR_ExportRawTxParam *pParam = dynamic_cast<XMR_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<XMR_ExportRawTxParam *> failed.");
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
            XMR_ExportRawTxItem item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("coinType", item.strCoinType);
            joItem.insert("strAmount", item.strAmount);
            joItem.insert("dAmount", item.dAmount);
            joItem.insert("srcAddress", item.strSrcAddr);
            joItem.insert("dstAddress", item.strDstAddr);

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



    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::ImportRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
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

    XMR_ImportRawTxParam *pParam = dynamic_cast<XMR_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<XMR_ImportRawTxParam *> failed.");
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

    if(!(0 == joInfo.value("coinType").toString().compare("XMR", Qt::CaseInsensitive)) )
    {
        strErrMsg = QString("`cointype` is `%1`, is not `%2` ").arg(joInfo.value("coinType").toString()).arg("XMR");
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
        XMR_ImportRawTxItem  importItem;
        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("orderId")&& joItem.contains("coinType")  \
             && joItem.contains("strAmount")  && joItem.contains("srcAddress")\
             && joItem.contains("dstAddress")  && joItem.contains("dAmount")\
             && joItem.contains("rawSignedTxHex") \
             && joItem.contains("complete")  ))
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
        importItem.strSrcAddr = joItem["srcAddress"].toString();
        importItem.strDstAddr   = joItem["dstAddress"].toString();
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

int CXmrRawTxUtilsImpl::BroadcastRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    //1.参数检查
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    XMR_BroadcastRawTxParam *pParam = dynamic_cast<XMR_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<XMR_BroadcastRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<XMR_BroadcastRawTxParam *> failed.").toStdString() );
    }

    QString strErrMsg = "";
    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( QString( "internal error:" + strErrMsg ).toStdString()  );
    }

     //2.拼装请求参数
    QString strReqUrl = "";
    if( pParam->bIsManualWithdraw )
        strReqUrl =  g_qstr_WalletHttpIpPort + STR_xmr_manual_sendrawtransaction;
    else
        strReqUrl =  g_qstr_WalletHttpIpPort + STR_xmr_auto_sendrawtransaction;

    QJsonObject joParams;
    joParams.insert( "orderId", pParam->strOrderId );
    joParams.insert( "signed_tx_hex", pParam->strSignedRawTx );

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
    QJsonArray jaHashList = joResult.value("tx_hash_list").toArray();
    if( jaHashList.isEmpty() )
    {
        QString strErrMsg = "`tx_hash_list` is empty ";
        pParam->strErrMsg = strErrMsg; //QString("error msg: tx result is 'false', tx failed. more details: %1").arg(strErrMsg);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << pParam->strErrMsg;
        throw runtime_error(pParam->strErrMsg.toStdString());
    }


    QString strTmpTxid = jaHashList.at(0).toString();

    //返回
    pParam->strTxid = strTmpTxid;

    return IRawTxUtils::NO_ERROR;
}

int CXmrRawTxUtilsImpl::SignTx( const QString &strUnsignedTxSet,  QString &strSignedTxHex ) noexcept(false)
{
    QString strErrMsg  = "";


    QString strUrl = STR_XMR_LOCAL_DEAMON_IP_PORT;

    QJsonObject  joParams;

    QJsonObject joOutputDataHex;
    joOutputDataHex.insert("unsigned_txset", strUnsignedTxSet);
    joParams.insert("params", joOutputDataHex);
    joParams.insert("jsonrpc", "2.0");
    joParams.insert("method", "sign_transfer");


    QByteArray bytesRsp;
    QString strContentType = "application/json";
    IRawTxUtils::_SyncPostByJson(strUrl, joParams, bytesRsp, strContentType);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL() called, success return.";

     //3.解析返回接口
    QJsonParseError jError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRsp, &jError));
    if(jError.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse error: %1 ").arg( jError.errorString() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") )
    {
        if(joRes.contains("error"))
        {
            QJsonDocument joDoc(joRes.value("error").toObject());
            strErrMsg = joDoc.toJson();
        }
        else
        {
            strErrMsg = QString("json data missing `result`");
        }
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joResult = joRes.value("result").toObject();
    if(joResult.isEmpty())
    {
        strErrMsg = QString("error msg: result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: result is empty. ");
        throw runtime_error(QString("error msg: result is empty. ").toStdString());
    }

    if( !( joResult.contains("signed_txset")  && joResult.contains("tx_hash_list")) )
    {
        strErrMsg = QString("error msg: response json miss `signed_txset` or `tx_hash_list`. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(  strErrMsg.toStdString() );
    }

    QString strTmpSignedTxHex = joResult.value("signed_txset").toString();

    if( strTmpSignedTxHex.isEmpty()  ||  !boost::all( strTmpSignedTxHex.toStdString(), boost::is_xdigit() )  )
    {
        QString strErrMsg = QString("strTmpSignedTxHex is invalid hexstring ");
		qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //返回
    strSignedTxHex = strTmpSignedTxHex;

    return IRawTxUtils::NO_ERROR;
}
