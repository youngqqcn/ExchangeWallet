#include "usdt_off_sig.h"

CUSDTOffSig::CUSDTOffSig():CBTCOffSig()
{

}

CUSDTOffSig::~CUSDTOffSig()
{

}




int CUSDTOffSig::TxOfflineSignature(
       QString &strSignedRawTxHex, //签名后交易数据(十六进制)
       double dAmount,            //USDT交易金额
       const vector<QString> &vctTxInAddrs,  //交易输入地址
       const map<QString , double>  &mapTxOutMap,  //交易输出信息, 用于验证
       const QString &strUnsignedRawTxHex, //交易数据, 十六进制
       const vector<QString> &vctPrivKeys, //私钥
       const vector<UTXO> &vctUtxos,//UTXO的信息
       const QString &strSigHashType,/* = "ALL",//签名类型*/
       unsigned char nAddrType/* = CBTCAddress::P2SKH*/) noexcept(false) //地址类型
{

    QString strErrMsg;
    (void) dAmount;

    //1.参数检查, 错误码: 1, CUSDTOfflineSig::ARGS_ERR
    if(true)
    {
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "没有对lstTxInAddrs进行检查";
        if(vctTxInAddrs.empty())
        {
            strErrMsg = QString("internal error: lstTxInAddrs is empty");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(vctPrivKeys.empty())
        {
            strErrMsg = QString("internal error: lstPrivKeys is empty");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(vctTxInAddrs.size() != vctPrivKeys.size()) //输入地址数和私钥个数不相等
        {
            strErrMsg = QString("internal error:  vctTxInAddrs's size is not equals  vctPrivKeys's size");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(mapTxOutMap.empty())
        {
            strErrMsg = QString("internal error: mapTxOutMap's size is empty");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(strUnsignedRawTxHex.isEmpty())
        {
            strErrMsg = QString("internal error: strUnsignedRawTxHex is empty");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(vctUtxos.empty())
        {
            strErrMsg = QString("internal error: lstPrevTx's size is empty");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( !("ALL" == strSigHashType || "NONE" == strSigHashType
              ||  "SINGLE" == strSigHashType || "ALL|ANYONECANPAY" == strSigHashType
              || "NONE|ANYONECANPAY" == strSigHashType || "SINGLE|ANYONECANPAY" == strSigHashType ))
        {
            strErrMsg = QString("internal error: strSigHashType is invalid.");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!(BTCAddrType::P2SKH == nAddrType /* || CBTCAddress::P2SH == nAddrType || CBTCAddress::P2WSH == nAddrType*/))
        {
            if(BTCAddrType::P2SH == nAddrType)
            {
                strErrMsg = QString("internal error: not support P2SH address in this version.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            if(BTCAddrType::P2WSH == nAddrType)
            {
                strErrMsg = QString("internal error: not support P2WSH address in this version.");
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            strErrMsg = QString("internal error: address type %1 is not supported in this version.").arg(nAddrType);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }


    //3.组装参数, 调用rpc接口的  signrawtransaction
    if(true)
    {
        QJsonObject joParams;

        QJsonArray jaParams;
        jaParams.append(strUnsignedRawTxHex); //未签名裸交易

        QJsonArray jaUTXOs;

        for(auto utxo : vctUtxos)
        {
            QJsonObject joTmpUTXO;
            joTmpUTXO.insert("txid", utxo.txid);
            joTmpUTXO.insert("vout", QJsonValue((int)(utxo.vout)) );
            //joTmpUTXO.insert("redeemScript", utxo.redeemScript); //暂时不支持ps2h类型的地址, 如需添加, 后续放开即可
            //qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"============警告:非测试模式去掉redeemScript";
            joTmpUTXO.insert("scriptPubKey", utxo.scriptPubKey);
            joTmpUTXO.insert("amount", utxo.amount);

            jaUTXOs.append(joTmpUTXO);
        }
        jaParams.append(jaUTXOs);   //添加UTXOs

        joParams.insert("jsonrpc", QString("1.0"));
        joParams.insert("id", QString("curltest"));
        joParams.insert("method", QString("signrawtransaction"));


        QJsonArray jaPrivKeys;
        for(auto strPrivKey : vctPrivKeys)  //添加私钥
        {
            jaPrivKeys.append(strPrivKey);
        }
        jaParams.append(jaPrivKeys); //qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "decoderawtransaction返回的数据:" << byteArrayDecode;
        joParams.insert("params", jaParams);
        qDebug()<<tr("request params:[")<< joParams<< "]";

        QByteArray byteArrReply;


        //4.http请求
        _SyncPostData(QString(STR_USDT_NODE_IP_PORT), joParams, byteArrReply); //同步方式请求


        //5.解析http请求结果
        QJsonParseError jParseErr;
        QJsonDocument jDoc = QJsonDocument::fromJson(byteArrReply, &jParseErr);
        if(QJsonParseError::NoError != jParseErr.error)
        {
            strErrMsg = QString("http response json parse error: %1").arg(jParseErr.errorString());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRes = jDoc.object();
        if(!joRes.contains("error") )
        {
            strErrMsg = QString("node response json missing `error`");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(false == joRes.value("error").toObject().isEmpty())
        {
            int iErrCode = joRes.value("error").toObject().value("code").toInt();
            strErrMsg = QString("node returned error: %1, %2").arg(iErrCode).arg( joRes.value("error").toObject().value("message").toString());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QString strHexSignedRawTx;
        bool bComplete = false;
        if(!joRes.contains("result"))
        {
            strErrMsg = "node reponse json missing `result`.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(joRes.value("result").toObject().isEmpty())
        {
            strErrMsg = "node reponse json `result` is empty";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!joRes.value("result").toObject().contains("hex"))
        {
            strErrMsg = "node reponse json `result` missing `hex`.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!joRes.value("result").toObject().contains("complete"))
        {
            strErrMsg = "node reponse json `result` missing `complete`.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        strHexSignedRawTx = joRes.value("result").toObject().value("hex").toString();
        bComplete = joRes.value("result").toObject().value("complete").toBool();

        if(false == bComplete)
        {
            strErrMsg = "node reponse json complete is false ";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        //6.验证返回的已签名的交易
        //2.1 decoderawtransaction 解码裸交易
        if(true)
        {
            QJsonObject joParams;

            QJsonArray jaParams;
            jaParams.append(strHexSignedRawTx);

            joParams.insert("jsonrpc", QString("1.0"));
            joParams.insert("id", QString("curltest"));
            joParams.insert("method", QString("decoderawtransaction"));
            joParams.insert("params", jaParams);

            QByteArray  byteArrayDecode;
            _SyncPostData(QString(STR_USDT_NODE_IP_PORT), joParams, byteArrayDecode);


            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "decoderawtransaction返回的数据:" << byteArrayDecode;

            QJsonParseError parseJsonErr;
            QJsonDocument jdoc = QJsonDocument::fromJson(byteArrayDecode, &parseJsonErr);
            if(QJsonParseError::NoError != parseJsonErr.error)
            {
                strErrMsg = "json parse error: decoderawtransaction response" + parseJsonErr.errorString();
                qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            QJsonObject joDecode = jdoc.object();
            if(joDecode.contains("error"))
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
            else
            {
                strErrMsg = "decoderawtransaction response missing `error` ";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }


        //7.组装返回数据
        strSignedRawTxHex = strHexSignedRawTx;
    }
    return CBTCOffSig::NO_ERROR;
}

