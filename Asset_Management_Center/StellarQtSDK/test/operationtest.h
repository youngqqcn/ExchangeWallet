#ifndef OPERATIONTEST_H
#define OPERATIONTEST_H
#include <QObject>
#include <QtTest>
#include "testcollector.h"


#include "src/strkey.h"
#include "src/util.h"
#include "src/operation.h"
#include "src/price.h"

#include "src/createaccountoperation.h"
#include "src/paymentoperation.h"
#include "src/asset.h"
#include "src/assettypenative.h"
#include "src/accountmergeoperation.h"
#include "src/assettypecreditalphanum4.h"
#include "src/assettypecreditalphanum12.h"
#include "src/pathpaymentoperation.h"
#include "src/allowtrustoperation.h"
#include "src/changetrustoperation.h"
#include "src/createpassivesellofferoperation.h"
#include "src/managedataoperation.h"
#include "src/signer.h"
#include "src/setoptionsoperation.h"
#include "src/account.h"
#include "src/network.h"
#include "src/transaction.h"
#include "src/managesellofferoperation.h"
#include "src/managebuyofferoperation.h"
#include "src/bumpsequenceoperation.h"
#include "src/pathpaymentstrictreceiveoperation.h"
#include "src/pathpaymentstrictsendoperation.h"



class OperationTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testConversion(){
        QCOMPARE(Operation::fromXdrAmount(1234567890),QString("123.456789"));
        QCOMPARE(Operation::fromXdrAmount(1234500001),QString("123.4500001"));
        QVERIFY(Operation::toXdrAmount("123.4500001")==1234500001UL);
        QVERIFY(Operation::toXdrAmount("123.45")==1234500000UL);
        QVERIFY(Operation::toXdrAmount("123")==1230000000UL);        
    }
     void testCreateAccountOperation(){
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair *source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair *destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

       QString startingAmount("1000");
       CreateAccountOperation* operation = new CreateAccountOperation(destination, startingAmount);
       operation->setSourceAccount(source);


       stellar::Operation xdr = operation->toXdr();
       CreateAccountOperation* parsedOperation = static_cast<CreateAccountOperation*>(Operation::fromXdr(xdr));

       QVERIFY(10000000000L==xdr.operationCreateAccount.startingBalance);

       QVERIFY(source->getAccountId()==parsedOperation->getSourceAccount()->getAccountId());

       QVERIFY(destination->getAccountId()==parsedOperation->getDestination()->getAccountId());
       QVERIFY(startingAmount==parsedOperation->getStartingBalance());
       QString expected("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAACVAvkAA==");

       QVERIFY(expected == operation->toXdrBase64());



     }


     void testPaymentOperation() {
         // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
         KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
         // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
         KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

         Asset* asset = new AssetTypeNative();
         QString amount = "1000";

         Operation* operation = PaymentOperation::create(destination, asset, amount)->setSourceAccount(*source);

         stellar::Operation xdr = operation->toXdr();


         PaymentOperation* parsedOperation = static_cast<PaymentOperation*>(Operation::fromXdr(xdr));

         QVERIFY(10000000000L==xdr.operationPayment.amount);
         QVERIFY(source->getAccountId()==parsedOperation->getSourceAccount()->getAccountId());
         QVERIFY(destination->getAccountId()== parsedOperation->getDestination()->getAccountId());
         QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getAsset()));
         QVERIFY(amount == parsedOperation->getAmount());
         QString expected("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAEAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAAAAAAAlQL5AA=");
         //qDebug() << QByteArray::fromBase64(expected.toLatin1()).toHex();
         //qDebug() << QByteArray::fromBase64(operation->toXdrBase64().toLatin1()).toHex();

         QVERIFY(expected == operation->toXdrBase64());
     }

     void testPathPaymentOperation(){
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
       // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
       KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));

       // GAVAQKT2M7B4V3NN7RNNXPU5CWNDKC27MYHKLF5UNYXH4FNLFVDXKRSV
       KeyPair* pathIssuer1 = KeyPair::fromSecretSeed(QString("SALDLG5XU5AEJWUOHAJPSC4HJ2IK3Z6BXXP4GWRHFT7P7ILSCFFQ7TC5"));
       // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
       KeyPair* pathIssuer2 = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

       Asset* sendAsset = new AssetTypeNative();
       QString sendMax = "0.0001";
       Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
       QString destAmount = "0.0001";
       QList<Asset*> path;
       path.append(new AssetTypeCreditAlphaNum4("USD", pathIssuer1));
       path.append( new AssetTypeCreditAlphaNum12("TESTTEST", pathIssuer2));


       PathPaymentOperation* operation = new PathPaymentOperation(
           sendAsset, sendMax, destination, destAsset, destAmount, path);
       operation->setSourceAccount(source);



       auto xdr = operation->toXdr();
       PathPaymentOperation* parsedOperation = static_cast<PathPaymentOperation*>(Operation::fromXdr(xdr));

       QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.sendMax);
       QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.destAmount);
       QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());
       QCOMPARE(destination->getAccountId(), parsedOperation->getDestination()->getAccountId());
       QCOMPARE(sendMax, parsedOperation->getSendMax());
       QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
       QCOMPARE(destAmount, parsedOperation->getDestAmount());
       QCOMPARE(path.length(), parsedOperation->getPath().length());

       QString expected = QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAIAAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAAAPoAAAAAgAAAAFVU0QAAAAAACoIKnpnw8rtrfxa276dFZo1C19mDqWXtG4ufhWrLUd1AAAAAlRFU1RURVNUAAAAAAAAAABE/ttVl8BLV0csW/xgXtbXOVf1lMyDluMiafl0IDVFIg==");

       QVERIFY(expected==operation->toXdrBase64());
     }

     void testPathPaymentEmptyPathOperation() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
       // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
       KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));


       Asset* sendAsset = new AssetTypeNative();
       QString sendMax = "0.0001";
       Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
       QString destAmount = "0.0001";

       PathPaymentOperation* operation = new PathPaymentOperation(
               sendAsset, sendMax, destination, destAsset, destAmount);
       operation->setSourceAccount(source);

       auto xdr = operation->toXdr();
       PathPaymentOperation* parsedOperation = static_cast<PathPaymentOperation*>(Operation::fromXdr(xdr));

       QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.sendMax);

       QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.destAmount);
       QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());
       QCOMPARE(destination->getAccountId(), parsedOperation->getDestination()->getAccountId());
       QCOMPARE(sendMax, parsedOperation->getSendMax());
       QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
       QCOMPARE(destAmount, parsedOperation->getDestAmount());
       QCOMPARE(0, parsedOperation->getPath().size());

       QCOMPARE(operation->toXdrBase64(),
                QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAIAAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAAAPoAAAAAA=="));
     }

     void testPathPaymentStrictSendOperation()  {
         // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
         KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
         // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
         KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
         // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
         KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));

         // GAVAQKT2M7B4V3NN7RNNXPU5CWNDKC27MYHKLF5UNYXH4FNLFVDXKRSV
         KeyPair* pathIssuer1 = KeyPair::fromSecretSeed(QString("SALDLG5XU5AEJWUOHAJPSC4HJ2IK3Z6BXXP4GWRHFT7P7ILSCFFQ7TC5"));
         // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
         KeyPair* pathIssuer2 = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

         Asset* sendAsset = new AssetTypeNative();
         QString sendAmount = "0.0001";
         Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
         QString destMin = "0.0009";
         QList<Asset*> path = {new AssetTypeCreditAlphaNum4("USD", pathIssuer1), new AssetTypeCreditAlphaNum12("TESTTEST", pathIssuer2)};

         PathPaymentStrictSendOperation* operation = PathPaymentStrictSendOperation::create(
                     sendAsset, sendAmount, destination, destAsset, destMin)
                 ->setPath(path)
                 ->setSourceAccount(source);

         auto xdr = operation->toXdr();
         PathPaymentStrictSendOperation* parsedOperation = static_cast<PathPaymentStrictSendOperation*>(Operation::fromXdr(xdr));

         QCOMPARE(xdr.operationPathPaymentStrictSend.sendAmount,1000L);
         QCOMPARE(xdr.operationPathPaymentStrictSend.destMin,9000L);
         QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
         QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());
         QCOMPARE(destination->getAccountId(), parsedOperation->getDestination()->getAccountId());
         QCOMPARE(sendAmount, parsedOperation->getSendAmount());
         QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
         QCOMPARE(destMin, parsedOperation->getDestMin());
         QVERIFY((path.size() == parsedOperation->getPath().size()) && std::equal(path.begin(),path.end(),parsedOperation->getPath().begin(),[](Asset* a, Asset* b){return a->equals(b);}));

         QCOMPARE(operation->toXdrBase64(),QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAA0AAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAACMoAAAAAgAAAAFVU0QAAAAAACoIKnpnw8rtrfxa276dFZo1C19mDqWXtG4ufhWrLUd1AAAAAlRFU1RURVNUAAAAAAAAAABE/ttVl8BLV0csW/xgXtbXOVf1lMyDluMiafl0IDVFIg=="));
     }

     void testPathPaymentStrictSendEmptyPathOperation()
     {
         // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
         KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
         // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
         KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
         // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
         KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));

//         // GAVAQKT2M7B4V3NN7RNNXPU5CWNDKC27MYHKLF5UNYXH4FNLFVDXKRSV
//         KeyPair* pathIssuer1 = KeyPair::fromSecretSeed(QString("SALDLG5XU5AEJWUOHAJPSC4HJ2IK3Z6BXXP4GWRHFT7P7ILSCFFQ7TC5"));
//         // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
//         KeyPair* pathIssuer2 = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

         Asset* sendAsset = new AssetTypeNative();
         QString sendAmount = "0.0001";
         Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
         QString destMin = "0.0009";

         PathPaymentStrictSendOperation* operation = PathPaymentStrictSendOperation::create(
                     sendAsset, sendAmount, destination, destAsset, destMin)->setSourceAccount(source);

         auto xdr = operation->toXdr();
         PathPaymentStrictSendOperation* parsedOperation = static_cast<PathPaymentStrictSendOperation*>(Operation::fromXdr(xdr));

         QCOMPARE(xdr.operationPathPaymentStrictSend.sendAmount,1000L);
         QCOMPARE(xdr.operationPathPaymentStrictSend.destMin,9000L);
         QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
         QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());
         QCOMPARE(destination->getAccountId(), parsedOperation->getDestination()->getAccountId());
         QCOMPARE(sendAmount, parsedOperation->getSendAmount());
         QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
         QCOMPARE(destMin, parsedOperation->getDestMin());
         QCOMPARE(parsedOperation->getPath().length(), 0);

         QCOMPARE(operation->toXdrBase64(),QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAA0AAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAACMoAAAAAA=="));
     }

     void testChangeTrustOperation() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

       Asset* asset = new AssetTypeNative();
       QString limit = "922337203685.4775807";

       ChangeTrustOperation* operation = new ChangeTrustOperation(asset, limit);
       operation->setSourceAccount(source);

       auto xdr = operation->toXdr();
       ChangeTrustOperation* parsedOperation = static_cast<ChangeTrustOperation*>(Operation::fromXdr(xdr));

       QCOMPARE(9223372036854775807L, xdr.operationChangeTrust.limit);
       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());
       QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getAsset()));
       QCOMPARE(limit, parsedOperation->getLimit());

       QCOMPARE(operation->toXdrBase64(),
               QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAYAAAAAf/////////8="));
     }


     void testAllowTrustOperation(){
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair* trustor = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

       QString assetCode = "USDA";
       bool authorize = true;

       AllowTrustOperation operation(trustor, assetCode, authorize);
       operation.setSourceAccount(source);


       auto xdr = operation.toXdr();
       AllowTrustOperation* parsedOperation = static_cast<AllowTrustOperation*>(Operation::fromXdr(xdr));

       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());
       QCOMPARE(trustor->getAccountId(), parsedOperation->getTrustor()->getAccountId());
       QCOMPARE(assetCode, parsedOperation->getAssetCode());
       QCOMPARE(authorize, parsedOperation->getAuthorize());
       QCOMPARE(operation.toXdrBase64(),
               QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAcAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEQQAAAAE=")
               );
     }

     void testAllowTrustOperationAssetCodeBuffer() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair* trustor = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

       QString assetCode = "USDABC";
       bool authorize = true;

       AllowTrustOperation* operation = AllowTrustOperation::create(trustor, assetCode, authorize)
           ->setSourceAccount(source);

       auto xdr = operation->toXdr();
       AllowTrustOperation* parsedOperation = static_cast<AllowTrustOperation*>(Operation::fromXdr(xdr));

       QCOMPARE(assetCode, parsedOperation->getAssetCode());
     }

     void testSetOptionsOperation(){
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair* inflationDestination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
       // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
       auto signer = Signer::ed25519PublicKey(KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ")));

       Integer clearFlags = 1;
       Integer setFlags = 1;
       Integer masterKeyWeight = 1;
       Integer lowThreshold = 2;
       Integer mediumThreshold = 3;
       Integer highThreshold = 4;
       QString homeDomain = "stellar.org";
       Integer signerWeight = 1;

       SetOptionsOperation* operation = SetOptionsOperation::create()
           ->setInflationDestination(inflationDestination)
           ->setClearFlags(clearFlags)
           ->setSetFlags(setFlags)
           ->setMasterKeyWeight(masterKeyWeight)
           ->setLowThreshold(lowThreshold)
           ->setMediumThreshold(mediumThreshold)
           ->setHighThreshold(highThreshold)
           ->setHomeDomain(homeDomain)
           ->setSigner(signer, signerWeight)
           ->setSourceAccount(source);

       auto xdr = operation->toXdr();
       SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

       QCOMPARE(inflationDestination->getAccountId(), parsedOperation->getInflationDestination()->getAccountId());
       QCOMPARE(clearFlags, parsedOperation->getClearFlags());
       QCOMPARE(setFlags, parsedOperation->getSetFlags());
       QCOMPARE(masterKeyWeight, parsedOperation->getMasterKeyWeight());
       QCOMPARE(lowThreshold, parsedOperation->getLowThreshold());
       QCOMPARE(mediumThreshold, parsedOperation->getMediumThreshold());
       QCOMPARE(highThreshold, parsedOperation->getHighThreshold());
       QCOMPARE(homeDomain, parsedOperation->getHomeDomain());
       QCOMPARE(signer.type, parsedOperation->getSigner()->type);
       QVERIFY(memcmp(signer.ed25519, parsedOperation->getSigner()->ed25519,sizeof(signer.ed25519))==0);
       QCOMPARE(signerWeight, parsedOperation->getSignerWeight());
       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());

       QCOMPARE(operation->toXdrBase64()
                ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAABAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAAAQAAAAEAAAABAAAAAQAAAAEAAAABAAAAAQAAAAIAAAABAAAAAwAAAAEAAAAEAAAAAQAAAAtzdGVsbGFyLm9yZwAAAAABAAAAAET+21WXwEtXRyxb/GBe1tc5V/WUzIOW4yJp+XQgNUUiAAAAAQ=="));
     }


     void testSetOptionsOperationSingleField() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

       QString homeDomain = "stellar.org";

       SetOptionsOperation* operation = SetOptionsOperation::create()
           ->setHomeDomain(homeDomain)
           ->setSourceAccount(source);

       auto xdr = operation->toXdr();
       SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

       QVERIFY(parsedOperation->getInflationDestination()==nullptr);
       QVERIFY(parsedOperation->getClearFlags().isNull());
       QVERIFY(parsedOperation->getSetFlags().isNull());
       QVERIFY(parsedOperation->getMasterKeyWeight().isNull());
       QVERIFY(parsedOperation->getLowThreshold().isNull());
       QVERIFY(parsedOperation->getMediumThreshold().isNull());
       QVERIFY(parsedOperation->getHighThreshold().isNull());
       QCOMPARE(homeDomain, parsedOperation->getHomeDomain());
       QVERIFY(parsedOperation->getSigner()==nullptr);
       QVERIFY(parsedOperation->getSignerWeight().isNull());
       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());

       QCOMPARE(operation->toXdrBase64()
                ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAtzdGVsbGFyLm9yZwAAAAAA"));
     }


     void testSetOptionsOperationSignerSha256() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

       QByteArray preimage = "stellar.org";
       QByteArray hash = Util::hash(preimage);

       SetOptionsOperation* operation = SetOptionsOperation::create()
               ->setSigner(Signer::sha256Hash(hash), 10)
               ->setSourceAccount(source);

       auto xdr = operation->toXdr();

       SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

       QVERIFY(!parsedOperation->getInflationDestination());

       QVERIFY(parsedOperation->getClearFlags().isNull());

       QVERIFY(parsedOperation->getSetFlags().isNull());
       QVERIFY(parsedOperation->getMasterKeyWeight().isNull());
       QVERIFY(parsedOperation->getLowThreshold().isNull());
       QVERIFY(parsedOperation->getMediumThreshold().isNull());
       QVERIFY(parsedOperation->getHighThreshold().isNull());
       QVERIFY(parsedOperation->getHomeDomain().isNull());
       QVERIFY(memcmp(hash.data(), parsedOperation->getSigner()->hashX,static_cast<size_t>(hash.size()))==0);
       QCOMPARE(10, static_cast<int>(parsedOperation->getSignerWeight()));
       QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());

       QCOMPARE(operation->toXdrBase64()
                ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAACbpRqMkaQAfCYSk/n3xIl4fCoHfKqxF34ht2iuvSYEJQAAAAK"));
     }

     void testSetOptionsOperationPreAuthTxSigner() {
        Network::useTestNetwork();
       // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
       KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
       KeyPair* destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

       qint64 sequenceNumber = 2908908335136768L;
       Account* account = new Account(source, sequenceNumber);


       Transaction* transaction = Transaction::Builder(account).addOperation(new CreateAccountOperation(destination,"2000")).setTimeout(Transaction::Builder::TIMEOUT_INFINITE).build();


       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* opSource = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

       SetOptionsOperation* operation = SetOptionsOperation::create()
               ->setSigner(Signer::preAuthTx(transaction), 10)
               ->setSourceAccount(opSource);

       auto xdr = operation->toXdr();
       SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

       QVERIFY(!parsedOperation->getInflationDestination());
       QVERIFY(parsedOperation->getClearFlags().isNull());
       QVERIFY(parsedOperation->getSetFlags().isNull());
       QVERIFY(parsedOperation->getMasterKeyWeight().isNull());
       QVERIFY(parsedOperation->getLowThreshold().isNull());
       QVERIFY(parsedOperation->getMediumThreshold().isNull());
       QVERIFY(parsedOperation->getHighThreshold().isNull());
       QVERIFY(parsedOperation->getHomeDomain().isNull());
       QVERIFY(memcmp(transaction->hash().data(), parsedOperation->getSigner()->preAuthTx,sizeof(parsedOperation->getSigner()->preAuthTx))==0);
       QCOMPARE(Integer(10), parsedOperation->getSignerWeight());
       QCOMPARE(opSource->getAccountId(), parsedOperation->getSourceAccount()->getAccountId());

       QCOMPARE(operation->toXdrBase64()
                ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAB1vRBIRC3w7ZH5rQa17hIBKUwZTvBP4kNmSP7jVyw1fQAAAAK"));
     }


     void testManageSellOfferOperation() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
       KeyPair* issuer = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

       Asset* selling = new AssetTypeNative();
       Asset* buying = Asset::createNonNativeAsset("USD", issuer);
       QString amount = "0.00001";
       QString price = "0.85334384"; // n=5333399 d=6250000
       Price* priceObj = Price::fromString(price);
       qint64 offerId = 1;

       ManageSellOfferOperation* operation = ManageSellOfferOperation::create(selling, buying, amount, price)
           ->setOfferId(offerId)
           ->setSourceAccount(source);

       auto xdr = operation->toXdr();
       ManageSellOfferOperation* parsedOperation = (ManageSellOfferOperation*) ManageSellOfferOperation::fromXdr(xdr);


       QCOMPARE(xdr.operationManageSellOffer.amount,(qint64)100L);
       QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSelling()));
       QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getBuying()));
       QVERIFY(parsedOperation->getBuying()->equals(buying));
       QCOMPARE(parsedOperation->getAmount(),amount);
       QCOMPARE(parsedOperation->getPrice(),price);
       QCOMPARE(priceObj->getNumerator(), 5333399);
       QCOMPARE(priceObj->getDenominator(), 6250000);
       QCOMPARE(parsedOperation->getOfferId(),offerId);

       QCOMPARE(operation->toXdrBase64()
                ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAMAAAAAAAAAAVVTRAAAAAAARP7bVZfAS1dHLFv8YF7W1zlX9ZTMg5bjImn5dCA1RSIAAAAAAAAAZABRYZcAX14QAAAAAAAAAAE="));
     }

     void testManageBuyOfferOperation() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
       KeyPair* issuer = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

       Asset* selling = new AssetTypeNative();
       Asset* buying = Asset::createNonNativeAsset("USD", issuer);
       QString amount = "0.00001";
       QString price = "0.85334384"; // n=5333399 d=6250000
       Price* priceObj = Price::fromString(price);
       qint64 offerId = 1;

       ManageBuyOfferOperation* operation = ManageBuyOfferOperation::create(selling, buying, amount, price)
           ->setOfferId(offerId)
           ->setSourceAccount(source);

       auto xdr = operation->toXdr();
       ManageBuyOfferOperation* parsedOperation = (ManageBuyOfferOperation*) ManageBuyOfferOperation::fromXdr(xdr);


       QCOMPARE(xdr.operationManageBuyOffer.buyAmount,(qint64)100L);
       QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSelling()));
       QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getBuying()));
       QVERIFY(parsedOperation->getBuying()->equals(buying));
       QCOMPARE(parsedOperation->getAmount(),amount);
       QCOMPARE(parsedOperation->getPrice(),price);
       QCOMPARE(priceObj->getNumerator(), 5333399);
       QCOMPARE(priceObj->getDenominator(), 6250000);
       QCOMPARE(parsedOperation->getOfferId(),offerId);

       QCOMPARE(operation->toXdrBase64()
                ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAwAAAAAAAAAAVVTRAAAAAAARP7bVZfAS1dHLFv8YF7W1zlX9ZTMg5bjImn5dCA1RSIAAAAAAAAAZABRYZcAX14QAAAAAAAAAAE="));
     }

     void testManageSellOfferOperation_BadArithmeticRegression(){
        // from https://github.com/stellar/java-stellar-sdk/issues/183

        QByteArray transactionEnvelopeToDecode = QByteArray::fromBase64("AAAAAButy5zasS3DLZ5uFpZHL25aiHUfKRwdv1+3Wp12Ce7XAAAAZAEyGwYAAAAOAAAAAAAAAAAAAAABAAAAAQAAAAAbrcuc2rEtwy2ebhaWRy9uWoh1HykcHb9ft1qddgnu1wAAAAMAAAAAAAAAAUtJTgAAAAAARkrT28ebM6YQyhVZi1ttlwq/dk6ijTpyTNuHIMgUp+EAAAAAAAARPSfDKZ0AAv7oAAAAAAAAAAAAAAAAAAAAAXYJ7tcAAABAbE8rEoFt0Hcv41iwVCl74C1Hyr+Lj8ZyaYn7zTJhezClbc+pTW1KgYFIZOJiGVth2xFnBT1pMXuQkVdTlB3FCw==");

        stellar::TransactionEnvelope transactionEnvelope;
        QDataStream stream(&transactionEnvelopeToDecode,QIODevice::ReadOnly);
        stream >> transactionEnvelope;

        QCOMPARE(transactionEnvelope.tx.operations.value.length(),1);
        auto & opXdr = transactionEnvelope.tx.operations.value[0];
        ManageSellOfferOperation* op = (ManageSellOfferOperation*)Operation::fromXdr(opXdr);

        QCOMPARE(op->getPrice(),"3397.893306099996");
      }

     void testManageBuyOfferOperation_BadArithmeticRegression(){
        // from https://github.com/stellar/java-stellar-sdk/issues/183

        QByteArray transactionEnvelopeToDecode = QByteArray::fromBase64("AAAAAButy5zasS3DLZ5uFpZHL25aiHUfKRwdv1+3Wp12Ce7XAAAAZAEyGwYAAAAxAAAAAAAAAAAAAAABAAAAAQAAAAAbrcuc2rEtwy2ebhaWRy9uWoh1HykcHb9ft1qddgnu1wAAAAwAAAABS0lOAAAAAABGStPbx5szphDKFVmLW22XCr92TqKNOnJM24cgyBSn4QAAAAAAAAAAACNyOCfDKZ0AAv7oAAAAAAABv1IAAAAAAAAAAA==");

        stellar::TransactionEnvelope transactionEnvelope;
        QDataStream stream(&transactionEnvelopeToDecode,QIODevice::ReadOnly);
        stream >> transactionEnvelope;

        QCOMPARE(transactionEnvelope.tx.operations.value.length(),1);

        Transaction * t = Transaction::fromEnvelopeXdr(transactionEnvelope);

        ManageBuyOfferOperation* op = (ManageBuyOfferOperation*)t->getOperations().first();//Operation::fromXdr(transactionEnvelope.tx.operations.value[0]);

        QCOMPARE(op->getPrice(),"3397.893306099996");//3397.893306099995925186
      }

     void testCreatePassiveSellOfferOperation() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
       KeyPair* issuer = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

       Asset* selling = new AssetTypeNative();
       Asset* buying = Asset::createNonNativeAsset("USD", issuer);
       QString amount = "0.00001";
       QString price = "2.93850088"; // n=36731261 d=12500000
       Price* priceObj = Price::fromString(price);

       CreatePassiveSellOfferOperation* operation = new CreatePassiveSellOfferOperation(selling, buying, amount, price);
       operation->setSourceAccount(source);

       auto xdr = operation->toXdr();
       CreatePassiveSellOfferOperation* parsedOperation = static_cast<CreatePassiveSellOfferOperation*>(CreatePassiveSellOfferOperation::fromXdr(xdr));

       QVERIFY(100L == xdr.operationCreatePassiveSellOffer.amount);
       QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSelling()));
       QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getBuying()));
       QVERIFY(parsedOperation->getBuying()->equals(buying));
       QCOMPARE(amount, parsedOperation->getAmount());
       QCOMPARE(price, parsedOperation->getPrice());
       QCOMPARE(priceObj->getNumerator(), 36731261);
       QCOMPARE(priceObj->getDenominator(), 12500000);

       QCOMPARE(operation->toXdrBase64(),
               QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAQAAAAAAAAAAVVTRAAAAAAARP7bVZfAS1dHLFv8YF7W1zlX9ZTMg5bjImn5dCA1RSIAAAAAAAAAZAIweX0Avrwg"));
     }


     void testAccountMergeOperation() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
       KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

       AccountMergeOperation* operation = new AccountMergeOperation(destination);
       operation->setSourceAccount(source);

       stellar::Operation xdr = operation->toXdr();

       AccountMergeOperation* parsedOperation = (AccountMergeOperation*) Operation::fromXdr(xdr);

       QCOMPARE(destination->getAccountId(), parsedOperation->getDestination()->getAccountId());

       QCOMPARE(QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAgAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxA="),
               operation->toXdrBase64());
     }

     void testManageDataOperation() {
         // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
         KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

         QByteArray data = QByteArray::fromHex("0001020304");

         ManageDataOperation* operation = new ManageDataOperation("test", data);
         operation->setSourceAccount(source);


         auto xdr = operation->toXdr();

         ManageDataOperation* parsedOperation = static_cast<ManageDataOperation*>(Operation::fromXdr(xdr));

         QCOMPARE(parsedOperation->getName(),QString("test"));
         QCOMPARE(parsedOperation->getValue(),data);

         QCOMPARE(operation->toXdrBase64(),
                  QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAoAAAAEdGVzdAAAAAEAAAAFAAECAwQAAAA="));
     }

     void testManageDataOperationEmptyValue() {
         // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
         KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

         ManageDataOperation* operation = new ManageDataOperation("test");
         operation->setSourceAccount(source);

         auto xdr = operation->toXdr();

         ManageDataOperation* parsedOperation = static_cast<ManageDataOperation*>(Operation::fromXdr(xdr));

         QCOMPARE(parsedOperation->getName(),QString("test"));
         QCOMPARE(parsedOperation->getValue(),QByteArray());

         QCOMPARE(operation->toXdrBase64()
                  ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAoAAAAEdGVzdAAAAAA="));
     }

     void testBumpSequence() {
       // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
       KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
       BumpSequenceOperation* operation = BumpSequenceOperation::create(156L)
               ->setSourceAccount(source);
        auto xdr = operation->toXdr();
        BumpSequenceOperation* parsedOperation = static_cast<BumpSequenceOperation*>(Operation::fromXdr(xdr));
        QCOMPARE(parsedOperation->getBumpTo(),156L);
        QCOMPARE(operation->toXdrBase64(),QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAsAAAAAAAAAnA=="));
     }

     void testToXdrAmount() {
       QVERIFY(0L== Operation::toXdrAmount("0"));
       QVERIFY(1L== Operation::toXdrAmount("0.0000001"));
       QVERIFY(10000000L== Operation::toXdrAmount("1"));
       QVERIFY(11234567L== Operation::toXdrAmount("1.1234567"));
       QVERIFY(729912843007381L== Operation::toXdrAmount("72991284.3007381"));
       QVERIFY(729912843007381L== Operation::toXdrAmount("72991284.30073810"));
       QVERIFY(1014016711446800155L== Operation::toXdrAmount("101401671144.6800155"));
       qDebug() << Operation::toXdrAmount("922337203685.4775807");
       QVERIFY(9223372036854775807UL== Operation::toXdrAmount("922337203685.4775807"));
       try {
         Operation::toXdrAmount("0.00000001");
         QFAIL("expected exception");
       }
       catch (std::exception e) {
       }
       try {
         Operation::toXdrAmount("72991284.30073811");
         QFAIL("expected exception");
       }
       catch(std::exception e){
       }
     }


     void testFromXdrAmount() {
       QCOMPARE(QString("0"), Operation::fromXdrAmount(0L));
       QCOMPARE(QString("0.0000001"), Operation::fromXdrAmount(1L));
       QCOMPARE(QString("1"), Operation::fromXdrAmount(10000000L));
       QCOMPARE(QString("1.1234567"), Operation::fromXdrAmount(11234567L));
       QCOMPARE(QString("72991284.3007381"), Operation::fromXdrAmount(729912843007381L));
       QCOMPARE(QString("101401671144.6800155"), Operation::fromXdrAmount(1014016711446800155L));
       QCOMPARE(QString("922337203685.4775807"), Operation::fromXdrAmount(9223372036854775807L));
   }


};

ADD_TEST(OperationTest)
#endif // OPERATIONTEST_H
