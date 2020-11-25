#ifndef TRADEAGGREGATIONSREQUESTBUILDERTEST_H
#define TRADEAGGREGATIONSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"
class TradeAggregationsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testTradeAggregations() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.tradeAggregations(
                    new AssetTypeNative(),
                    Asset::createNonNativeAsset("BTC", KeyPair::fromAccountId("GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH")),
                    1512689100000L,
                    1512775500000L,
                    300000L,
                    3600L
                    ).limit(200).order(RequestBuilder::Order::ASC).buildUri();

        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/trade_aggregations?"
                                        "base_asset_type=native&"
                                        "counter_asset_type=credit_alphanum4&"
                                        "counter_asset_code=BTC&"
                                        "counter_asset_issuer=GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH&"
                                        "start_time=1512689100000&"
                                        "end_time=1512775500000&"
                                        "resolution=300000&"
                                        "offset=3600&"
                                        "limit=200&"
                                        "order=asc"));

    }

};

ADD_TEST(TradeAggregationsRequestBuilderTest)
#endif // TRADEAGGREGATIONSREQUESTBUILDERTEST_H
