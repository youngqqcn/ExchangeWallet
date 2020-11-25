

#include "btc_off_sig.h"
using namespace std;


CBTCOffSig::CBTCOffSig(QObject *parent) : QObject(parent)
{
    m_pReply = NULL;
    m_pNetAccMgr = NULL;//201905231741增加
    m_pNetAccMgr = new QNetworkAccessManager(this);

    m_strRPCAuth = "btc:btc2018";
    m_strDeamonIpPort = STR_BTC_NODE_IP_PORT;
    m_strCoinType = "BTC";
}

CBTCOffSig::~CBTCOffSig()
{
    //201905231741增加
    if (NULL != m_pNetAccMgr)
    {
        m_pNetAccMgr->destroyed();
        m_pNetAccMgr = NULL;
    }
}



int CBTCOffSig::_SyncPostData(const QString &strUrl, const QJsonObject &joPostData, QByteArray &byteArray) noexcept(false)
{
    QString strErrMsg;

    QString strTmpUrl = strUrl;
    if(strTmpUrl.isEmpty())
    {
        strErrMsg = "args error: url is empty";
        qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QString strHAuth = m_strRPCAuth;
    QByteArray strAuth = "Basic "+ strHAuth.toLatin1().toBase64();

    QUrl url = strTmpUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    req.setRawHeader("Authorization", strAuth);
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求数据: " << url << joPostData;

    m_pNetAccMgr->disconnect();
    m_pReply = m_pNetAccMgr->post( req, QJsonDocument(joPostData).toJson(QJsonDocument::Compact));

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(m_pReply);

    byteArray = m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArray;


    m_pReply->close();
    return 0;
}




//交易离线签名接口
int CBTCOffSig::TxOfflineSignature(
        bool  bIsCollection,    //是否为归集签名
       QString &strSignedRawTxHex, //签名后交易数据(十六进制)
       const vector<QString> &vctTxInAddrs,  //交易输入地址
       const map<QString , double>  &mapTxOutMap,  //交易输出信息, 用于验证
       const QString &strUnsignedRawTxHex, //交易数据, 十六进制
       const vector<QString> &vctPrivKeys, //私钥
       const vector<UTXO> &vctUtxos,//UTXO的信息
       const QString &strSigHashType,/* = "ALL",//签名类型*/
       unsigned char nAddrType/* = CBTCAddress::P2SKH*///地址类型
    )noexcept(false)
{
    QString strErrMsg;

    //1.参数检查, 错误码: 1, CBTCOfflineSig::ARGS_ERR
    if(true)
    {
        if(vctTxInAddrs.empty())
        {
            strErrMsg = "internal error: lstTxInAddrs is empty";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(vctPrivKeys.empty())
        {
            strErrMsg = "internal error: lstPrivKeys is empty";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(vctTxInAddrs.size() != vctPrivKeys.size()) //输入地址数和私钥个数不相等
        {
            strErrMsg = QString("internal error: vctTxInAddrs's size (%1) is not equals  vctPrivKeys's size(%2).\
                    you must missing some address's priv-keys. please check it. ").arg(vctTxInAddrs.size()).arg(vctPrivKeys.size());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(mapTxOutMap.empty())
        {
            strErrMsg = "internal error: mapTxOutMap is empty.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(strUnsignedRawTxHex.isEmpty())
        {
            strErrMsg = "internal error: strUnsignedRawTxHex is empty.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(vctUtxos.empty())
        {
            strErrMsg = "internal error: vctUtxos is empty.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( !("ALL" == strSigHashType || "NONE" == strSigHashType
              ||  "SINGLE" == strSigHashType || "ALL|ANYONECANPAY" == strSigHashType
              || "NONE|ANYONECANPAY" == strSigHashType || "SINGLE|ANYONECANPAY" == strSigHashType ))
        {
            strErrMsg = "args error: strSigHashType is invalid.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //检查地址类型, 目前支持 P2SKH 地址
        if(!(BTCAddrType::P2SKH == nAddrType /* || CBTCAddress::P2SH == nAddrType || CBTCAddress::P2WSH == nAddrType*/))
        {
            if(BTCAddrType::P2SH == nAddrType)
            {
                strErrMsg = "internal error: current version do not supports for P2SH type address.";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            else if(BTCAddrType::P2WSH == nAddrType)
            {
                strErrMsg = "internal error: current version do not supports for P2WSH type address.";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            else
            {
                strErrMsg = "internal error: strSigHashType is invalid";
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }
    }


    //2.交易检查(交易内容, 是否与 mapTxOutMap的信息对应)
    if(true)
    {
        //2.1 decoderawtransaction 解码裸交易
        QJsonObject joParams;

        QJsonArray jaParams;
        jaParams.append(strUnsignedRawTxHex);

        joParams.insert("jsonrpc", QString("1.0"));
        joParams.insert("id", QString("curltest"));
        joParams.insert("method", QString("decoderawtransaction"));
        joParams.insert("params", jaParams);

        QByteArray  byteArrayDecode;
        _SyncPostData(m_strDeamonIpPort, joParams, byteArrayDecode);


        qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "decoderawtransaction返回的数据:" << byteArrayDecode;


        //2.2 解析decoderawtransaction返回的数据
        QJsonParseError parseJsonErr;
        QJsonDocument jdoc = QJsonDocument::fromJson(byteArrayDecode, &parseJsonErr);
        if(QJsonParseError::NoError != parseJsonErr.error)
        {
            strErrMsg = "json parse error: decoderawtransaction response" + parseJsonErr.errorString();
            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joDecode = jdoc.object();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " id :" << joDecode["id"].toString();

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

        std::map<QString, double>  mapDecodeTxOutMap; //解析后的
        if(joDecode.contains("result"))
        {
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
        }
        else
        {
            strErrMsg = "missing `result`.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(bIsCollection)   //归集签名
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


        if(mapDecodeTxOutMap.size()  != mapTxOutMap.size())
        {
            strErrMsg = "safe check: mapDecodeTxOutMap's size is not equals inputs args .";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //2019-11-20  by yqq
        //BCH 和 BSV不进行检查,  实际的目的地址(rawtx中的) 和 mapTxOutMap(接口返回的) 不一样(黑客恶意操作)
        //后面有时间可以参考一下代码, 实现cashaddress的转换
        // https://github.com/Bitcoin-ABC/bitcoin-abc/blob/master/src/cashaddr.cpp
        //https://github.com/Bitcoin-ABC/bitcoin-abc/blob/master/src/cashaddrenc.cpp
        //https://github.com/Electron-Cash/Electron-Cash/blob/master/lib/cashaddr.py
        //cashaddress/convert.py
        if(!(0 == m_strCoinType.compare("BCH", Qt::CaseInsensitive) || 0 == m_strCoinType.compare("BSV", Qt::CaseInsensitive)
            ||  0 == m_strCoinType.compare("LTC", Qt::CaseInsensitive)))
        {
            for(auto it = mapDecodeTxOutMap.begin(); it != mapDecodeTxOutMap.end(); it++)
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  it->first << "  " << it->second ;

                auto itTmp = mapTxOutMap.find(it->first);
                if(itTmp == mapTxOutMap.end())
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
        }
    }//if



    //3.组装参数, 调用rpc接口的  signrawtransactionwithkey
    if(true)
    {
        QJsonObject joParams;

        QJsonArray jaParams;
        jaParams.append(strUnsignedRawTxHex); //未签名裸交易

        QJsonArray jaPrivKeys;
        for(auto strPrivKey : vctPrivKeys)  //添加私钥
        {
            jaPrivKeys.append(strPrivKey);
        }
        jaParams.append(jaPrivKeys);

        QJsonArray jaUTXOs;

        for(auto utxo : vctUtxos)
        {
            QJsonObject joTmpUTXO;
            joTmpUTXO.insert("txid", utxo.txid);
            joTmpUTXO.insert("vout", QJsonValue((int)(utxo.vout)) );
            if(!utxo.redeemScript.trimmed().isEmpty())
            {
                joTmpUTXO.insert("redeemScript", utxo.redeemScript); //暂时不支持ps2h类型的地址, 如需添加, 后续放开即可
            }
            qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"============警告:非测试模式去掉redeemScript";
            joTmpUTXO.insert("scriptPubKey", utxo.scriptPubKey);
            joTmpUTXO.insert("amount", utxo.amount);

            jaUTXOs.append(joTmpUTXO);
        }
        jaParams.append(jaUTXOs);




        joParams.insert("jsonrpc", QString("1.0"));
        joParams.insert("id", QString("curltest"));
        joParams.insert("method", QString("signrawtransactionwithkey"));

        //DASH 和  BSV  的参数顺序和 函数名不一样
        if(0 == this->m_strCoinType.compare("DASH", Qt::CaseInsensitive)
            /*|| 0 == this->m_strCoinType.compare("BSV", Qt::CaseInsensitive)*/     //BSV使用BCH节点进行签名
            )
        {
            jaParams.replace(1, jaUTXOs);
            jaParams.replace(2, jaPrivKeys);

            joParams["method"] = QString("signrawtransaction");
        }

        joParams.insert("params", jaParams);

        //qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "decoderawtransaction返回的数据:" << byteArrayDecode;

        qDebug()<<tr("request params:[")<< joParams<< "]";

        QByteArray byteArrReply;


        //4.http请求
        _SyncPostData(m_strDeamonIpPort, joParams, byteArrReply);


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

        if(!joRes.contains("error") )
        {
            strErrMsg = "missing `error`";
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



        if(!joRes.contains("result") )
        {
            strErrMsg = "missing `result`";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(joRes.value("result").toObject().isEmpty())
        {
            strErrMsg = " `result` is empty";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!joRes.value("result").toObject().contains("hex"))
        {
            strErrMsg = "missing `hex`";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!joRes.value("result").toObject().contains("complete"))
        {
            strErrMsg = " missing `complete` ";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QString strHexSignedRawTx = joRes.value("result").toObject().value("hex").toString();
        bool bComplete = joRes.value("result").toObject().value("complete").toBool(false);
        if(false == bComplete)
        {
            strErrMsg = "complete is false ";
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
            _SyncPostData(m_strDeamonIpPort, joParams, byteArrayDecode);


            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "decoderawtransaction返回的数据:" << byteArrayDecode;

            QJsonParseError parseJsonErr;
            QJsonDocument jdoc = QJsonDocument::fromJson(byteArrayDecode, &parseJsonErr);
            if(QJsonParseError::NoError != parseJsonErr.error)
            {
                strErrMsg = "json parse error: "  + parseJsonErr.errorString();
                qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            //正确: {"result":{....}, "error":null,"id":"curltest"}
            //错误情况: {"result":null,"error":{"code":-22,"message":"TX decode failed"},"id":"curltest"}
            QJsonObject joDecode = jdoc.object();
            if(!joDecode.value("error").toObject().isEmpty())
            {
                int iErrCode =  joDecode.value("error").toObject().value("code").toInt() ;
                QString strTmpMsg = joDecode.value("error").toObject().value("message").toString() ;
                strErrMsg = QString("%1 : %2").arg(iErrCode).arg(strTmpMsg);

                qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  "decoderawtransaction 成功 ";
        }

        //7.组装返回数据
        strSignedRawTxHex = strHexSignedRawTx;
    }
    return CBTCOffSig::NO_ERROR;
}


