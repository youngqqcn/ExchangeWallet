/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      获取区块监测程序的充币信息(所有平台有的币种)
*修改记录:

修改日期         修改人       修改说明                      版本号
********************************************************************/
#ifndef COINCHARGE_H
#define COINCHARGE_H

#include "comman.h"
#include "coinchargecomman.h"
#include <QObject>
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QQueue>
#include <QThread>
#include <QSemaphore>
#include <QEventLoop>

class CCoinCharge : public QThread
{
    Q_OBJECT
public:
    explicit CCoinCharge(QThread *parent = nullptr);
    CCoinCharge(const QString &strUrl, uint64_t uMsec);      //post请求
    //析构
    virtual ~CCoinCharge();

    void setTimestamp(QMap<QString, QVariant> &mapTimestamp, const QString &strCoinType);    //设置传给区块监测的时间戳

public:
    void run();

private:
    //获取区块监测程序数据的post请求
    QNetworkReply *postCoinChargeInfo(const QString &strUrl, QMap<QString, QVariant> &mapKeyValue);
    //解析请求返回的充币数据
    void requestAndParseCoinChargeInfo(const QString &strUrl,const  QMap<QString, QVariant> &mapKeyValue,  const QString &strCoinType);

public:
    static int GetInfo(QQueue<CoinChargeInfo> &outPutQueue, int n);     //传出数据

private:
    void __GetBtcChargeData();              //获取BTC充币数据
    void __GetEthChargeData();              //获取ETH充币数据
    void __GetEtcChargeData();              //获取ETC充币数据
    void __GetUsdtChargeData();             //获取USDT充币数据
    void __GetUsdpChargeData();             //获取USDP充币数据
    void __GetHtdfChargeData();             //获取HTDF充币数据
    void __GetLtcChargeData();              //获取LTC充币数据
    void __GetDashChargeData();             //获取DASH充币数据
    void __GetBsvChargeData();              //获取BSV充币数据
    void __GetBchChargeData();              //获取BCH充币数据
    void __GetHetChargeData( );              //获取HET充币数据
    void __GetXrpChargeData();              //获取XRP充币数据
    void __GetEosChargeData( );              //获取EOS充币数据
    void __GetXlmChargeData( );              //获取XLM充币数据
    void __GetTrxChargeData( );              //获取TRX充币数据
    void __GetXmrChargeData( );              //获取XMR充币数据
    //获取ETH充币数据和ers20币种的数据
    void __GetERC20ChargeData(const QString &strCoinType, const QString &strUrl, QMap<QString, QVariant> &mapBlockNum);

    //HRC20 代币
    void __Get_HRC20_ChargeData(const QString &strCoinType, const QString &strUrl, QMap<QString, QVariant> &mapBlockNum);

public:
    static bool ms_bThreadExit;

private:
//    QNetworkAccessManager   *m_pNetworkMng;         //创建http请求
    uint64_t                m_uProducerMsec;        //定时时间
    QString                 m_strProducerUrl;       //区块监测程序的URL
};

#endif // COINCHARGE_H
