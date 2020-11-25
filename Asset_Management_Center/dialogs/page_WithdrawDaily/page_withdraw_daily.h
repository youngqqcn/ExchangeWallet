/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      提现日报界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_WITHDRAW_DAILY_H
#define PAGE_WITHDRAW_DAILY_H

#include "am_main.h"
#include <QObject>

class CPageWithdrawDaily : public CAMMain
{
    Q_OBJECT
public:
//    explicit CPageWithdrawDaily(QObject *parent = nullptr);
    explicit CPageWithdrawDaily(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageWithdrawDaily();

    virtual int Init() override;

public:
    int WithdrawDailySearch(const vector<QString> &vctCondition, vector<QStringList> &vctWithdrawDaily);

signals:

public slots:
};

#endif // PAGE_WITHDRAW_DAILY_H
