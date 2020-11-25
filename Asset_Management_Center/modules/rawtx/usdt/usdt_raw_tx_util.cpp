#include "usdt_raw_tx_util.h"
#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"
#include "utils.h"

CUSDTRawTxUtil::CUSDTRawTxUtil():CBTCRawTxUtil()
{
}

CUSDTRawTxUtil::~CUSDTRawTxUtil()
{
}


int CUSDTRawTxUtil::CreateRawTransaction(const QString &strURL, const QString &strSrcAddr, USDTExportData &usdtExportData) noexcept(false)
{
    QString strErrMsg = "";
    if(strURL.isEmpty())
    {
        strErrMsg = QString("strURL is empty");
        qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(false == utils::IsValidAddr("USDT", strSrcAddr))
    {
        strErrMsg = QString("address '%1' is invalid.").arg(strSrcAddr);
        qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "源地址: " << strSrcAddr;

    //1.组装参数,调用服务端createrawtransaction接口 创建交易
    QJsonObject joParams;

    QJsonArray  jaSrc;
    jaSrc.append( strSrcAddr );

    joParams.insert("src", jaSrc);


    QJsonArray jaDst;
    for( size_t i = 0; i < usdtExportData.vctExportItems.size(); i++ )
    {
        USDTExportItem  &item = usdtExportData.vctExportItems[i];

        QJsonObject joDstItem;
        joDstItem.insert("addr", item.vctDstAddr[0]);
        joDstItem.insert("amount", item.strTokenAmount); //usdt 使用 tokenAmount  不能搞混
        joDstItem.insert("orderId", item.strOrderID);

        jaDst.append(joDstItem);
    }

    joParams.insert("dst", jaDst);

    QNetworkRequest req;
    req.setUrl(QUrl(strURL));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    SET_HTTPS_SSL_CONFIG(req);


    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求数据: " << QUrl(strURL) << strURL <<  joParams ;

    m_pNetAccMgr->disconnect();
    m_pReply = m_pNetAccMgr->post( req, QJsonDocument(joParams).toJson(QJsonDocument::Compact));

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(m_pReply);

    QByteArray bytesReply = m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << bytesReply;

    m_pReply->close();

    QJsonParseError parseJsonErr;
    QJsonDocument jdoc = QJsonDocument::fromJson(bytesReply, &parseJsonErr);
    if(QJsonParseError::NoError != parseJsonErr.error)
    {
        qDebug() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "QJsonParseError";
        throw std::runtime_error(std::string("json prase error:")  + parseJsonErr.errorString().toStdString());
    }

    QJsonObject joReply = jdoc.object();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " id :" << joReply["id"].toString();

    if(!joReply.contains("result") || !joReply.contains("success"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " missing `result` or `success` fileds. ";
        throw std::runtime_error(std::string("response foramt error:  missing `result` or `success` fileds."));
    }


    //判断返回状态success是否true
    if(0 == joReply.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        QString strErrText  = joReply.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "错误信息:" << strErrText;
        throw std::runtime_error(std::string("server returns:") + std::string(strErrText.toStdString()));
    }


    //2.解析服务端返回数据并校验
    QJsonArray jaResult = joReply.value("result").toArray();



    usdtExportData.bIsCollection = true; //USDT批量提币
    usdtExportData.strCoinType = "USDT";
    usdtExportData.uTimestamp = QDateTime::currentSecsSinceEpoch();

    for(int i = 0; i < jaResult.size(); i++)
    {

        QJsonObject joTxItem = jaResult[i].toObject();

        //获取txout
        if(!joTxItem.contains("txout") || !joTxItem.contains("utxos") || !joTxItem.contains("hex"))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "response missing `txout` or `utxos` or `hex` ";
            throw std::runtime_error( std::string("response foramt error: response missing `txout` or `utxos` or `hex` ") );
        }

        if(!joTxItem.contains("orderId") || !joTxItem.contains("tokenId") || !joTxItem.contains("tokenAmount") )
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "response missing `orderId` or `tokenId` or `tokenAmount` ";
            throw std::runtime_error( std::string("response missing `orderId` or `tokenId` or `tokenAmount` ") );
        }

        double dSumAmout = 0.0;


        QString strRetOrderId = joTxItem.value("orderId").toString();
        USDTExportItem *pUsdtExportItem = NULL;
        for( size_t iItem = 0; iItem < usdtExportData.vctExportItems.size(); iItem++ )
        {
            strRetOrderId = strRetOrderId.trimmed();
            QString strTmpOrderId = usdtExportData.vctExportItems[iItem].strOrderID.trimmed();
            if(0 == strRetOrderId.compare(strTmpOrderId, Qt::CaseInsensitive))
            {
                pUsdtExportItem = &(usdtExportData.vctExportItems[iItem]);
            }
        }

        if(NULL == pUsdtExportItem)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " orderId not found ";
            throw std::runtime_error( std::string("orderId not found ") );
        }

        USDTExportItem  &usdtExportItem = (*pUsdtExportItem);

        //获取 txout
        QJsonObject joTxout = joTxItem.value("txout").toObject();
        auto mapTmpTxout = joTxout.toVariantMap().toStdMap();
        for(auto it = mapTmpTxout.begin(); it != mapTmpTxout.end(); it++)
        {
            try{
                double dAmount = boost::lexical_cast<double>( QVariant(it->second).toString().toStdString() );
                usdtExportItem.mapTxOut.insert(make_pair(it->first, dAmount ));

                dSumAmout += dAmount;

            }catch(boost::bad_lexical_cast &e){
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(e.what());
                throw std::runtime_error(std::string("bad_lexical_cast error: ") + std::string(e.what()) );
            }
        }//for

        //总金额
        usdtExportItem.strAmount = QString::asprintf("%.8f", dSumAmout);  //BTC的金额   USDT的金额是  strTokenAmount

        //交易十六进制
        usdtExportItem.strTxHex = joTxItem.value("hex").toString();

        //获取手续费
        usdtExportItem.strTxFee = joTxItem.value("txFee").toString();

        //获取 UTXO
        QJsonArray jaUtxos = joTxItem.value("utxos").toArray();
        for(int i = 0; i < jaUtxos.size(); i++)
        {
            UTXO utxo;
            QJsonObject joUtxo = jaUtxos[i].toObject();

            utxo.address = joUtxo.value("address").toString();
            utxo.amount = joUtxo.value("amount").toDouble();
            utxo.scriptPubKey = joUtxo.value("scriptPubKey").toString();
            utxo.txid = joUtxo.value("txid").toString();
            utxo.vout = joUtxo.value("vout").toInt();

            usdtExportItem.vctUTXOs.push_back( utxo );

            //暂时将支付手续费的地址也显示出来,如需更改,后期再说
            //地址不重复
            if(usdtExportItem.vctSrcAddr.end() == std::find( usdtExportItem.vctSrcAddr.begin(), usdtExportItem.vctSrcAddr.end(), utxo.address))
            {
                usdtExportItem.vctSrcAddr.push_back( utxo.address );
            }

            qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("UTXO内容:") << i << utxo.address << utxo.amount << utxo.scriptPubKey\
                    <<utxo.txid << utxo.vout << utxo.redeemScript ;
        }//for


        //获取 tokenId
        usdtExportItem.uTokenId = joTxItem.value("tokenId").toInt(0);

        //获取 tokenAmount
        //usdtExportItem.strTokenAmount = joTxItem.value("tokenAmount").toString();


        //usdtExportData.vctExportItems.push_back( usdtExportItem );

    }//for


   qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "成功返回";
    return NO_ERROR; //返回错误码
}





//归集
int CUSDTRawTxUtil::CreateRawTransactionEx_Collection(const QString &strURL, const vector<QString> &vctSrcAddr,
                                                      const QString &strDstAddr, BTCExportData &btcExportData) noexcept(false)
{
    QString strErrMsg;
    //参数检查
    if(strURL.isEmpty())
    {
        strErrMsg = "internal error: strURL is empty.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    if(0 == vctSrcAddr.size() || strDstAddr.isEmpty() )
    {
        strErrMsg = "internal error: vctSrcAddr or  strDstAddr is empty .";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joParams;
    QJsonArray jaParamDst;
    QJsonArray jaParams;

    for(uint i = 0; i < vctSrcAddr.size(); i++) //待归集地址
    {
        if(false == utils::IsValidAddr("USDT", vctSrcAddr[i]))
        {
            strErrMsg = QString("srcaddr:%1 is invalid.").arg(vctSrcAddr[i]);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
        jaParams.append(vctSrcAddr[i]);
    }
    joParams.insert("src", jaParams);

    jaParamDst.append( strDstAddr.trimmed() );
    joParams.insert("dest", jaParamDst);


    //发送创建未签名归集交易请求
    QNetworkRequest req;
    req.setUrl(QUrl(strURL));
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求数据: " << QUrl(strURL) << strURL <<  joParams ;

    m_pNetAccMgr->disconnect();
    m_pReply = m_pNetAccMgr->post( req, QJsonDocument(joParams).toJson(QJsonDocument::Compact));

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(m_pReply);

    QByteArray bytesReply = m_pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << bytesReply;

    m_pReply->close();

    QJsonParseError parseJsonErr;
    QJsonDocument jdoc = QJsonDocument::fromJson(bytesReply, &parseJsonErr);
    if(QJsonParseError::NoError != parseJsonErr.error)
    {
        strErrMsg = QString("json parse error:%1").arg(parseJsonErr.errorString());
        qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joReply = jdoc.object();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " id :" << joReply["id"].toString();

    if(!joReply.contains("result"))
    {
        strErrMsg = QString("response json error: missing `result` field.");
        qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!joReply.contains("success"))
    {
        strErrMsg = QString("response json error: missing `success` field.");
        qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //判断返回状态success是否true
    if(0 == joReply.value("success").toString().compare("false", Qt::CaseInsensitive))
    {
        strErrMsg = QString("server returned `false`: %1").arg( joReply.value("result").toString());
        qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    QJsonArray jaResult = joReply.value("result").toArray();


    btcExportData.bIsCollection = true;
    btcExportData.strCoinType = "USDT";
    btcExportData.uTimestamp = QDateTime::currentSecsSinceEpoch();


    for(int i = 0; i < jaResult.size(); i++)
    {
        BTCExportItem  btcExportItem;


        btcExportItem.vctDstAddr.push_back(strDstAddr);

        btcExportItem.strOrderID = utils::GenOrderID();//QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

        QJsonObject joTxItem = jaResult[i].toObject();

        //获取txout
        if(!joTxItem.contains("txout"))
        {   
            strErrMsg = QString("response json error: missing `txout` field.");
            qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!joTxItem.contains("utxos"))
        {
            strErrMsg = QString("response json error: missing `utxos` field.");
            qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
        if(!joTxItem.contains("hex"))
        {
            strErrMsg = QString("response json error: missing `hex` field.");
            qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        double dSumAmout = 0.0;




        //获取 txout
        QJsonObject joTxout = joTxItem.value("txout").toObject();
        auto mapTmpTxout = joTxout.toVariantMap().toStdMap();
        for(auto it = mapTmpTxout.begin(); it != mapTmpTxout.end(); it++)
        {
            try{
                double dAmount = boost::lexical_cast<double>( QVariant(it->second).toString().toStdString() );
                btcExportItem.mapTxOut.insert(make_pair(it->first, dAmount ));

                dSumAmout += dAmount;

            }catch(boost::bad_lexical_cast &e){
                strErrMsg = QString(" boost::lexical_cast<double> error: %1").arg(QString(e.what()));
                qCritical() <<"[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
        }//for

        //总金额
        btcExportItem.strAmount = QString::asprintf("%.8f", dSumAmout);

        //交易十六进制
        btcExportItem.strTxHex = joTxItem.value("hex").toString();

        //获取手续费
        btcExportItem.strTxFee = joTxItem.value("txFee").toString();

        //获取 UTXO
        QJsonArray jaUtxos = joTxItem.value("utxos").toArray();
        for(int i = 0; i < jaUtxos.size(); i++)
        {
            UTXO utxo;
            QJsonObject joUtxo = jaUtxos[i].toObject();

            utxo.address = joUtxo.value("address").toString();
            utxo.amount = joUtxo.value("amount").toDouble();
            utxo.scriptPubKey = joUtxo.value("scriptPubKey").toString();
            utxo.txid = joUtxo.value("txid").toString();
            utxo.vout = joUtxo.value("vout").toInt();

            btcExportItem.vctUTXOs.push_back( utxo );

            //暂时将支付手续费的地址也显示出来,如需更改,后期再说
            //if(0 != utxo.address.compare(strDstAddr, Qt::CaseInsensitive))
            {
                //获取源地址
                btcExportItem.vctSrcAddr.push_back( utxo.address );
            }


            qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("UTXO内容:") << i << utxo.address << utxo.amount << utxo.scriptPubKey\
                    <<utxo.txid << utxo.vout << utxo.redeemScript ;
        }//for


        //获取 tokenId
        btcExportItem.uTokenId = joTxItem.value("tokenId").toInt(0);

        //获取 tokenAmount
        btcExportItem.strTokenAmount = joTxItem.value("tokenAmount").toString();


        btcExportData.vctExportItems.push_back( btcExportItem );

    }//for

    return NO_ERROR;
}

















