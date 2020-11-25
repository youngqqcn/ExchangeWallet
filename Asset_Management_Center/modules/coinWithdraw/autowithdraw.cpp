#include "autowithdraw.h"
#include "eth/eth_raw_tx_utils.h"  //以太坊裸交易工具类  2019-04-28 yqq
#include "cosmos/cosmos_raw_tx_utils.h" //cosmos裸交易工具类, 2019-05-12 yqq
#include "utils.h"
#include "sms.h" //短信模块
#include "xrp/xrp_raw_tx_utils.h" // XRP   2019-12-12  yqq
#include "eos/eos_raw_tx_utils.h"  //EOS  2019-12-31  yqq
#include "xlm/xlm_raw_tx_utils.h" //XLM 2020-02-12 yqq
#include "trx/trx_raw_tx_utils.h"  //TRX 2020-03-05  yqq
#include "xmr/xmr_raw_tx_utils.h"  //XMR 2020-04-07 yqq
#include <random>



qint64 CAutoWithdraw::ms_uTimestamp  = 0; //请求订单数据的起始时间戳
bool CAutoWithdraw::ms_bExitThread = false;


CAutoWithdraw::CAutoWithdraw()
{
    __m_nWalletServerHealthStateMonitorCounter = 0;
}

CAutoWithdraw::~CAutoWithdraw()
{
}

int CAutoWithdraw::Init() noexcept(false)
{
    QString  strErrMsg;

    //2.初始化数据库
    __StartDBService(); //连接数据库

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CAutoWithdraw::Init() 初始化数据库成功";

    //获取币种表中所有币种配置信息每次启动程序只获取一次
    __GetAllCoinConfigFromDB();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CAutoWithdraw::Init() 初始化成功";
    return CAutoWithdraw::NO_ERROR;
}


//获取所有币种,及自动提币地址私钥
int CAutoWithdraw::__GetAllCoinConfigFromDB()  noexcept(false)
{
    QString strErrMsg;

    //清空
    __m_qmapAllCoinConfig.clear();


    //获取某个币种的 自动提币私钥
    QSqlQuery  sql(__m_db);
    QString strSql = QString("select * from tb_coin_type;");
    sql.prepare(strSql);
    const int iTry = 5;
    for(int i = 0; i < iTry; i++)
    {
        if(sql.exec()) break;
        if(iTry  == i + 1) //最后一次尝试
        {
            strErrMsg = "sql.exec() error: " + sql.lastError().text();
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }
        msleep(200);
    }


    if(!sql.next())
    {
        strErrMsg = "sql result is empty";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    //因为 前面已经.next(), 所以用 do...while
    do{

        //TODO: 后期在此获取  金额限制, 手续费, 等等   2019-05-19  by yqq
        QString strCoinType = sql.value("coin_nick").toString().toLower(); //统一小写
        QString strSrcAddr = sql.value("withdraw_src_addr").toString();
        QString strMinWithdrawAmount = sql.value("min_withdraw_count").toString(); //自动提币和人工审核提币的分界
        QString strDayWithdrawAmount = sql.value("day_withdraw_count").toString();//单日最大提币数量

        //2020-04-24 增加  "半自动" 提币流程
        QString  strSemiAutoWithdrawAmount = sql.value("semi_auto_windraw_amount").toString();
        double dSemiAutoWithdrawAmount = 0.0;

        double dMaxAutoWithdrawAmount = 0.0; //自动提币和人工审核提币的分界, 即自动提币的最大数量
        double dDayWithdrawAmount = 0.0;
        try
        {
            dMaxAutoWithdrawAmount = boost::lexical_cast<double>(strMinWithdrawAmount.toStdString());
            dDayWithdrawAmount = boost::lexical_cast<double>(strDayWithdrawAmount.toStdString());

            if(!strSemiAutoWithdrawAmount.isEmpty())
            {
                dSemiAutoWithdrawAmount = boost::lexical_cast<double>(strSemiAutoWithdrawAmount.toStdString());
            }
        }
        catch(boost::bad_lexical_cast &e)
        {
            strErrMsg = QString("invalid `min_withdraw_count` or `day_withdraw_count` or `semi_auto_windraw_amount` in DB. errMsg: %1").arg( QString(e.what()));
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }


        //TODO: 私钥暂时使用base64编解码, 后期使用AES编码   2019-05-19  by yqq
        QString strPrivKey = QByteArray::fromBase64( sql.value("encrypted_privkey").toString().toUtf8() , \
                                                     QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);

        QMap<QString, QVariant> mapTmp;
        mapTmp.insert(QString("srcAddr"), strSrcAddr);
        mapTmp.insert(QString("srcPrivKey"), strPrivKey);
        mapTmp.insert(QString("dMaxAutoWithdawAmount"), dMaxAutoWithdrawAmount);
        mapTmp.insert(QString("dDayMaxAmount"), dDayWithdrawAmount);

        //增加 "半自动"额度  2020-04-24
        mapTmp.insert( QString("dSemiAutoWithdrawAmount"), dSemiAutoWithdrawAmount );


        __m_qmapAllCoinConfig.insert(strCoinType, mapTmp);


        //警告: 禁止将私钥相关的信息(包括加密后的私钥) 输出到日志!!!   2019-05-20 by yqq
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " query result cointype: " << strCoinType;
    }while(sql.next());


    return CAutoWithdraw::NO_ERROR;
}


void CAutoWithdraw::run()
{
    QString strErrMsg;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " CAutoWithdraw::run() 线程启动";
    const unsigned long long  uSleepSec = (g_bBlockChainMainnet) ? (60) : (60); //加快测试环境的频率, 使问题尽量在测试环境暴露出来
    while(!ms_bExitThread)
    {
        try
        {
            //处理提币订单
            HandleOrder();
        }
        catch(std::exception &e)
        {
            //处理异常
            strErrMsg = QString(e.what());
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"error:" << strErrMsg;
        }
        catch(...)
        {
            //处理异常
            qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"error:未知错误";
        }


        /*
            不能在这里调用 QThread::exec()和 quit() 来处理  deleteLater() 的事件,  因为 QThread::quit() 之后
            会导致  http请求的时候的 创建的 事件循环直接退出(不会等待)
            立即处理  deleteLater()  提交的事件, 释放内存
            参考 &QThread::exit() 官方文档

            No QEventLoops will be started anymore in this thread until QThread::exec() has been called again.
            If the eventloop in QThread::exec() is not running then the next call to
            QThread::exec() will also return immediately.

            if(true)
            {
               QTimer t;
               t.setSingleShot(true);
               connect(&t, &QTimer::timeout, this, &QThread::quit, Qt::DirectConnection);
               t.start(10);
               QThread::exec();
            }
        */

        for(int i = 0; i < uSleepSec; i++)
        {
            if(ms_bExitThread) break;
            sleep(1);
        }

    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CAutoWithdraw::run() 线程正常退出";
}


int CAutoWithdraw::ParseBytesToOrderData(const QByteArray &byteArrOrderData, vector<OrderData> &vctOrderData) noexcept(false)
{
    QString strErrMsg = "";
    QJsonParseError error;
    QJsonDocument jsonRes = QJsonDocument::fromJson( byteArrOrderData, &error);
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = error.errorString();
        qCritical() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"Json parse error:" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(!jsonRes.isObject())
    {
        strErrMsg =  "reponse json error";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    QJsonObject joRes = jsonRes.object();
    int iErrCode = joRes.value("errCode").toInt();
    if(0 != iErrCode)
    {
        strErrMsg = QString("api request failed: %1, %2").arg( iErrCode).arg( joRes.value("errMsg").toString() );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    QJsonArray jaData = joRes.value("data").toArray();

    if(jaData.isEmpty())
    {
        strErrMsg = "data is empty";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        //throw runtime_error( strErrMsg.toStdString() );
        return CAutoWithdraw::NO_ERROR;
    }

    //解析从Java后台获取到的订单数据, 并初始化将提交给Java后台的字段
    for(int i = 0; i < jaData.size(); i++)
    {
        OrderData  orderDataTmp;
        QJsonObject joOrderItem = jaData[i].toObject();

        //数据检查
        if(!(joOrderItem.contains("orderId") &&\
             joOrderItem.contains("userId") &&\
             joOrderItem.contains("coin")&&\
             joOrderItem.contains("coinAddress")&&\
             joOrderItem.contains("amount")&&\
             joOrderItem.contains("txFee")&&\
             joOrderItem.contains("createTime")&&\
             joOrderItem.contains("way")&&\
             joOrderItem.contains("fee")
             ))
        {
            strErrMsg = " response is invalid. ";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }


        //从Java后台接收到的数据
        orderDataTmp.strOrderId = joOrderItem.value("orderId").toString();
        orderDataTmp.strUserId  = QString::number( joOrderItem.value("userId").toInt() , 10);
        orderDataTmp.strCoinType = joOrderItem.value("coin").toString();
        orderDataTmp.uTimeOrderCreate =  joOrderItem.value("createTime").toString().toULongLong(nullptr, 10)/1000; //统一都存为秒
        orderDataTmp.iWay =(WithdrawWay::WAY_AUTO == joOrderItem.value("way").toString().toInt()) ? (WithdrawWay::WAY_AUTO) : (WithdrawWay::WAY_MANUAL); //默认是人工
        orderDataTmp.strDstAddr = joOrderItem.value("coinAddress").toString();


        // 2020-08-25 判断订单是否存在
        bool isContain = false;
        for(const OrderData& order : vctOrderData)
        {
            if(order.strOrderId == orderDataTmp.strOrderId)
            {
                isContain = true;
                strErrMsg = " order: . " + order.strOrderId + " already in vctOrderData , skip it.";
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            }
        }
        if(isContain)
        {
            continue;
        }


        //如果Java传来的是科学计数法, 直接拒绝!!!
        if(joOrderItem.value("amount").toString().contains("E")
                || joOrderItem.value("fee").toString().contains("E")
                || joOrderItem.value("txFee").toString().contains("E"))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "`amount` or `fee` or `txFee` contains 'E', DO NOT use scientific notation";
            continue;
        }



        try
        {
            std::string strTmp;

            strTmp.clear();
            strTmp = joOrderItem.value("amount").toString().toStdString();
            orderDataTmp.dAmount =  boost::lexical_cast<double>(strTmp);

            strTmp.clear();
            strTmp = joOrderItem.value("fee").toString().toStdString();
            orderDataTmp.dTradeFee = boost::lexical_cast<double>(strTmp);

            strTmp.clear();
            strTmp = joOrderItem.value("txFee").toString().toStdString();
            orderDataTmp.dTxFee =  boost::lexical_cast<double>(strTmp);
        }
        catch(boost::bad_lexical_cast &e)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << orderDataTmp.strOrderId << ": " << QString::fromStdString(e.what());
            continue;
        }

        if(!(orderDataTmp.dAmount > 0.00000001 && orderDataTmp.dTradeFee > 0.00000001 && orderDataTmp.dTxFee > 0.00000001))
        {
            strErrMsg =  "`amount` or `fee` or `txFee` is too small. It's a invalid order. It has been refused .";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "order:" << orderDataTmp.strOrderId << strErrMsg;
            continue;
        }




        orderDataTmp.strSrcAddr = "---";
        orderDataTmp.strAuditor = "---";
        orderDataTmp.strRemarks = "---";

        //提交给Java后台的数据
        orderDataTmp.strTxid = "";
        orderDataTmp.uTimeTxComplete = 0; //交易完成时间, 初始化为0
        orderDataTmp.iTxStatus = TxStatus::TX_INCOMPLETE; //未完成
        orderDataTmp.iOrderStatus = OrderStatus::ORDER_PENDING; //待处理状态
        orderDataTmp.iAuditStatus = AuditStatus::AUDIT_PENDING; //待处理
        orderDataTmp.iUploadStatus = UploadStatus::UPLOAD_PENDING; //待上传
        orderDataTmp.uTimeAudit =  0;// 自动提币字字段设置为0, 人工审核提币设置为审核通过时间


        //2019-12-13 by yqq
        //XRP,  Java传过来的格式是:  目的地址_标签
        //其中 标签是 0 ~ (2^32 - 1) 的无符号整数, 即x86下的 unsigned int
        //例如: rpxSrARAeTE2Y699CsY5PwVEJ5v21TDrLi_100012
        //用户提币时标签是选填的(至于用户操作错误的情况,不考虑), 如果用户不填写, 1 作为默认标签
        if(0 == orderDataTmp.strCoinType.compare("XRP", Qt::CaseInsensitive))
        {
            try
            {
                QString str_Addr_Tag = orderDataTmp.strDstAddr.trimmed();
                if( !str_Addr_Tag.contains('_') || 2 != str_Addr_Tag.split('_').size() )
                    throw std::runtime_error("地址标签格式错误");

                QStringList strTmpList = str_Addr_Tag.split('_');
                QString strTmpXrpAddr = strTmpList[0];
                QString strTmpDstTag = strTmpList[1];

                if(!(strTmpDstTag.length() < 10   && boost::all( strTmpDstTag.toStdString(),  boost::is_alnum() ) ))
                    throw std::runtime_error("标签非法");

                uint64_t uDstTag = boost::lexical_cast<uint64_t>(strTmpDstTag.toStdString());
                if( uDstTag > INT_MAX) //缩小范围   只允许   2^31 - 1
                    throw std::runtime_error("标签过大");
            }
            catch(std::exception &e)
            {
                orderDataTmp.strRemarks = QString(e.what());
                orderDataTmp.strAuditor = QString("system");
                orderDataTmp.iAuditStatus = AUDIT_FAILED;
                orderDataTmp.iOrderStatus = ORDER_FAILED;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单号:%1").arg(orderDataTmp.strOrderId)
                        << orderDataTmp.strDstAddr << " 错误信息:"<< QString(e.what());
            }

        }
        else if(0 == orderDataTmp.strCoinType.compare("XLM", Qt::CaseInsensitive))
        {
            try
            {
                QString str_Addr_Tag = orderDataTmp.strDstAddr.trimmed();
                if( !str_Addr_Tag.contains('_') || 2 != str_Addr_Tag.split('_').size() )
                    throw std::runtime_error("地址标签格式错误");

                QStringList strTmpList = str_Addr_Tag.split('_');
                QString strTmpAddr = strTmpList[0];
                QString strTmpMemo = strTmpList[1];

                if(56 != strTmpAddr.length() || !strTmpAddr.startsWith('G'))
                    throw std::runtime_error("地址非法");


                //检查memo是否是  text_memo  并且长度是否小于 28个字符
                if(!(strTmpMemo.length() < 28   && boost::all( strTmpMemo.toStdString(),  boost::is_alnum()  ) ))
                    throw std::runtime_error("标签非法");

                //判断实际到账金额是否大于 1 XLM, 否则直接拒绝
                //参考: https://www.stellar.org/developers/guides/concepts/list-of-operations.html
                //因为, 如果目的账户不存在,当提币金额小于 1 XLM 时,使用CreateAccountOperation 会导致交易失败
                //错误类型: op_low_reserve
                //所以 直接拒绝提币金额小于  1 XLM  的提币订单, 是最简单处理方式
                if(orderDataTmp.dAmount < 1)
                    throw std::runtime_error("金额太小,必须大于1");


            }
            catch(std::exception &e)
            {
                orderDataTmp.strRemarks = QString(e.what());
                orderDataTmp.strAuditor = QString("system");
                orderDataTmp.iAuditStatus = AUDIT_FAILED;
                orderDataTmp.iOrderStatus = ORDER_FAILED;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单号:%1").arg(orderDataTmp.strOrderId)
                        << orderDataTmp.strDstAddr << " 错误信息:"<< QString(e.what());
            }


        }
        else if(0 == orderDataTmp.strCoinType.compare("EOS", Qt::CaseInsensitive))
        {
            try
            {
                QString str_Addr_Tag = orderDataTmp.strDstAddr.trimmed();
                if( !str_Addr_Tag.contains('_') || 2 != str_Addr_Tag.split('_').size() )
                    throw std::runtime_error("地址标签格式错误");

                QStringList strTmpList = str_Addr_Tag.split('_');
                QString strTmpAcct = strTmpList[0];
                QString strTmpMemo = strTmpList[1];

                if(!utils::IsValidAddr(orderDataTmp.strCoinType, strTmpAcct) )
                    throw std::runtime_error("地址非法");

                if(strTmpMemo.length() > 20)
                    throw std::runtime_error("标签长度过长");
            }
            catch(std::exception &e)
            {
                orderDataTmp.strRemarks = QString(e.what());
                orderDataTmp.strAuditor = QString("system");
                orderDataTmp.iAuditStatus = AUDIT_FAILED;
                orderDataTmp.iOrderStatus = ORDER_FAILED;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单号:%1").arg(orderDataTmp.strOrderId)
                        << orderDataTmp.strDstAddr << " 错误信息:"<< QString(e.what());
            }
        }




        //2019-04-09 非法地址,直接拒绝
        if(false == utils::IsValidAddr(orderDataTmp.strCoinType, orderDataTmp.strDstAddr))
        {
            orderDataTmp.strRemarks = QString("地址错误");
            orderDataTmp.strAuditor = QString("system");
            orderDataTmp.iAuditStatus = AUDIT_FAILED;  //5260 提币时输入错误地址，人工提币列表显示状态为待初审 2019.04.29 by liling
            orderDataTmp.iOrderStatus = ORDER_FAILED;
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单号:%1").arg(orderDataTmp.strOrderId) << orderDataTmp.strDstAddr << "地址非法, 修改订单状态为失败";
        }
        else //地址合法
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("目的地址: %1 检查通过").arg(orderDataTmp.strDstAddr);

            /*TODO:
              * 2019-04-29 yqq
              * 1.人工提币的出币地址,需要管理手动输入? 如果是, 则进行相应修改
              */
            //2019-04-08 限制自动提币
            __ModifyOrderWithdrawWayAndAuditStatusByWithdrawRules( orderDataTmp );

            //if(false == __IsAutoWithdraw(orderDataTmp.strCoinType, orderDataTmp.strUserId, orderDataTmp.strOrderId, orderDataTmp.dAmount))
            if( WithdrawWay::WAY_MANUAL == orderDataTmp.iWay )
            {
                //orderDataTmp.iWay = WithdrawWay::WAY_MANUAL; //改为人工提币
                if(utils::IsERC20Token( orderDataTmp.strCoinType ))
                {
                    orderDataTmp.strSrcAddr = QString("填写ERC20代币%1出币地址").arg(orderDataTmp.strCoinType);
                }
                else
                {
                    orderDataTmp.strSrcAddr = QString("填写%1出币地址").arg(orderDataTmp.strCoinType.toUpper());
                }

                qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  QString("订单: %1").arg(orderDataTmp.strOrderId)<< ",额度超过限制, 已强制改为人工审核提币";
            }
            else
            {
                //自动提币的出币币种和提币方式也统一在此处设置，后续不再判断设置，人工提币地址与自动提币地址分开，避免冲突  20190510 by liling
                //orderDataTmp.iWay = WithdrawWay::WAY_AUTO; //改为自动提币
                //TODO:检查地址合法性  2019-05-19  by yqq
                QString strTmpCoinType = orderDataTmp.strCoinType.toLower();
                QMap<QString, QVariant> mapTmpValue  = __m_qmapAllCoinConfig.value(strTmpCoinType);
                QString strSrcAddr = mapTmpValue.value("srcAddr").toString();
                orderDataTmp.strSrcAddr = strSrcAddr;

                if(AuditStatus::AUDIT_SUCCESS ==  orderDataTmp.iAuditStatus )
                {
                    orderDataTmp.strRemarks = "全自动提币";
                    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单:%1").arg(orderDataTmp.strOrderId) <<  "符合`全自动`提币条件";
                }
                else if(AuditStatus::AUDIT_PENDING ==  orderDataTmp.iAuditStatus )
                {
                    orderDataTmp.strRemarks = "半自动提币";
                    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单:%1").arg(orderDataTmp.strOrderId) <<  "符合`半自动`提币条件";
                }
            }
        }


        //2019-05-19  判断此币种是否在币种表中已经添加, 如果币种表中没有此币种,则 "暂时"不处理,此笔订单数据
        if(true == __m_qmapAllCoinConfig.contains(orderDataTmp.strCoinType.toLower()))
        {

            vctOrderData.push_back(orderDataTmp);
        }
        else
        {

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("订单号:%1 , 币种是:%2  , 由于此币种尚未添加, 暂不处理")
                        .arg(orderDataTmp.strOrderId).arg(orderDataTmp.strCoinType);

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("the orderid:'%1', cointype is '%2' withdraw not be support, \
                                                                                      due to this cointype not in DB, please add this cointype into db by 添加币种.")
                                                                                      .arg(orderDataTmp.strOrderId).arg(orderDataTmp.strCoinType);
        }


    }


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "一共: " <<  vctOrderData.size() << " 个提币订单请求";
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "成功返回";
    return CAutoWithdraw::NO_ERROR;
}


int CAutoWithdraw::PullOrderDataFromServer(const QString &strURL, QString &strStartTime, QByteArray &byteArrOrderData)
{
    //qsrand(time(NULL));
    //int nrand = qrand()  % (26 * 60 * 60); // 26 小时内随机



    //下面是调用加密接口  2019-05-19 徐昌
    QMap<QString, QVariant> mapObj;
    mapObj.insert("pageSize", "100");
//    mapObj.insert("startTime", "");
    mapObj.insert("startTime", strStartTime);
    QJsonDocument requestJsonDoc = QJsonDocument::fromVariant(mapObj);
    //接口请求需要的参数
    QByteArray requestJson = requestJsonDoc.toJson(QJsonDocument::Compact/*不加空白字符*/);

//#ifdef QT_DEBUG
//    QString strTmp = "{ \"coin\": \"BTC\", \"amount\": \"10066\", \"coinAddress\": \"1KWUCaN31hcRGbqsLgi95MZosC5hTJZLLq\", \"code\": \"0xaefbc9823948223948432423423\", \"status\": 0, \"createTime\": \"1559721127006\"}";
//    requestJson = QJsonDocument::fromJson(strTmp.toLatin1()).toJson(QJsonDocument::Compact);
//#endif

    //__m_pEncryptionModule->postEx(strURL, requestJson, byteArrOrderData);
    CEncryptionModule encMoudle;
    encMoudle.postEx(strURL, requestJson, byteArrOrderData);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " __m_pEncryptionModule->post  successed.";
    return CAutoWithdraw::NO_ERROR;
}

int CAutoWithdraw::UpdateUserInfoToDB() noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("开始更新用户信息表");
    QSqlQuery sql(__m_db);
    QString strSql = QString("SELECT distinct user_id FROM tb_auto_withdraw");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql语句:" << strSql;
    if(!sql.exec(strSql))
    {
        strErrMsg = "sql error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QStringList listAllUserID;
    while (sql.next())
    {
        listAllUserID << sql.value(0).toString();
    }

    QString strUserId, strUserTel, strUserEmail, strUserNick, strUserName;
    for(int i= 0; i < listAllUserID.size(); i++)
    {
        sql.clear();
        strSql = QString("select user_id,user_nick, user_family_name, tel, email from tb_user where user_id = '%1'").arg(listAllUserID[i]);


        const int ciTryCount =  5;

        for(int i = 0; i < ciTryCount; i++)
        {
            if(sql.exec(strSql)) break;

            if(ciTryCount - 1 == i) //最后一次
            {
                strErrMsg = "sql error:" + sql.lastError().text();;
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error( strErrMsg.toStdString() );
            }

            msleep(200);
        }


        //获取查询结果,如果查询不到则从java后台获取
        if(sql.next())
        {
            //qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("数据库中已存%1用户信息").arg(listAllUserID[i]);
            continue;
        }

        msleep(1000);
        //用户流水接口资源消耗巨大, 改用其他接口, Java最好独立一个用户信息接口   2019-12-02 by yqq
        //QUrl url(g_qstr_JAVA_getUserFlow);
        QUrl url(g_qstr_JAVA_getUserAsset);
        QNetworkRequest netReq(url);
        netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject joPostData;
        joPostData.insert("userId", listAllUserID[i]);
        //joPostData.insert("pageSize", 50);
        joPostData.insert("coin", QString("BTC"));

        QNetworkAccessManager  qnetMng;
        QNetworkReply *pReply = qnetMng.post( netReq, QJsonDocument(joPostData).toJson());
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http request data:" << url << joPostData;

        //异步转为同步
        ASYNC_TO_SYNC_CAN_THROW(pReply);

        QByteArray byteArrOrderData = pReply->readAll();
        pReply->deleteLater();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http response:" << byteArrOrderData;

        //解析java返回数据
        QJsonParseError error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(byteArrOrderData, &error));
        if(QJsonParseError::NoError != error.error  )
        {
            strErrMsg = "json parse error: " + error.errorString();
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject rootObj = jsonDoc.object();

        //判断返回状态是否OK
        if(!(rootObj.contains("errCode") && 0 == rootObj.value("errCode").toInt()))
        {
            strErrMsg = QString("java returns error %1 : %2").arg(rootObj.value("errCode").toInt()).arg(rootObj.value("errMsg").toString());
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        if(!rootObj.contains("data"))  //没有数据
        {
            strErrMsg = "missing `data`";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject subObjData = rootObj.value("data").toObject();
        if(!subObjData.contains("userInfo"))
        {
            strErrMsg = "`data` missing  `userInfo`";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonObject objUserInfo = subObjData.value("userInfo").toObject();
        if(!(objUserInfo.contains("userId") && objUserInfo.contains("realName") && objUserInfo.contains("phone") && objUserInfo.contains("email")))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "the response of java return is invalid. missing some fields.";
            //return CAutoWithdraw::JSON_ERR;
            continue; //如果此用户数据不存在, 继续获取其他用户的数据
        }

        strUserId = ((objUserInfo.value("userId").toString().isEmpty()) ? ("") : (objUserInfo.value("userId").toString()));
        strUserNick = ( (objUserInfo.value("realName").toString().isEmpty()) ? ("") : (objUserInfo.value("realName").toString()));
        strUserName = ( (objUserInfo.value("realName").toString().isEmpty()) ? ("") : (objUserInfo.value("realName").toString()));
        strUserTel = ( (objUserInfo.value("phone").toString().isEmpty()) ? ("") : (objUserInfo.value("phone").toString()) );
        strUserEmail = ( (objUserInfo.value("email").toString().isEmpty()) ?("") : (objUserInfo.value("email").toString()) );

        sql.clear();
        strSql = QString("insert ignore into tb_user values(\'%1\',\'%2\',\'%3\',\'%4\',\'%5\')")
                .arg(strUserId).arg(strUserNick).arg(strUserName).arg(strUserTel).arg(strUserEmail);

        for(int i = 0; i < ciTryCount; i++)
        {
            if(sql.exec(strSql)) break;

            if(ciTryCount - 1 == i) //最后一次
            {
                strErrMsg = "sql error: " + sql.lastError().text();
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }

            msleep(200);
        }
        sql.clear();
    }

    return CAutoWithdraw::NO_ERROR;
}

int CAutoWithdraw::GetExistOrderDataFromDB(vector<OrderData> &vctOrderData) noexcept(false)
{
    QString strErrMsg;

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始获取已存在的订单数据";

    QSqlQuery  sql(__m_db);

    const int ciTryCount = 5;
    for(auto it = vctOrderData.begin(); it != vctOrderData.end(); it++)
    {

        sql.clear();

        //TODO:获取出已有txid的, 待上传的, 上传失败的,
        //后期如果提币订单量很大的话, 也是一条一条处理
        QString strSql = QString("select * from tb_auto_withdraw where order_id='%1';").arg(it->strOrderId);
        sql.prepare(strSql);

        for(int iTry = 0; iTry < ciTryCount; iTry++)
        {
            if(sql.exec()) break;

            if(iTry == ciTryCount - 1) //最后一次尝试失败,直接返回
            {
                strErrMsg = sql.lastError().text();
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql error:" << strErrMsg;
                throw runtime_error(strErrMsg.toStdString());
            }
            msleep(200);
        }


        //获取查询结果
        if(!sql.next())
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(" sql query orderid:%1 result is empty.").arg(it->strOrderId);
            continue;
        }

        //获取订单状态信息
        it->strTxid = sql.value("txid").toString();
        it->iAuditStatus = (it->strTxid.length() > 10) ? (AuditStatus::AUDIT_SUCCESS) : (sql.value("audit_status").toInt()); //如果已有txid,审核状态定为成功
        it->iTxStatus = sql.value("tx_status").toInt();
        it->iOrderStatus = sql.value("order_status").toInt();
        it->iUploadStatus = sql.value("upload_status").toInt();
        it->uTimeAudit = sql.value("audit_time").toLongLong();
        it->uTimeOrderCreate = sql.value("create_time").toLongLong();
        it->uTimeTxComplete = sql.value("complete_time").toLongLong();
        it->strRemarks = sql.value("remarks").toString();

        //如果此订单已经存在, 则使用数据库中的数据, 防止提币方式的变动
        //    (当然, 关于金额限制变动的情况, 变动之前的订单维持原来的提币方式, 变动后的使用新的规则)
        //从数据库中读取源地址, 防止原本是自动提币的订单变为人工提币
        it->iWay = sql.value("withdraw_way").toInt();
        it->strSrcAddr = sql.value("src_addr").toString();
        it->strDstAddr = sql.value("dst_addr").toString();

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "txid:" << it->strTxid << " tx_status:" << it->iTxStatus << "  order_status:"\
                 << it->iOrderStatus << "  upload_status:" << it->iUploadStatus << "  audit_status:" << it->iAuditStatus;
    }


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "获取已存在的订单完成";
    return CAutoWithdraw::NO_ERROR;
}

//insert or ignore
int CAutoWithdraw::PushFreshOrderIntoDB(const vector<OrderData> &vctOrderData)
{
    QString strErrMsg;

    //1.检查数据库打开状态
    if(false == __m_db.isOpen())
    {
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库异常关闭, 尝试重新打开连接";
        if(false == __m_db.open())
        {
            strErrMsg = "reopen database failed.";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "重新打开数据库连接失败" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库是连接状态正常, 连接名:" << __m_db.connectionName();


    QSqlQuery sql(__m_db);
    QString strSql = "";

    if (!sql.exec("SET AUTOCOMMIT=0;"))
    {
        strErrMsg = "database error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    sql.clear();
    if (!sql.exec("START TRANSACTION;"))
    {
        strErrMsg = "database error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    for(uint i = 0; i < vctOrderData.size(); i++)
    {
        strSql.clear();


        //如果订单已存在,则忽略
        strSql = QString("insert ignore into tb_auto_withdraw(`order_id`, `user_id`, `coin_type`, `amount`,`trade_fee`, `tx_fee`, \
                         `txid`, `dst_addr`, `src_addr`, `withdraw_way`, `audit_status`,`tx_status`, `order_status`, `upload_status`,\
                         `create_time`, `audit_time`, `remarks`, `auditor`, `complete_time`) \
                         values(\'%1\',\'%2\',\'%3\',\'%4\',\'%5\',\'%6\',\'%7\',\'%8\',\'%9\',%10,%11,%12,%13,%14,%15,%16,\'%17\',\'%18\',%19);")
                .arg(vctOrderData[i].strOrderId).arg(vctOrderData[i].strUserId).arg(vctOrderData[i].strCoinType)
                .arg(QString::asprintf("%.8f", vctOrderData[i].dAmount)).arg(QString::asprintf("%.8f", vctOrderData[i].dTradeFee))
                .arg(QString::asprintf("%.8f", vctOrderData[i].dTxFee)).arg(vctOrderData[i].strTxid).arg(vctOrderData[i].strDstAddr)
                .arg(vctOrderData[i].strSrcAddr).arg(vctOrderData[i].iWay).arg(vctOrderData[i].iAuditStatus).arg(vctOrderData[i].iTxStatus)
                .arg(vctOrderData[i].iOrderStatus).arg(vctOrderData[i].iUploadStatus).arg(vctOrderData[i].uTimeOrderCreate)
                .arg(vctOrderData[i].uTimeAudit).arg(vctOrderData[i].strRemarks).arg(vctOrderData[i].strAuditor).arg(vctOrderData[i].uTimeTxComplete);

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql:" << strSql;
        sql.clear();
        if (!sql.exec(strSql))
        {
            strErrMsg = "database error: " + sql.lastError().text();
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << strErrMsg;
            sql.clear();
            sql.exec("ROLLBACK");

            throw runtime_error(strErrMsg.toStdString());
        }
    }


    sql.clear();
    if (!sql.exec("COMMIT"))
    {
        strErrMsg = "database error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        sql.clear();
        sql.exec("ROLLBACK");

        throw runtime_error(strErrMsg.toStdString());
    }

    sql.clear();
    if (!sql.exec("SET AUTOCOMMIT=1"))
    {
        strErrMsg = "database error: " + sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "设置数据库自动提交事务失败:" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "事务提交成功:";
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "数据库插入成功" ;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "PushOrderDataIntoDB成功, 订单数据存入数据库成功!";
    sql.clear();

    return CAutoWithdraw::NO_ERROR;
}

int CAutoWithdraw::MakePushOrderData(const OrderData &orderData, QByteArray &byteArrPushData) noexcept(true)
{
    QJsonObject joPush;

    //joPush.insert("way", orderData.iWay); //提币方式  0:自动   1:人工   如果是其他状态Java后台会拒绝!
    joPush.insert("code", (orderData.strTxid.isEmpty()) ? ("") : (orderData.strTxid));
    joPush.insert("orderId", orderData.strOrderId);

    //订单状态:   1:成功    2:失败    如果是其他状态Java后台会拒绝!
    joPush.insert("orderStatus", (OrderStatus::ORDER_SUCCESS == orderData.iOrderStatus) ? (OrderStatus::ORDER_SUCCESS) : (OrderStatus::ORDER_FAILED));

     //审核状态:   1:成功    2:失败    如果是其他状态Java后台会拒绝!
    joPush.insert("auditStatus", (AuditStatus::AUDIT_SUCCESS == orderData.iAuditStatus) ? (AuditStatus::AUDIT_SUCCESS) : (AuditStatus::AUDIT_FAILED));
    joPush.insert("auditTime", QString::asprintf("%llu", orderData.uTimeAudit)); //审核时间
    joPush.insert("finishTime", QString::asprintf("%llu", orderData.uTimeTxComplete)); //交易完成时间
    joPush.insert("auditName", CUserInfo::loginUser().LoginName()); // 审核人   TODO:添加审核人

    byteArrPushData = QJsonDocument(joPush).toJson(QJsonDocument::Compact);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "MakePushOrderData 组装push数据成功";
    return CAutoWithdraw::NO_ERROR;
}


int CAutoWithdraw::PushOrderDataToServer(const QString &strURL, QByteArray &byteArrPushData) noexcept(false)
{
    QString strErrMsg;
    QByteArray byteArrPushRet;  //存储返回的数据

    //__m_pEncryptionModule->postEx(strURL, byteArrPushData, byteArrPushRet);
    CEncryptionModule encMoudle;
    encMoudle.postEx(strURL, byteArrPushData, byteArrPushRet);


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http返回的数据:" << byteArrPushRet;

    //解析Java后台返回的数据
    QJsonParseError error;
    QJsonDocument jdocRes = QJsonDocument::fromJson(byteArrPushRet, &error);
    if(QJsonParseError::NoError != error.error)
    {
        strErrMsg = "json parse error:" + error.errorString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QJsonObject joRes = jdocRes.object();
    if(!(joRes.contains("errCode") && joRes.contains("data") && joRes.contains("errMsg") && joRes.contains("timestamps")))
    {
        strErrMsg = "reponse json missing `errCode` or `data` or `errMsg` or `timestamps`";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    if(0 != joRes.value("errCode").toInt())
    {
        strErrMsg = QString("json returns error %1 : %2").arg(joRes.value("errCode").toInt()).arg(joRes.value("errMsg").toString());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "PushOrderDataToServer提交数据成功";
    return CAutoWithdraw::NO_ERROR;
}


int CAutoWithdraw::HandleOrder() noexcept(false)
{
    QString strErrMsg;
    //0. 从币种表里面读取所有币种, 如果币种表为空,则不开启提币
    if(__m_qmapAllCoinConfig.isEmpty())
    {
        strErrMsg  = "not found any cointype in db.So cannot withdraw now. Please add coin first.";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    int iRet = 0;
    QByteArray byteArrRet;
    vector<OrderData> vctOrderData;

    std::default_random_engine e(time(NULL));
    std::uniform_int_distribution<int> d(1598245200, (int)(QDateTime::currentDateTime().toTime_t()) );

    for(int i = 0; i < 3 && vctOrderData.size() <= 200; i++)
    {
        //1.从Java后台获取订单数据
        //uint nrand = abs( (int)e()) % (QDateTime::currentDateTime().toTime_t() - 1598245200) ;
        QString strStartTime = QString::asprintf("%u",  (uint)d(e) );
        strStartTime += "000";

        if(0 == i) strStartTime = ""; //第一次获取最新的

        try{
            PullOrderDataFromServer(g_qstr_JAVA_selectWithdrawOrderInfo, strStartTime, byteArrRet);
        }catch(std::exception &e){
            strErrMsg  = QString::fromStdString(e.what());
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            msleep(5000);
            continue;
        }

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "PullOrderDataFromServer 调用成功";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  "请求返回的数据: "<< byteArrRet.data();

        //2.解析Java后台返回的数据
        ParseBytesToOrderData(byteArrRet, vctOrderData);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ParseBytesToOrderData调用成功, 解析订单数据解析成功";
        msleep(5000);
    }



     //2.1.从数据库获取已有的订单数据(有txid的, 上传状态为0, 或者 上传状态为2)
    GetExistOrderDataFromDB(vctOrderData);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "GetExistOrderDataFromDB 从数据库中获取已有订单数据 成功";


    //2.3.存入数据库(如果订单号已存在,则忽略)
    PushFreshOrderIntoDB(vctOrderData);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " PushFreshOrderIntoDB调用成功, 订单数据插入数据库成功";


    //2.4 同步用户信息表 20190423 by liling
    UpdateUserInfoToDB();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(" UpdateUserInfotoDB成功, 用户信息更新成功");

    //2019-11-27 by yqq
    //2.5 ETH和ERC20的余额的临界调节检查:
    // 如果有两笔或两笔以上 ETH或ERC20代币的自动提币, 那么需要判断当前的余额能满足几笔
    //  例如: 余额是 10   第一笔 是 9,  第二笔是 5  那么  第二笔直接设置为订单失败
    try
    {
        try{
            BalanceThresholdCheckAndSMSNotify(vctOrderData);

            //如果没有异常, 则将计数器清0
            __m_nWalletServerHealthStateMonitorCounter = 0;
        }catch(std::exception &e){
            QString strExceptStr(e.what());
            if (strExceptStr.contains(STR_HTTP_ERROR, Qt::CaseInsensitive))
            {
                __m_nWalletServerHealthStateMonitorCounter += 1;
                __WalletServerHealthStateMonitor();
            }

            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("余额监控异常:%1").arg(e.what());
            throw e;
        }
    }
    catch(...){
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("余额监控异常退出");
    }


    //3.调用钱包服务端交易接口, 进行转账
    for(vector<OrderData>::iterator it = vctOrderData.begin(); it != vctOrderData.end(); it++)
    {
        msleep(500); //休眠500ms
        if(WithdrawWay::WAY_MANUAL == it->iWay)  //人工提币省略该步骤，20190510 by liling
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "订单: "<< it->strOrderId << "为人工提币，不进行转账操作";
            continue;
        }

        //2020-04-24 审核状态的修改  移至提币方式判断的函数中
        // 如果订单是是`全自动`的提币,  直接设置审核状态为AUDIT_SUCCESS
        // 如果订单是 `半自动`的提币, 设置审核状态 AUDIT_PENDING
        /*if(CAutoWithdraw::AUDIT_PENDING == it->iAuditStatus && WithdrawWay::WAY_AUTO == it->iWay)  //自动提币待审核状态的订单是否已超过5分钟
        {
#if 1 //dev环境 为了方便调试  不用设置

            if(true)
#else
            if(AUTOWITH_WAITTIME < (QDateTime::currentDateTime().toTime_t() - it->uTimeOrderCreate))
#endif
            {
                it->iAuditStatus = AUDIT_SUCCESS;
            }
        }*/

        try
        {
            //先实现功能, 再根据存在的问题一步步重构, 完善代码结构 2019-11-18 by yqq
            // Refactoring to Patterns
            if(it->strTxid.isEmpty()  && WithdrawWay::WAY_AUTO == it->iWay && AUDIT_SUCCESS == it->iAuditStatus)
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "订单: "<< it->strOrderId << "为自动提币，开始进行转账操作";

                if(0 == it->strCoinType.compare("btc", Qt::CaseInsensitive))
                {
                    BTC_AutoWithdraw(*it);
                }
                else if(0 == it->strCoinType.compare("ltc", Qt::CaseInsensitive) )
                {
                    LTC_AutoWithdraw(*it);
                }
                else if(0 == it->strCoinType.compare("bch", Qt::CaseInsensitive) )
                {
                    BCH_AutoWithdraw(*it);
                }
                 else if(0 == it->strCoinType.compare("bsv", Qt::CaseInsensitive) )
                {
                    BSV_AutoWithdraw(*it);
                }
                else if(0 == it->strCoinType.compare("dash", Qt::CaseInsensitive) )
                {
                    DASH_AutoWithdraw(*it);
                }
                else if(0 == it->strCoinType.compare("usdt", Qt::CaseInsensitive))
                {
                    USDT_AutoWithdraw(*it);
                }
                else if(0 == it->strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(it->strCoinType) //eth 或 ERC20代币
                    || 0 == it->strCoinType.compare("etc", Qt::CaseInsensitive) //ETC
                    )
                {
                    ETH_AutoWithdraw(*it);
                    ETH_PushNonceIntoDB(*it);
                }
                else if(0 == it->strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == it->strCoinType.compare("htdf", Qt::CaseInsensitive)
                        || 0 == it->strCoinType.compare("het", Qt::CaseInsensitive))
                {
                    QThread::sleep(3);
                    COSMOS_AutoWithdraw( *it);
                    QThread::sleep(3);
                }
                else if( utils::Is_HRC20_Token(  it->strCoinType) ) //2020-04-15 增加 HRC20 代币转账
                {
                    QThread::sleep(3);
                    HRC20_AutoWithdraw(*it);
                    QThread::sleep(3);
                }
                else if(0 == it->strCoinType.compare("XRP", Qt::CaseInsensitive))
                {
                    XRP_AutoWithdraw( *it );
                }
                else if(0 == it->strCoinType.compare("EOS", Qt::CaseInsensitive))
                {
                    EOS_AutoWithdraw( *it );
                }
                else if(0 == it->strCoinType.compare("XLM", Qt::CaseInsensitive))
                {
                    XLM_AutoWithdraw( *it );
                }
                else if(0 == it->strCoinType.compare("TRX", Qt::CaseInsensitive))
                {
                    TRX_AutoWithdraw( *it );
                }
                else if(0 == it->strCoinType.compare("XMR", Qt::CaseInsensitive))
                {
                    XMR_AutoWithdraw( *it );
                }


                else //其他币种,暂不支持
                {
                    it->strRemarks = "此币种暂不支持";
                    it->iOrderStatus = OrderStatus::ORDER_FAILED;
                    it->iAuditStatus = AuditStatus::AUDIT_FAILED;  //不支持的币种直接拒绝
                    iRet = UpdateDBOrderData(*it);
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "暂不支持此币种的提币功能";
                    continue;
                }
            }
            else //已有txid
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "订单: "<< it->strOrderId << "尚不满足自动提币出账条件，跳过";

                if(0 == it->strCoinType.compare("eth", Qt::CaseInsensitive))
                {
                    //如果已有txid, 则获取nonce值
                    iRet = ETH_GetNonceFromDB(*it);
                    if( CAutoWithdraw::NO_ERROR != iRet )
                    {
                        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ETH_GetNonceFromDB() return error. It's a serious ERROR !!! please contact with administrator.";
                        continue;
                    }
                }
            }
        }
        catch(std::exception &e)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(e.what());
        }
        catch(...)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "未知错误";
        }

        //如果是半自动 未审核通过的 则跳过
        if( it->strTxid.isEmpty() && WithdrawWay::WAY_AUTO == it->iWay
            && AUDIT_SUCCESS != it->iAuditStatus && AUDIT_FAILED != it->iAuditStatus )
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "订单:" << it->strOrderId << "审核状态:"
                         << it->iAuditStatus << "尚未审核通过或失败, 跳过, 不更新订单状态,";
            continue;
        }


        //更新提币订单的状态
        UpdateDBOrderData(*it);
    }


    //4.将订单完成的数据提交给Java后台
    //QString strPushOrderURL = "http://192.168.10.54:8071/btbOrder/upWithdrawOrderStatus" ;
    QString strPushOrderURL = g_qstr_JAVA_upWithdrawOrderStatus ;
    for(vector<OrderData>::iterator it = vctOrderData.begin(); it != vctOrderData.end(); it++)
    {
        msleep(500); //休眠500ms

        //检查是否符合上传要求
        if(!__CheckUploadable(*it))
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << it->strOrderId << "不符合上传要求, 暂不上传Java后台";
            continue;
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << it->strOrderId << "符合上传要求,准备上传Java后台";

        try
        {
            QByteArray bytePushData ;
            MakePushOrderData(*it, bytePushData);
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "MakePushOrderData成功返回, 组装完成订单push数据成功";

            PushOrderDataToServer(strPushOrderURL, bytePushData);

            //5.后续处理(更新上传状态)
            it->iUploadStatus = UploadStatus::UPLOAD_SUCCESS;
            UpdateDBOrderData(*it);
        }
        catch(std::exception &e)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(e.what());

            //更新上传状态
            it->iUploadStatus = UploadStatus::UPLOAD_FAILED; //设置为失败,下次继续上传
            try{
                UpdateDBOrderData(*it);
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新订单:" << it->strOrderId <<" 上传状态为'上传失败'成功";
            }catch(...){
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新订单:" << it->strOrderId <<" 上传状态为'上传失败'失败";
            }

            continue; //继续上传后面的订单
        }
    }


    return CAutoWithdraw::NO_ERROR;
}



int CAutoWithdraw::BTC_AutoWithdraw( OrderData &orderData)noexcept(false)
{
#if defined(BTC_SERIES_EX)

    //1.创建交易
    rawtx::btc::BTC_CreateRawTxParam createParam;
    createParam.dAmount = orderData.dAmount;
    createParam.vctSrcAddr.push_back(orderData.strSrcAddr);
    createParam.vctDstAddr.push_back(orderData.strDstAddr);
    createParam.strURL = g_qstr_WalletAPI_btc_createrawtransaction;
    createParam.strCoinType = orderData.strCoinType;
    createParam.mapTransfer.insert(std::make_pair(orderData.strDstAddr, orderData.dAmount));

    __m_btcRawTxUtilEx.CreateRawTx( &createParam );


    //2.签名
    rawtx::btc::BTC_SignRawTxParam  signParam;
    signParam.strCoinType = createParam.strCoinType;
    signParam.bIsCollection = false;
    signParam.mapTxout = createParam.mapTxOut;
    signParam.strUnsignedRawTxHex = createParam.strRawTxHex;
    signParam.vctInAddrs = createParam.vctSrcAddr;
    signParam.vctUTXOs = createParam.vctRefUTXOs;

    //获取私钥, 禁止打印日志
    {
        QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(createParam.strCoinType.toLower());
        QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
        signParam.vctPrivKeys.push_back(strTmpPrivKey);
    }

    __m_btcRawTxUtilEx.SignRawTx( &signParam );



    //3.广播交易
    rawtx::btc::BTC_BroadcastRawTxParam  bcastParam;
    bcastParam.strCoinType = signParam.strCoinType;
    bcastParam.strSignedRawTxHex = signParam.strSignedRawTxHex;
    bcastParam.strURL = g_qstr_WalletAPI_btc_sendrawtransaction;

    __m_btcRawTxUtilEx.BroadcastRawTx( &bcastParam );


    //若广播成功, 返回相应的数据
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strTxid = bcastParam.strTxid;


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "BTC_AutoWithdraw() successed. orderId:" << orderData.strOrderId
            << "txid:" <<orderData.strTxid << "amount:" << orderData.dAmount;

    return CAutoWithdraw::NO_ERROR;


#else
    //1.创建裸交易
    QString strURLTemp = g_qstr_WalletAPI_btc_createrawtransaction;
    QString strSrcAddr = orderData.strSrcAddr;
    QString strDstAddr = orderData.strDstAddr;
    double dAmount = orderData.dAmount;
    vector<UTXO>  vctUTXOs;
    map<QString, double> mapTxOut;
    QString strRawTex;
    CBTCRawTxUtil().CreateRawTransaction(strURLTemp, strSrcAddr, strDstAddr, dAmount, strRawTex, mapTxOut, vctUTXOs);


    //2.签名
    vector<QString> vctSrcAddr;
    vctSrcAddr.push_back(orderData.strSrcAddr);
    vector<QString> vctPrivKey;

    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value("btc");
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    vctPrivKey.push_back(strTmpPrivKey);
    QString strSignedHex;
    CBTCOffSig().TxOfflineSignature(false, strSignedHex, vctSrcAddr, mapTxOut, strRawTex, vctPrivKey, vctUTXOs);


    //3广播交易
    strURLTemp = g_qstr_WalletAPI_btc_sendrawtransaction ;
    CBTCRawTxUtil().BroadcastRawTx(strURLTemp, strSignedHex, orderData.strTxid);

    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "BTC_AutoWithdraw 成功返回";

    return CAutoWithdraw::NO_ERROR;
#endif
}


int CAutoWithdraw::LTC_AutoWithdraw( OrderData &orderData)noexcept(false)
{
    //1.创建裸交易
    QString strURLTemp = g_qstr_WalletAPI_ltc_createrawtransaction;
    QString strSrcAddr = orderData.strSrcAddr;
    QString strDstAddr = orderData.strDstAddr;
    double dAmount = orderData.dAmount;
    vector<UTXO>  vctUTXOs;
    map<QString, double> mapTxOut;
    QString strRawTex;
     CLTCRawTxUtil().CreateRawTransaction(strURLTemp, strSrcAddr, strDstAddr, dAmount, strRawTex, mapTxOut, vctUTXOs);


    //2.签名
    vector<QString> vctSrcAddr;
    vctSrcAddr.push_back(orderData.strSrcAddr);
    vector<QString> vctPrivKey;

    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value("ltc");
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    vctPrivKey.push_back(strTmpPrivKey);
    QString strSignedHex;
    CLTCOffSig().TxOfflineSignature(false, strSignedHex, vctSrcAddr, mapTxOut, strRawTex, vctPrivKey, vctUTXOs);


    //3广播交易
    strURLTemp = g_qstr_WalletAPI_ltc_sendrawtransaction ;
    CLTCRawTxUtil().BroadcastRawTx(strURLTemp, strSignedHex, orderData.strTxid);

    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "LTC_AutoWithdraw 成功返回";

    return CAutoWithdraw::NO_ERROR;
}


int CAutoWithdraw::BCH_AutoWithdraw( OrderData &orderData)noexcept(false)
{
    //1.创建裸交易
    QString strURLTemp = g_qstr_WalletAPI_bch_createrawtransaction;
    QString strSrcAddr = orderData.strSrcAddr;
    QString strDstAddr = orderData.strDstAddr;
    double dAmount = orderData.dAmount;
    vector<UTXO>  vctUTXOs;
    map<QString, double> mapTxOut;
    QString strRawTex;
    CBCHRawTxUtil().CreateRawTransaction(strURLTemp, strSrcAddr, strDstAddr, dAmount, strRawTex, mapTxOut, vctUTXOs);


    //2.签名
    vector<QString> vctSrcAddr;
    vctSrcAddr.push_back(orderData.strSrcAddr);
    vector<QString> vctPrivKey;

    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value("bch");
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    vctPrivKey.push_back(strTmpPrivKey);
    QString strSignedHex;
    CBCHOffSig().TxOfflineSignature(false, strSignedHex, vctSrcAddr, mapTxOut, strRawTex, vctPrivKey, vctUTXOs);


    //3广播交易
    strURLTemp = g_qstr_WalletAPI_bch_sendrawtransaction ;
    CBCHRawTxUtil().BroadcastRawTx(strURLTemp, strSignedHex, orderData.strTxid);

    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "LTC_AutoWithdraw 成功返回";

    return CAutoWithdraw::NO_ERROR;
}


int CAutoWithdraw::BSV_AutoWithdraw( OrderData &orderData)noexcept(false)
{
    //1.创建裸交易
    QString strURLTemp = g_qstr_WalletAPI_bsv_createrawtransaction;
    QString strSrcAddr = orderData.strSrcAddr;
    QString strDstAddr = orderData.strDstAddr;
    double dAmount = orderData.dAmount;
    vector<UTXO>  vctUTXOs;
    map<QString, double> mapTxOut;
    QString strRawTex;
    CBSVRawTxUtil().CreateRawTransaction(strURLTemp, strSrcAddr, strDstAddr, dAmount, strRawTex, mapTxOut, vctUTXOs);


    //2.签名
    vector<QString> vctSrcAddr;
    vctSrcAddr.push_back(orderData.strSrcAddr);
    vector<QString> vctPrivKey;

    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value("bsv");
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    vctPrivKey.push_back(strTmpPrivKey);
    QString strSignedHex;
    CBSVOffSig().TxOfflineSignature(false, strSignedHex, vctSrcAddr, mapTxOut, strRawTex, vctPrivKey, vctUTXOs);


    //3广播交易
    strURLTemp = g_qstr_WalletAPI_bsv_sendrawtransaction ;
    CBSVRawTxUtil().BroadcastRawTx(strURLTemp, strSignedHex, orderData.strTxid);

    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "LTC_AutoWithdraw 成功返回";

    return CAutoWithdraw::NO_ERROR;
}


int CAutoWithdraw::DASH_AutoWithdraw( OrderData &orderData)noexcept(false)
{
    //1.创建裸交易
    QString strURLTemp = g_qstr_WalletAPI_dash_createrawtransaction;
    QString strSrcAddr = orderData.strSrcAddr;
    QString strDstAddr = orderData.strDstAddr;
    double dAmount = orderData.dAmount;
    vector<UTXO>  vctUTXOs;
    map<QString, double> mapTxOut;
    QString strRawTex;
     CDASHRawTxUtil().CreateRawTransaction(strURLTemp, strSrcAddr, strDstAddr, dAmount, strRawTex, mapTxOut, vctUTXOs);


    //2.签名
    vector<QString> vctSrcAddr;
    vctSrcAddr.push_back(orderData.strSrcAddr);
    vector<QString> vctPrivKey;

    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value("dash");
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    vctPrivKey.push_back(strTmpPrivKey);
    QString strSignedHex;
    CDASHOffSig().TxOfflineSignature(false, strSignedHex, vctSrcAddr, mapTxOut, strRawTex, vctPrivKey, vctUTXOs);


    //3广播交易
    strURLTemp = g_qstr_WalletAPI_dash_sendrawtransaction ;
    CDASHRawTxUtil().BroadcastRawTx(strURLTemp, strSignedHex, orderData.strTxid);

    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "LTC_AutoWithdraw 成功返回";

    return CAutoWithdraw::NO_ERROR;
}





int CAutoWithdraw::UpdateDBOrderData(const OrderData &orderData) noexcept(false)
{
    QString strErrMsg = "";

    QSqlQuery sql(__m_db);
    QString strSql = QString("update tb_auto_withdraw set txid='%1', complete_time=%2, audit_time=%3,tx_status=%4, order_status=%5, "
                             "upload_status=%6 , withdraw_way=%7, audit_status=%8, remarks='%9' where order_id='%10';")
            .arg(orderData.strTxid).arg(QDateTime::currentDateTime().toTime_t()).arg(orderData.uTimeAudit)\
            .arg(orderData.iTxStatus).arg(orderData.iOrderStatus).arg(orderData.iUploadStatus)\
            .arg(orderData.iWay).arg(orderData.iAuditStatus).arg(orderData.strRemarks).arg(orderData.strOrderId);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql语句: " << strSql;

    try
    {
        if (!sql.exec("SET AUTOCOMMIT=0;"))
        {
            strErrMsg = sql.lastError().text();
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }

        sql.clear();
        if (!sql.exec("START TRANSACTION;"))
        {
            strErrMsg = sql.lastError().text();
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开启数据库事务成功";

        if(!sql.exec(strSql))
        {
            strErrMsg = sql.lastError().text() ;
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql错误:" << strErrMsg;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始事务回滚";

            sql.clear();
            if (!sql.exec("ROLLBACK;"))
            {
                strErrMsg = sql.lastError().text();
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "事务回滚失败:"<< strErrMsg;
                throw runtime_error( strErrMsg.toStdString() );
            }

            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "事务回滚成功";
            throw runtime_error( strErrMsg.toStdString() );
        }

        sql.clear();
        if (!sql.exec("COMMIT;"))
        {
            strErrMsg = sql.lastError().text();
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"事务提交失败:" << strErrMsg;
            throw runtime_error( strErrMsg.toStdString() );
        }

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "事务提交成功";
    }
    catch(std::exception &e)
    {
        strErrMsg = e.what();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "捕获到异常:" << strErrMsg;
        sql.clear();
        sql.exec("ROLLBACK;");

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    sql.clear();
    if (!sql.exec("SET AUTOCOMMIT=1;"))
    {
        strErrMsg = sql.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "设置自动提交失败" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新订单数据成功";
    return CAutoWithdraw::NO_ERROR;
}


//启动数据库服务
int CAutoWithdraw::__StartDBService() noexcept(false)
{
    QString strErrMsg;

    if (g_bIsLAN)
    {
        //测试网
        __m_db = QSqlDatabase::addDatabase("QMYSQL", "connWithdraw");
        __m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
        __m_db.setHostName("192.168.10.81");
        __m_db.setPort(3306);
        __m_db.setUserName("root");
        __m_db.setPassword("MyNewPass4!");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }
    else
    {
        //主网
        __m_db = QSqlDatabase::addDatabase("QMYSQL", "connWithdraw");
        __m_db.setDatabaseName("assertdb");
        __m_db.setHostName("rm-wz991r2rjs3wmd9t1io.mysql.rds.aliyuncs.com");
        __m_db.setPort(3306);
        __m_db.setUserName("accertdb");
        __m_db.setPassword("CG1R47JxIfBofG5uIofHcUKW0Ay1f8");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }

    if (!__m_db.open())
    {
        strErrMsg = "database error:"  +  __m_db.lastError().text();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }
    return CAutoWithdraw::NO_ERROR;
}


//bool CAutoWithdraw::__IsAutoWithdraw(
//            const QString &strCoinType,const QString &strUserId, const QString &strOrderId, const double dAmount) const noexcept(true)
int CAutoWithdraw::__ModifyOrderWithdrawWayAndAuditStatusByWithdrawRules( OrderData &orderData )
{
    //2019-06-01
    //在HTDF 和 USDP 公链未解决网络拥塞情况下不返回Txid的问题之前,  所有的 HTDF和USDP 都走人工提币
    /*if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("het", Qt::CaseInsensitive))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << QString("htdf和usdp暂时全部走人工提币");
        return false;
    }*/


    QString strCoinType = orderData.strCoinType;
    QString strUserId = orderData.strUserId;
    QString strOrderId = orderData.strOrderId;
    double dAmount = orderData.dAmount;

    //判断订单是否已经存在, 如果此订单已经存在, 不改变其提币方式, 即自动依然是自动,人工依然是人工
    if(true)
    {
        QSqlQuery sqlTmp(__m_db);
        QString strSqlTmp = QString("SELECT withdraw_way,audit_status FROM tb_auto_withdraw WHERE order_id='%1'").arg(strOrderId);
        sqlTmp.prepare(strSqlTmp);
        if(sqlTmp.exec())
        {
            if(sqlTmp.next())
            {
                int iWay =   sqlTmp.value(0).toInt();
                int iAuditStatus =  sqlTmp.value(1).toInt();
                if(WithdrawWay::WAY_AUTO == iWay)
                {
                    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("order:%1 is auto yet.").arg(strOrderId);
                    orderData.iWay = iWay;
                    orderData.iAuditStatus = iAuditStatus; //用于区分 , 全自动  还是  半自动
                }
                else
                {
                    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("order:%1 is manual-withdraw yet.").arg(strOrderId);
                    orderData.iWay = WithdrawWay::WAY_MANUAL;
                    //orderData.iAuditStatus = iAuditStatus;  //这里不用管, 稍后会从输入库获取已存在的订单数据
                }
                return CAutoWithdraw::NO_ERROR;
            }
        }
    }


    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(strCoinType.toLower());

    bool bOk = false;
    double dMaxAutoWithdawAmount = mapTmp.value("dMaxAutoWithdawAmount").toDouble(&bOk); //自动提币和人工审核提币的分界, 即自动提币的最大数量
    if(false == bOk)
    {
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dAutoWithdawAmount is not invalid float number.";
        //return false;

        orderData.iWay = WithdrawWay::WAY_MANUAL;
        return CAutoWithdraw::NO_ERROR;
    }

    //TODO: 需要测试
//    if( dAmount >= dMaxAutoWithdawAmount)
//    {
//        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString::asprintf("金额:%.8f大于自动提币最大额度, 不符合自动提币规则. 已改为人工审核提币.", dAmount);
//        return false;
//    }

    double dSemiAutodrawAmount = mapTmp.value("dSemiAutoWithdrawAmount").toDouble(&bOk);  //半自动额度  2020-04-24 添加 yqq
    if(false == bOk)
    {
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dAutoWithdawAmount is not invalid float number.";
        //return false;
        orderData.iWay = WithdrawWay::WAY_MANUAL;
        return CAutoWithdraw::NO_ERROR;
    }

    if( dAmount >= dSemiAutodrawAmount)
    {
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString::asprintf("金额:%.8f大于`半自动`提币最大额度, 不符合自动提币规则. 已改为人工审核提币.", dAmount);
//        return false;
        orderData.iWay = WithdrawWay::WAY_MANUAL;
        return CAutoWithdraw::NO_ERROR;
    }
    else if( dMaxAutoWithdawAmount  <= dAmount && dAmount < dSemiAutodrawAmount ) //半自动
    {
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString::asprintf("金额:%.8f大于`全自动`提币最大额度, 小于`半自动`提币最大额度, 设置为`半自动`提币.", dAmount);
        orderData.iWay = WithdrawWay::WAY_AUTO;
        orderData.iAuditStatus = AuditStatus::AUDIT_PENDING; //等待人工审核通过
        return CAutoWithdraw::NO_ERROR;
    }

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString::asprintf("金额:%.8f小于`半自动`提币最大额度, 开始进行单用户单日单币种提币额度限制的判断", dAmount);


    //判断用户单日提币额度, 如果额度超过限制, 则改为人工提币
    QSqlQuery  sql(__m_db);
    QString strCoinTypeTmp = strCoinType.trimmed();
    QString strUserIdTmp = strUserId.trimmed();

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << "币种:" << strCoinTypeTmp << "userId:" << strUserIdTmp << "orderId:" << strOrderId;

    //获取今天(当前日期)的 00:00:00 的时间戳
    time_t t = time(NULL);
    struct tm * tm = localtime(&t);
    if(NULL == tm)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << "localtime return NULL";
        orderData.iWay = WithdrawWay::WAY_MANUAL;
        return CAutoWithdraw::NO_ERROR;
//        return false;
    }

    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    uint uTimestamp00 =  (uint)mktime(tm); //00:00:00

    tm->tm_hour = 24;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    uint uTimestamp24 = (uint)mktime(tm); //24:00:00  即 第二天 00:00:00

    /*
     * 判断是否存在今天内的订单
     * 如果存在以下情况:
     *      用户昨天的订单,未被处理, 今天在处理时, 是否把它算作今天的订单并累计到今日提币额度?
    */

    //判断今天之内是否存在历史提币订单, 如果没有, 则通过
    if(true)
    {
        QSqlQuery  sqlTmp(__m_db);
        QString strSqlTmp = QString("SELECT * FROM tb_auto_withdraw where user_id='%1' and coin_type='%2' and create_time >=%3 and create_time < %4")\
                .arg(strUserIdTmp).arg(strCoinTypeTmp).arg(uTimestamp00).arg(uTimestamp24);

        qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql: " << strSqlTmp;

        sqlTmp.prepare(strSqlTmp);
        const int iTry = 5;
        for(int i = 0; i < iTry; i++)
        {
            if(sqlTmp.exec()) break;
            if(iTry  == i + 1) //最后一次尝试
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "GetAllCoinTypeFromDB()  sql.exec() always failed.";
                orderData.iWay = WithdrawWay::WAY_MANUAL;
                return CAutoWithdraw::NO_ERROR;
            }
            msleep(200);
        }

        if(sqlTmp.size() > 0)
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("userId: %1 has '%2' withdraw order today. ").arg(strUserIdTmp).arg( sqlTmp.size());
        }
        else
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<
                        QString("userId: %1 did not has any '%2'  withdraw order yet today. So it's a valid auto-withdraw order.").arg(strUserIdTmp).arg(strCoinTypeTmp);

            //return true; //是有效的自动提币
            orderData.iWay = WithdrawWay::WAY_AUTO;
            orderData.iAuditStatus = AuditStatus::AUDIT_SUCCESS; //走全自动
            return CAutoWithdraw::NO_ERROR;
        }
    }



    QString strSql = QString("SELECT round(SUM(amount), 8) FROM tb_auto_withdraw where user_id='%1' and order_id != '%2' and coin_type='%3' and create_time >=%4 and create_time < %5")\
                      .arg(strUserIdTmp).arg(strOrderId).arg(strCoinTypeTmp).arg(uTimestamp00).arg(uTimestamp24);
    sql.prepare(strSql);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << "sql: " << strSql;

    const int iTry = 5;
    for(int i = 0; i < iTry; i++)
    {
        msleep(200);
        if(sql.exec()) break;
        if(iTry  == i + 1) //最后一次尝试
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "GetAllCoinTypeFromDB()  sql.exec() always failed.";
//            return false;
            orderData.iWay = WithdrawWay::WAY_MANUAL;
            return CAutoWithdraw::NO_ERROR;
        }
        msleep(200);
    }

    if(!sql.next()) //该用户没有历史提币信息
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << \
                    QString("GetAllCoinTypeFromDB() sql result is empty. maybe this userId: %1 did not has any '%2' withdraw order yet. so it's a valid auto-withdraw")
                    .arg(strUserId).arg(strCoinTypeTmp);
        //return true;
        orderData.iWay = WithdrawWay::WAY_AUTO;
        orderData.iAuditStatus = AuditStatus::AUDIT_SUCCESS; //走全自动
        return CAutoWithdraw::NO_ERROR;
    }

    bOk = false;
    double dUserDaySum = sql.value(0).toDouble(&bOk); //用户单日已提笔金额
    if(false == bOk)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dUserDaySum is not invalid float number.";
//        return false;
        orderData.iWay = WithdrawWay::WAY_MANUAL;
        return CAutoWithdraw::NO_ERROR;
    }

    bOk = false;
    double dDayAmountLimit = mapTmp.value("dDayMaxAmount").toDouble(&bOk);
    if(false == bOk)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dDayMaxAmount is not invalid float number.";
        orderData.iWay = WithdrawWay::WAY_MANUAL;
        return CAutoWithdraw::NO_ERROR;
        //return false;
    }

    //之前已提笔金额 + 本次提币金额,  如果依然小于 单日提币限制, 则符合自动提币规则
    if(dUserDaySum + dAmount <= dDayAmountLimit)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"<< QString::asprintf("%.8f + %.8f < %.8f ", dUserDaySum, dAmount, dDayAmountLimit)
                 << QString(", so this order:%1 is valid auto-withdraw order").arg(strOrderId);
//        return true;

        orderData.iWay = WithdrawWay::WAY_AUTO;
        orderData.iAuditStatus = AuditStatus::AUDIT_SUCCESS; // 全自动  设置为审核成功
        return CAutoWithdraw::NO_ERROR;
    }
    else //单日单币种累计提币金额超过限定额度,
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"<< QString::asprintf("%.8f + %.8f > %.8f ", dUserDaySum, dAmount, dDayAmountLimit)
             << QString(", so this order:%1 is invalid auto-withdraw order,it had been changed to manual withdraw.").arg(strOrderId);

        orderData.iWay = WithdrawWay::WAY_AUTO;
        orderData.iAuditStatus = AuditStatus::AUDIT_PENDING;// 设置为 '半自动'即可, 不必设置为人工
        return CAutoWithdraw::NO_ERROR;
    }

    return CAutoWithdraw::NO_ERROR;
}

bool CAutoWithdraw::__CheckUploadable(const OrderData &orderData) const noexcept(true)
{
    //只关心哪些要上传, 其他的都不上传
    if(WithdrawWay::WAY_AUTO == orderData.iWay) //自动提币的
    {
        //成功的
        if(!orderData.strTxid.isEmpty()  && OrderStatus::ORDER_SUCCESS == orderData.iOrderStatus )
        {
            return true;
        }

        //失败的
        if(OrderStatus::ORDER_FAILED == orderData.iOrderStatus || OrderStatus::ORDER_CLIENT_ERR == orderData.iOrderStatus \
                || AUDIT_FAILED == orderData.iAuditStatus) //客户端错误,比如金额错误等,新增审核失败的
        {
            return true;
        }

        //其他的就是未处理的,或者是因为服务器里面金额不足,暂时不能提币
    }
    else if(WithdrawWay::WAY_MANUAL == orderData.iWay) //人工提币
    {
        //审核失败的
        if(AuditStatus::AUDIT_FAILED == orderData.iAuditStatus)
        {
            return true;
        }

        //订单失败的
        if(OrderStatus::ORDER_FAILED == orderData.iOrderStatus || OrderStatus::ORDER_CLIENT_ERR == orderData.iOrderStatus)
        {
            return true;
        }

        //初审复审广播都成功的
        if(AuditStatus::AUDIT_SUCCESS == orderData.iAuditStatus  && !orderData.strTxid.isEmpty())
        {
            return true;
        }
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << orderData.strOrderId << "提币方式错误";
    }


    return false;
}



void  CAutoWithdraw::__WalletServerHealthStateMonitor() noexcept(false)
{
    if( 10 > __m_nWalletServerHealthStateMonitorCounter )
        return;

    QString  strEnvFlag = g_bBlockChainMainnet ? ("PRO") : ("SIT"); //环境标志
    QString strContent = tr("【Hetbi】尊敬的管理员，十万火急！%1，已导致用户提币失败，请火速处理，谢谢您的配合！(%3,%4)")
                                .arg("钱包服务端健康状况异常").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(strEnvFlag);
    QString strTelList = (g_bBlockChainMainnet) ? ("18565659593,18676702645"/*增加运维的手机号*/) : ("18565659593,15727780717"); //手机号码, 如果有多个手机号, 则用英文逗号分隔
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "send to "<< strTelList << "  " " sms content:" << strContent;
    CShortMsg::SendShortMsg(strTelList , strContent );

    __m_nWalletServerHealthStateMonitorCounter = 0;
}



int CAutoWithdraw::USDT_AutoWithdraw(OrderData &orderData) noexcept(false)
{
    //1.创建裸交易
    map<QString, QString> mapDstTx;
    USDTExportData usdtExportData;

    QString strAmount = QString::asprintf("%.8f", orderData.dAmount);
    mapDstTx.insert(std::make_pair(orderData.strDstAddr, strAmount));

    {
        USDTExportItem  tmpItem;
        tmpItem.vctSrcAddr.push_back(orderData.strSrcAddr);
        tmpItem.vctDstAddr.push_back(orderData.strDstAddr);
        tmpItem.bComplete = false;
        tmpItem.strOrderID = orderData.strOrderId;
        tmpItem.strTokenAmount = QString::asprintf("%.8f", orderData.dAmount);  //USDT的金额

        usdtExportData.vctExportItems.push_back(tmpItem);
    }

    //CreateRawTransaction 会抛出异常, 注意处理
    __m_usdtRawTxUtil.CreateRawTransaction(g_qstr_WalletHttpIpPort + STR_usdt_createrawtransaction, orderData.strSrcAddr, usdtExportData);



    //2.签名
    vector<QString> vctSrcAddr;
    vctSrcAddr.push_back(orderData.strSrcAddr);


    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value("usdt");
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    vector<QString> vctPrivKey;
    vctPrivKey.push_back(strTmpPrivKey);


    QString strSignedHex;
    USDTExportItem  const &usdtExportItem = usdtExportData.vctExportItems[0];
    __m_usdtOffSign.TxOfflineSignature(strSignedHex, orderData.dAmount, vctSrcAddr,
                                              usdtExportItem.mapTxOut, usdtExportItem.strTxHex,
                                              vctPrivKey, usdtExportItem.vctUTXOs);

    //3.广播交易
    // fix bug: 2020-03-30 yqq
    //fix_bug: 2020-03-30 yqq
    //如果是 USDT 则使用 USDT 自己的节点广播, 不然会导致 交易池的差异,
    //导致双花问题, 因为创建的时候使用USDT节点, 而广播的使用使用的是 USDT节点,
    //__m_usdtRawTxUtil.BroadcastRawTx(g_qstr_WalletAPI_btc_sendrawtransaction, strSignedHex, orderData.strTxid);
    __m_usdtRawTxUtil.BroadcastRawTx(g_qstr_WalletHttpIpPort + STR_usdt_sendrawtransaction, strSignedHex, orderData.strTxid);

    orderData.iOrderStatus = CAutoWithdraw::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "USDT_AutoWithdraw 成功返回";

    QThread::sleep(5);

    return NO_ERROR;
}



//2019-04-28 add by yqq
/*
 * 函数功能说明: 以太坊自动提币函数, 具体流程如下:
 *      1.从Java获取订单数据( 目的地址, 金额[以ether单位], 手续费 )
 *      2.存入数据库
 *      3.获取自动提币地址的私钥
 *      4.根据交易信息(chainid, nonce, from, to, value, gasprice, gasstart, privkey) 调用ETH裸交易工具类
 *         的 CreateRawTx() 接口创建签名交易(注意是已签名的! 不需要额外签名了!)
 *      5.将已签名交易, 通过服务端的交易广播接口广播出去, 获取txid , nonce值
 */
//TODO: 以太坊提币配置,从数据库中获取
int CAutoWithdraw::ETH_AutoWithdraw( OrderData &orderData)
{
    QString strErrMsg;

    //1.准备 DLL 接口调用参数
    rawtx::ETH_CreateRawTxParam param;

    //ETH 和 ETC 测试环境, 都是用 Rinkeby
    param.uChainId          = (g_bBlockChainMainnet) ? (eth::ETHChainID::Mainnet) : (eth::ETHChainID::Rinkeby);//TODO:仅Rinkeby测试网络, 迁移到主网时需进行切换
    if(g_bBlockChainMainnet && 0 == orderData.strCoinType.compare("ETC", Qt::CaseInsensitive))
    {
        param.uChainId      = eth::ETHChainID::EthereumClassic;   //ETC
    }

    param.strAddrFrom       = orderData.strSrcAddr.toStdString();       //from地址 , TODO:从数据库中获取


    //先判断余额
    rawtx::CETHRawTxUtilsImpl  ethUtil;
    std::map<QString, double> mapBalances;
    ethUtil.GetBalance(orderData.strSrcAddr, mapBalances,  orderData.strCoinType);

    QString strGasPrice = ethUtil.GetGasPrice(); //获取实时的gasPrice
    std::string cstrGasPrice = strGasPrice.toStdString();

    const double dMinEth = 0.05;
    double dBalance = mapBalances.at(orderData.strCoinType);
    double dSumValue = utils::IsERC20Token(orderData.strCoinType) ? (orderData.dAmount) : (orderData.dAmount + dMinEth) ;
    if(dBalance <  dSumValue )
    {
        orderData.strRemarks = QString("出币地址%1余额不足").arg(orderData.strCoinType);
        strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                       You selected 1 addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                       dSumValue, dBalance);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    //如果是ERC20 代币, 还需要判断 ETH的余额是否足够支付手续费
    if( utils::IsERC20Token(orderData.strCoinType) )
    {
        double dETHBalance = mapBalances.at("ETH");
        if(dETHBalance < dMinEth )
        {
            orderData.strRemarks = "出币地址ETH余额不足支付手续费";

            strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The ETH balance of source-address is not enough to pay tx-fee.\
                                      You selected 1 addresses to make ERC20 transaction. It totally need %.8fETH to pay transaction fee , \
                                        but ETH balance is %.8f ETH. Please get some ETH first.",  dMinEth , dETHBalance);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }
    }



    QString strTmpCoinType  = orderData.strCoinType;
    strTmpCoinType = strTmpCoinType.toLower();
    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(strTmpCoinType);
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    param.strPrivKey        =  strTmpPrivKey.toStdString(); //from地址的私钥,此私钥从数据库获取并解密

    if( utils::IsERC20Token( orderData.strCoinType ) )
    {
        param.float100Value     = 0;//金额, 如果是ERC20代币, 此值必须为 0
        param.strAddrTo         = QString(utils::GetERC20ContractAddr(orderData.strCoinType)).toStdString(); //目的地址-智能合约的地址, 以"0x"开头,共42字符
        param.ubigintGasPrice   = ubigint(cstrGasPrice);  // gasprice, 单位是wei,

        QString strGasLimit = utils::GetGasLimit(orderData.strCoinType);
        param.ubigintGasStart   = ubigint(strGasLimit.toStdString());   //gasStart 即 gasLimit


        //构造inputdata字段
        std::string cstrRetData = "";
        std::string cstrValue = "";
        int nDecimals = utils::GetERC20Decimals(orderData.strCoinType);
        rawtx::CETHRawTxUtilsImpl::ToWeiStr(orderData.dAmount, nDecimals, cstrValue);

        QString strValue = QString::fromStdString( cstrValue );
        rawtx::CETHRawTxUtilsImpl::MakeStdERC20TransferData(orderData.strDstAddr, strValue, cstrRetData);

        memcpy(param.szData, cstrRetData.c_str(), cstrRetData.size());
        param.uDataLen = cstrRetData.size();
    }
    else //ETH和ETC转账
    {
        param.float100Value     = float100(QString::asprintf("%.8f", orderData.dAmount).toStdString());//金额, 以 ether为单位  1 ether = 10^18 wei
        param.strAddrTo         = orderData.strDstAddr.toStdString();       //目的地址, 以"0x"开头,共42字符
        //param.szData             = NULL;                                     //附加数据, 为空即可
        param.uDataLen          = 0;                                        //附加数据的长度, 为0即可
        param.ubigintGasPrice   = ubigint(cstrGasPrice);                                // gasprice, 单位是wei, 默认值是1GWei , 其中1 GWei = 10^9 wei
                                                                            //可以通过接口获取: https://ethgasstation.info/json/ethgasAPI.json
        param.ubigintGasStart   = ubigint(STR_NORMAL_ETH_TX_GASLIMIT);//21000;      //gasStart, 单位是 wei, 默认值是 21000
    }


    ethUtil.GetNonce(QString(param.strAddrFrom.c_str()),  param.uNonce, orderData.strCoinType);
    orderData.uNonce = param.uNonce; //保存nonce值, 用于后期追加交易手续费
    param.strCoinType = orderData.strCoinType;

    //2.检查参数
    if(false == param.ParamsCheck(&strErrMsg))
    {
        orderData.iOrderStatus  = OrderStatus::ORDER_FAILED;
        orderData.iAuditStatus = AuditStatus::AUDIT_FAILED;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    //3.调用 DLL 接口, 创建签名交易
    ethUtil.CreateRawTx(&param);

    /*创建签名交易失败的原因:
         *   1.参数错误(金额超过 9999,  手续费太高(超过普通值的10倍)
         *   2.参数错误(地址格式不对, 私钥格式不对, chainId不对)
         *   3.等等...
         */

    //4.组装参数, 调用广播接口进行广播
    rawtx::ETH_BroadcastRawTxParam  bcastParam;
    bcastParam.strAddrFrom          = QString(param.strAddrFrom.c_str());
    bcastParam.strNonce             = QString::asprintf("%u", param.uNonce);
    bcastParam.uChainId             = param.uChainId;
    bcastParam.strSignedRawTxHex    = QString( rawtx::Bin2HexString(param.szRetBuf, param.uRetDataLen).c_str() );
    bcastParam.strOrderId           = orderData.strOrderId;

    bcastParam.strCoinType          = orderData.strCoinType;

    ethUtil.BroadcastRawTx( &bcastParam );

    /*广播失败的可能原因:
     *  1.发送地址余额不足
     *  2.手续费太低
     *  3.签名交易数据错误
     */


    //5.返回广播结果
    orderData.strTxid   = bcastParam.strTxid;
    orderData.uNonce    = param.uNonce;

    //更新订单状态
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"
                 << QString("BroadcastRawTx() return success: ") << "txid: " << bcastParam.strTxid
                 << " , nonce:" << bcastParam.strNonce << " , chainId:" << QString::asprintf("%d", bcastParam.uChainId)
                 << "  ,  from:" << bcastParam.strAddrFrom  << " to: " << QString( param.strAddrTo.c_str() )
                 << " , rawtx: " <<  bcastParam.strSignedRawTxHex;


    return CAutoWithdraw::NO_ERROR;
}


/*
 * 作者: yqq
 * 日期: 2019-04-29
 * 函数说明: 保存以太坊的交易nonce值,用于后期提高交易手续费重发交易,
 *          以覆盖原有的(低手续费)的交易
 */
int CAutoWithdraw::ETH_PushNonceIntoDB(OrderData &orderData) noexcept(false)
{
    QSqlQuery  sql(__m_db);
    QString strSql = QString("insert ignore into tb_eth_tx_nonce values('%1','%2','%3',%4);")\
            .arg(orderData.strOrderId).arg(orderData.strSrcAddr).arg(orderData.strTxid).arg(orderData.uNonce);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql is : `"  <<  strSql;
    sql.prepare(strSql);

    const int ciSqlTryMaxTime = 100;
    for(int i = 0; i < ciSqlTryMaxTime; i++ )
    {
        if(sql.exec()) break;
        msleep(100);

        if( ciSqlTryMaxTime - 1 == i) //最后一次尝试
        {
            QString strErrMsg = QString("sql.exec() error: %1").arg( sql.lastError().text() );
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            sql.clear();
            throw runtime_error( strErrMsg.toStdString() );
        }
        else
        {
            qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " sql.exec() failed. will try again.";
        }
    }

    qDebug() <<  "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql.exec() success,  insert eth'nonce success.";
    sql.clear();
    return CAutoWithdraw::NO_ERROR;
}



/*
 * 作者: yqq
 * 日期: 2019-04-29
 * 函数说明: 从数据库获取订单nonce值, 此函数用于那些已完成的交易(有txid的), 但是,
 *         因为手续费太低,以太坊网络拥塞, 交易迟迟未被打包, 导致用户提币很久都没到账,
 *         这时,如果用户投诉,则需要提高交易的手续费,用高于原来交易手续费 110% 重发交易,
 *        以覆盖老的交易, 提高交易被打包的概率
 * 注意: nonce要使用谨慎
 */
int CAutoWithdraw::ETH_GetNonceFromDB(OrderData &orderData) noexcept(false)
{
    QString strErrMsg = "";

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "start CAutoWithdraw::ETH_GetNonceFromDB()" ;
    QSqlQuery  sql(__m_db);

    QString strSql = QString("select nonce from tb_eth_tx_nonce where order_id='%1' and txid='%2';")
                     .arg(orderData.strOrderId.trimmed()).arg(orderData.strTxid.trimmed());

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "sql is:" << strSql;

    sql.prepare(strSql);

    const int ciSqlTryMaxTime = 100;
    for(int i = 0; i < ciSqlTryMaxTime; i++ )
    {
        if(sql.exec()) break;
        msleep(100);

        if( ciSqlTryMaxTime - 1 == i) //最后一次尝试
        {
            QString strErrMsg = QString("sql.exec() error: %1").arg( sql.lastError().text() );
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            sql.clear();
            throw runtime_error( strErrMsg.toStdString() );
        }
        else
        {
            qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << " sql.exec() failed. will try again.";
        }
    }

    //sql返回的数据集, 迭代器默认停在第1条数据之前, 必须是用 next() 来获取第一条数据
    if(!sql.next())
    {
        strErrMsg = "sql result is empty.";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }


    bool bOk = false;
    uint64_t  uTmpNonce = sql.value("nonce").toULongLong(&bOk);
    if(false == bOk)
    {
        strErrMsg = "convert sql result nonce to uint64_t  failed.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    if(ULONG_MAX == uTmpNonce)
    {
        strErrMsg = "the sql result `nonce` is  ULONG_MAX , to large, it's a error value for eth.";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error( strErrMsg.toStdString() );
    }

    orderData.uNonce = uTmpNonce;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("get order='%1', nonce='%2' success").arg(orderData.strOrderId).arg( QString::asprintf("%llu", orderData.uNonce));

    return CAutoWithdraw::NO_ERROR;
}



/* 作者 : yqq
 * 日期: 2019-11-22
 * 函数说明: 处理 ETH ERC20 余额不足的情况下, 比如,  两笔ERC20的提币订单同时拿到,
 * 自动出币的余额只能满足第一笔, 不能满足第二笔, 那么, 就会出现, 第二笔广播成功后, 因余额不足
 * 而交易失败, 影响用户提币
 *
 * 处理方式:
 *     1.一次性拿出所有的同币种的提币订单
 *     2.进行累加->nSum
 *     3.如果某自动的提币 + nSum > nBalance , 则直接将提币订单设置失败, 并备注余额不足
 *
 * 注意:  ETH和所有ERC20代币共用一个自动出币地址,  否则, 此函数会有问题
 */

int CAutoWithdraw::BalanceThresholdCheckAndSMSNotify(vector<OrderData> &vctOrderData)noexcept(false)
{
    //先判断余额
    //rawtx::CETHRawTxUtilsImpl  ethUtil;
    std::map<QString, double> mapBalances;
    std::map< QString, double > mapEachSum;
    std::map< QString, double > mapFeeSum;//TODO: Omni-USDT  和 所有 ERC20代币,  都需要检查 主链币的余额(手续费)
    std::map<QString, QString> mapSMSNotify; //发送短信通知的
    QString  strEnvFlag = g_bBlockChainMainnet ? ("PRO") : ("SIT"); //环境标志

    //手续费因子
    auto funcFeeFactor = [=](const QString &strCoinType) -> double{
        if(0 == strCoinType.compare("BTC", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("BCH", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("BSV", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("DASH", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("LTC", Qt::CaseInsensitive))
        {
            return  0.0001;
        }
        else if(0 == strCoinType.compare("ETH", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("ETC", Qt::CaseInsensitive))
        {
            return  0.03;
        }
        else if(0 == strCoinType.compare("USDP", Qt::CaseInsensitive)
                || 0 == strCoinType.compare("HET", Qt::CaseInsensitive) )
        {
            return 0.0002;
        }
        else if(0 == strCoinType.compare("HTDF", Qt::CaseInsensitive)  )
        {
            return 0.1; //实际  0.03
        }
        else if(0 == strCoinType.compare("XRP", Qt::CaseInsensitive))
        {
            return 0.000100;
        }
        else if(0 == strCoinType.compare("TRX", Qt::CaseInsensitive))
        {
            return 0.1;
        }
        return 0.0001;
    };

    //当前提币列表中的所有币种, 那些不在当前提币列表中的币种不获取余额
    std::set<QString>  setCurCoinType;
    for(const auto &order : vctOrderData)
    {
        setCurCoinType.insert(order.strCoinType);
    }

    //获取所有币种的余额
    __GetAllCoinBalance(setCurCoinType, mapBalances);

    for(auto &order : vctOrderData)
    {
        QString strCoinType = order.strCoinType.toUpper();
        if(WithdrawWay::WAY_AUTO != order.iWay) //不是自动的, 跳过
            continue;
        if(AuditStatus::AUDIT_FAILED == order.iAuditStatus)//审核失败的, 跳过
            continue;
        if(TxStatus::TX_SUCCESS == order.iTxStatus) //交易成功的,跳过
            continue;
        if(OrderStatus::ORDER_FAILED  == order.iOrderStatus || OrderStatus::ORDER_SUCCESS == order.iOrderStatus ) //订单已经失败或者成功的
            continue;

        if(AuditStatus::AUDIT_SUCCESS != order.iAuditStatus ) //半自动提币  如果尚未复审通过, 跳过  2020-04-24
            continue;



        //判断Omni-USDT出币地址的BTC余额, ERC20代币出币地址的ETH余额手续费
        //如果后面有TRC20代币, 则需要判断TRON的余额, 此是后话   2019-11-04  by yqq
        //增加 HRC20 代币  判断 HTDF余额  2020-05-12  by yqq
        if(0 == strCoinType.compare("USDT", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
            || utils::Is_HRC20_Token(strCoinType) )
        {
            double dFeeFactor = ( utils::IsERC20Token(strCoinType)  ) ? (0.03) : (0.0001);
            double dTempFeeSum = dFeeFactor;
            QString strMainCoinType = ( utils::IsERC20Token(strCoinType) ) ? ("ETH") : ("USDT_BTC"); //用于程序区分
            QString strShowMainCoinType = ( utils::IsERC20Token(strCoinType) ) ? ("ETH") : ("BTC"); //显示的给用户看的币种类型

            if( utils::Is_HRC20_Token(strCoinType) )
            {
                dFeeFactor = 0.1;
                dTempFeeSum = dFeeFactor;
                strMainCoinType =  "HTDF";
                strShowMainCoinType = "HTDF";
            }


            std::map< QString, double >::iterator itFeeSum = mapFeeSum.find(strMainCoinType);
            if( mapFeeSum.end() == itFeeSum  )
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "mapFeeSum.insert  " << strMainCoinType  ;
                mapFeeSum.insert( std::make_pair(strMainCoinType,  dTempFeeSum ) );
            }
            else
            {
                dTempFeeSum += itFeeSum->second;
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dTempFeeSum = " << dTempFeeSum;
            }

            std::map<QString , double>::iterator itMainBalance = mapBalances.find(strMainCoinType); //注意USDT的BTC余额是 USDT_BTC
            if(mapBalances.end()  == itMainBalance)
            {
                //没有这个币种的余额信息, 此处不处理, 让其他函数处理
                continue;
            }

            double balance = itMainBalance->second;
            msleep(137);

            if( dTempFeeSum >= balance )
            {
                //主币(手续费)余额不够, 则直接, 将这笔订单设置为失败
                //紧急通知管理员进行充币

                order.strRemarks = QString("%1手续费不足,已拒绝,已通知管理员充值").arg(strShowMainCoinType);
                order.iOrderStatus = OrderStatus::ORDER_FAILED;
                order.iAuditStatus = AuditStatus::AUDIT_FAILED;
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "主币余额不足以支付手续费,已将订单" << order.strOrderId <<"状态修改为失败";

                //需要发送短信通知管理员, 紧急充币
                QString strContent = tr("【Hetbi】尊敬的管理员，手续费不足！%1 的自动出币地址手续费余额 %2%3，已导致用户提币失败，请立即充值%4，谢谢您的配合！(%5,%6)")
                                    .arg(strCoinType).arg( balance ).arg(strShowMainCoinType).arg(strShowMainCoinType)
                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg( strEnvFlag);
                mapSMSNotify[ strMainCoinType ] = strContent; //如果已存在预警通知,则直接替换为紧急通知
                continue;
            }

            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "订单: " << order.strOrderId << " 的累计手续费未超出主币余额, 检查通过!";
            mapFeeSum[strMainCoinType] = dTempFeeSum;

            if(mapSMSNotify.end() == mapSMSNotify.find(strMainCoinType))
            {
                double dFeeThershold =  dFeeFactor * 400;
                if( balance <=  dFeeThershold )
                {
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "手续费余额预警: " << strMainCoinType << " 余额: " << balance << "小于预警值: " << dFeeThershold ;
                    QString strContent = tr("【Hetbi】尊敬的管理员，手续费余额预警! %1 的自动出币地址手续费余额 %2%3 已小于警戒值，请立即充值%4，谢谢您的配合！(%5,%6)")
                                .arg(strCoinType).arg( balance ).arg(strShowMainCoinType).arg(strShowMainCoinType)
                                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg( strEnvFlag );
                    mapSMSNotify.insert( std::make_pair(strCoinType, strContent) );
                }
            }

        }



        //正常的余额判断
        double dTmpSum = order.dAmount;
        std::map< QString, double >::iterator itFound = mapEachSum.find( strCoinType );
        if( mapEachSum.end() ==  itFound)
        {
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "mapEachSum.insert  " << strCoinType << " " << dTmpSum ;
            mapEachSum.insert( std::make_pair( strCoinType,  dTmpSum) );
        }
        else
        {
            dTmpSum += itFound->second;
            dTmpSum += funcFeeFactor(strCoinType); //手续费
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dTmpSum = " << dTmpSum;
        }


        std::map<QString , double>::iterator itBalance = mapBalances.find(strCoinType);
        if(mapBalances.end()  == itBalance)
        {
            //没有这个币种的余额信息, 此处不处理, 让其他函数处理
            continue;
        }

        double balance = itBalance->second;

        //2020-03-09  设置防火带,  防止因余额更新不及时, 导致ERC20 失败转账的情况
        double  dFireBreaks = 0.0;
        if(true)
        {
            QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(strCoinType.toLower());
            bool bOk = false;
            double dMaxAutoWithdawAmount = mapTmp.value("dMaxAutoWithdawAmount").toDouble(&bOk); //自动提币的最大数量
            if(!bOk)
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dMaxAutoWithdawAmount转换double失败! 值: " << dMaxAutoWithdawAmount;
            }
            else //转换成功
            {
                dFireBreaks = dMaxAutoWithdawAmount * 3;
            }


            //2020-08-25 特殊处理
            if(utils::IsERC20Token(strCoinType))
            {
                dFireBreaks = 1000;
            }
            else if(utils::Is_HRC20_Token(strCoinType))
            {
                dFireBreaks = 2000;
            }
            else if(0 == strCoinType.compare("HTDF", Qt::CaseInsensitive) || 0 == strCoinType.compare("HET", Qt::CaseInsensitive) )
            {
                dFireBreaks = 50000;
            }
            else
            {
                dFireBreaks = dFireBreaks / 30;
                if(dFireBreaks < 0.5) dFireBreaks = 0.5;
            }

            if(
            0 == strCoinType.compare("LINK", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("OMG", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("ELF", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("MCO", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("REP", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("FSN", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("CTXC", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("REQ", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("XMX", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("TNB", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("ABT", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("CVC", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("TNT", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("PPT", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("LOOM", Qt::CaseInsensitive)
            )
            {
                dFireBreaks = 60;
            }

            if(  0 == strCoinType.compare("MKR", Qt::CaseInsensitive) )
            {
                dFireBreaks = 0.2;
            }


        }






        msleep(500);
        if( dTmpSum >= balance - dFireBreaks )
        {
            //余额不够, 则直接, 将这笔订单设置为失败
            //紧急通知管理员进行充币

            order.strRemarks = "报警:余额不足,已拒绝,已通知管理员充值";
            order.iOrderStatus = OrderStatus::ORDER_FAILED;
            order.iAuditStatus = AuditStatus::AUDIT_FAILED;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "总金额超过余额,已将订单" << order.strOrderId << "修改为失败";

            //需要发送短信通知管理员, 紧急充币
            double dTmpBalance = balance - (dTmpSum - order.dAmount); //扣掉即将转出的
            dTmpBalance = (dTmpBalance > 0.0001) ? (dTmpBalance) : (0.00001);

            QString strContent = tr("【Hetbi】尊敬的管理员，十万火急！%1 的自动出币地址当前余额 %2，已导致用户提币失败，请火速充值，谢谢您的配合！(%3,%4)")
                                .arg(strCoinType).arg( dTmpBalance ).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(strEnvFlag);
            mapSMSNotify[strCoinType] = strContent; //如果已存在预警通知,则直接替换为紧急通知
            continue;
        }

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "订单: " << order.strOrderId << " 的累计金额未超出余额, 检查通过!";
        mapEachSum[ strCoinType ] = dTmpSum;


        //如果已有紧急通知, 则不再发预警通知
        //如果没有紧急通知, 则需要判断当前余额是小于预警值
        if(mapSMSNotify.end() == mapSMSNotify.find(strCoinType))
        {
            QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(strCoinType.toLower());
            bool bOk = false;
            double dMaxAutoWithdawAmount = mapTmp.value("dMaxAutoWithdawAmount").toDouble(&bOk); //自动提币的最大数量
            if(!bOk)
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "dMaxAutoWithdawAmount转换double失败! 值: " << dMaxAutoWithdawAmount;
            }
            else //转换成功
            {
                double dThreshold = dMaxAutoWithdawAmount * 6; //预警值 = 最大自动 * 6

                //2020-08-25特殊处理
                if(utils::IsERC20Token(strCoinType) || utils::Is_HRC20_Token(strCoinType) ||
                   0 == strCoinType.compare("HTDF", Qt::CaseInsensitive) || 0 == strCoinType.compare("HET", Qt::CaseInsensitive) )
                {
                   dThreshold = dFireBreaks * 6;
                }


                double dTmpBalance = balance - dTmpSum ; //扣掉即将转出的
                dTmpBalance = (dTmpBalance > 0.0001) ? (dTmpBalance) : (0.00001);

                if( dTmpBalance <= dThreshold )
                {

                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "余额预警: " << strCoinType << " 余额: " << balance << "小于预警值: " <<dThreshold ;
                    QString strContent = tr("【Hetbi】尊敬的管理员，%1 的自动出币地址当前余额 %2 已小于警戒值，请立即充币，谢谢您的配合！(%3,%4)")
                                .arg(strCoinType).arg( dTmpBalance ).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(strEnvFlag);
                    mapSMSNotify.insert( std::make_pair(strCoinType, strContent) );
                }
            }
        }
    }

    //结合余额预警功能, 余额低于预警值时就会通知管理员,所以,一般情况不会出现这中极端情况
    //但也不能排除这种极端情况
    //这种情况下, 已经出现用户提币失败了, 紧急发送短信给管理员
    QString strTelList = (g_bBlockChainMainnet) ? ("18565659593,18138857721") : ("18565659593,15727780717"); //手机号码, 如果有多个手机号, 则用英文逗号分隔
    for(auto  item :  mapSMSNotify)
    {
        try
        {
            QString strCoinType = item.first;


            qint64  nCurTimestamp =  QDateTime::currentSecsSinceEpoch();
            bool bSendFlag = false;
            auto itRecord = __m_mapSMSSentRecord.find(strCoinType);
            if( __m_mapSMSSentRecord.end() !=  itRecord)
            {
                qint64  nTimeGap = nCurTimestamp - itRecord->second;
                if( 30 * 60 < nTimeGap  && nTimeGap < nCurTimestamp /*防止整数上溢*/ )//30分钟,
                {
                    bSendFlag = true;
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "距离上一次"<< item.first << "的短信, 已大于限频时间, 本次需要发送短信.";
                }
                else
                {
                    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "距离上一次"<< item.first << "的短信, 小于限频时间, 本次不发送短信.";
                }
            }
            else
            {
                bSendFlag = true;
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  "未发现" << item.first << "的短信记录, 本次需要发送短信!";
            }


            if(bSendFlag)
            {
                qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "send to "<< strTelList << "  " << item.first <<" sms content:" << item.second;
                //if(g_bBlockChainMainnet) //生产环境才真正发短信, 测试环境只是打日志
                //{
                    CShortMsg::SendShortMsg(strTelList , item.second);
                //}

                __m_mapSMSSentRecord[strCoinType] = QDateTime::currentSecsSinceEpoch();
            }


            sleep(15);
        }
        catch(std::exception &e)
        {
            //异常处理
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString(e.what());
            sleep(40);
        }
        catch(...)
        {
            //异常处理
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<  "未知错误";
            sleep(40);
        }

    }


    return CAutoWithdraw::NO_ERROR;
}





//2019-05-12 added by yqq
/*
 * 作者: yqq
 * 日期: 2019-05-12
 *
 * 申明: 因USDP和HTDF都是基于Cosmos的SDK开发, 所以统称为Cosmos,
 *      以后如果没另行说明, 代码中Cosmos指的是 USDP或HTDF
 *
 * 函数功能说明:  Cosmos自动提币函数 , Cosmos 和 ETH 类似 ,
 *       1.从java后台获取提币订单
 *       2.存入数据库
 *       3.获取自动提币地址的私钥
 *       4.根据交易信息 (chainId, accountNumber, sequence, from, to, value, feeAmount, feeGas, privKey, meomo(为空即可) )
 *          调用 Cosmos裸交易工具类的 CreateRawTx() 接口实现创建并签名.得到广播数据(可以直接广播的数据, 不需要额外操作)
 *       5.广播已签名交易数据, 获取到txid, gasUsed(非必须, 如果需要), sequence(非必须, 如果需要则保存).
 *
 */
int CAutoWithdraw::COSMOS_AutoWithdraw(OrderData &orderData)
{
    QString strErrMsg;
     //1.组装创建裸交易并签名的交易的参数

    cosmos::CsRawTx  csRawTx;

    bool isHTDF = (0 == orderData.strCoinType.compare(STR_HTDF, Qt::CaseInsensitive));

    strcpy_s(csRawTx.szChainId, (g_bBlockChainMainnet)?(STR_MAINCHAIN):(STR_TESTCHAIN));
    strcpy_s(csRawTx.szFeeDenom, STR_SATOSHI);
    strcpy_s(csRawTx.szMemo, orderData.strOrderId.toStdString().c_str());
    strcpy_s(csRawTx.szMsgDenom, STR_SATOSHI);
    strcpy_s(csRawTx.szMsgFrom, orderData.strSrcAddr.toStdString().c_str());
    strcpy_s(csRawTx.szMsgTo, orderData.strDstAddr.toStdString().c_str());
    csRawTx.uMsgAmount          = (uint64_t)(orderData.dAmount * 100000000);  //注意整型溢出(经过计算, dAmount的上限远远大于总发行量)
    csRawTx.uGas                = (isHTDF) ? (30000) : 200000;
    csRawTx.uFeeAmount          = (isHTDF) ? (100) : (20); //使用 HTDF_2020版


    //获取 sequence 和 account_number
    rawtx::CCosmosRawTxUtilsImpl  cosUtil;
    rawtx::CosmosAccountInfo cosAccountInfo;
    cosAccountInfo.strCoinType = orderData.strCoinType.toLower().trimmed();
    cosAccountInfo.strAddress = orderData.strSrcAddr;

    cosUtil.GetAccountInfo( cosAccountInfo );


     //判断余额 , 必须满足   余额 > 转账金额 + 手续费
    if(!(cosAccountInfo.dBalance > orderData.dAmount + orderData.dTxFee)) //使用 usdp作为单位
    {
        //如果余额不足直接, 拒绝用户的提币
        orderData.iOrderStatus = OrderStatus::ORDER_FAILED;//OrderStatus::ORDER_CLIENT_ERR;
        orderData.iAuditStatus = AuditStatus::AUDIT_FAILED;
        orderData.strRemarks = "报警:自动出币地址余额不足";

        strErrMsg = QString("balance is not enough. current balance is %1 , less than %2")
                .arg(cosAccountInfo.dBalance).arg(orderData.dAmount + csRawTx.uFeeAmount);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;

        throw runtime_error(strErrMsg.toStdString());
    }
    csRawTx.uSequence      =  cosAccountInfo.uSequence;
    csRawTx.uAccountNumber =  cosAccountInfo.uAccountNumber;


    rawtx::Cosmos_CreateRawTxParam param;
    param.csRawTx = csRawTx;
    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(orderData.strCoinType.toLower());
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    param.strPrivKey = strTmpPrivKey; //自动提币地址私钥


    //2.检查参数

    if(false == param.ParamsCheck( &strErrMsg ))
    {
        orderData.iOrderStatus = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //3.调用接口创建
    cosUtil.CreateRawTx( &param );


    //4.组装参数, 调用广播接口进行广播
    rawtx::Cosmos_BroadcastRawTxParam  bcastParam;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.strChainId   = param.csRawTx.szChainId;
    bcastParam.strSequence  = QString::asprintf("%u", param.csRawTx.uSequence);
    bcastParam.strCoinType = orderData.strCoinType ;//param.csRawTx.szMsgDenom;
    bcastParam.strFrom = param.csRawTx.szMsgFrom;
    bcastParam.strTo = param.csRawTx.szMsgTo;
    bcastParam.strValue = QString::asprintf("%llu", orderData.dAmount * 100000000);//转为satoshi

    bcastParam.strSignedRawTxHex = param.strHexData; //广播数据

    if(false == bcastParam.ParamsCheck( &strErrMsg ))
    {
        orderData.iOrderStatus =  OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"bcastParam.ParamsCheck() error:" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    cosUtil.BroadcastRawTx( &bcastParam );


    //5.返回广播结果
    orderData.strTxid = bcastParam.strTxid;
    //orderData.uNonce = param.csRawTx.uSequence; //TODO : 如果需要保存,

    orderData.iOrderStatus  = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success:") << bcastParam.strErrMsg\
            << " txid: " << bcastParam.strTxid \
            << "  chainId:" << bcastParam.strChainId << "sequence: " << bcastParam.strSequence  \
            << "  from:"  << bcastParam.strFrom <<  "to: " << bcastParam.strTo \
            << " value: " << bcastParam.strValue << "  rawtx: " << bcastParam.strSignedRawTxHex ;


    return CAutoWithdraw::NO_ERROR;
}



/*
 * 作者: yqq
 * 日期: 2019-11-27
 * 说明:
 *      获取所有币种的余额
 */
int CAutoWithdraw::__GetAllCoinBalance(const std::set<QString> &setCurCoinTypes, std::map<QString, double> &mapAllCoinBalances) noexcept(false)
{
    //获取 ETH 和 ERC20代币的余额
    {
        QMap<QString, QVariant> mapTmpValue  = __m_qmapAllCoinConfig.value("eth");
        QString strSrcAddr = mapTmpValue.value("srcAddr").toString();
        QString strAutoAddr = strSrcAddr;
        rawtx::CETHRawTxUtilsImpl().GetBalance(strAutoAddr, mapAllCoinBalances, "ETH");
    }

    //获取 HTDF 和 HRC20 代币的余额
    {
        QMap<QString, QVariant> mapTmpValue  = __m_qmapAllCoinConfig.value("htdf");
        QString strSrcAddr = mapTmpValue.value("srcAddr").toString();
        QString strAutoAddr = strSrcAddr;
        rawtx::CCosmosRawTxUtilsImpl().GetBalance_HRC20(strAutoAddr, mapAllCoinBalances);
    }


    auto funcGetBalance = [this, &mapAllCoinBalances, &setCurCoinTypes](QString uri, QString cointype, QString balancetype){

        //仅获取当前有提币订单的币种 自动出币地址余额, 2020-02-21  yqq
        if(  setCurCoinTypes.end() != setCurCoinTypes.find(cointype.toUpper())
            || setCurCoinTypes.end() !=  setCurCoinTypes.find( cointype.toLower() ) )
        {
            if( __m_qmapAllCoinConfig.end() != __m_qmapAllCoinConfig.find(cointype.toLower()) )
            {
                QMap<QString, QVariant> mapTmpValue  = __m_qmapAllCoinConfig.value(cointype.toLower());
                QString strSrcAddr = mapTmpValue.value("srcAddr").toString();
                QString strAutoAddr = strSrcAddr;
                double dBalance = 0.0;
                __GetBalance(g_qstr_WalletHttpIpPort + uri, strAutoAddr, dBalance);
                mapAllCoinBalances.insert( std::make_pair( balancetype,  dBalance) );
            }
            else //2020-01-14 如果币种尚未添加,直接将余额设置为 0.0001 , 防止余额监控异常退出
            {
                mapAllCoinBalances.insert( std::make_pair( balancetype,  0.0001) );
            }
        }
    };

    funcGetBalance(STR_btc_getbalance, "BTC", "BTC");
    funcGetBalance(STR_usdt_getbalance, "USDT", "USDT");
    funcGetBalance(STR_usdt_btc_getbalance, "USDT", "USDT_BTC"); //获取USDT地址的BTC余额
    funcGetBalance(STR_bch_getbalance, "BCH", "BCH");
    funcGetBalance(STR_ltc_getbalance, "LTC", "LTC");
    funcGetBalance(STR_dash_getbalance, "DASH", "DASH");
    funcGetBalance(STR_bsv_getbalance, "BSV", "BSV");
    funcGetBalance(STR_usdp_getbalance, "USDP", "USDP");
    funcGetBalance(STR_het_getbalance, "HET", "HET");
    funcGetBalance(STR_xrp_getbalance, "XRP", "XRP");
    funcGetBalance(STR_eos_getbalance, "EOS", "EOS");
    funcGetBalance(STR_etc_getbalance, "ETC", "ETC"); //ETC直接获取余额, 不考虑代币
    funcGetBalance(STR_xlm_getbalance, "XLM", "XLM");
    funcGetBalance(STR_trx_getbalance, "TRX", "TRX"); //如果后期有 TRC20-USDT, 再说


    return CAutoWithdraw::NO_ERROR;
}


/**
 * @brief CAutoWithdraw::__GetBalance
 * @param strURL
 * @param strSrcAddr
 * @param dBalance
 * @return
 */
int CAutoWithdraw::__GetBalance(const QString &strURL, const QString &strSrcAddr, double &dBalance) noexcept(false)
{
    QString strErrMsg = "";
    //参数检查
    if(strURL.isEmpty() || strSrcAddr.isEmpty())
    {
        strErrMsg = QString("internal error: strURL or strSrcAddr is empty");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    //请求http数据
    QByteArray byteArrayRsp;
    QString strDetailUrl = strURL + "address=" + strSrcAddr;
    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "GetBalance with strURL: " << strDetailUrl;


    QUrl url = strDetailUrl;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    SET_HTTPS_SSL_CONFIG(req);

    qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求链接: " << url;
    QNetworkAccessManager  qnetMng;
    QNetworkReply *pReply = qnetMng.get(req);

    //异步转为同步
    ASYNC_TO_SYNC_CAN_THROW(pReply);

    byteArrayRsp = pReply->readAll();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArrayRsp;
    pReply->deleteLater();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求成功";




    //获取交易ID
    QJsonParseError error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byteArrayRsp, &error));
    if(error.error != QJsonParseError::NoError)
    {
        strErrMsg = "json parse error:" + error.errorString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]"  << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }


    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("success") || !rootObj.contains("result") )
    {
        strErrMsg = "response json missing `success` or `result` ";
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    if("false" == rootObj.value("success").toString())
    {
        strErrMsg = rootObj.value("result").toString();
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    QString strBalance = rootObj.value("result").toString();
    std::string cstrBalance = strBalance.toStdString();
    dBalance = boost::lexical_cast<double>(cstrBalance);
    return NO_ERROR;
}



/* 2019-12-13  by yqq
 *
 * XRP 自动提币(创建交易的同时并用本地私钥签名, 然后并广播)
 *
 * 注意点:
 *    1. Java传来的XRP目的地址格式:    目的地址_标签
 *       在解析订单的数据函数中并未进行分割处理, 仅进行了合法性验证
 *      所以, 在构造创建交易的参数时, 需要再进行一次分割
 *
 *    2.Java传过来的金额是  浮点类型,  在构造创建交易的参数时,
 *     需要进行  单位换算  , 例如:
 *           1 XRP ---> 1000000
 *        0.1 XRP --->  100000
 *
*/
int CAutoWithdraw::XRP_AutoWithdraw( OrderData &orderData ) noexcept(false)
{
    QString strErrMsg;

    //1.组装参数
    rawtx::Xrp_CreateRawTxParam  param;
    param.bSigned = false;
    param.strChainNet = STR_XRP_CHAINNET;
    param.strSrcAddr = orderData.strSrcAddr.trimmed();
    param.strSignedRawTx = "";

    //金额
    {
        param.dAmountInXRP =  orderData.dAmount; //XRP
        uint64_t uAmountInMicro = (uint64_t)(orderData.dAmount * 1000000);
        param.strAmountInDrop = QString::asprintf("%llu", uAmountInMicro);//微XRP (1XRP = 10^6 微XRP)
    }


    //获取私钥, 禁止打印明文私钥 或 密码私钥!!
    {
        QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(orderData.strCoinType.toLower());
        param.strPrivKey        =  mapTmp.value("srcPrivKey").toString(); //from地址的私钥,此私钥从数据库获取并解密
    }

    //获取  地址, 标签
    {
        QString strTmpAddr_Tag = orderData.strDstAddr;
        param.strDstAddr = utils::GetXrpAddrFrom_Addr_Tag(strTmpAddr_Tag); //截取地址
        QString strTmpDstTag = strTmpAddr_Tag.mid( strTmpAddr_Tag.indexOf('_') + 1); //截取标签
        param.uDestinationTag = boost::lexical_cast<uint64_t>(strTmpDstTag.toStdString());
    }

    //获取sequence 和 balance
    rawtx::CXrpRawTxUtilsImpl xrpUtil;
    rawtx::XrpAccountInfo  accInfo;
    accInfo.strAddress = param.strSrcAddr; //源地址
    xrpUtil.GetAccountInfo(accInfo);
    double dSumAmountInXRP = orderData.dAmount + 0.000100;
    if(accInfo.dBalanceInXRP < dSumAmountInXRP)
    {
        orderData.strRemarks = QString("%1出币地址余额不足").arg(orderData.strCoinType);
        strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                       You selected 1 addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                       dSumAmountInXRP, accInfo.dBalanceInXRP);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    param.uFeeInDrop = UINT_XRP_DEFAULT_FEE_IN_DROP; // 默认的即可
    param.uSequence = accInfo.uSequqnce;



    //2.调用裸交易工具类的接口进行交易创建
    if(false == param.ParamsCheck(&strErrMsg))
    {
        orderData.iOrderStatus  = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    xrpUtil.CreateRawTx( &param );


    //3.进行广播
    rawtx::Xrp_BroadcastRawTxParam  bcastParam;
    bcastParam.dAmountInXRP = param.dAmountInXRP;
    bcastParam.strAmountInDrop = param.strAmountInDrop;
    bcastParam.strChainNet = param.strChainNet;
    bcastParam.strCoinType = orderData.strCoinType;
    bcastParam.strDstAddr  = param.strDstAddr;
    bcastParam.uDestinationTag = param.uDestinationTag;
    bcastParam.uSequence = param.uSequence;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.strSrcAddr = param.strSrcAddr;

    //签名后的交易
    bcastParam.strSignedRawTx = param.strSignedRawTx;

    xrpUtil.BroadcastRawTx( &bcastParam );


    //4.返回广播结果
    orderData.strTxid = bcastParam.strTxid;
    orderData.uNonce = bcastParam.uSequence;

    //更新订单状态
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");


     qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ")
                    << " , orderId:" << bcastParam.strOrderId << "txid: " << bcastParam.strTxid
                    << " , sequence:" << bcastParam.uSequence << " , destinationTag:" << QString::asprintf("%d", bcastParam.uDestinationTag)
                     << "  ,  from:" << bcastParam.strSrcAddr  << " to: " << QString( bcastParam.strDstAddr )
                     << " , rawtx: " <<  bcastParam.strSignedRawTx;


    return NO_ERROR;
}






/* 2019-12-31   by yqq
 *
 * EOS 自动提币 (调用服务端接口创建交易-->本地签名-->广播交易)
 *
 * 注意点:
 *   1. Java传来的 EOS 目的地址格式:    目的地址_标签
 *       在解析订单的数据函数中并未进行分割处理, 仅进行了合法性验证
 *      所以, 在构造创建交易的参数时, 需要再进行一次分割
 *
 *   TODO: 2.需要判断出币地址的资源是否足够?
*/
int CAutoWithdraw::EOS_AutoWithdraw( OrderData &orderData ) noexcept(false)
{
    QString strErrMsg;


    //1.组装参数
    rawtx::Eos_CreateRawTxParam  param;
    param.bSigned           = false;
    param.strCoinType       = orderData.strCoinType;
    param.strChainNet       = STR_EOS_CHAINNET;
    param.strSrcAcct        = orderData.strSrcAddr.trimmed();
    param.strTxDigestHex    = "";
    param.strSignedRawTx    = "";
    param.dAmount           = orderData.dAmount;
    param.strAmount     = QString::asprintf("%.4f", orderData.dAmount);

    //地址和 memo
    param.strDstAcct        = utils::GetXrpAddrFrom_Addr_Tag( orderData.strDstAddr );
    param.strMemo           = utils::GetTagFrom_Addr_Tag( orderData.strDstAddr );


    //检查余额和资源是否足够
    rawtx::CEosRawTxUtilsImpl eosUtil;
    rawtx::EosAccountInfo  accInfo;
    accInfo.strAccountName  = param.strSrcAcct;
    eosUtil.GetAccountInfo( accInfo );

    if(  accInfo.dEOSBalance < param.dAmount )
    {
        orderData.strRemarks = QString("%1出币地址余额不足").arg(orderData.strCoinType);
        strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                       You selected 1 addresses to make transaction. It totally need %.4f , but balance is %.4f.",
                                       param.dAmount , accInfo.dEOSBalance);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    try
    {
        if(  accInfo.uCPU_Avail < 1000 )
        {
            orderData.strRemarks = QString("%1出币地址CPU资源不足").arg(orderData.strCoinType);
            strErrMsg = QString::asprintf("CPU NOT ENOUGH: The CPU time of source-address is not enough. CPU_total: %1, \
                                            CPU_used: %2 , CPU_avail:%3", accInfo.uCPU_Total, accInfo.uCPU_Used , accInfo.uCPU_Avail);

            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString() );
        }

        if( accInfo.uNET_Avail < 1000 )
        {
            orderData.strRemarks = QString("%1出币地址NET资源不足").arg(orderData.strCoinType);
            strErrMsg = QString::asprintf("CPU NOT ENOUGH: The NET  of source-address is not enough. NET_total: %1, \
                                            NET_used: %2 , NET_avail:%3", accInfo.uNET_Total, accInfo.uNET_Used , accInfo.uNET_Avail);

            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString() );
        }

        if( accInfo.uRAM_Total - accInfo.uRAM_Used  < 4000 )
        {
            orderData.strRemarks = QString("%1出币地址RAM资源不足").arg(orderData.strCoinType);
            strErrMsg = QString::asprintf("RAM NOT ENOUGH: The RAM of source-address is not enough. RAM_total: %1, \
                                            RAM_used: %2 , RAM_avail:%3", accInfo.uRAM_Total, accInfo.uRAM_Used , accInfo.uRAM_Total - accInfo.uRAM_Used);

            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString() );
        }
    }
    catch(std::exception &e)
    {
         QString  strEnvFlag = g_bBlockChainMainnet ? ("PRO") : ("SIT"); //环境标志
         QString strContent = tr("【Hetbi】尊敬的管理员，十万火急！%1，已导致用户提币失败，请火速处理，谢谢您的配合！(%3,%4)")
                                .arg(orderData.strRemarks).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(strEnvFlag);
         QString strTelList = (g_bBlockChainMainnet) ? ("18565659593,18138857721") : ("18565659593,15727780717"); //手机号码, 如果有多个手机号, 则用英文逗号分隔
         qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "send to "<< strTelList << "  " " sms content:" << strContent;
         CShortMsg::SendShortMsg(strTelList , strContent );
         throw e;
    }


    //2.调用裸交易工具类进行交易创建
    if( false == param.ParamsCheck(&strErrMsg) )
    {
        orderData.iOrderStatus  = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    eosUtil.CreateRawTx( &param );



    //3.签名交易
    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(orderData.strCoinType.toLower());
    QString strPrivKey  =  mapTmp.value("srcPrivKey").toString(); //from地址的私钥,此私钥从数据库获取并解密
    QString  strSignature = "";
    eosUtil.SignTxDigestWithPrivKey(param.strTxDigestHex, strPrivKey, strSignature);

    //将 签名结果  嵌入   param.strSignedRawTx 中的 signatures:[] 数组中
    QString strEmbedSigRawTxJson = param.strSignedRawTx;
    if(strEmbedSigRawTxJson.contains("\"signatures\": []"))
    {
        strEmbedSigRawTxJson.replace("\"signatures\": []",  "\"signatures\": [\"" + strSignature + "\"]");
    }
    else if( strEmbedSigRawTxJson.contains("\"signatures\":[]"))
    {
        strEmbedSigRawTxJson.replace("\"signatures\":[]",  "\"signatures\": [\"" + strSignature + "\"]");
    }



    //4.广播交易
    rawtx::Eos_BroadcastRawTxParam   bcastParam;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.dAmount = param.dAmount;
    bcastParam.strAmount = param.strAmount;
    bcastParam.strChainNet = param.strChainNet;
    bcastParam.strCoinType = param.strCoinType;
    bcastParam.strSrcAcct = param.strSrcAcct;
    bcastParam.strDstAcct = param.strDstAcct;
    bcastParam.strMemo = param.strMemo;
    bcastParam.strTxDigestHex = param.strTxDigestHex;

    bcastParam.strSignedRawTx = strEmbedSigRawTxJson;//param.strSignedRawTx;
    bcastParam.strTxid = "";

    eosUtil.BroadcastRawTx( &bcastParam );


    //5.返回广播结果
    orderData.strTxid = bcastParam.strTxid;

    //更新订单状态
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");


     qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ")
                    << " , orderId:" << bcastParam.strOrderId << "txid: " << bcastParam.strTxid
                     << " , memo:" << bcastParam.strMemo
                     << "  ,  from:" << bcastParam.strSrcAcct << " to: " << bcastParam.strDstAcct
                     << " , rawtx: " <<  bcastParam.strSignedRawTx;

    return NO_ERROR;
}


/* 2020-02-12  by yqq
 *
 *  XLM 自动提币 ( 调用服务端接口获取  源账户的Sequence 和 balance  --> 本地创建并签名-->广播)
 *
 * 注意点:
 *  1. 需要考虑 目的账户 是否存在,  如果已经存在则使用 paymentoperation, 如果不存在则使用 createaccountoperation
 *
*/
int CAutoWithdraw::XLM_AutoWithdraw( OrderData &orderData ) noexcept(false)
{
    QString strErrMsg;

    //1.组装参数
    rawtx::Xlm_CreateRawTxParam  param;
    param.bSigned = false;
    param.strChainNet = STR_XLM_CHAINNET;
    param.strSrcAddr = orderData.strSrcAddr.trimmed();
    param.strSignedRawTx = "";

    //金额
    {
        param.dAmount =  orderData.dAmount;
        param.strAmount = QString::asprintf("%.7f", param.dAmount );
    }


    //获取私钥, 禁止打印明文私钥 或 密码私钥!!
    {
        QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(orderData.strCoinType.toLower());
        param.strPrivKey        =  mapTmp.value("srcPrivKey").toString(); //from地址的私钥,此私钥从数据库获取并解密
    }

    //获取  地址, 标签
    {
        QString strTmpAddr_Tag = orderData.strDstAddr;
        param.strDstAddr = utils::GetXrpAddrFrom_Addr_Tag(strTmpAddr_Tag); //截取地址
        QString strMemo = strTmpAddr_Tag.mid( strTmpAddr_Tag.indexOf('_') + 1); //截取标签
        param.strMemo = strMemo;
    }


    //过去sequence 和 balance , 以及  账户是否存在标志
    rawtx::CXlmRawTxUtilsImpl   xlmUtil;
    rawtx::XlmAccountInfo  accInfo;
    accInfo.strAddress = param.strSrcAddr;
    xlmUtil.GetAccountInfo(accInfo);

    double dSumAmount = orderData.dAmount + 0.0001;
    if(accInfo.dBalance < dSumAmount)
    {
        orderData.strRemarks = QString("%1出币地址余额不足").arg(orderData.strCoinType);
        strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                       You selected 1 addresses to make transaction. It totally need %.8f , but balance is %.8f.",
                                       dSumAmount, accInfo.dBalance);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    param.uBaseFee = 100; // 默认的即可
    param.uSequence = accInfo.uSequence;

    //需要获取目的账户是否存在
    rawtx::XlmAccountInfo  dstAccInfo;
    dstAccInfo.strAddress =  param.strSrcAddr;
    xlmUtil.GetAccountInfo( dstAccInfo );
    param.bDstAccountExists  = dstAccInfo.bFound;


    //2.调用裸交易工具类的接口进行交易创建
    if(false == param.ParamsCheck(&strErrMsg))
    {
        orderData.iOrderStatus  = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    xlmUtil.CreateRawTx( &param );


    //3.进行广播
    rawtx::Xlm_BroadcastRawTxParam  bcastParam;
    bcastParam.dAmount = param.dAmount;
    bcastParam.strAmount = param.strAmount;
    bcastParam.strChainNet = param.strChainNet;
    bcastParam.strCoinType = orderData.strCoinType;
    bcastParam.strDstAddr  = param.strDstAddr;
    //bcastParam.strMemo = param.strMemo;
    bcastParam.uSequence = param.uSequence;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.strSrcAddr = param.strSrcAddr;

    //签名后的交易
    bcastParam.strSignedRawTx = param.strSignedRawTx;

    xlmUtil.BroadcastRawTx( &bcastParam );


    //4.返回广播结果
    orderData.strTxid = bcastParam.strTxid;
    orderData.uNonce = bcastParam.uSequence;

    //更新订单状态
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");


     qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ")
                    << " , orderId:" << bcastParam.strOrderId << "txid: " << bcastParam.strTxid
                    << " , sequence:" << bcastParam.uSequence << " ,memo:" <<  param.strMemo
                     << "  ,  from:" << bcastParam.strSrcAddr  << " to: " << QString( bcastParam.strDstAddr )
                     << " , rawtx: " <<  bcastParam.strSignedRawTx;



    return NO_ERROR;
}






/* 2020-03-05  by yqq
 *
 *  TRX 自动提币  ( 调用服务端接口创建交易--->本地签名--->广播)
 *
 *  注意:
 *      1.如果目的账户不存在,  则需要消耗源账户 0.1 TRX 作为创建账户的手续费
 *      2.如果源账户的可用带宽资源不足, 则需要消耗 TRX , 消耗TRX的数量和交易的大小成正比
 *
 */

int CAutoWithdraw::TRX_AutoWithdraw( OrderData &orderData ) noexcept(false)
{
    QString strErrMsg;

    //1.组装参数
    rawtx::Trx_CreateRawTxParam   param;
    param.bSigned           = false;
    param.strCoinType       = orderData.strCoinType;
    param.strSrcAcct        = orderData.strSrcAddr.trimmed();
    param.strTxid           = "";
    param.strSignedRawTx    = "";
    param.dAmount           = orderData.dAmount;
    param.strAmount         = QString::asprintf("%.6f", orderData.dAmount);
    param.strDstAcct        = orderData.strDstAddr;


    //检查自动出币地址的  余额是否足够
    rawtx::CTrxRawTxUtilsImpl  trxUtil;
    rawtx::TrxAccountInfo  trxAccInfo;

    trxAccInfo.strAddress = param.strSrcAcct;
    trxUtil.GetAccountInfo( trxAccInfo );

    if( trxAccInfo.dBalance < param.dAmount )
    {
        orderData.strRemarks = QString("%1出币地址余额不足").arg(orderData.strCoinType);
        strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                       You selected 1 addresses to make transaction. It totally need %.6f , but balance is %.6f.",
                                       param.dAmount , trxAccInfo.dBalance);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    //2.调用裸交易工具类进行交易创建
    if( false == param.ParamsCheck(&strErrMsg) )
    {
        orderData.iOrderStatus  = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    trxUtil.CreateRawTx( &param );

    //3.签名交易
    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(orderData.strCoinType.toLower());
    QString strPrivKey  =  mapTmp.value("srcPrivKey").toString(); //from地址的私钥,此私钥从数据库获取并解密
    QString  strSignature = "";
    trxUtil.SignTxidWithPrivKey(param.strTxid, strPrivKey, strSignature);

    //将签名嵌入   param.strSignedRawTx 中的 signatures:[] 数组中对应的位置
    // 设计的占位符:   "signatures":["this_is_placeholder_for_signature"]

    const QString strPlaceholder = "this_is_placeholder_for_signature";
    QString   strEembedSigRawTrxJson = param.strSignedRawTx;
    if( ! strEembedSigRawTrxJson.contains( strPlaceholder ) )
    {
        strErrMsg = QString("raw tx json does not contains \"this_is_placeholder_for_signature\"");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    strEembedSigRawTrxJson.replace( strPlaceholder,  strSignature );



    //4.广播交易
    rawtx::Trx_BroadcastRawTxParam   bcastParam;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.dAmount = param.dAmount;
    bcastParam.strAmount = param.strAmount;
    bcastParam.strCoinType = param.strCoinType;
    bcastParam.strSrcAcct = param.strSrcAcct;
    bcastParam.strDstAcct = param.strDstAcct;
    bcastParam.strTxid = param.strTxid ;  //注意: 这是本地事先计算好的的txid,

    // 注意: 这是存放 服务端返回的 txid,  注意月  strTxid 的区分,
    //如果服务端返回的和 strTxid 不同, 以服务端返回的为准, 这里考虑了重复出账的情况
    bcastParam.strSuccessedTxid = "";

    bcastParam.strSignedRawTx = strEembedSigRawTrxJson; //已经嵌入交易签名的json字符串

    trxUtil.BroadcastRawTx( &bcastParam );


     //5.返回广播结果
    orderData.strTxid = bcastParam.strSuccessedTxid;

    //更新订单状态
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");

    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ")
                    << " , orderId:" << bcastParam.strOrderId << "txid: " << bcastParam.strSuccessedTxid
                     << "  ,  from:" << bcastParam.strSrcAcct << " to: " << bcastParam.strDstAcct
                     << " , rawtx: " <<  bcastParam.strSignedRawTx;


    return NO_ERROR;
}



/*2020-04-07  by yqq
 *
 * XMR  自动提币
 *
 * 步骤:
 *   1) hot wallet   exporttxoutput   导出交易输出
 *   2) cold wallet  importtxoutput    导入交易输出
 *   3) cold wallet  exportkeyimages   导出已签名 key images
 *   4) hot wallet  importkeyimages   导入已签名 key images
 *   5) hot wallet  createrawtransaction  创建交易
 *   6) cold wallet  signtx    签名交易
 *   7) hot wallet  sendrawtransaction  广播交易
 *
 */
int CAutoWithdraw::XMR_AutoWithdraw( OrderData &orderData ) noexcept(false)
{
    QString strErrMsg;

    rawtx::CXmrRawTxUtilsImpl  xmrUtil;

    // 1) hot wallet   exporttxoutput   导出交易输出
    QString strOutputDataHex = "";
    QString strReqUrl =  g_qstr_WalletHttpIpPort + STR_xmr_auto_exporttxoutputs;
    xmrUtil.ExportTxOutputs( strReqUrl, strOutputDataHex );

    //2) cold wallet  importtxoutput    导入交易输出
    xmrUtil.ImportTxOutputs(strOutputDataHex);

    // 3) cold wallet  exportkeyimages   导出已签名 key images
    QString strSignedKeyImages = "";
    xmrUtil.ExportKeyImages(strSignedKeyImages);

    // 4) hot wallet  importkeyimages   导入已签名 key images
    double dAvailBalance = 0.0; //获取可用余额
    strReqUrl = g_qstr_WalletHttpIpPort + STR_xmr_auto_importkeyimages;
    xmrUtil.ImportKeyImages(strReqUrl, strSignedKeyImages, dAvailBalance);


    //获取真实余额 , (unlocked_balance  , locking_balance )
    rawtx::XMR_CreateRawTxParam   param;
    param.bIsManualWithdraw = false;
    param.bSigned           = false;
    param.strCoinType       = orderData.strCoinType;
    param.strSrcAddr        = orderData.strSrcAddr.trimmed();
    param.strDstAddr        = orderData.strDstAddr;
    param.dAmount           = orderData.dAmount;
    param.strAmount         = QString::asprintf("%.8f", orderData.dAmount);
    param.strSignedRawTx    = "";

    if( dAvailBalance < param.dAmount + 0.1 )
    {
        orderData.strRemarks = QString("%1出币地址余额不足").arg(orderData.strCoinType);
        strErrMsg = QString::asprintf("BALANCE NOT ENOUGH: The balance of source-address is not enough.\
                                       You selected 1 addresses to make transaction. It totally need %.6f(0.1 for txfee) , but balance is %.6f.",
                                       param.dAmount + 0.1 , dAvailBalance);

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

     //调用裸交易工具类进行交易创建
    if( false == param.ParamsCheck(&strErrMsg) )
    {
        orderData.iOrderStatus  = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
        throw runtime_error(strErrMsg.toStdString() );
    }

    //5) 创建交易
    xmrUtil.CreateRawTx( &param );

    //6) 签名交易
    QString strSignedTxHex = "";
    xmrUtil.SignTx( param.strSignedRawTx , strSignedTxHex);


    //7) 广播交易
    rawtx::XMR_BroadcastRawTxParam   bcastParam;
    bcastParam.bIsManualWithdraw = false;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.dAmount = param.dAmount;
    bcastParam.strAmount = param.strAmount;
    bcastParam.strCoinType = param.strCoinType;
    bcastParam.strSrcAddr = param.strSrcAddr;
    bcastParam.strDstAddr = param.strDstAddr;
    bcastParam.strTxid = "";

    bcastParam.strSignedRawTx = strSignedTxHex; //已签名交易

    xmrUtil.BroadcastRawTx( &bcastParam );


    //8) 处理广播返回数据
    orderData.strTxid = bcastParam.strTxid;

    //更新订单状态
    orderData.iOrderStatus = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");

    qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success: ")
                    << " , orderId:" << bcastParam.strOrderId << "txid: " << bcastParam.strTxid
                     << "  ,  from:" << bcastParam.strSrcAddr << " to: " << bcastParam.strDstAddr
                     << " , rawtx: " <<  bcastParam.strSignedRawTx;


    return NO_ERROR;
}

int CAutoWithdraw::HRC20_AutoWithdraw( OrderData &orderData ) noexcept(false)
{
    QThread::msleep(7 * 1000); //休眠一下, 避免与HTDF交易冲突
    QString strErrMsg;

    //检查  HRC20 打币余额是否足够
    rawtx::CCosmosRawTxUtilsImpl  cosUtil;
    std::map<QString , double> mapBalancesOut;
    cosUtil.GetBalance_HRC20( orderData.strSrcAddr, mapBalancesOut );

    double  dBalance = mapBalancesOut.at( orderData.strCoinType  ); //如果不存在,抛异常
    if(  orderData.dAmount * 1.1 >  dBalance )
    {
        //如果余额不足直接, 拒绝用户的提币
        orderData.iOrderStatus = OrderStatus::ORDER_FAILED;//OrderStatus::ORDER_CLIENT_ERR;
        orderData.iAuditStatus = AuditStatus::AUDIT_FAILED;
        orderData.strRemarks = "报警:代币余额不足,小于防火余额";

        strErrMsg = QString("balance is not enough. current balance is %1 , less than %2")
                .arg(dBalance).arg( orderData.dAmount * 2 );
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;

        throw runtime_error(strErrMsg.toStdString());
    }


    cosmos::CsRawTx  csRawTx;
    strcpy_s(csRawTx.szChainId, (g_bBlockChainMainnet)?(STR_MAINCHAIN):(STR_TESTCHAIN));
    strcpy_s(csRawTx.szFeeDenom, STR_SATOSHI);
    strcpy_s(csRawTx.szMemo, orderData.strOrderId.toStdString().c_str());
    strcpy_s(csRawTx.szMsgDenom, STR_SATOSHI);
    strcpy_s(csRawTx.szMsgFrom, orderData.strSrcAddr.toStdString().c_str());
    csRawTx.uMsgAmount          = 0;  // HRC20 代币转账,  设为 0即可
    csRawTx.uGas                = 200000;
    csRawTx.uFeeAmount          = 100; //使用 HTDF_2020版

    QString str_HRC20_ContractAddr = utils::Get_HRC20_ContractAddr(orderData.strCoinType);
    strcpy_s(csRawTx.szMsgTo, str_HRC20_ContractAddr.toStdString().c_str());

    //构造 Data 字段 (HRC20 交易)
    std::string cstrRetHexStrData = "";
    std::string cstrValue = "";
    int nDecimals = utils::Get_HRC20_Decimals( orderData.strCoinType );
    rawtx::CCosmosRawTxUtilsImpl::ToWeiStr(orderData.dAmount, nDecimals, cstrValue );

    QString strAmountInWei_Hex = QString::fromStdString(cstrValue);
    rawtx::CCosmosRawTxUtilsImpl::Make_HRC20_Transfer_Data(orderData.strDstAddr, strAmountInWei_Hex, cstrRetHexStrData);

    memset(csRawTx.szData, 0, sizeof(csRawTx.szData));
    memcpy( csRawTx.szData, cstrRetHexStrData.c_str(), cstrRetHexStrData.size());

    //获取 sequence 和 account_number

    rawtx::CosmosAccountInfo cosAccountInfo;
    cosAccountInfo.strCoinType = "htdf" ; //获取 主币中 htdf 的余额  和  sequence
    cosAccountInfo.strAddress = orderData.strSrcAddr;

    cosUtil.GetAccountInfo( cosAccountInfo );
    if( cosAccountInfo.dBalance  < 0.20 + 0.01 )  // 200000 * 100 satoshi => 0.2 HTDF
    {
        //如果余额不足直接, 拒绝用户的提币
        orderData.iOrderStatus = OrderStatus::ORDER_FAILED;//OrderStatus::ORDER_CLIENT_ERR;
        orderData.iAuditStatus = AuditStatus::AUDIT_FAILED;
        orderData.strRemarks = "报警:HTDF余额不足以支付手续费";

        strErrMsg = QString("balance is not enough. current balance is %1 , less than %2")
                .arg(cosAccountInfo.dBalance).arg(0.20 + 0.01);
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;

        throw runtime_error(strErrMsg.toStdString());
    }
    csRawTx.uSequence      =  cosAccountInfo.uSequence;
    csRawTx.uAccountNumber =  cosAccountInfo.uAccountNumber;



    rawtx::Cosmos_CreateRawTxParam param;
    param.csRawTx = csRawTx;
    QMap<QString, QVariant> mapTmp = __m_qmapAllCoinConfig.value(orderData.strCoinType.toLower());
    QString strTmpPrivKey = mapTmp.value("srcPrivKey").toString();
    param.strPrivKey = strTmpPrivKey; //自动提币地址私钥


    //2.检查参数

    if(false == param.ParamsCheck( &strErrMsg ))
    {
        orderData.iOrderStatus = OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    //3.调用接口创建
    cosUtil.CreateRawTx( &param );


    //4.组装参数, 调用广播接口进行广播
    rawtx::Cosmos_BroadcastRawTxParam  bcastParam;
    bcastParam.strOrderId = orderData.strOrderId;
    bcastParam.strChainId   = param.csRawTx.szChainId;
    bcastParam.strSequence  = QString::asprintf("%u", param.csRawTx.uSequence);
    bcastParam.strCoinType = "HTDF";//直接用 HTDF的广播接口即可
    bcastParam.strFrom = param.csRawTx.szMsgFrom;
    bcastParam.strTo = str_HRC20_ContractAddr; //HRC20 合约地址
    bcastParam.strValue = "0.0"; //转为satoshi

    bcastParam.strSignedRawTxHex = param.strHexData; //广播数据

    if(false == bcastParam.ParamsCheck( &strErrMsg ))
    {
        orderData.iOrderStatus =  OrderStatus::ORDER_CLIENT_ERR;
        orderData.strRemarks = strErrMsg;

        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"bcastParam.ParamsCheck() error:" << strErrMsg;
        throw runtime_error(strErrMsg.toStdString());
    }

    cosUtil.BroadcastRawTx( &bcastParam );


    //5.返回广播结果
    orderData.strTxid = bcastParam.strTxid;
    //orderData.uNonce = param.csRawTx.uSequence; //TODO : 如果需要保存,

    orderData.iOrderStatus  = OrderStatus::ORDER_SUCCESS;
    orderData.iTxStatus = TxStatus::TX_SUCCESS;
    orderData.strRemarks = QString("提币成功");

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("BroadcastRawTx() return success:") << bcastParam.strErrMsg\
            << " txid: " << bcastParam.strTxid \
            << "  chainId:" << bcastParam.strChainId << "sequence: " << bcastParam.strSequence  \
            << "  from:"  << bcastParam.strFrom <<  "to: " << bcastParam.strTo \
            << " value: " << bcastParam.strValue << "  rawtx: " << bcastParam.strSignedRawTxHex ;

    return NO_ERROR;
}











