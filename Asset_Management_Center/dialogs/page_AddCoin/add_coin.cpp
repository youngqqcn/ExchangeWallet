#include "add_coin.h"

//CSysMngAddCoinType::CAddCoin(QObject *parent) : QObject(parent)
//{

//}

CAddCoin::CAddCoin(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CAddCoin::~CAddCoin()
{

}

int CAddCoin::Init()
{
    return CAMMain::Init();
}
