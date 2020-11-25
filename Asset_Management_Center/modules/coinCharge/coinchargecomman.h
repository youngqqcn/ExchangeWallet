#ifndef COINCHARGECOMMAN_H
#define COINCHARGECOMMAN_H

#include "comman.h"
#include <QObject>
#include <QSemaphore>
#include <QQueue>
#include <QSemaphore>
#include <vector>
#include <QMap>
#include <atomic>
#pragma execution_character_set("utf-8")

using namespace std;



//以下两个宏定义格式:   http://192.168.10.243:9001/web/btb/btbOrder/saveRechargeOrder
#define STR_GET_ORDER_ID_URL            (g_qstr_JavaHttpIpPort + QString(STR_web_btb_btbOrder) + QString(STR_saveRechargeOrder))
#define STR_COIN_CHARGE_SUCCESS_URL     (g_qstr_JavaHttpIpPort + QString(STR_web_btb_btbOrder) + QString(STR_upRechargeOrderStatus))


//以下宏定义的格式:  http://192.168.10.79:9000/htdf/crawltransactions?blknumber=
//获取区块监测程序BTC充币信息接口
#define STR_GET_BTC_DATA                (g_qstr_WalletHttpIpPort + QString(STR_btc_crawltransactions_blocktime))
//获取区块监测程序USDT充币信息接口
#define STR_GET_USDT_DATA               (g_qstr_WalletHttpIpPort + QString(STR_usdt_crawltransactions_blocktime))
//获取区块监测程序ETH充币信息接口
#define STR_GET_ETH_DATA                (g_qstr_WalletHttpIpPort + QString(STR_eth_crawltransactions_blocktime))

//获取区块监测程序ETH充币信息接口
#define STR_GET_ETC_DATA                (g_qstr_WalletHttpIpPort + QString(STR_etc_crawltransactions_blocktime))

//获取区块监测程序USDP充币信息接口
#define STR_GET_USDP_DATA               (g_qstr_WalletHttpIpPort + QString(STR_usdp_crawltransactions_blocktime))
//获取区块监测程序HTDF充币信息接口
#define STR_GET_HTDF_DATA               (g_qstr_WalletHttpIpPort + QString(STR_htdf_crawltransactions_blocktime))
//获取区块监测程序LTC充币信息接口
#define STR_GET_LTC_DATA                (g_qstr_WalletHttpIpPort + QString(STR_ltc_crawltransactions_blocktime))
//获取区块监测程序DASH充币信息接口
#define STR_GET_DASH_DATA               (g_qstr_WalletHttpIpPort + QString(STR_DASH_crawltransactions_blocktime))
//BCH
#define STR_GET_BCH_DATA                (g_qstr_WalletHttpIpPort + QString(STR_BCH_crawltransactions_blocktime))
//BSV
#define STR_GET_BSV_DATA                (g_qstr_WalletHttpIpPort + QString(STR_BSV_crawltransactions_blocktime))
//HET
#define STR_GET_HET_DATA                (g_qstr_WalletHttpIpPort + QString(STR_het_crawltransactions_blocktime))
//XRP
#define STR_GET_XRP_DATA                (g_qstr_WalletHttpIpPort + QString(STR_Xrp_crawltransactions_blocktime))
//EOS
#define STR_GET_EOS_DATA                (g_qstr_WalletHttpIpPort + QString(STR_Eos_crawltransactions_blocktime))
//XLM
#define STR_GET_XLM_DATA                (g_qstr_WalletHttpIpPort + QString(STR_Xlm_crawltransactions_blocktime))

//TRX
#define STR_GET_TRX_DATA                (g_qstr_WalletHttpIpPort + QString(STR_Trx_crawltransactions_blknumber))

//XMR
#define STR_GET_XMR_DATA                (g_qstr_WalletHttpIpPort + QString(STR_Xmr_crawltransactions_blknumber))
//========================充币相关信息================================
//存储区块获取的币种信息
typedef struct _CoinChargeInfo
{
    _CoinChargeInfo()
    {
        strChargeTime = "";
        strBlockNumber = "";
        strCoinType = "";
        strChargeNum = "";
        iChargeStatus = 100;
        strTXID = "";
        strSrcAddr = "";
        strDstAddr = "";
        iConfirmNum = 100;
        strSrcAddrFlag = "";
    }
    QString     strChargeTime;          //充币时间
    QString     strBlockNumber;         //区块高度
    QString     strCoinType;            //充币币种
    QString     strChargeNum;           //充币数量
    int         iChargeStatus;          //充值状态(第一次发送数据传0,确认数大于等于6发送数据传1)
    QString     strTXID;                //交易ID
    QString     strSrcAddr;             //来源地址
    QString     strDstAddr;             //目的地址
    int         iConfirmNum;            //区块确认数
    QString     strSrcAddrFlag;         //判断地址是否是黑名单
}CoinChargeInfo;

//订单编号获取接口返回数据
typedef struct _OrderId
{
    QString     strOrderId;             //订单编号
    QString     strTXID;                //交易ID
}orderId;

//确认数大于等于6时,发送充币完成请求接口
typedef struct _CoinChargeFinish
{
    QString     strChargeTime;          //充币完成时间
    QString     strCoinType;            //充币币种
    QString     strOrderId;             //订单编号
    int         iChargeStatus;          //充币状态
}CoinChargeFinish;


#define INT_SEMAPHORE_MAX 1000000

extern QQueue<CoinChargeInfo> g_queueCoinChargeInfo;       //线程共享队列资源即buffer

extern QSemaphore g_freeSpace;             //空闲的空间
extern QSemaphore g_usedSpace;                                //已使用的空间

//释放标志,当释放一个充币信息时,置为1
extern std::atomic_int8_t g_iReleaseFlag ;


//充值表
#define LIST_STR_TB_RECHARGE QString("charge_no")<<QString("order_id")<<QString("admin_id")<<QString("coin_type")<<QString("recharge_count")<<QString("recharge_status")  \
    <<QString("upload_status")<<QString("src_addr")<<QString("receive_addr")<<QString("txid")<<QString("recharge_time")<<QString("block_number")<<QString("collection_status")


#endif // COMMAN_H
