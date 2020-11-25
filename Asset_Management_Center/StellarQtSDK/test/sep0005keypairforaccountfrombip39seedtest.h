#ifndef SEP0005KEYPAIRFORACCOUNTFROMBIP39SEEDTEST_H
#define SEP0005KEYPAIRFORACCOUNTFROMBIP39SEEDTEST_H

#include "slip10.h"
/**
 * Test cases from https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md
 */
#include <QObject>
#include "src/strkey.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/account.h"


/**
 * Test cases from https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md
 */
class SEP0005KeyPairForAccountFromBip39SeedTest: public QObject
{
    Q_OBJECT

    const QString case1Bip39Seed = "e4a5a632e70943ae7f07659df1332160937fad82587216a4c64315a0fb39497ee4a01f76ddab4cba68147977f3a147b6ad584c41808e8238a07f6cc4b582f186";
private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void case1() {
      testDerivedAccounts(case1Bip39Seed,QList<QString>()
              <<"m/44'/148'/0' GDRXE2BQUC3AZNPVFSCEZ76NJ3WWL25FYFK6RGZGIEKWE4SOOHSUJUJ6 SBGWSG6BTNCKCOB3DIFBGCVMUPQFYPA2G4O34RMTB343OYPXU5DJDVMN"
              <<"m/44'/148'/1' GBAW5XGWORWVFE2XTJYDTLDHXTY2Q2MO73HYCGB3XMFMQ562Q2W2GJQX SCEPFFWGAG5P2VX5DHIYK3XEMZYLTYWIPWYEKXFHSK25RVMIUNJ7CTIS"
              <<"m/44'/148'/2' GAY5PRAHJ2HIYBYCLZXTHID6SPVELOOYH2LBPH3LD4RUMXUW3DOYTLXW SDAILLEZCSA67DUEP3XUPZJ7NYG7KGVRM46XA7K5QWWUIGADUZCZWTJP"
              <<"m/44'/148'/3' GAOD5NRAEORFE34G5D4EOSKIJB6V4Z2FGPBCJNQI6MNICVITE6CSYIAE SBMWLNV75BPI2VB4G27RWOMABVRTSSF7352CCYGVELZDSHCXWCYFKXIX"
              <<"m/44'/148'/4' GBCUXLFLSL2JE3NWLHAWXQZN6SQC6577YMAU3M3BEMWKYPFWXBSRCWV4 SCPCY3CEHMOP2TADSV2ERNNZBNHBGP4V32VGOORIEV6QJLXD5NMCJUXI"
              <<"m/44'/148'/5' GBRQY5JFN5UBG5PGOSUOL4M6D7VRMAYU6WW2ZWXBMCKB7GPT3YCBU2XZ SCK27SFHI3WUDOEMJREV7ZJQG34SCBR6YWCE6OLEXUS2VVYTSNGCRS6X"
              <<"m/44'/148'/6' GBY27SJVFEWR3DUACNBSMJB6T4ZPR4C7ZXSTHT6GMZUDL23LAM5S2PQX SDJ4WDPOQAJYR3YIAJOJP3E6E4BMRB7VZ4QAEGCP7EYVDW6NQD3LRJMZ"
              <<"m/44'/148'/7' GAY7T23Z34DWLSTEAUKVBPHHBUE4E3EMZBAQSLV6ZHS764U3TKUSNJOF SA3HXJUCE2N27TBIZ5JRBLEBF3TLPQEBINP47E6BTMIWW2RJ5UKR2B3L"
              <<"m/44'/148'/8' GDJTCF62UUYSAFAVIXHPRBR4AUZV6NYJR75INVDXLLRZLZQ62S44443R SCD5OSHUUC75MSJG44BAT3HFZL2HZMMQ5M4GPDL7KA6HJHV3FLMUJAME"
              <<"m/44'/148'/9' GBTVYYDIYWGUQUTKX6ZMLGSZGMTESJYJKJWAATGZGITA25ZB6T5REF44 SCJGVMJ66WAUHQHNLMWDFGY2E72QKSI3XGSBYV6BANDFUFE7VY4XNXXR");
    }

    void case2() {
      testDerivedAccounts("7b36d4e725b48695c3ffd2b4b317d5552cb157c1a26c46d36a05317f0d3053eb8b3b6496ba39ebd9312d10e3f9937b47a6790541e7c577da027a564862e92811",
                          QList<QString>()
              <<"m/44'/148'/0' GAVXVW5MCK7Q66RIBWZZKZEDQTRXWCZUP4DIIFXCCENGW2P6W4OA34RH SAKS7I2PNDBE5SJSUSU2XLJ7K5XJ3V3K4UDFAHMSBQYPOKE247VHAGDB"
              <<"m/44'/148'/1' GDFCYVCICATX5YPJUDS22KM2GW5QU2KKSPPPT2IC5AQIU6TP3BZSLR5K SAZ2H5GLAVWCUWNPQMB6I3OHRI63T2ACUUAWSH7NAGYYPXGIOPLPW3Q4"
              <<"m/44'/148'/2' GAUA3XK3SGEQFNCBM423WIM5WCZ4CR4ZDPDFCYSFLCTODGGGJMPOHAAE SDVSSLPL76I33DKAI4LFTOAKCHJNCXUERGPCMVFT655Z4GRLWM6ZZTSC"
              <<"m/44'/148'/3' GAH3S77QXTAPZ77REY6LGFIJ2XWVXFOKXHCFLA6HQTL3POLVZJDHHUDM SCH56YSGOBYVBC6DO3ZI2PY62GBVXT4SEJSXJOBQYGC2GCEZSB5PEVBZ"
              <<"m/44'/148'/4' GCSCZVGV2Y3EQ2RATJ7TE6PVWTW5OH5SMG754AF6W6YM3KJF7RMNPB4Y SBWBM73VUNBGBMFD4E2BA7Q756AKVEAAVTQH34RYEUFD6X64VYL5KXQ2"
              <<"m/44'/148'/5' GDKWYAJE3W6PWCXDZNMFNFQSPTF6BUDANE6OVRYMJKBYNGL62VKKCNCC SAVS4CDQZI6PSA5DPCC42S5WLKYIPKXPCJSFYY4N3VDK25T2XX2BTGVX"
              <<"m/44'/148'/6' GCDTVB4XDLNX22HI5GUWHBXJFBCPB6JNU6ZON7E57FA3LFURS74CWDJH SDFC7WZT3GDQVQUQMXN7TC7UWDW5E3GSMFPHUT2TSTQ7RKWTRA4PLBAL"
              <<"m/44'/148'/7' GBTDPL5S4IOUQHDLCZ7I2UXJ2TEHO6DYIQ3F2P5OOP3IS7JSJI4UMHQJ SA6UO2FIYC6AS2MSDECLR6F7NKCJTG67F7R4LV2GYB4HCZYXJZRLPOBB"
              <<"m/44'/148'/8' GD3KWA24OIM7V3MZKDAVSLN3NBHGKVURNJ72ZCTAJSDTF7RIGFXPW5FQ SBDNHDDICLLMBIDZ2IF2D3LH44OVUGGAVHQVQ6BZQI5IQO6AB6KNJCOV"
              <<"m/44'/148'/9' GB3C6RRQB3V7EPDXEDJCMTS45LVDLSZQ46PTIGKZUY37DXXEOAKJIWSV SDHRG2J34MGDAYHMOVKVJC6LX2QZMCTIKRO5I4JQ6BJQ36KVL6QUTT72");
    }

    void case3() {
      testDerivedAccounts("937ae91f6ab6f12461d9936dfc1375ea5312d097f3f1eb6fed6a82fbe38c85824da8704389831482db0433e5f6c6c9700ff1946aa75ad8cc2654d6e40f567866",
                          QList<QString>()
              <<"m/44'/148'/0' GC3MMSXBWHL6CPOAVERSJITX7BH76YU252WGLUOM5CJX3E7UCYZBTPJQ SAEWIVK3VLNEJ3WEJRZXQGDAS5NVG2BYSYDFRSH4GKVTS5RXNVED5AX7"
              <<"m/44'/148'/1' GB3MTYFXPBZBUINVG72XR7AQ6P2I32CYSXWNRKJ2PV5H5C7EAM5YYISO SBKSABCPDWXDFSZISAVJ5XKVIEWV4M5O3KBRRLSPY3COQI7ZP423FYB4"
              <<"m/44'/148'/2' GDYF7GIHS2TRGJ5WW4MZ4ELIUIBINRNYPPAWVQBPLAZXC2JRDI4DGAKU SD5CCQAFRIPB3BWBHQYQ5SC66IB2AVMFNWWPBYGSUXVRZNCIRJ7IHESQ"
              <<"m/44'/148'/3' GAFLH7DGM3VXFVUID7JUKSGOYG52ZRAQPZHQASVCEQERYC5I4PPJUWBD SBSGSAIKEF7JYQWQSGXKB4SRHNSKDXTEI33WZDRR6UHYQCQ5I6ZGZQPK"
              <<"m/44'/148'/4' GAXG3LWEXWCAWUABRO6SMAEUKJXLB5BBX6J2KMHFRIWKAMDJKCFGS3NN SBIZH53PIRFTPI73JG7QYA3YAINOAT2XMNAUARB3QOWWVZVBAROHGXWM"
              <<"m/44'/148'/5' GA6RUD4DZ2NEMAQY4VZJ4C6K6VSEYEJITNSLUQKLCFHJ2JOGC5UCGCFQ SCVM6ZNVRUOP4NMCMMKLTVBEMAF2THIOMHPYSSMPCD2ZU7VDPARQQ6OY"
              <<"m/44'/148'/6' GCUDW6ZF5SCGCMS3QUTELZ6LSAH6IVVXNRPRLAUNJ2XYLCA7KH7ZCVQS SBSHUZQNC45IAIRSAHMWJEJ35RY7YNW6SMOEBZHTMMG64NKV7Y52ZEO2"
              <<"m/44'/148'/7' GBJ646Q524WGBN5X5NOAPIF5VQCR2WZCN6QZIDOSY6VA2PMHJ2X636G4 SC2QO2K2B4EBNBJMBZIKOYSHEX4EZAZNIF4UNLH63AQYV6BE7SMYWC6E"
              <<"m/44'/148'/8' GDHX4LU6YBSXGYTR7SX2P4ZYZSN24VXNJBVAFOB2GEBKNN3I54IYSRM4 SCGMC5AHAAVB3D4JXQPCORWW37T44XJZUNPEMLRW6DCOEARY3H5MAQST"
              <<"m/44'/148'/9' GDXOY6HXPIDT2QD352CH7VWX257PHVFR72COWQ74QE3TEV4PK2KCKZX7 SCPA5OX4EYINOPAUEQCPY6TJMYICUS5M7TVXYKWXR3G5ZRAJXY3C37GF");
    }

    void case4() {
      testDerivedAccounts("d425d39998fb42ce4cf31425f0eaec2f0a68f47655ea030d6d26e70200d8ff8bd4326b4bdf562ea8640a1501ae93ccd0fd7992116da5dfa24900e570a742a489",
                          QList<QString>()
              <<"m/44'/148'/0' GDAHPZ2NSYIIHZXM56Y36SBVTV5QKFIZGYMMBHOU53ETUSWTP62B63EQ SAFWTGXVS7ELMNCXELFWCFZOPMHUZ5LXNBGUVRCY3FHLFPXK4QPXYP2X"
              <<"m/44'/148'/1' GDY47CJARRHHL66JH3RJURDYXAMIQ5DMXZLP3TDAUJ6IN2GUOFX4OJOC SBQPDFUGLMWJYEYXFRM5TQX3AX2BR47WKI4FDS7EJQUSEUUVY72MZPJF"
              <<"m/44'/148'/2' GCLAQF5H5LGJ2A6ACOMNEHSWYDJ3VKVBUBHDWFGRBEPAVZ56L4D7JJID SAF2LXRW6FOSVQNC4HHIIDURZL4SCGCG7UEGG23ZQG6Q2DKIGMPZV6BZ"
              <<"m/44'/148'/3' GBC36J4KG7ZSIQ5UOSJFQNUP4IBRN6LVUFAHQWT2ODEQ7Y3ASWC5ZN3B SDCCVBIYZDMXOR4VPC3IYMIPODNEDZCS44LDN7B5ZWECIE57N3BTV4GQ"
              <<"m/44'/148'/4' GA6NHA4KPH5LFYD6LZH35SIX3DU5CWU3GX6GCKPJPPTQCCQPP627E3CB SA5TRXTO7BG2Z6QTQT3O2LC7A7DLZZ2RBTGUNCTG346PLVSSHXPNDVNT"
              <<"m/44'/148'/5' GBOWMXTLABFNEWO34UJNSJJNVEF6ESLCNNS36S5SX46UZT2MNYJOLA5L SDEOED2KPHV355YNOLLDLVQB7HDPQVIGKXCAJMA3HTM4325ZHFZSKKUC"
              <<"m/44'/148'/6' GBL3F5JUZN3SQKZ7SL4XSXEJI2SNSVGO6WZWNJLG666WOJHNDDLEXTSZ SDYNO6TLFNV3IM6THLNGUG5FII4ET2H7NH3KCT6OAHIUSHKR4XBEEI6A"
              <<"m/44'/148'/7' GA5XPPWXL22HFFL5K5CE37CEPUHXYGSP3NNWGM6IK6K4C3EFHZFKSAND SDXMJXAY45W3WEFWMYEPLPIF4CXAD5ECQ37XKMGY5EKLM472SSRJXCYD"
              <<"m/44'/148'/8' GDS5I7L7LWFUVSYVAOHXJET2565MGGHJ4VHGVJXIKVKNO5D4JWXIZ3XU SAIZA26BUP55TDCJ4U7I2MSQEAJDPDSZSBKBPWQTD5OQZQSJAGNN2IQB"
              <<"m/44'/148'/9' GBOSMFQYKWFDHJWCMCZSMGUMWCZOM4KFMXXS64INDHVCJ2A2JAABCYRR SDXDYPDNRMGOF25AWYYKPHFAD3M54IT7LCLG7RWTGR3TS32A4HTUXNOS");
    }

    void case5() {
      testDerivedAccounts("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4",
                          QList<QString>()
              <<"m/44'/148'/0' GB3JDWCQJCWMJ3IILWIGDTQJJC5567PGVEVXSCVPEQOTDN64VJBDQBYX SBUV3MRWKNS6AYKZ6E6MOUVF2OYMON3MIUASWL3JLY5E3ISDJFELYBRZ"
              <<"m/44'/148'/1' GDVSYYTUAJ3ACHTPQNSTQBDQ4LDHQCMNY4FCEQH5TJUMSSLWQSTG42MV SCHDCVCWGAKGIMTORV6K5DYYV3BY4WG3RA4M6MCBGJLHUCWU2MC6DL66"
              <<"m/44'/148'/2' GBFPWBTN4AXHPWPTQVQBP4KRZ2YVYYOGRMV2PEYL2OBPPJDP7LECEVHR SAPLVTLUXSDLFRDGCCFLPDZMTCEVMP3ZXTM74EBJCVKZKM34LGQPF7K3"
              <<"m/44'/148'/3' GCCCOWAKYVFY5M6SYHOW33TSNC7Z5IBRUEU2XQVVT34CIZU7CXZ4OQ4O SDQYXOP2EAUZP4YOEQ5BUJIQ3RDSP5XV4ZFI6C5Y3QCD5Y63LWPXT7PW"
              <<"m/44'/148'/4' GCQ3J35MKPKJX7JDXRHC5YTXTULFMCBMZ5IC63EDR66QA3LO7264ZL7Q SCT7DUHYZD6DRCETT6M73GWKFJI4D56P3SNWNWNJ7ANLJZS6XIFYYXSB"
              <<"m/44'/148'/5' GDTA7622ZA5PW7F7JL7NOEFGW62M7GW2GY764EQC2TUJ42YJQE2A3QUL SDTWG5AFDI6GRQNLPWOC7IYS7AKOGMI2GX4OXTBTZHHYPMNZ2PX4ONWU"
              <<"m/44'/148'/6' GD7A7EACTPTBCYCURD43IEZXGIBCEXNBHN3OFWV2FOX67XKUIGRCTBNU SDJMWY4KFRS4PTA5WBFVCPS2GKYLXOMCLQSBNEIBG7KRGHNQOM25KMCP"
              <<"m/44'/148'/7' GAF4AGPVLQXFKEWQV3DZU5YEFU6YP7XJHAEEQH4G3R664MSF77FLLRK3 SDOJH5JRCNGT57QTPTJEQGBEBZJPXE7XUDYDB24VTOPP7PH3ALKHAHFG"
              <<"m/44'/148'/8' GABTYCZJMCP55SS6I46SR76IHETZDLG4L37MLZRZKQDGBLS5RMP65TSX SC6N6GYQ2VA4T7CUP2BWGBRT2P6L2HQSZIUNQRHNDLISF6ND7TW4P4ER"
              <<"m/44'/148'/9' GAKFARYSPI33KUJE7HYLT47DCX2PFWJ77W3LZMRBPSGPGYPMSDBE7W7X SALJ5LPBTXCFML2CQ7ORP7WJNJOZSVBVRQAAODMVHMUF4P4XXFZB7MKY");
    }

    /**
     * The standard test cases do not cover 2, 3 and 4 byte accounts.
     */
    void larger_account_numbers() {
      testDerivedAccounts(case1Bip39Seed,
                          QList<QString>()
              <<"m/44'/148'/255' GCTRTUTKLQH6FDTYKPJRHJ6WDWWFDSLHAGACOHO24EOLBAH6C5FV7A42 SC2FHENKO4ZVTE5FHJFCUHVYXPEX5ZAK5CD22RNDGX7U4FKEN7GILR2L"
              <<"m/44'/148'/256' GDQPW7IEO2JJFDYZIWHAAURS4LVEVJLS32ORNDT5C65VX4QBBZDBXMTK SA22OBLJSYNDSI6Q7U7MWAZS46VSOLZIZBEMFGIRJQLN7T52ALHRBY76"
              <<"m/44'/148'/65535' GA5C44SIX3OT26KRWEMSDQ6WUYM7X2S3FBCSVQFVXFAPDEZVE52XGGZ5 SCBKC5IYD2QOBDOJBBCS35AL4BCFUAZ4TUKMMQWRWQO6DF5NMMCHUNWT"
              <<"m/44'/148'/65536' GA3UR4B2UTDK43NVWN2CRGY3UK3SBXW43Z6JEOFFN6RCWKLJ4MVU6SBG SDEYL3NP4FTV6CMZQG4LLJDOG6K3JGU43KRMEWEUQJYNAGHL4TMJ3CPY"
              <<"m/44'/148'/16777215' GCVG47JM5NZT6QGT54XDSH3N2JQ6BGROZLV4OEPUVSX5Y73EXCWTELFA SDMCLW2644HORS5AXBNBTWK5UPFJ6OGPZNNONVU67SFG3BTYZZ5QGJ7N"
              <<"m/44'/148'/16777216' GDCOX23B54OWNO5QS7DRMHUVOPQSCUYBPL6FUA3XN4QXFQG5VFMQ2CLC SAOGZUMN6H3KU5I6VAE75DQZMZID4BQB54ODWBIYSBETBWTF4AMQPNV2"
              <<"m/44'/148'/2147483647' GDFBFRFJ32FAS747QDP3IQBL4JQHJNSVPUYDD2VL74GY7XIPFPX4CNS4 SASBZQXVQJCOIVGW5NHMFJZ7ZDASQAIG6EYNMJWOEKAUSVKEI4LKUDDW");
    }
    void requesting_a_hardened_account_yields_same_key_pair_as_unhardened() {
      QByteArray seed = QByteArray::fromHex(case1Bip39Seed.toLatin1());
      KeyPair * pair = KeyPair::fromBip39Seed(seed,100);
      KeyPair * pairFromHardened = KeyPair::fromBip39Seed(seed, static_cast<int>(100 | 0x80000000));
      QCOMPARE(pair->getAccountId(),pairFromHardened->getAccountId());
      delete pair;
      delete pairFromHardened;
    }
private:
    static void testDerivedAccounts(QString bip39Seed, QList<QString> testCases) {
        for (QString testLine : testCases) {
            QList<QString> testParts = testLine.split(" ");
            Q_ASSERT(testParts.size()==3);
            int account = extractAccount(testParts[0]);
            testDerivedAccount(bip39Seed, account, testParts[1], testParts[2]);
        }
    }

    static int extractAccount(QString path) {
      QString prefix = "m/44'/148'/";
      Q_ASSERT(path.startsWith(prefix));
      int index = path.lastIndexOf('\'');
      return path.mid(prefix.length(),index-prefix.length()).toInt();
    }

    static void testDerivedAccount(QString bip39Seed, int accountNumber, QString expectedAccountId, QString expectedSecret) {
        auto seed = QByteArray::fromHex(bip39Seed.toLatin1());
        KeyPair * pair = KeyPair::fromBip39Seed(seed,accountNumber);
        QCOMPARE(pair->getAccountId(),expectedAccountId);
        QCOMPARE(pair->getSecretSeed(),expectedSecret);
        delete pair;
    }

};

ADD_TEST(SEP0005KeyPairForAccountFromBip39SeedTest)

#endif // SEP0005KEYPAIRFORACCOUNTFROMBIP39SEEDTEST_H
