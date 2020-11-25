#include "encryption_module.h"
#include "coinchargecomman.h"

#include "crypto_utils.h"
#include <boost/smart_ptr.hpp> //为了使用  boost::shared_array
#include <QDebug>

CEncryptionModule::CEncryptionModule(QNetworkAccessManager *parent)
    :QNetworkAccessManager(parent)
{ 
}

CEncryptionModule::~CEncryptionModule()
{
}



int CEncryptionModule::postEx(const QString strUrl, QByteArray &bytesRequest, QByteArray &outData) noexcept(false)
{
    QString strErrMsg;

    bool bVerifyRspSig = true; //是否需要对返回的数据进行RSA签名验证


    if (  (0 == strUrl.compare(STR_COIN_CHARGE_SUCCESS_URL, Qt::CaseSensitive)) //充币完成接口
          || (0 == strUrl.compare(g_qstr_JAVA_upWithdrawOrderStatus, Qt::CaseSensitive))) //提币完成接口
    {
        bVerifyRspSig  = false; //不需要对返回数据进行验签
    }
    else if (0 == strUrl.compare(STR_GET_ORDER_ID_URL, Qt::CaseSensitive) //充币开始接口
             || 0 == strUrl.compare(g_qstr_JAVA_selectWithdrawOrderInfo, Qt::CaseSensitive)) //获取提币订单接口
    {
        bVerifyRspSig  = true; // 需要对返回数据进行验签
    }
    else
    {
        strErrMsg =  "internal error: this API isn't need Encryptions, please switch to normal functions.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http request data:"<< bytesRequest;


    __EncryptPost(strUrl, bytesRequest, outData, bVerifyRspSig);

    return NO_ERROR;
}


#define  STR_A_PRIV_KEY_FILE_PATH G_STR_A_PRIV_PEM_FILE_PATH
#define  STR_B_PUB_KEY_FILE_PATH G_STR_B_PUB_PEM_FILE_PATH
#define  N_AES_KEY_BITS   (128 / 8)
#define  STR_AES_IV "qqqqqqqqqqqqqqqq"



int CEncryptionModule::__EncryptPost(const QString &strURL, const QByteArray &bytesPostData, QByteArray &bytesRet,  bool bVerifyRspSig) noexcept(false)
{
    QString strErrMsg;
    int iRet = -1;

    QString strPostURL = strURL;
    std::string cstrPostData = bytesPostData.toStdString();

    //=========================================  1.AES 加密 =========================================
    std::string strB64JsonDataEncodeByAES_Post;
    unsigned char uszAESKey[N_AES_KEY_BITS] = {0};
    std::string cstrAESIV = STR_AES_IV;
    if(true)
    {
        //随机生成 16字节的key
        srand(time(NULL));
        memset(uszAESKey, 0, sizeof(uszAESKey));
        for(int i = 0; i < sizeof(uszAESKey) / sizeof(unsigned char); i++)
        {
            //unsigned char uTmpRand = 'a' + (unsigned char)(rand() % 26); // a-z 小写字母
            //unsigned char uTmpRand = (unsigned char)(rand() % 0xff); //0x00 ~ 0xFF
            unsigned char uTmpRand = '!' + (unsigned char)(rand() % ('~' - '!')); //可见的ascii码
            memcpy(uszAESKey + (i * sizeof(unsigned char)), &uTmpRand, sizeof(uTmpRand) );
        }

        unsigned int nEncOutLen = 0;
        int nTmpAllocBufLen = cstrPostData.length() + 1024;
        boost::shared_array<unsigned char>  spAesEncData(new unsigned char [ nTmpAllocBufLen ]); //托管裸指针,不需要手动释放
        memset(spAesEncData.get(), 0, nTmpAllocBufLen);

        std::string cstrTmpAESIV = cstrAESIV;
        iRet = CryptoUtils::AES_128_CBC_Encrypt((unsigned char *)cstrPostData.c_str(), cstrPostData.length(), uszAESKey,
                                                (unsigned char *)cstrTmpAESIV.c_str(), spAesEncData.get(), &nEncOutLen );
        if(0 != iRet)
        {
            strErrMsg =  QString("EncryptPost error, AES_128_CBC_Encrypt return error: %1 ").arg(iRet);
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        strB64JsonDataEncodeByAES_Post = CryptoUtils::Base64Encode((char *)spAesEncData.get(), nEncOutLen, false);
        std::cout << strB64JsonDataEncodeByAES_Post << std::endl;
    }
    //==============================================================================================


    //========================================= 2.RSA签名 ===========================================
    std::string strB64SigOfData_Post;
    if(true)
    {
        unsigned char * uszInData = (unsigned char *)strB64JsonDataEncodeByAES_Post.c_str();
        unsigned int uRsaInDataLen = strB64JsonDataEncodeByAES_Post.length();

        unsigned char uszOutData[1024] = { 0 }; //  2048 bit 的私钥 --> 2048 bit 签名,   注意数组越界
        memset(uszOutData, 0, sizeof(uszOutData));


        unsigned int uOutLen = 0;
        std::string cstrPrivFilePwd = g_qstr_PrivFilePwd.toStdString();
        //iRet = CryptoUtils::SHA256WithRSA_Sign((char *)STR_A_PRIV_KEY_FILE_PATH, uszInData, uRsaInDataLen, uszOutData, &uOutLen );
        iRet = CryptoUtils::SHA256WithRSA_Sign_Ex((char *)STR_A_PRIV_KEY_FILE_PATH, uszInData, uRsaInDataLen,
                                                  uszOutData, &uOutLen, (unsigned char *)cstrPrivFilePwd.c_str());
        if (0 != iRet)
        {
            strErrMsg =  QString("EncryptPost error, SHA256WithRSA_Sign return error: %1 ").arg(iRet);
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        /*if (0 != CryptoUtils::SHA256WithRSA_Verify((char *)STR_PUB_KEY_FILE_PATH, uszInData, uRsaInDataLen, uszOutData, uOutLen ))
        {
            return -1;
        }*/

        strB64SigOfData_Post = CryptoUtils::Base64Encode((const char *)uszOutData, uOutLen, false);
        std::cout << "base64编码后: " << strB64SigOfData_Post << std::endl;
        //printf("=============================================\n\n");
    }

    //==============================================================================================


    //========================================= 3.RSA加密AES的密码 ==================================
    std::string strB64AESPwdEncodedByRSA_Post;
    if(true)
    {
        //unsigned char uszAESEncodedWithRsa[16 + 32] = { 0 }; //数组越界  2019-08-07
        unsigned char uszAESEncodedWithRsa[1024] = { 0 }; //2048 bit 的私钥 --> 2048 bit 加密数据,   注意数组越界
        memset(uszAESEncodedWithRsa, 0, sizeof(uszAESEncodedWithRsa));
        int nRsaEncodeOutLen = 0;


        iRet = CryptoUtils::RSA_EncrptByPubkey((char *)STR_B_PUB_KEY_FILE_PATH, uszAESKey, sizeof(uszAESKey), uszAESEncodedWithRsa, &nRsaEncodeOutLen);
        if (0 !=  iRet)
        {
            strErrMsg =  QString("EncryptPost error, RSA_EncrptByPubkey return error: %1 ").arg(iRet);
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        strB64AESPwdEncodedByRSA_Post = CryptoUtils::Base64Encode((char *)uszAESEncodedWithRsa, nRsaEncodeOutLen, false);
        std::cout << "base64编码后: " << strB64AESPwdEncodedByRSA_Post << std::endl;

        /*printf("my_rsa_prikey_encrypt ok,rsalen is [%d]\n", rsalen);
        memset(out, 0, sizeof(out));
        ret = CryptoUtils::RSA_DecrptByPrivkey((char *)STR_PRIV_KEY_FILE_PATH, uszAESEncodedWithRsa, rsalen, out, &outlen);
        if (ret < 0)
        {
            printf("my_rsa_prikey_decrypt err [%d]\n", ret);
            return 1 ;
        }*/
    }
    //==============================================================================================




    //========================================= 4.http请求 =========================================
    QByteArray bytesHttpRawRsp;
    if(true)
    {
        QJsonObject  joPost ;
        joPost.insert("encryptedData", QString::fromStdString(strB64JsonDataEncodeByAES_Post));
        joPost.insert("signaturData", QString::fromStdString(strB64SigOfData_Post));
        joPost.insert("protectData", QString::fromStdString(strB64AESPwdEncodedByRSA_Post));

        QByteArray bytesTmpPostData = QJsonDocument(joPost).toJson(QJsonDocument::Compact);

        //设置请求头
        QNetworkRequest reqHeaders;
        reqHeaders.setUrl(QUrl(strPostURL));
        reqHeaders.setRawHeader("Cache-Control","no-cache");
        reqHeaders.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("post Url:") << strPostURL << "  jsondata:" << bytesTmpPostData;

        QNetworkReply *pReply = QNetworkAccessManager::post(reqHeaders , bytesTmpPostData);
        ASYNC_TO_SYNC_CAN_THROW(pReply);

        bytesHttpRawRsp = pReply->readAll();     //获取返回的所有数据
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "java encrypted response:" << bytesHttpRawRsp;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "java encrypted response string format:" << QString(bytesHttpRawRsp);
        //pReply->close();
        pReply->deleteLater();
    }
    //=============================================================================================




    //========================================= 5.解析返回的数据 =====================================
    bytesRet = bytesHttpRawRsp; //防止后面处理特殊情况(不需要验签的), 忘记返回数据

    std::string cstrB64JsonDataEncodedByAES;
    std::string cstrJsonDataEncodedByAES;
    std::string cstrSigOf_B64JsonDataEncodedByAES;
    std::string cstrAESPwdEncodedWithRSAPubKey;

    if(true)
    {

        QJsonParseError error;
        QJsonDocument jdocResponse = QJsonDocument::fromJson(bytesHttpRawRsp, &error );
        if(QJsonParseError::NoError != error.error)
        {
            strErrMsg =  QString("EncryptPost error, http response json parse error: %1 ").arg(error.errorString());
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject joRoot = jdocResponse.object();
        if(! (joRoot.contains("data") && joRoot.contains("errMsg") && joRoot.contains("errCode")))
        {
            strErrMsg =  QString("EncryptPost error, Java response missing some neccessary fields.");
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //第三方上币,特有错误码
        if(10832 == joRoot.value("errCode").toInt(-1) )
        {
            return NO_ERROR;
        }


        if(0 != joRoot.value("errCode").toInt(-1) )
        {
            strErrMsg =  QString("EncryptPost error, Java server return error %1, errMsg:%2.")
                    .arg(joRoot.value("errCode").toInt()).arg(joRoot.value("errMsg").toString());
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        if(!bVerifyRspSig)
        {
            return NO_ERROR;
        }


        QJsonObject joData  =  jdocResponse.object().value("data").toObject();
        if(!(joData.contains("encryptedData") && joData.contains("signaturData") && joData.contains("protectData")))
        {
            strErrMsg =  QString("EncryptPost error, Java server response `data`  missing some neccessary fields.");
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        QString strB64JsonDataEncodedByAES  = joData.value("encryptedData").toString(); //aes加密的json数据
        QString strB64SigOf_B64JsonDataEncodedByAES  = joData.value("signaturData").toString(); // rsa签名
        QString strB64AESPwdEncodedWithRSAPubKey = joData.value("protectData").toString(); // rsa 加密的 aes 密码


        cstrB64JsonDataEncodedByAES = strB64JsonDataEncodedByAES.toStdString(); //未解码
        cstrJsonDataEncodedByAES = CryptoUtils::Base64Decode( cstrB64JsonDataEncodedByAES.c_str(), cstrB64JsonDataEncodedByAES.length(), false );
        cstrSigOf_B64JsonDataEncodedByAES  = CryptoUtils::Base64Decode(  strB64SigOf_B64JsonDataEncodedByAES.toStdString().c_str(),
                                                                         strB64SigOf_B64JsonDataEncodedByAES.toStdString().length(), false);
        cstrAESPwdEncodedWithRSAPubKey =  CryptoUtils::Base64Decode(  strB64AESPwdEncodedWithRSAPubKey.toStdString().c_str(),
                                                                      strB64AESPwdEncodedWithRSAPubKey.toStdString().length(), false);
    }
    //================================================================================================



    //=======================================  6.RSA验签 =============================================
    if(true)
    {
        iRet = CryptoUtils::SHA256WithRSA_Verify((char *)STR_B_PUB_KEY_FILE_PATH,
                                                     (unsigned char *)cstrB64JsonDataEncodedByAES.c_str(),
                                                     cstrB64JsonDataEncodedByAES.length(),
                                                     (unsigned char *)cstrSigOf_B64JsonDataEncodedByAES.c_str(),
                                                     cstrSigOf_B64JsonDataEncodedByAES.length() );
        if (iRet != 0)
        {
            strErrMsg =  QString("EncryptPost error, SHA256WithRSA_Verify error: %1 ").arg(iRet);
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

    }
    //================================================================================================




    //======================================= 7.RSA私钥解密获取AES密码 =================================
    unsigned char uszRspAESKeyDecoded[1024] = {0};
    memset(uszRspAESKeyDecoded, 0, sizeof(uszRspAESKeyDecoded));

    if(true)
    {
        int nRspAESKeyLenDecoded = 0;
        std::string strPrivFilePwd = g_qstr_PrivFilePwd.toStdString();
        iRet = CryptoUtils::RSA_DecrptByPrivkeyEx((char *)STR_A_PRIV_KEY_FILE_PATH,
                                                (unsigned char *)cstrAESPwdEncodedWithRSAPubKey.c_str(),
                                                cstrAESPwdEncodedWithRSAPubKey.length(),
                                                uszRspAESKeyDecoded,
                                                &nRspAESKeyLenDecoded,
                                                (unsigned char *)strPrivFilePwd.c_str());
        if (iRet < 0)
        {
            strErrMsg =  QString("EncryptPost error, RSA_DecrptByPrivkey error: %1 ").arg(iRet);
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(nRspAESKeyLenDecoded != 16)
        {
            strErrMsg =  QString("EncryptPost error, nRspAESKeyLenDecoded != 16  ");
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }
    //================================================================================================



    //======================================= 8.AES解密获取有效的json数据 ===============================
    QByteArray bytesTmpDecoded;
    if(true)
    {
        int nTmpAllocBufLen = cstrJsonDataEncodedByAES.length() + 1024;
        boost::shared_array <unsigned char> spuszRspJsonDataDecoded( new unsigned char[ nTmpAllocBufLen ]  ); //托管裸指针,  //不需要手动释放
        memset(spuszRspJsonDataDecoded.get(), 0, nTmpAllocBufLen);
        unsigned int nRspJsonDataLenDecoded = 0;

        //加密数据体base64解码
        std::string cstrTmpAESIV = cstrAESIV;
        iRet = CryptoUtils::AES_128_CBC_Decrypt((unsigned char *)cstrJsonDataEncodedByAES.c_str(),
                                                cstrJsonDataEncodedByAES.length(), uszRspAESKeyDecoded,
                                                (unsigned char *)cstrTmpAESIV.c_str(),
                                                spuszRspJsonDataDecoded.get(),  &nRspJsonDataLenDecoded);
        if( 0 != iRet)
        {
            strErrMsg =  QString("EncryptPost error, AES_128_CBC_Decrypt error : %1").arg(iRet);
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        //不能用QByteArray::fromRawData,  此函数没有拷贝,而是指向原始数据!!
        //bytesTmpDecoded = QByteArray::fromRawData((char *)spuszRspJsonDataDecoded.get(), nRspJsonDataLenDecoded);

        bytesTmpDecoded.clear();
        bytesTmpDecoded.append((char *)spuszRspJsonDataDecoded.get(), nRspJsonDataLenDecoded);


        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("===============aes解密====================");
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << bytesTmpDecoded;
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("===============aes解密====================");
    }
    //==================================================================================================



    //====================================== 9.组装返回的数据 , 适配原来的数据格式 =========================
    QJsonObject joRetMake;
    if(true)
    {
        QJsonParseError error;
        joRetMake = QJsonDocument::fromJson(bytesHttpRawRsp, &error ).object();
        if(error.NoError != error.error )
        {
            strErrMsg =  QString("EncryptPost error, Java response is invalid json. parse erorr:%1 ").arg(error.errorString());
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonDocument joTmpDecoded =  QJsonDocument::fromJson(bytesTmpDecoded, &error);
        if(QJsonParseError::NoError != error.error )
        {
            strErrMsg =  QString("EncryptPost error, The the decoded data of `data` is invalid json. parse erorr:%1 ").arg(error.errorString());
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if( QString(bytesTmpDecoded).startsWith(QChar('['), Qt::CaseInsensitive))
        {
            joRetMake.insert("data", joTmpDecoded.array() );
        }
        else if( QString(bytesTmpDecoded).startsWith(QChar('{'), Qt::CaseInsensitive))
        {
            joRetMake.insert("data", joTmpDecoded.object() );
        }
        else
        {
            strErrMsg =  QString("EncryptPost error, The the decoded data of `data` isn't array or object.").arg(error.errorString());
            qCritical()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }

    //=================================================================================================



    //解密数据返回
    //格式1:  {"errCode": 0,"data": [],"errMsg": "操作成功","timestamps": 1552966250839}
    //格式2:  {"errCode": 0,"data": {},"errMsg": "操作成功","timestamps": 1552966250839}
    bytesRet = QJsonDocument(joRetMake).toJson();

    return NO_ERROR;
}



























