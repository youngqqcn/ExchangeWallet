/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      从Java获取充币订单编号和上传充币完成信息
*修改记录:

修改日期         修改人       修改说明                      版本号
********************************************************************/
#ifndef TXSTATUSMONITOR_H
#define TXSTATUSMONITOR_H

#include "comman.h"
#include "coinchargecomman.h"
#include "encryption_module.h"
#include <QObject>
//#include <QtWebSockets/QWebSocket>
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QQueue>
#include <QThread>
#include <QByteArray>
#include <QEventLoop>

class CTxStatusMonitor : public QObject
{
    Q_OBJECT
public:
    explicit CTxStatusMonitor(const QString &strUrl);
    virtual ~CTxStatusMonitor();

public:
    int getOrderId(orderId &outOrderId, const QQueue<CoinChargeInfo> &queueData);                               //开始充币数据传出接口
    int sendCoinChargeFinish(orderId &orderIdTmp, const QQueue<CoinChargeFinish> &queueChargeFinish);           //充币完成发送数据接口

private:
    int parseData(QByteArray &byteArrayData, orderId &outOrderId);          //解析数据
    int requestData(const QString strUrl, QMap<QString, QVariant> mapObj, QByteArray &byteArrayReply);          //请求数据

public:
//    QNetworkAccessManager       *m_pNetworkManager;
    QString                     m_strUrl;           //接收传入的请求url

private:
    //CEncryptionModule           *__m_pEncryptionModule;
};

#endif // TXSTATUSMONITOR_H
