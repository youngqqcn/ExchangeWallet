/*************************************************************************************************
*文件作者:   yangqingqing
*作者邮箱:   youngqqcn@gmail.com
*创建日期:   2019-05-10  14:41
*文件说明:   cosmos裸交易工具类  支持 USDP, HTDF 的交易创建和签名  以及 广播数据的拼装编码 等
*修改记录:

修改日期          修改人                  修改说明                                             版本号
2019-05-10      yangqingqing            创建文件                                            v0.1

*************************************************************************************************/

#include "cosmos/cosmos_raw_tx_utils.h"
#include "utils.h"

#include "bech32/bech32.h" //hrc20 地址转账
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/special_functions/gamma.hpp>

using namespace boost::multiprecision;
using ubigint = boost::multiprecision::uint128_t;
//typedef number<backends::cpp_bin_float<200> > cpp_bin_float_200;
using float100 = boost::multiprecision::cpp_bin_float_100;



//TODO: 写入统一配置文件
#define   STR_WALLET_API_HOST_PORT   g_qstr_WalletHttpIpPort

#define STR_strUrlGetAccountInfo_HTDF (STR_WALLET_API_HOST_PORT + STR_htdf_getaccountinfo_address)
#define STR_strUrlSendRawTransaction_HTDF (STR_WALLET_API_HOST_PORT + STR_htdf_sendrawtransaction_tx)
#define STR_strUrlGetBalance_HTDF  (STR_WALLET_API_HOST_PORT + STR_htdf_getbalance_address)

#define STR_strUrlGetAccountInfo_USDP (STR_WALLET_API_HOST_PORT + STR_usdp_getaccountinfo_address)
#define STR_strUrlSendRawTransaction_USDP (STR_WALLET_API_HOST_PORT + STR_usdp_sendrawtransaction_tx)
#define STR_strUrlGetBalance_USDP (STR_WALLET_API_HOST_PORT + STR_usdp_getbalance_address)


#define STR_strUrlGetAccountInfo_HET (STR_WALLET_API_HOST_PORT + STR_het_getaccountinfo_address)
#define STR_strUrlSendRawTransaction_HET (STR_WALLET_API_HOST_PORT + STR_het_sendrawtransaction_tx)
#define STR_strUrlGetBalance_HET (STR_WALLET_API_HOST_PORT + STR_het_getbalance_address)


int rawtx::CCosmosRawTxUtilsImpl::CreateRawTx(rawtx::IRawTxUtilsParamBase *_pParam)noexcept(false)
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

    Cosmos_CreateRawTxParam *pParam = dynamic_cast<Cosmos_CreateRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg =  QString("internal error: dynamic_cast<Cosmos_CreateRawTxParam *> failed.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //2.组装参数调用签名接口
    std::string  cstrRawTxJsonStr ;

    //如果是 HTDF  则需要调用   ToString_2020       2020-02-25
    if(  true == boost::starts_with(pParam->csRawTx.szMsgFrom, std::string(STR_HTDF) + "1") )
    {
        if(false == pParam->csRawTx.ToString_2020(cstrRawTxJsonStr))
        {
            strErrMsg = "internal error: csRawTx.ToString_2020(strRawTxJsonStr)  error. please check json.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }
    }
    else if(false == pParam->csRawTx.ToString(cstrRawTxJsonStr))
    {
        strErrMsg = "internal error: csRawTx.ToString(strRawTxJsonStr)  error. please check json.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "json str:  " << QString( cstrRawTxJsonStr.c_str());


    //2.1 对交易数据进行sha256
    unsigned char uszSha256Data[256/8] = {0}; memset(uszSha256Data, 0, sizeof(uszSha256Data));
    SHA256( (unsigned char *)cstrRawTxJsonStr.data(), cstrRawTxJsonStr.size(),  uszSha256Data);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sha256Data:" << QString(cosmos::Bin2HexStr(uszSha256Data, sizeof(uszSha256Data)).c_str());


    std::string  cstrPrivKey = pParam->strPrivKey.toStdString();
    std::string  cstrPrivKeyBin = cosmos::HexToBin(cstrPrivKey);


    unsigned char uszSigRS[64] = {0}; memset(uszSigRS, 0, sizeof(uszSigRS));
    unsigned int  uSigOutLen = 0;
    char szErrMsg [1024] = {0}; memset(szErrMsg, 0, sizeof(szErrMsg));
    iRet = cosmos::ECDSA_Sign(
                uszSha256Data,                              //交易信息的json字符串(不含空格)的sha256 哈希值
                sizeof(uszSha256Data),                      //hash的size
                (unsigned char *)(cstrPrivKeyBin.data()),   //私钥
                cstrPrivKeyBin.size(),                      //私钥size(字节数)
                uszSigRS,                                   //签名信息buf
                sizeof(uszSigRS),                           //buf的size
                &uSigOutLen,                                //输出数据的size
                szErrMsg );                                 //错误信息

    if(cosmos::NO_ERROR != iRet)
    {
        strErrMsg = QString(" cosmos::ECDSA_Sign() error: ") + QString(szErrMsg);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sigdata : " << QString( cosmos::Bin2HexStr(uszSigRS, sizeof(uszSigRS)).c_str() );


    //3.组装签名后的交易数据(组装成可以直接广播的十六进制字符串)

    //友元方式访问保护成员
    pParam->_csBcastTx.csRawTx          = pParam->csRawTx;
    pParam->_csBcastTx.strType          = STR_BROADCAST_TYPE;
    pParam->_csBcastTx.strMsgType       = (QString(pParam->csRawTx.szMsgFrom).startsWith("0x")) ? (STR_BROADCAST_MSG_TYPE_HET) : (STR_BROADCAST_MSG_TYPE);
    pParam->_csBcastTx.strPubKeyType    = STR_BROADCAST_PUB_KEY_TYPE;
    pParam->_csBcastTx.strPubkeyValue   = pParam->GetPubKey(true);          //base64编码的公钥
    pParam->_csBcastTx.strSignature     = cosmos::Base64Encode((const char *)uszSigRS, sizeof(uszSigRS), false);  //base64编码的 r,s

    std::string cstrErrMsgTmp;
    if(false == pParam->_csBcastTx.ParamsCheck(cstrErrMsgTmp) )
    {
        strErrMsg = QString(cstrErrMsgTmp.c_str());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " pParam->_csBcastTx.ParamsCheck()  error:" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //获取广播数据
    std::string cstrHexData;
    //如果是 HTDF  则需要调用   ToString_2020       2020-02-25
    if(  true == boost::starts_with(pParam->csRawTx.szMsgFrom, std::string(STR_HTDF) + "1") )
    {
        if(false == pParam->_csBcastTx.ToHexStr_2020(cstrHexData))
        {
            strErrMsg = QString(cstrHexData.c_str());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "pParam->_csBcastTx.ToHexStr_2020() error:" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }
    else if(false == pParam->_csBcastTx.ToHexStr(cstrHexData))
    {
        strErrMsg = QString(cstrHexData.c_str());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "pParam->_csBcastTx.ToHexStr() error:" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    pParam->strHexData = QString(cstrHexData.c_str());

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CCosmosRawTxUtilsImpl::ExportRawTx(rawtx::IRawTxUtilsParamBase *_pParam)noexcept(false)
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

    Cosmos_ExportRawTxParam *pParam = dynamic_cast<Cosmos_ExportRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        strErrMsg = QString("dynamic_cast<Cosmos_ExportRawTxParam *> failed.");
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
            Cosmos_ExportRawTxItem item = (*pParam)[i];
            QJsonObject joItem;
            joItem.insert("complete", item.bComplete);
            joItem.insert("orderId", item.strOrderId);
            joItem.insert("accountNumber", item.strAccountNumber);
            joItem.insert("coinType", item.strCoinType);
            joItem.insert("chainId", item.strChainId);
            joItem.insert("value", item.strValue);
            joItem.insert("sequence", item.strSequence);
            joItem.insert("from", item.strFrom);
            joItem.insert("to", item.strTo);
            joItem.insert("feeAmount", item.strFeeAmount);
            joItem.insert("feeGas", item.strFeeGas);
            joItem.insert("memo", item.strMemo); //统一使用订单号作为memo

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

int rawtx::CCosmosRawTxUtilsImpl::ImportRawTx(rawtx::IRawTxUtilsParamBase *_pParam) noexcept(false)
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

    Cosmos_ImportRawTxParam *pParam = dynamic_cast<Cosmos_ImportRawTxParam *>(_pParam);
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

    QString strTxFileCoinType = joInfo.value("coinType").toString().toUpper();
    if(!(0 == strTxFileCoinType.compare(STR_USDP, Qt::CaseInsensitive)
        || 0 == strTxFileCoinType.compare(STR_HTDF, Qt::CaseInsensitive) || utils::Is_HRC20_Token(strTxFileCoinType)
         || 0 == strTxFileCoinType.compare(STR_HET, Qt::CaseInsensitive))
        )
    {
        strErrMsg = QString("`cointype` is `%1`, is not `%2` or `%3` or `%4`.").arg(joInfo.value("coinType").toString()).arg(STR_USDP).arg(STR_HTDF).arg(STR_HET);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //2020-06-29  修复(HRC20代币)签名时界面上选择的币种  和 交易文件中的币种不一致的情况下 也能签名通过的bug
    if( 0 != pParam->m_strCoinType.toUpper().compare(strTxFileCoinType, Qt::CaseInsensitive))
    {
        strErrMsg = QString("you select `%1`, but txfile's cointype is `%2` .")
                                .arg(pParam->m_strCoinType).arg(strTxFileCoinType);

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
        || joInfo.value("timestamp").toString().toULong() > 1631264033 /*2021-09-10 16:53:53*/)
    {
        strErrMsg = QString("`timestamp` is illegal, too small or too large. timestamp  must between  2019-04-27 16:54:27 and 2021-09-10 16:53:53. \
                            if the current timestamp is larger than  2021-09-10 16:53:53 , please contact with developers.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //5.获取数据
    //TODO:在info中加上hash, 防止文件内容被修改
    //2019-05-11 yqq

    QJsonArray  jaItems = joImport.value("items").toArray();
    if(jaItems.isEmpty())
    {
        strErrMsg = QString("`items` is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    for(int i = 0; i < jaItems.size(); i++)
    {
        Cosmos_ImportRawTxItem  importItem;
        QJsonObject joItem = jaItems[i].toObject();
        if(!(joItem.contains("orderId")&& joItem.contains("coinType") && joItem.contains("chainId") \
             && joItem.contains("value") && joItem.contains("sequence") &&joItem.contains("from")\
             && joItem.contains("to") && joItem.contains("feeAmount") && joItem.contains("feeGas") \
             && joItem.contains("rawSignedTxHex") && joItem.contains("accountNumber") && joItem.contains("memo")\
             && joItem.contains("complete") ))
        {
            strErrMsg =  QString("`items` missing some neccessary fields");
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        importItem.strMemo      = joItem["memo"].toString();
        importItem.bComplete    = joItem["complete"].toBool(false);
        importItem.strOrderId   = joItem["orderId"].toString();
        importItem.strAccountNumber = joItem["accountNumber"].toString();
        importItem.strCoinType  = joItem["coinType"].toString();
        importItem.strChainId   = joItem["chainId"].toString();
        importItem.strValue     = joItem["value"].toString();
        importItem.strSequence  = joItem["sequence"].toString();
        importItem.strFrom      = joItem["from"].toString();
        importItem.strTo        = joItem["to"].toString();
        importItem.strFeeAmount = joItem["feeAmount"].toString();
        importItem.strFeeGas    = joItem["feeGas"].toString();
        importItem.strSignedRawTxHex    = joItem["rawSignedTxHex"].toString();

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

int rawtx::CCosmosRawTxUtilsImpl::BroadcastRawTx(rawtx::IRawTxUtilsParamBase *_pParam) noexcept(false)
{
    //1.参数检查
    int iRet = -1;
    if(NULL == _pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("_Param is nullptr.");
        throw runtime_error(QString("internal error: _Param is nullptr.").toStdString());
    }

    Cosmos_BroadcastRawTxParam *pParam = dynamic_cast<Cosmos_BroadcastRawTxParam *>(_pParam);
    if(NULL == pParam)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("dynamic_cast<Cosmos_ExportRawTxParam *> failed.");
        throw runtime_error( QString("internal error: dynamic_cast<Cosmos_ExportRawTxParam *> failed.").toStdString() );
    }

    QString strErrMsg = "";
    if(false == pParam->ParamsCheck(&strErrMsg))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParamsCheck() error:" << strErrMsg;
        throw runtime_error( QString( "internal error:" + strErrMsg ).toStdString()  );
    }


    //2.装参数调用服务端- USDP 或 HTDF 交易广播接口,进行广播
    QString strReqUrl ;
    if(0 == pParam->strCoinType.compare("htdf", Qt::CaseInsensitive) || utils::Is_HRC20_Token(pParam->strCoinType))
        strReqUrl = STR_strUrlSendRawTransaction_HTDF  ;
    else if(0 == pParam->strCoinType.compare("het", Qt::CaseInsensitive))
        strReqUrl = STR_strUrlSendRawTransaction_HET  ;
    else //usdp
        strReqUrl = STR_strUrlSendRawTransaction_USDP;

    strReqUrl.trimmed();
    strReqUrl += pParam->strSignedRawTxHex;
    strReqUrl += "&orderId=";
    strReqUrl += pParam->strOrderId.trimmed();
    strReqUrl = strReqUrl.trimmed();

    QByteArray bytesRsp;
    iRet = _SyncPostByURL(strReqUrl, bytesRsp);
    if(IRawTxUtils::NO_ERROR  != iRet)
    {
        pParam->strErrMsg = QString("_SyncPostByURL() error:") + QString(bytesRsp);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL called ,but error :" << bytesRsp;
        return iRet;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "_SyncPostByURL() called, success return.";

    //2019-06-01 临时处理 "timed out...."不能获取到txid的问题
    if(QString(bytesRsp).contains("timed out", Qt::CaseInsensitive))
    {
        strErrMsg = QString("交易已经成功广播,但是未获取到交易ID(txid),请自行跟踪此笔交易.不要再重新创建此笔提币订单的交易.");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    //3.解析返回结果
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

    if(!(joResult.contains("sure") && joResult.contains("orderId") && joResult.contains("txid")))
    {
        pParam->strErrMsg = QString("error msg: response json miss `sure` or `orderId` or `txid`. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("error msg: response json miss `blockNumber` or `gasUsed` or `txid`. ");
        throw runtime_error(  QString("error msg: response json miss `sure` or `orderId` or `txid`. ").toStdString() );
    }

    QString strTxid = joResult.value("txid").toString();
    bool bSure = joResult.value("sure").toBool(false);
    QString strOrderId = joResult.value("orderId").toString().trimmed();

    if(0 != strOrderId.compare(pParam->strOrderId.trimmed(), Qt::CaseInsensitive))
    {
        strErrMsg = QString("error msg: orderId is dismatched. req-orderId:%1, but rsp-orderId:%2").arg(pParam->strOrderId).arg(strOrderId);
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
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " BroadcastRawTx() success. txid:" << strTxid  << ".  nonce=" << pParam->strSequence << ".";

    //4.返回txid
    pParam->strTxid = strTxid;
    pParam->bSure = bSure;

    return IRawTxUtils::NO_ERROR;
}

int rawtx::CCosmosRawTxUtilsImpl::GetAccountInfo(rawtx::CosmosAccountInfo &accountInfo) noexcept(false)
{
    QString strErrMsg;

    //1.参数检查
    if(!( (UINT_ADDR_LEN(accountInfo.strAddress.toStdString()) == accountInfo.strAddress.size())   //长度检查
        &&  (accountInfo.strAddress.startsWith("usdp1") || accountInfo.strAddress.startsWith("htdf1")
             || accountInfo.strAddress.startsWith("0x1") ) )) //前缀检查
    {
        strErrMsg = QString("%1: args error. `%2` is invalid address.").arg(__FUNCTION__).arg(accountInfo.strAddress);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if(!(0== accountInfo.strCoinType.compare(STR_USDP, Qt::CaseInsensitive)
       ||(0 == accountInfo.strCoinType.compare(STR_HTDF, Qt::CaseInsensitive)) || utils::Is_HRC20_Token( accountInfo.strCoinType )
        ||(0 == accountInfo.strCoinType.compare(STR_HET, Qt::CaseInsensitive))))
    {
        strErrMsg = QString("%1: args error. `%1` is invalid coinType.").arg(__FUNCTION__).arg(accountInfo.strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求URL
    QString  strReqUrl ;
    if(0== accountInfo.strCoinType.compare(STR_USDP, Qt::CaseInsensitive))
        strReqUrl = STR_strUrlGetAccountInfo_USDP;
    else if(0 == accountInfo.strCoinType.compare(STR_HTDF, Qt::CaseInsensitive) ||  utils::Is_HRC20_Token( accountInfo.strCoinType ))
        strReqUrl = STR_strUrlGetAccountInfo_HTDF;
    else if(0 == accountInfo.strCoinType.compare(STR_HET, Qt::CaseInsensitive))
        strReqUrl = STR_strUrlGetAccountInfo_HET;
    else{}

    strReqUrl = strReqUrl.trimmed();
    strReqUrl += accountInfo.strAddress.trimmed();
    strReqUrl = strReqUrl.trimmed();

    //3.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRet);

    //4.解析返回结果
    //{"result": {"balance": "9997877.00154987", "sequence": "19", "account_number": "0", "address": "usdp1d7xsdldypy0k8khw59p2hvr8378c3vd8p5c0qf"}, "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1557648235121}
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
        && joResult.contains("account_number") && joResult.contains("address")))
    {
        strErrMsg = QString("reponse json missing `balance` or `sequence` or `account_number` or `address` field.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    try
    {
        //QString strAddr = joResult.value("address").toString();
        accountInfo.uSequence =  boost::lexical_cast<uint64_t>(joResult.value("sequence").toString().toStdString());
        accountInfo.uAccountNumber = boost::lexical_cast<uint64_t>(joResult.value("account_number").toString().toStdString());
        accountInfo.dBalance = boost::lexical_cast<double>(joResult.value("balance").toString().toStdString()); //json中本来是浮点数字符串
    }
    catch(boost::bad_lexical_cast &e)
    {
        strErrMsg = QString::asprintf("boost::lexical_cast<>() error: %s", e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    accountInfo.bValidAccountFlag = true;
    return IRawTxUtils::NO_ERROR;
}

int rawtx::CCosmosRawTxUtilsImpl::GetBalance(const QString& strCoinType, const QString& strAddr, double& dOutBalance)
{
    //ubigintBalance= 0;
    QString strErrMsg;

    //1.参数检查
    if(!(0==strCoinType.compare("usdp", Qt::CaseInsensitive)
         || 0==strCoinType.compare("htdf", Qt::CaseInsensitive) ||  utils::Is_HRC20_Token( strCoinType )
         || 0==strCoinType.compare("het", Qt::CaseInsensitive)))
    {
        strErrMsg = QString("参数错误，币种应为usdp或htdf或het,实际为%1").arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    if(strAddr.isEmpty())
    {
        strErrMsg = QString("internal error: address is empty.");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if(false == utils::IsValidAddr(strCoinType, strAddr))
    {
        strErrMsg = QString("interal error: address '%1' is illegal %2's address.").arg(strAddr).arg(strCoinType);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //2.拼装请求URL
    QString  strReqUrl = "";
    if(0 == strCoinType.compare("usdp",Qt::CaseInsensitive))
        strReqUrl = STR_strUrlGetBalance_USDP;
    else if(0 == strCoinType.compare("htdf",Qt::CaseInsensitive) ||  utils::Is_HRC20_Token( strCoinType ))
        strReqUrl = STR_strUrlGetBalance_HTDF;
    else if(0 == strCoinType.compare("het",Qt::CaseInsensitive))
        strReqUrl = STR_strUrlGetBalance_HET;
    else {}

    strReqUrl = strReqUrl.trimmed();
    strReqUrl += strAddr;
    strReqUrl = strReqUrl.trimmed();

    //3.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRet);


    //4.解析返回结果
    //{"result": "514.82768527", "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1557989898516}
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(bytesRet, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg  = QString("json parse failed! errorCode = %1").arg( error.errorString());
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
        strErrMsg = QString("error msg: ")+QString(joRes.value("result").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QString strBalance = joRes.value("result").toString();
    if(strBalance.isEmpty())
    {
        strErrMsg = QString("error msg: result is empty. ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    dOutBalance = strBalance.toFloat();

    return IRawTxUtils::NO_ERROR;

}


int rawtx::CCosmosRawTxUtilsImpl::GetBalance_HRC20(const QString &strAddr,  std::map<QString, double> &mapBalances)
{
    QString strErrMsg = "";

    //1.参数检查
    if(strAddr.isEmpty())
    {
        strErrMsg = QString("internal error: address is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    /*if(  )
    {
        strErrMsg =  QString("args error:'%1' is invalid ETH address").arg(strAddr);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    */

    //2.拼装请求URL
    QString  strReqUrl = STR_strUrlGetBalance_HTDF;
    strReqUrl = strReqUrl.trimmed();
    strReqUrl += strAddr;
    strReqUrl = strReqUrl.trimmed();

    //3.请求数据
    QByteArray  bytesRet;
    IRawTxUtils::_SyncPostByURL(strReqUrl, bytesRet);


    //4.解析返回结果
    //{"result": { "HTDF":"0.12345678", "HRC20-AJC":"1.98765432" }, "jsonrpc": "2.0", "id": [0], "success": "true", "sysTime": 1556243232748}
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


bool  rawtx::CCosmosRawTxUtilsImpl::Bech32AddrToHexStrAddr(const QString &strBech32AddrIn, QString &strHexStrAddrOut) noexcept(false)
{
    QString strErrMsg = "";

    if(strBech32AddrIn.length() < 40 || !strBech32AddrIn.startsWith("htdf", Qt::CaseSensitive))
    {
        strErrMsg = QString(" bech32addr's length less 40 or bech32addr is not htdf address.  ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    std::string  cstrAddr = strBech32AddrIn.toStdString();

    std::pair<std::string , std::vector<uint8_t>> retpair = bech32::decode( cstrAddr );
    if(std::string("htdf") != retpair.first  ||  retpair.second.size() < 30)
    {
        strErrMsg = QString(" hrp is not `htdf` or decode data's size less than 30 ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    std::vector<uint8_t> outdata;
    if(! bech32::convertbits<5, 8, false>(outdata, retpair.second) )
    {
        strErrMsg = QString("bech32::convertbits failed  ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    std::string  cstrHexRet = utils::Bin2HexStr( outdata.data(), outdata.size());
    strHexStrAddrOut = QString::fromStdString(cstrHexRet);

    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << "Bech32AddrToHexStrAddr successed. bech32addr"
                 << strBech32AddrIn << " hexaddr: " << strHexStrAddrOut;

    return true;
}

int  rawtx::CCosmosRawTxUtilsImpl::Make_HRC20_Transfer_Data(const QString &strTo, const QString &strHexValueInWei, std::string &cstrRetHexStrData) noexcept(false)
{
    QString strAbiTransfer = STR_HRC20_transfer_sig;

    QString strTmpToHex = "";

    CCosmosRawTxUtilsImpl::Bech32AddrToHexStrAddr(strTo, strTmpToHex);
    strTmpToHex = strTmpToHex.replace(QString("0x"), QString(""), Qt::CaseInsensitive);
    QString strTmpValue = strHexValueInWei;
    strTmpValue = strTmpValue.replace(QString("0x"), QLatin1String(""), Qt::CaseInsensitive);
    QString strToPadded = utils::ZeroPadLeft(strTmpToHex , 64);
    QString strHexValueInWeiPadded = utils::ZeroPadLeft( strTmpValue, 64);

    QString strData = strAbiTransfer + strToPadded + strHexValueInWeiPadded;
    if(strData.length() != 4*2 + 32*2 + 32*2)
    {
        QString strErrMsg = QString("data's length is invalid.  ");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    cstrRetHexStrData = strData.toStdString();
    //cstrRetData = utils::HexToBin(strData.toStdString());
    return IRawTxUtils::NO_ERROR;
}


template <class _T, class _Out>
inline void ToBigEndian(_T _val, _Out& o_out)
{
	for (auto i = o_out.size(); i-- != 0; _val >>= 8)
		o_out[i] = (typename _Out::value_type)(uint8_t)_val;
}

inline std::string ToBigEndianString(ubigint _val)
{
    std::string ret(32, '\0');
    ToBigEndian(_val, ret);
    return ret;
}


int  rawtx::CCosmosRawTxUtilsImpl::ToWeiStr(double dAmount , int nDecimals, std::string &cstrRet) noexcept(false)
{
    float100 floatTmpValue =   float100(QString::asprintf("%.8f", dAmount).toStdString());
    while((nDecimals--) > 0) floatTmpValue *= 10;
    ubigint ubigintTmpValue  = floatTmpValue.convert_to<ubigint>();
    std::string strTmp = ToBigEndianString(ubigintTmpValue);
    cstrRet = utils::Bin2HexStr( (unsigned char *)strTmp.c_str(), strTmp.size() );
    return IRawTxUtils::NO_ERROR;
}

