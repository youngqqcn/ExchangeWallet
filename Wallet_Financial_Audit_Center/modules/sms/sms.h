/*
说明: 短信模块, 发送短信
时间: 2019-11-26
作者: yqq

几点说明:
 1.调用方的ip必须先提供给短信服务商的客服, 让客服添加调用方的ip地址, 否则会不能调用接口
 2.相同的短信内容不能多次重复发送, 否则会被封号
 3.10分钟内不能发送超过10条
 4.不能并发调用接口, 否则会出问题
 5.发送的短信内容不宜过长
 6.如果短信内容是中文, 注意编码格式必须是gbk(gb2312), 否则收到的短信会乱码
 7.短信内容的前缀必须是 【Hetbi】  开头, 否则用户不能接收到短信


 调用例子:
    try
    {
        //QString strTelList = "18565659593"; //手机号码, 如果有多个手机号, 则用英文逗号分隔
        QString strTelList = "18565659593,18727615790,15727780717";
        QString strContent = tr("【Hetbi】尊敬的管理员，当前余额为1.9235BTC，请及时充币以保证自动提币通道的畅通，谢谢您的配合！ ");
        int iRet = CShortMsg::SendShortMsg(strTelList , strContent);
        if(0 == iRet)
        {
            QMessageBox::information(this, "提示", "发送成功!");
        }
    }
    catch(std::exception &e)
    {
        //异常处理
        QMessageBox::warning(this, "错误", QString(e.what()));
    }
    catch(...)
    {
        //异常处理
        QMessageBox::warning(this, "错误",  "未知错误" );
    }

*/


#ifndef __SMS_H__
#define __SMS_H__
#pragma execution_character_set("utf-8")


#include <QObject>
#include <QDebug>
#include <QCryptographicHash>
#include <QDomDocument>    //需要在 pro 文件中添加   QT += xml
#include "comman.h"
#include <boost/algorithm/string.hpp>
#include <mutex>
//#include <boost/noncopyable.hpp>


#define  STR_MANDAO_SMS_API_URL   "http://sdk2.zucp.net:8060/webservice.asmx/mt?"
#define  STR_MANDAO_SMS_SN      "SDK-SSW-010-00084"
#define  STR_MANDAO_SMS_PWD     "f8de8860-9b"


#define  GET_ERROR_DETAIL_BY_ERRCODE(nErrCode, strErrMsg)\
std::map<QString,  QString>  mapErr;\
mapErr.insert(std::make_pair(QString("1"), QString("没有需要取得的数据")));\
mapErr.insert(std::make_pair(QString("-1"), QString("重复注册")));\
mapErr.insert(std::make_pair(QString("-2"), QString("帐号/密码不正确")));\
mapErr.insert(std::make_pair(QString("-4"), QString("余额不足支持本次发送")));\
mapErr.insert(std::make_pair(QString("-5"), QString("数据格式错误")));\
mapErr.insert(std::make_pair(QString("-6"), QString("参数有误")));\
mapErr.insert(std::make_pair(QString("-7"), QString("权限受限")));\
mapErr.insert(std::make_pair(QString("-8"), QString("流量控制错误")));\
mapErr.insert(std::make_pair(QString("-9"), QString("扩展码权限错误")));\
mapErr.insert(std::make_pair(QString("-10"), QString("内容长度长")));\
mapErr.insert(std::make_pair(QString("-12"), QString("序列号状态错误")));\
mapErr.insert(std::make_pair(QString("-13"), QString("没有提交增值内容")));\
mapErr.insert(std::make_pair(QString("-14"), QString("服务器写文件失败")));\
mapErr.insert(std::make_pair(QString("-15"), QString("文件内容base64编码错误")));\
mapErr.insert(std::make_pair(QString("-16"), QString("返回报告库参数错误")));\
mapErr.insert(std::make_pair(QString("-17"), QString("没有权限")));\
mapErr.insert(std::make_pair(QString("-18"), QString("上次提交没有等待返回不能继续提交")));\
mapErr.insert(std::make_pair(QString("-19"), QString("禁止同时使用多个接口地址")));\
mapErr.insert(std::make_pair(QString("-20"), QString("相同手机号，相同内容重复提交")));\
mapErr.insert(std::make_pair(QString("-21"), QString("21:Ip鉴权失败,IP不是所绑定的IP")));\
mapErr.insert(std::make_pair(QString("-22"), QString("22:Ip鉴权失败,IP不是所绑定的IP")));\
auto itErr = mapErr.find(nErrCode);\
if(mapErr.end() != itErr){\
    strErrMsg = itErr->second;\
}else{\
    strErrMsg = "";\
}


class CShortMsg : public QObject
{
    Q_OBJECT

private:
    static std::mutex  __m_Mutex;

public:
     /** 对中文进行URL编码 使用 GBK(GB2312)
     * @brief EncodeURI
     * @param str
     * @return  编码后的中文内容
     */
    static QByteArray EncodeURI(QString str) noexcept(true);

     /**
     * @brief SendShortMsg 发送短信
     * @param strTelNo  手机号码
     * @param strContent   短信内容
     * @return  正常 返回0 ,   错误  会抛出异常, 注意异常捕获处理
     * 具体使用说明, 请参考本文件头部说明
     */
    static int SendShortMsg( const QString &strTelNoList , const QString &strContent) noexcept(false);

};




#endif //__SMS_H__


