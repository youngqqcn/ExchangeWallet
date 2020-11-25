#ifndef SYS_MNG_ADD_COIN_TYPE_H
#define SYS_MNG_ADD_COIN_TYPE_H

#include "am_main.h"
#include <QObject>

class CAddCoin : public CAMMain
{
    Q_OBJECT
public:
//    explicit CSysMngAddCoinType(QObject *parent = nullptr);
    explicit CAddCoin(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CAddCoin();

public:
    virtual int Init() override;



signals:

public slots:
};

#endif // SYS_MNG_ADD_COIN_TYPE_H
