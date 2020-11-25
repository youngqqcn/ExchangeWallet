/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      充值日报界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_CHARGE_DAILY_H
#define PAGE_CHARGE_DAILY_H

#include "am_main.h"
#include <QObject>

class CPageChargeDaily : public CAMMain
{
    Q_OBJECT
public:
//    explicit CPageChargeDaily(QObject *parent = nullptr);
    explicit CPageChargeDaily(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageChargeDaily();

    virtual int Init() override;

public:
    int ChargeDailySearch(const vector<QString> &vctCondition, vector<QStringList> &vctChargeDaily);

signals:

public slots:
};

#endif // PAGE_CHARGE_DAILY_H
