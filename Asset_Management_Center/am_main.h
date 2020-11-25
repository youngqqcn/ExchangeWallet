#ifndef ASSET_MNG_MAIN_H
#define ASSET_MNG_MAIN_H

/*********************************************************************************
 * 作者: yqq
 * 时间: 2019-03-27
 * 功能: 资产管理 所有业务逻辑处理类的基类
 * 说明: 所有子功能模块管理都应继承此类
 *
 *********************************************************************************/


#include "comman.h"
#include <QObject>
class CAMMain : public QObject
{
    Q_OBJECT
public:
    explicit CAMMain(QObject *parent = nullptr);
    explicit CAMMain(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CAMMain();


public:
    enum{
        NO_ERROR = 0,
        ARGS_ERR = 10002,
        DB_ERR = 10001,
        PARAM_ERR = 1003,
        JSONPARSE_ERR = 1004,
        STATUS_ERR = 1005,
        INVALID_DATA_ERR = 1006,
        FILE_ERROR = 1007,
        FILE_OPEN_ERR = 1008,

        //manual withdraw
        NOT_SUPPORT_COIN_TYPE = 1500,
        COIN_TYPE_ERROR = 1501,
        COIN_TYPE_NOT_THE_SAME = 1502,

        //eth
        GET_NONCE_FAILED_ERR = 2000,
        INVALID_NONCE_ERR = 2001,
        INVALID_ETH_ADDR  = 2002,
        INVALID_ETH_PRIV_KEY = 2003,
        BROADCAST_FAILED_ERR = 2004,

        AMOUNT_NOT_ENOUGH = 10009, //冷地址金额不足
        TRANSACTION_ALREADY_DONE = 10008,  //交易已存在
        TRANSACTION_STATE_ERROR = 10009,   //交易状态错误

        IMPORT_DATA_ERROR = 10010,
    };

public:

    //设置数据库文件路径
    void SetDBFilePath(const QString &strDBFilePath);

    //设置数据库连接名, 注意: 不能重复,  建议 以子类对象的名称命名
    void SetDBConnectName(const QString &strConnectName);

    //初始化函数,  子类必须重写此函数, 并在子类的Init()中调用父类的CAMMain::Init()  , 并且必须调用父类的构造函数
    virtual int Init() = 0;

public:
    int SyncGetData(const QString &strInUrl, QByteArray &byteArray) noexcept(false);

    int SyncPostByJson(const QString &strUrl,  const QJsonObject &joParams, QByteArray &byteArrayRet, QString strContentType="application/json");

protected:

    QSqlDatabase  _m_db;//数据库
    QString _m_strDBFilePath; //数据库文件路径
    QString  _m_strConnectName;//数据库连接名
    QNetworkAccessManager* _m_pNetManager;
    QNetworkReply *_m_pReply;

signals:

public slots:
};

#endif // ASSET_MNG_MAIN_H
