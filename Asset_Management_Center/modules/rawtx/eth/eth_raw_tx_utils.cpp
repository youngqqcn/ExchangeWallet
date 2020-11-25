#include "eth_raw_tx_utils.h"
#include "utils.h"

using namespace rawtx;
using namespace eth;


#define   STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort

//获取nonce值
#define STR_strUrlGetTrasactionCount  (STR_WALLET_API_HOST_PORT + STR_eth_gettransactioncount_address)
#define STR_strUrlGetBalance          (STR_WALLET_API_HOST_PORT + STR_eth_getbalance_address)
#define STR_strUrlSendRawTransaction  (STR_WALLET_API_HOST_PORT + STR_eth_sendrawtransaction_data)
#define STR_strUrlGasPrice          (STR_WALLET_API_HOST_PORT + STR_eth_gasprice)

#define STR_strUrlGetTrasactionCount_ETC  (STR_WALLET_API_HOST_PORT + STR_etc_gettransactioncount_address)
#define STR_strUrlGetBalance_ETC          (STR_WALLET_API_HOST_PORT + STR_etc_getbalance_address)
#define STR_strUrlSendRawTransaction_ETC  (STR_WALLET_API_HOST_PORT + STR_etc_sendrawtransaction_data)




int CETHRawTxUtilsImpl::CreateRawTx(IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    int iRet = -1;

     QString strErrMsg = "";

    //1.参数检查
    if(NULL == _pParam)
    {
        strErrMsg = QString("internal error: _Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    ETH_CreateRawTxParam *pParam = dynamic_cast<ETH_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<ETH_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //2.获取dll接口的入参, 注意 char *  的生命周期, 禁止使用匿名对象!!!
    //必须使用局部对象保存中间变量
    //    std::string strTmp = UBigint2String(pParam->ubigintGasPrice);
    std::string strNonce    = QString::asprintf("%llu", pParam->uNonce).toStdString();
    std::string strGasStart = UBigint2String(pParam->ubigintGasStart);
    std::string strGasPrice = UBigint2String(pParam->ubigintGasPrice);
    std::string strValue    = Float2UBigintString(pParam->float100Value); //value


    EthTxData txData;
    txData.uChainId     = pParam->uChainId;
    txData.pszNonce     = (char *)(strNonce.c_str());
    txData.pszAddrTo    = (char *)(pParam->strAddrTo.c_str());
    txData.pszValue     = (char *)(strValue.c_str());
    txData.pszGasStart  = (char *)(strGasStart.c_str());
    txData.pszGasPrice  = (char *)(strGasPrice.c_str());
    txData.pData        = (pParam->szData);
    txData.uDataLen     = pParam->uDataLen;


    /*
     * ETH_NO_ERROR = 0,
        ETH_ERR_BadRLP = 95,
        ETH_ERR_ErrChainID = 96, //错误的chainID
        ETH_ERR_SECP256K1_ECDSA_SIGN_RECOVERABLE_faild = 97, //函数调用失败
        ETH_ERR_INVALID_SIG = 99, //无效签名
     */
    //3.调用接口,并返回结果
    iRet = Transaction::Sign(txData, pParam->strPrivKey.c_str(), pParam->szRetBuf, pParam->uBufSize, &pParam->uRetDataLen);
    if(eth::Transaction::ETH_NO_ERROR != iRet)
    {
        memset( pParam->szRetBuf, 0, pParam->uBufSize);
        pParam->uRetDataLen = 0;

        strErrMsg = QString("internal error: call libethsig.dll function returns error, code: %1").arg(iRet);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("call libethsig.dll success.");

    return IRawTxUtils::NO_ERROR;
}

int CETHRawTxUtilsImpl::ExportRawTx(IRawTxUtilsParamBase *_pParam)
{
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        strErrMsg  = QString("_Param is nullptr.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //1.类型转换
    ETH_ExportRawTxParam *pParam = dynamic_cast<ETH_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<ETH_ExportRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //2.检查参数

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //3.组成json对象
    QJsonObject  joExp;

    //info
    if(true)
    {
        QJsonObject joInfo;
        joInfo.insert("isClcSign", pParam->m_bIsCollection);
        joInfo.insert("coinType",  pParam->m_strCoinType );
        //joInfo.insert("coinType",  "ETH" ); //coinType 仍然是 ETH, 具体的币种使用  item中的 symbol字段来确定 2019-09-04 yqq
        joInfo.insert("timestamp", QString::asprintf("%u", QDateTime::currentDateTime().toTime_t()) );

        joExp.insert("info", joInfo);
    }

    //items
    if(true)
    {
        QJsonArray  jaItems;
        for(size_t i = 0; i < pParam->size(); i++)
        {
            ETH_ExportRawTxItem  item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("symbol", item.strSymbol.toUpper());
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("chainId", item.uChainId);
            joItem.insert("value" , item.strValue);
            joItem.insert("ERC20TokenValue", item.strERC20TokenValue);
            joItem.insert("ERC20Recipient", item.strERC20Recipient);
            joItem.insert("nonce", item.strNonce);
            joItem.insert("from", item.strAddrFrom);
            joItem.insert("to", item.strAddrTo);
            joItem.insert("gasPrice", item.strGasPrice);
            joItem.insert("gasStart", item.strGasStart);

            //资产管理导出时此字段为空,钱包管理导出时此字段不能为空
            joItem.insert("rawSignedTxHex", item.strSignedRawTxHex);

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

int CETHRawTxUtilsImpl::ImportRawTx(IRawTxUtilsParamBase *_pParam)
{
    //int iRet = -1;
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    //1.类型转换
    ETH_ImportRawTxParam *pParam = dynamic_cast<ETH_ImportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<ETH_ImportRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<ETH_ImportRawTxParam *> failed.").toStdString() );
    }

    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        strErrMsg = QString("internal error:") + strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.检查参数

    if(pParam->m_strFilePath.isEmpty())
    {
        strErrMsg = "file Path is empty" ;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //3.打开文件
    QFile fileImport(pParam->m_strFilePath);
    if(false == fileImport.exists())
    {
        strErrMsg = QString("import json file not exists! FILE PATH:\"%1\"").arg(pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    if(false == fileImport.open(QIODevice::ReadOnly))
    {
        strErrMsg = QString("import json file open failed! FILE PATH:\"%1\"").arg(pParam->m_strFilePath);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //4.读取文件内容,解析成jsondocument
    QByteArray bytesImport = fileImport.readAll();
    fileImport.close();

    QJsonParseError jError;
    QJsonDocument jdocImport(QJsonDocument::fromJson(bytesImport, &jError));
    if(QJsonParseError::NoError != jError.error )
    {
        strErrMsg =  "json parse error! errorMsg:" + jError.errorString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //5.读取jsondocument中的数据,存入 __m_vctItems
    QJsonObject joImport = jdocImport.object();
    if(!joImport.contains("info") || !joImport.contains("items"))
    {
        strErrMsg = "json file missing `info` or `items` fields";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    QJsonObject joInfo = joImport.value("info").toObject();
    if(!(joInfo.contains("coinType") && joInfo.contains("timestamp")&& joInfo.contains("isClcSign")))
    {
        strErrMsg = "json file `info` missing some fields.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }
    pParam->m_bIsCollection = joInfo.value("isClcSign").toBool();

    QString strTxFileCoinType = joInfo.value("coinType").toString().trimmed();
    strTxFileCoinType = strTxFileCoinType.toUpper();

    if( !(0 == strTxFileCoinType.compare("ETH", Qt::CaseInsensitive) || utils::IsERC20Token( strTxFileCoinType )
        || 0 == strTxFileCoinType.compare("ETC", Qt::CaseInsensitive)
        ))
    {
        strErrMsg = QString("`cointype` is `%1`, is not `ETH` or ERC20 Token .").arg(joInfo.value("coinType").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    //2020-06-29  修复(ERC20代币)签名时界面上选择的币种  和 交易文件中的币种不一致的情况下 也能签名通过的bug
    if( 0 != pParam->m_strCoinType.toUpper().compare(strTxFileCoinType, Qt::CaseInsensitive))
    {
        strErrMsg = QString("you select cointype is `%1`, but txfile's cointype is `%2` .")
                                .arg(pParam->m_strCoinType).arg(strTxFileCoinType);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if(joInfo.value("timestamp").toString().isEmpty())
    {
        strErrMsg = QString("`timestamp` is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString());
    }

    if(joInfo.value("timestamp").toString().toULong() < 1556355267/*2019-04-27 16:54:27*/
        || joInfo.value("timestamp").toString().toULong() > 1631264033 /*2021-09-10 16:53:53*/)
    {
        strErrMsg = QString("`timestamp` is illegal, too small or too large. timestamp  must between  2019-04-27 16:54:27 and 2021-09-10 16:53:53. \
                            if the current timestamp is larger than  2021-09-10 16:53:53 , please contact with developers.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //TODO:在info中加上hash, 防止文件内容被修改
    //2019-04-27 yqq
    //QCryptographicHash::Sha3_256();
    //QCryptographicHash::Md5();
    //QByteArray hash(const QByteArray & data, Algorithm method)

    QJsonArray  jaItems = joImport.value("items").toArray();
    if(jaItems.isEmpty())
    {
        strErrMsg = QString("`items` is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(int i = 0; i <  jaItems.size(); i++)
    {
        ETH_ImportRawTxItem importItem;

        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("complete") && joItem.contains("orderId") && joItem.contains("value") && joItem.contains("nonce")\
             && joItem.contains("from") && joItem.contains("to") && joItem.contains("gasPrice")\
             && joItem.contains("gasStart") && joItem.contains("chainId") && joItem.contains("rawSignedTxHex")))
        {
            strErrMsg =  QString("`items` missing some neccessary fields");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(false == CheckChainId(joItem["chainId"].toInt()))
        {
            strErrMsg = QString("`chainId` is invlaid.");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        importItem.strSymbol        = joItem["symbol"].toString().toUpper(); //ETH 为 ETH ,  其他的 ERC20
        importItem.bComplete        = joItem["complete"].toBool();
        importItem.uChainId         = UInt2ChainId( joItem["chainId"].toInt() );
        importItem.strAddrFrom      = joItem["from"].toString();
        importItem.strAddrTo        = joItem["to"].toString();
        importItem.strGasPrice      = joItem["gasPrice"].toString();
        importItem.strGasStart      = joItem["gasStart"].toString();
        importItem.strNonce         = joItem["nonce"].toString();
        importItem.strOrderId       = joItem["orderId"].toString();
        importItem.strValue         = joItem["value"].toString();
        importItem.strERC20TokenValue = joItem["ERC20TokenValue"].toString();
        importItem.strERC20Recipient= joItem["ERC20Recipient"].toString(); //ERC20代币的接收方地址, 注意和to地址的区分
        importItem.strSignedRawTxHex = joItem["rawSignedTxHex"].toString();

        pParam->push_back(importItem);
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("ImportRawTx() success.");


    return IRawTxUtils::NO_ERROR;
}

int CETHRawTxUtilsImpl::BroadcastRawTx(IRawTxUtilsParamBase *_pParam)
{
    QString strErrMsg = "";
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    //1.类型转换
    ETH_BroadcastRawTxParam  *pParam = dynamic_cast<ETH_BroadcastRawTxParam*>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<ETH_BroadcastRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<ETH_BroadcastRawTxParam *> failed.").toStdString() );
    }

    //2.检查参数
    if(false == pParam->ParamsCheck( &pParam->strErrMsg ))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << pParam->strErrMsg;
        throw runtime_error( QString( "internal error:" + pParam->strErrMsg ).toStdString() );
    }

    //3.装参数调用服务端-以太坊交易广播接口,进行广播
    QString strReqUrl =  STR_strUrlSendRawTransaction;
    if(0 == pParam->strCoinType.compare("ETC", Qt::CaseInsensitive))
        strReqUrl = STR_strUrlSendRawTransaction_ETC;

    strReqUrl.trimmed();
    strReqUrl += pParam->strSignedRawTxHex;
    strReqUrl += "&orderId=" + pParam->strOrderId;
    strReqUrl = strReqUrl.trimmed();

    QByteArray bytesRsp;
    _SyncPostByURL(strReqUrl, bytesRsp);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL() called, success return.";


    //4.解析返回结果
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
        strErrMsg = QString("json data missing `result` or `success` ");
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = joRes.value("result").toString();
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joResult =  joRes.value("result").toObject();
    if(!(joResult.contains("txid") && joResult.contains("orderId")))
    {
        strErrMsg = QString("error msg: server response missing `txid` or `orderId`. ");
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QString strOrderId = joResult.value("orderId").toString(); //订单号
    /*if(0 != strOrderId.compare(pParam->strOrderId))
    {
        strErrMsg = QString("error msg: order is not match. ");
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }*/

    QString strTxid = joResult.value("txid").toString(); //交易id
    if(strTxid.isEmpty())
    {
        strErrMsg = QString("error msg: result is empty. ");
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(66 != strTxid.length())
    {
        strErrMsg = QString("error msg: txid's length is illegal. the correct length is  66.");
        pParam->strErrMsg = strErrMsg;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " BroadcastRawTx() success. txid:" << strTxid  << ".  nonce=" << pParam->strNonce << ".";

    //5.返回txid
    pParam->strTxid = strTxid;

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CETHRawTxUtilsImpl::GetNonce(const QString &strAddr, uint64_t &Nonce,  QString strCoinType)
{
    //191.168.10.79:9000/eth/gettransactioncount?address=0x954d1a58c7abd4ac8ebe05f59191Cf718eb0cB89
    Nonce = INT_MAX; //初始化未有符号整型最大值
    QString strErrMsg = "";

    //1.参数检查
    if(strAddr.isEmpty())
    {
        strErrMsg = QString("args error: `strAddr` is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == utils::IsValidAddr("ETH", strAddr)
        ||false == utils::IsValidAddr("ETC", strAddr) )
    {
        strErrMsg =  QString("args error:'%1' is invalid ETH address").arg(strAddr);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求URL
    QString  strReqUrl = STR_strUrlGetTrasactionCount;
    if(0 ==  strCoinType.compare("ETC", Qt::CaseInsensitive) )
        strReqUrl = STR_strUrlGetTrasactionCount_ETC;
    strReqUrl = strReqUrl.trimmed();
    strReqUrl += strAddr;
    strReqUrl = strReqUrl.trimmed();

    //3.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRet);

    //4.解析返回结果
    //{"result": 56, "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1556243232748}
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg =  QString("json parse error:%1").arg( error.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        strErrMsg = QString("json missing `result` or `success`");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = joRes.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(joRes.value("result").toString().isEmpty())
    {
        strErrMsg = QString("result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //uint64_t uTmp = joRes.value("result").toInt(INT_MAX);  //error 当那个nonce过大时,会导致数据被截断
    bool bConvertOk = false;
    uint64_t  uTmp = joRes.value("result").toString().toULongLong(&bConvertOk, 10);
    if(false == bConvertOk)
    {
        strErrMsg = QString("internal error : joRes.value(\"result\").toString().toULongLong() error");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(uTmp > UINT_MAX)
    {
        //只是警告而已!!! 切莫直接返回 !!
        strErrMsg = "nonce is too large, please contact with developers.";
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //5.返回数据
    Nonce = uTmp;

    return IRawTxUtils::NO_ERROR;
}

//http://192.168.10.79:9000/eth/getbalance?address=0xbe1085bc3e0812f3df63deced87e29b3bc2db524
//{"result": 60894239646733, "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1556246364015}
//注意: result是 bigint类型, 不要用浮点数
//int CETHRawTxUtilsImpl::GetBalance(const QString &strAddr,  std::map<QString, float100> &mapBalances)
int CETHRawTxUtilsImpl::GetBalance(const QString &strAddr,  std::map<QString, double> &mapBalances,  QString strCoinType)
{
    //ubigintBalance= 0;
    QString strErrMsg = "";

    //1.参数检查
    if(strAddr.isEmpty())
    {
        strErrMsg = QString("internal error: address is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == utils::IsValidAddr("ETH", strAddr)
        || false == utils::IsValidAddr("ETC", strAddr))
    {
        strErrMsg =  QString("args error:'%1' is invalid ETH address").arg(strAddr);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求URL
    QString  strReqUrl = STR_strUrlGetBalance;
    if(0 == strCoinType.compare("ETC", Qt::CaseInsensitive))
        strReqUrl = STR_strUrlGetBalance_ETC;
    strReqUrl = strReqUrl.trimmed();
    strReqUrl += strAddr;
    strReqUrl = strReqUrl.trimmed();

    //3.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRet);


    //4.解析返回结果
    //{"result": 56, "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1556243232748}
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse failed! errorCode = %1").arg( error.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        strErrMsg = QString("reponse json missing `result` or `success` field.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg =  QString(joRes.value("result").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joBalances = joRes.value("result").toObject();
    if(joBalances.isEmpty())
    {
        strErrMsg = QString("error json response `result` is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }



    try
    {
        for(auto it = joBalances.begin(); it != joBalances.end(); it++)
        {
            QString strCoinType = it.key();
            std::string cstrBalance = it.value().toString().toStdString();
            double dBalance = boost::lexical_cast<double>(cstrBalance);  //暂时先用 double类型

            mapBalances.insert(std::make_pair(strCoinType,   dBalance ));
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = "GetBalance() error: " + QString(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    return IRawTxUtils::NO_ERROR;
}



int CETHRawTxUtilsImpl::MakeStdERC20TransferData(const QString &strTo, const QString &strHexValueInWei, std::string &cstrRetData) noexcept(false)
{
    QString strAbiTransfer = STR_erc20_transfer_sig;

    QString strTmpTo = strTo;
    strTmpTo = strTmpTo.replace(QString("0x"), QString(""), Qt::CaseInsensitive);
    QString strTmpValue = strHexValueInWei;
    strTmpValue = strTmpValue.replace(QString("0x"), QLatin1String(""), Qt::CaseInsensitive);
    QString strToPadded = utils::ZeroPadLeft(strTmpTo , 64);
    QString strHexValueInWeiPadded = utils::ZeroPadLeft( strTmpValue, 64);

    QString strData = strAbiTransfer + strToPadded + strHexValueInWeiPadded;

    cstrRetData = utils::HexToBin(strData.toStdString());
    return IRawTxUtils::NO_ERROR;
}

int CETHRawTxUtilsImpl::ToWeiStr(double dAmount , int nDecimals, std::string &cstrRet) noexcept(false)
{
    float100 floatTmpValue =   float100(QString::asprintf("%.8f", dAmount).toStdString());
    while((nDecimals--) > 0) floatTmpValue *= 10;
    ubigint ubigintTmpValue  = floatTmpValue.convert_to<ubigint>();
    std::string strTmp = ToBigEndianString(ubigintTmpValue);
    cstrRet = utils::Bin2HexStr( (unsigned char *)strTmp.c_str(), strTmp.size() );
    return IRawTxUtils::NO_ERROR;
}


QString CETHRawTxUtilsImpl::GetGasPrice()  noexcept(false)
{
    QString strErrMsg = "";

    //1.拼装请求URL
    QString  strReqUrl = STR_strUrlGasPrice;
    strReqUrl = strReqUrl.trimmed();

    //2.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncGetByURL(strReqUrl, bytesRet);


    //3.解析返回结果
    //{"result": 132000000000, "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1556243232748}
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = QString("json parse failed! errorCode = %1").arg( error.errorString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jsonDoc.object();
    if(!joRes.contains("result") || !joRes.contains("success"))
    {
        strErrMsg = QString("reponse json missing `result` or `success` field.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(0 == joRes.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg =  QString(joRes.value("result").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QString strGasPrice = joRes.value("result").toString();
    if(strGasPrice.isEmpty())
    {
        strErrMsg = QString("error json response `result` is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    bool ok = false;
    quint64 uGasPrice = strGasPrice.toULongLong(&ok ) + 50000000000LL; //在现有的价格上增加 20Gwei
    if(!ok)
    {
        strErrMsg = "gasprice error: " + strGasPrice;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString())    ;
    }

    // 防止 gas price 太大或太小
    if( MAX_GASPRICE < uGasPrice)
    {
        return QString::asprintf("%llu", MAX_GASPRICE);
    }
    else if(uGasPrice < MIN_GASPRICE)
    {
        return QString::asprintf("%llu", MIN_GASPRICE);
    }

    strGasPrice = QString::asprintf("%llu", uGasPrice);

    return strGasPrice;
}



































