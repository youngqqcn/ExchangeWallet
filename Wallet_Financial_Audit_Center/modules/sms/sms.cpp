
#include "modules/sms/sms.h"

std::mutex CShortMsg::__m_Mutex;


/**
 * @brief _SendShortMsg 发送短信
 * @param strTelNo  手机号码
 * @param strContent   短信内容
 * @return  正常 返回0 ,   错误  会抛出异常, 注意异常捕获处理
 * 具体使用说明, 请参考本文件头部说明
 */
int CShortMsg::SendShortMsg( const QString &strTelNoList , const QString &strContent) noexcept(false)
{
    //加锁
    __m_Mutex.lock();

    try
    {
        QString strErrMsg;

        //检查每个手机号码的格式是否正确
        QStringList strlstTelNo =  strTelNoList.split(",");
        for(QString strTmpTelNo : strlstTelNo )
        {
            if(strTmpTelNo.length() != 11 )
            {
                strErrMsg = QString("illegal tel : %1, telNo's length is illegal! must be 11.").arg(strTmpTelNo);
                throw std::runtime_error(strErrMsg.toStdString());
            }

            if( !boost::all(  strTmpTelNo.toStdString() , boost::is_digit()))
            {
                strErrMsg = QString("error tel : %1, telNo's is illegal! must be digital number.").arg(strTmpTelNo);
                throw std::runtime_error(strErrMsg.toStdString());
            }
        }

        //短信内容不能太长
        if(strContent.isEmpty() || strContent.length() > 600)
        {
            strErrMsg = "msg content is too long! please check the short msg content!.";
            throw std::runtime_error(strErrMsg.toStdString());
        }

        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData( QString(QString(STR_MANDAO_SMS_SN) + QString(STR_MANDAO_SMS_PWD)).toLatin1());
        QString strMd5Pwd =  QString(md5.result().toHex()).toUpper();



        //如果短信是中文, 必选对中文内容进行编码, 否则接收到的短信是乱码形式
        QString strConvertedContenct = CShortMsg::EncodeURI(strContent);


        //拼装参数
        QString  strReqURL = STR_MANDAO_SMS_API_URL;
        strReqURL += QString("sn=") + STR_MANDAO_SMS_SN;
        strReqURL += QString("&pwd=") + strMd5Pwd;
        strReqURL += QString("&content=") + strConvertedContenct; //是否需要转码?
        strReqURL += QString("&mobile=") + strTelNoList;
        strReqURL += QString("&ext=&rrid=&stime=");

        QUrl url(strReqURL);
        QNetworkRequest netReq(url);
        netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded;");


        //调用接口
        QNetworkAccessManager  qnetMng;
        QNetworkReply *pReply = qnetMng.get( netReq);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http request data:" << strReqURL;

        //异步转为同步
        ASYNC_TO_SYNC_CAN_THROW(pReply);


        QByteArray byteArrData = pReply->readAll();
        if(QNetworkReply::NoError != pReply->error())
        {
            pReply->deleteLater();
            strErrMsg = QString("error code:%1, errsMsg: %2, ").arg(pReply->error()).arg(pReply->errorString()) +  QString(byteArrData);
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "network error:" << strErrMsg;
            throw std::runtime_error(strErrMsg.toStdString());
        }


        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "response data:" << byteArrData;
        pReply->deleteLater();

        //解析返回值
        QString  strRetCode = "";
        QDomDocument  dom;
        dom.setContent (byteArrData);

        QDomNode node = dom.firstChild();
        for ( ;!node.isNull(); node = node.nextSibling())
        {
            //获取错误码, 如果大于等于 则是正确的
            //<?xml version="1.0" encoding="utf-8"?>\r\n<string xmlns="http://tempuri.org/">-21</string>
            if ( node.isElement() &&   0 == node.toElement().tagName().compare( "string" , Qt::CaseInsensitive))
            {
                 strRetCode = node.toElement().text().trimmed();
                 break;
            }
         }

        //返回值, 如果失败, 返回错误码是负数;   如果发送成功则  会返回大于0的数
        if(  strRetCode.length() > 0 && strRetCode.startsWith("-"))
        {
            //获取错误详细信息
            GET_ERROR_DETAIL_BY_ERRCODE(strRetCode, strErrMsg);

            strErrMsg = strRetCode + QString(" , ") + strErrMsg;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw std::runtime_error(strErrMsg.toStdString());
        }
        else
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  "sms api response:" << strRetCode ;
        }

    }
    catch(std::runtime_error &e)
    {
        __m_Mutex.unlock();
        throw std::runtime_error(e.what());
    }
    catch(std::exception &e)
    {
        __m_Mutex.unlock();
        throw std::exception(e.what());
    }
    catch(...)
    {
        __m_Mutex.unlock();
        throw std::runtime_error("未知异常");
    }


    //释放
    __m_Mutex.unlock();
    return 0;
}


 /** 对中文进行URL编码 使用 GBK(GB2312)
 * @brief EncodeURI
 * @param str
 * @return  编码后的中文内容
 */
QByteArray CShortMsg::EncodeURI(QString str) noexcept(true)
{
    QByteArray array;
    QTextCodec *codec=QTextCodec::codecForName("GBK");
    QByteArray tmpArray;
    tmpArray = codec->fromUnicode(str);
    for(int i=0,size = tmpArray.length();i<size;i++)
    {
        char ch = tmpArray.at(i);
        if((ch>='0'&&ch<='9')||(ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z'))
        {
            array.append(ch);
        }
        else
        {
            uchar low = ch & 0xff;
            char c[3];
            sprintf(c,"%02X",low);
            array.append("%").append(c);
        }
    }
    return array;
}
