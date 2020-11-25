/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      充值列表界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_CHARGE_LIST_H
#define PAGE_CHARGE_LIST_H

/*******************************************************************************************************************
 *
 * 说明: 处理充值列表的业务主要业务
 *
*******************************************************************************************************************/

#include "comman.h"
//#include "coincharge.h"
#include "ccoinchargemng.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDateTime>
#include "am_main.h"


//充币列表的 命名空间
namespace ChargeList {


//搜索条件
typedef struct _SEARCHCOND
{
    QString     strTxid;
    QString     strDstAddr;
    QString     strCoinType;
    QString     strChargeStatus;
    QString     strUploadStatus;
    uint        uDatetimeStart;
    uint        uDatetimeEnd;
    QString     strAuditStatus;
    uint        uCurrentPage;
}SEARCHCOND;

//搜索结果  充币信息
typedef struct _CHARGEINFO
{
    QString     strTime;
    QString     strOrderId;
    QString     strCoinType;
    QString     strCount;
    QString     strChargeStatus;
    QString     strUploadStatus;
    QString     strAuditStatus;
    QString     strAuditor;
    QString     strAuditRemark;
    QString     strTxid;
    QString     strSrcAddr;
    QString     strDstAddr;
}CHARGEINFO;
}




class CPageChargeList : public CAMMain     //继承自资产基类
{
    Q_OBJECT
public:
    //explicit CPageChargeList(QObject *parent = nullptr);
    explicit CPageChargeList(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageChargeList();


public:
    virtual int Init() override;

public:
    //根据条件查询数据库
    int  ConditionSearch(const ChargeList::SEARCHCOND &searchCond,  vector<QStringList> &vctChargeInfo, uint &uMaxPage);

    //重新上传 那些状态为 "上传失败" 的充币信息
    int  ReUploadFailedOrder(const int &iRet, const QString &strDatetime, const QString &strTXID, const QString &strCoinType);

    //短信提醒设置
    int ShortMsgSetting(const vector<QString> &vctStrTel, const QString &strGoogleCode);

    //查询数据库币种表中的所有币种,写在这里是因为充值列表这个对象是最先初始化的
    int CheckAllCoinType(QStringList &strlist);

    //充币审核-确定
    void coinChargeFinishUpdateDB(const ChargeList::CHARGEINFO &selectedChargeInfo); //上传充币完成成功,更新数据库状态

private:
    //查询短信表中序号最大值
    int __QueryMaxNum();

    //审核通过
    int parseData(QByteArray &byteArrayData, orderId &outOrderId);          //解析数据
    int requestData(const QString strUrl, QMap<QString, QVariant> mapObj, QByteArray &byteArrayReply);          //请求数据

protected:

private:
    CCoinChargeMng      __m_CoinChargeMng;
    QString             __m_strStartUrl;
    QString             __m_strChargeFinishUrl;
    CEncryptionModule   *__m_pEncryptionModule;
};

#endif // PAGE_CHARGE_LIST_H
