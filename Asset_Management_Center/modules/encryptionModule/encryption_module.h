#ifndef ENCRYPTION_MODULE_H
#define ENCRYPTION_MODULE_H

#include "comman.h"
#include <QObject>

class CEncryptionModule : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit CEncryptionModule(QNetworkAccessManager *parent = nullptr);
    virtual ~CEncryptionModule();

    //增加异常, 如有调用, 必须处理异常, 否则会导致程序崩溃
    virtual int postEx(const QString strUrl, QByteArray &requstParameter, QByteArray &outData) noexcept(false);


private:

    //加密传输
    /*
     *
     * 请求数据加密过程:
     * 1. AES加密有效数据  encryptedData:  Bae64Encode( AES_128_CBC_Encode(请求的json数据, AES密码 ) )
     * 2. RSA签名  signaturData   Base64Encode( SHA256WithRSA_Sign (  encryptedData , 私钥A )  )
     * 3. RSA加密AES的密码 protectData  Base64Encode( RSA_Encode_By_PubKey( AES密码, 公钥B  ) )
     *
     *
     * 返回数据解密过程:
     * 原始数据{"encryptedData":"xxxx", "signaturData":"xxxxxx", "protectData":"xxxxxxxx" }
     *
     * 1. RSA验签:  SHA256WithRSA_Verify(  Base64Decode( signaturData ), 公钥B )
     * 2. RSA解密AES密码:  RSA_Decode_By_PrivKey(  Base64Decode(protectData), 私钥A )   ---获取到---> AES密码
     * 3. AES解密有效数据: AES_128_CBC_Decode(  Base64Decode(encryptedData) , AES密码)
     *
     *
     * ================= 跟Java交互的过程 ===========================
     *
     *     C++Client                           Java-Server
     *
     *   私钥A签名  公钥B加密    --------->      公钥A验签  私钥B解密
     *   公钥B验签  私钥A解密   <---------       私钥B签名  公钥A加密
     *
     *
     * C++ Client保存:   私钥A  公钥B
     * Java Server保存:  私钥B  公钥A
     */

    int __EncryptPost(const QString &strURL, const QByteArray &joPostData, QByteArray &bytesRet, bool bVerifyRspSig) noexcept(false);



};

#endif // ENCRYPTION_MODULE_H
