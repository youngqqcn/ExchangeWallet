#include "xlm_raw_tx_utils.h"


// StellarSDK
#include "src/keypair.h"
#include "src/util.h"
#include "src/account.h"
#include "src/transaction.h"
#include "src/createaccountoperation.h"
#include "src/paymentoperation.h"
#include "src/assettypenative.h"
#include "src/network.h"


#define STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort
#define STR_strUrlGetAccountInfo_XLM (STR_WALLET_API_HOST_PORT + STR_xlm_getaccountinfo_address)
#define STR_strUrlSendRawTransaction_XLM (STR_WALLET_API_HOST_PORT + STR_xlm_sendrawtransaction_data)

#define UINT_TXID_CHAR_LEN 64
#define UINT_XLM_ADDR_LEN  56



int rawtx::CXlmRawTxUtilsImpl::CreateRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    Xlm_CreateRawTxParam *pParam = dynamic_cast<Xlm_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<Xlm_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(g_bBlockChainMainnet)
    {
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "use mainnet";
        Network::usePublicNetwork();
    }
    else
    {
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "use testnet ";
        Network::useTestNetwork();
    }

    //构造交易并签名, 返回  XDR格式字符串
    KeyPair *srcPrivKeyPair = KeyPair::fromSecretSeed( pParam->strPrivKey );
    KeyPair *destination = KeyPair::fromAccountId( pParam->strDstAddr );



    Account *account = new Account(srcPrivKeyPair, pParam->uSequence );
    auto txBuilder =  Transaction::Builder(account);
    txBuilder.setTimeout( Transaction::Builder::TIMEOUT_INFINITE ) ;

    //如果目的账号不存在, 则创建创建账号
    if(!pParam->bDstAccountExists)
    {
        qInfo() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("account: %1 not exist, will use createoperation");
        txBuilder.addOperation(new CreateAccountOperation(destination, pParam->strAmount));
    }
    else
    {
        qInfo() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("account: %1 not exist, will use paymentoperation");
        txBuilder.addOperation(new PaymentOperation(destination, new AssetTypeNative(), pParam->strAmount));
    }


    //如果有memo
    if(!pParam->strMemo.isEmpty())
    {
        txBuilder.addMemo( Memo::text(pParam->strMemo) );
    }

    Transaction *transaction = txBuilder.build();

    //签名
    try
    {
        transaction->sign(srcPrivKeyPair);
    }
    catch(...)
    {
        qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "tx sign failed ";
        throw std::current_exception();
    }


    //返回已签名交易的 XDR base64 字符串形式
    pParam->strSignedRawTx  = transaction->toEnvelopeXdrBase64();
    pParam->bSigned = true;

    return NO_ERROR;
}

int rawtx::CXlmRawTxUtilsImpl::ExportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    //1.参数检查
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    Xlm_ExportRawTxParam *pParam = dynamic_cast<Xlm_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Xlm_ExportRawTxParam *> failed.");
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
            Xlm_ExportRawTxItem item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("memo", item.strMemo);
            joItem.insert("coinType", item.strCoinType);
            joItem.insert("chainNet", item.strChainNet);
            joItem.insert("strAmount", item.strAmount);
            joItem.insert("dAmount", item.dAmount);
            joItem.insert("strSequence",  QString::asprintf("%llu" , item.uSequence)); //转为字符串
            joItem.insert("srcAddr", item.strSrcAddr);
            joItem.insert("dstAddr", item.strDstAddr);
            joItem.insert("fee",  QJsonValue::fromVariant(QVariant( item.uBaseFee)));
            joItem.insert("dstAccountExists", item.bDstAccountExists ); //目的账号是否存在

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

int rawtx::CXlmRawTxUtilsImpl::ImportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
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

    Xlm_ImportRawTxParam *pParam = dynamic_cast<Xlm_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Xlm_ImportRawTxParam *> failed.");
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

    if(!(0 == joInfo.value("coinType").toString().compare("XLM", Qt::CaseInsensitive)) )
    {
        strErrMsg = QString("`cointype` is `%1`, is not `%2` ").arg(joInfo.value("coinType").toString()).arg("XLM");
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
        strErrMsg = QString("`timestamp` is illegal, too small or too large. timestamp  must between  2020-09-09 15:27:10 and 2020-09-09 15:27:10. \
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
        Xlm_ImportRawTxItem  importItem;
        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("orderId")&& joItem.contains("coinType") && joItem.contains("chainNet") \
             && joItem.contains("strAmount") && joItem.contains("strSequence") && joItem.contains("srcAddr")\
             && joItem.contains("dstAddr") && joItem.contains("fee") && joItem.contains("dAmount")\
             && joItem.contains("rawSignedTxHex") && joItem.contains("memo")\
             && joItem.contains("complete") ))
        {
            strErrMsg =  QString("`items` missing some neccessary fields");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        importItem.bComplete    = joItem["complete"].toBool(false);
        importItem.strOrderId   = joItem["orderId"].toString();
        importItem.strMemo      = joItem["memo"].toString();
        importItem.strCoinType  = joItem["coinType"].toString();
        importItem.strChainNet  = joItem["chainNet"].toString();
        importItem.strAmount    = joItem["strAmount"].toString();
        importItem.dAmount      = joItem["dAmount"].toDouble();
        importItem.uSequence    = joItem["strSequence"].toVariant().toLongLong();
        importItem.strSrcAddr   = joItem["srcAddr"].toString();
        importItem.strDstAddr   = joItem["dstAddr"].toString();
        importItem.uBaseFee     = joItem["fee"].toVariant().toUInt();
        importItem.strSignedRawTx    = joItem["rawSignedTxHex"].toString();
        importItem.bDstAccountExists    = joItem["dstAccountExists"].toBool(true);

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

int rawtx::CXlmRawTxUtilsImpl::BroadcastRawTx(rawtx::IRawTxUtilsParamBase *_pParam)
{
    //1.参数检查
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    Xlm_BroadcastRawTxParam *pParam = dynamic_cast<Xlm_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<Xlm_BroadcastRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<Xlm_BroadcastRawTxParam *> failed.").toStdString() );
    }

    QString strErrMsg = "";
    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( QString( "internal error:" + strErrMsg ).toStdString()  );
    }


    //2.拼装请求参数
    QString strReqUrl ;
    strReqUrl = STR_strUrlSendRawTransaction_XLM;


    QJsonObject joParams;
    joParams.insert( "orderId", pParam->strOrderId );
    joParams.insert( "data", pParam->strSignedRawTx );

    QByteArray bytesRsp;
    IRawTxUtils::_SyncPostByJson(strReqUrl, joParams, bytesRsp);


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

    if(!(joResult.contains("order_id") && joResult.contains("txid") && joResult.contains("msg") ))
    {
        pParam->strErrMsg = QString("error msg: response json miss `order_id` or `txid` or  `msg` . ");
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
    QString strSubmitMsg = joResult.value("msg").toString();

    if(UINT_TXID_CHAR_LEN != strTxid.length())
    {
        strErrMsg = QString("error msg: txid's length is illegal. the correct length is  %1.").arg(UINT_TXID_CHAR_LEN);
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " BroadcastRawTx() success. txid:"
            << strTxid  << ".  sequence=" << pParam->uSequence << ".";


    //4.返回txid
    pParam->strTxid = strTxid;
    pParam->strErrMsg = "";

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CXlmRawTxUtilsImpl::GetAccountInfo(XlmAccountInfo &accountInfo)
{
    QString strErrMsg = "";
    //1.参数检查
    if( accountInfo.strAddress.isEmpty()
        || !accountInfo.strAddress.startsWith('G')
        || accountInfo.strAddress.length() != UINT_XLM_ADDR_LEN)
    {
        strErrMsg = QString("%1: args error. `%2` is invalid address.").arg(__FUNCTION__).arg(accountInfo.strAddress);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求参数
    QString  strReqUrl;
    strReqUrl = STR_strUrlGetAccountInfo_XLM;
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
        && joResult.contains("account") && joResult.contains("active") ))
    {
        strErrMsg = QString("reponse json missing `balance` or `sequence` or `account` or `active` field.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //5.返回
    try
    {
        accountInfo.bFound = joResult.value("active").toBool(); //账户是否存在
        accountInfo.uSequence =  boost::lexical_cast<uint64_t>(joResult.value("sequence").toString().toStdString());
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
