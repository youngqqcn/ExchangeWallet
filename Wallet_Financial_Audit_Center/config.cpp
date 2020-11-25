#include "config.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <QFile>
#include <QProcessEnvironment>


std::vector<QString> g_vctERC20Tokens ;
QMap<QString, QVariant> g_mapERC20Url;

std::vector<QString> g_vct_HRC20_Tokens;
QMap<QString, QVariant> g_map_HRC20_Url;

QString g_qstr_MainExeName = ""; //本程序的名称(不包含路径)   如   AM_Mng_Exe.exe
QString g_qstr_PrivFilePwd = ""; //RSA 私钥文件加密密码

bool g_bBlockChainMainnet = false; //是否是区块链主网   false:不是主网   true:是主网
bool g_bEnableWithdraw = false; //是否开启提币
bool g_bEnableCharge = false; //是否开启充币

bool g_bEnableBitcoindExe = false; //开启bitcoind.exe 用于btc自动提币签名
bool g_bEnableOmnicoredExe = false; //开启omnicored.exe 用于usdt自动提币签名
bool g_bEnableLitecoindExe = false;  //开启litecoind.exe 用于ltc自动提币签名
bool g_bEnableBchcoindExe = false; //开启bchcoind.exe 用于bch自动提币签名
bool g_bEnableBsvcoindExe = false; //开启bsvcoind.exe 用于bsv自动提币签名
bool g_bEnableDashdExe = false; //开启dashd.exe 用于dash自动提币签名
bool g_bEnableRippledExe = false; //开启 rippled.exe
bool g_bEnableMoneroWalletRpcExe = false; //开启  monerod.exe 用于签名

QString g_qstr_EncryptionKey = ""; //用于跟Java交互的数据加密,在环境变量中配置:这个值得格式为 "00 00 0s ds 23 43 56 4f" 前后不能有空白字符,必须由数字和字母组成


QString g_qstr_JavaHttpIpPort = ""; //格式 http://192.168.10.243:9001
QString g_qstr_WalletHttpIpPort = ""; //格式  //格式  http://192.168.10.79:9000

QString g_qstr_JAVA_selectWithdrawOrderInfo = "";
QString g_qstr_JAVA_upWithdrawOrderStatus = "";

QString g_qstr_JAVA_getUserAsset = "";
QString g_qstr_JAVA_getUserFlow = "";



QString g_qstr_WalletAPI_btc_createrawtransaction = "";
QString g_qstr_WalletAPI_btc_sendrawtransaction = "";

QString g_qstr_WalletAPI_ltc_createrawtransaction = "";
QString g_qstr_WalletAPI_ltc_sendrawtransaction = "";

QString g_qstr_WalletAPI_bch_createrawtransaction = "";
QString g_qstr_WalletAPI_bch_sendrawtransaction = "";

QString g_qstr_WalletAPI_bsv_createrawtransaction = "";
QString g_qstr_WalletAPI_bsv_sendrawtransaction = "";

QString g_qstr_WalletAPI_dash_createrawtransaction = "";
QString g_qstr_WalletAPI_dash_sendrawtransaction = "";


QProcess *g_ptr_qprocess_Bitcoind = NULL ;
QProcess *g_ptr_qprocess_Omnicored = NULL;
QProcess *g_ptr_qprocess_Litecoind = NULL;
QProcess *g_ptr_qprocess_Bchcoind = NULL ;
//QProcess *g_ptr_qprocess_Bsvcoind = NULL ;
QProcess *g_ptr_qprocess_Dashd = NULL ;
QProcess *g_ptr_qprocess_Rippled = NULL;
QProcess *g_ptr_qprocess_MoneroWalletRpc = NULL;

QString g_qstr_http_or_https = "";

QString g_qstr_Usdt_OffSig_Port = "";
QString g_qstr_Btc_OffSig_Port = "";
QString g_qstr_Ltc_OffSig_Port = "";
QString g_qstr_Bch_OffSig_Port = "";
QString g_qstr_Bsv_OffSig_Port = "";
QString g_qstr_Dash_OffSig_Port = "";
QString g_qstr_Xmr_OffSig_Port = "";

//QString g_qstr_Btc_Node_Http_Ip_Port = "";
//QString g_qstr_Usdt_Node_Http_Ip_Port = "";
//QString g_qstr_Ltc_Node_Http_Ip_Port = "";

#define MID_ERC20_GASLIMIT   "60000"

const std::map<QString/*代币简称(必须大写)*/, std::tuple<QString/*主网合约地址*/, QString/*测试网合约地址*/, int/*小数位*/, QString> >  gc_mapERC20TokensCfg = {
    {"AJC",std::make_tuple("0x38362bbc51086eb738123602059983201ef55631",    "0xadafb030ca6946f33390780f0f640de1132b6d6c",   18, MID_ERC20_GASLIMIT)}, //51237
    {"KTV",std::make_tuple("0xf6da9A11C213c6039b8d56f3a3eba8A692D6afdc",    "0x75c38E56694d12361B462E30531e966bdd50A019",   18, MID_ERC20_GASLIMIT)}, //55491

    {"LILY",std::make_tuple("0xcde4b9531eea02e1a5a4cd596790661c1c181ca7",    "0xd2eab29c836C35C39C42776a612134c1Dc81df96",   18, MID_ERC20_GASLIMIT)}, //51192
    {"EUTD",std::make_tuple("0xd8e97cc29552cee2bca5bd7f9e979c48ae6076d6",    "0x2Ad350664F1e195a7e248Da8688fBa7043838A78",   18, MID_ERC20_GASLIMIT)}, //51216

    {"ERC20-USDT",std::make_tuple("0xdAC17F958D2ee523a2206206994597C13D831ec7",    "0xEca059F3d6De135E520E789CDfEeCBf5CECa3770",   6, MID_ERC20_GASLIMIT)}, //56221
    {"OMG",    std::make_tuple("0xd26114cd6EE289AccF82350c8d8487fedB8A0C07",       "0x34db9c9127f523e1bfc5d4e5bd1f0a679d7317c2",  18, MID_ERC20_GASLIMIT)}, //52298
    {"MKR",    std::make_tuple("0x9f8f72aa9304c8b593d555f12ef6589cc3a579a2",       "0xeE6712Ac79954c165e9A2f9C9d4533dd43FC2828",  18, MID_ERC20_GASLIMIT)}, //52502
    {"LOOM",   std::make_tuple("0xa4e8c3ec456107ea67d3075bf9e3df3a75823db0",       "0x297B89C58Beb30291dE98a746e58D6E6BDE54Da9",  18, MID_ERC20_GASLIMIT)}, //52065
    {"MCO",    std::make_tuple("0xb63b606ac810a52cca15e44bb630fd42d8d1d83d",       "0xf6971E7E0af35aB1Ae773C391D13dc173c783a1B",   8, MID_ERC20_GASLIMIT)}, //52668
    {"CVC",    std::make_tuple("0x41e5560054824ea6b0732e656e3ad64e20e94e45",       "0xb28964D3191988b7c5a1edAfD58c74C815617bc1",   8, MID_ERC20_GASLIMIT)}, //51447
    {"REP",    std::make_tuple("0x1985365e9f78359a9B6AD760e32412f4a445E862",       "0x6527Fe1Eb1ad971c0a816d5A558C99cD49f00418",  18, "90000")}, //82704
    {"CTXC",   std::make_tuple("0xEa11755Ae41D889CeEc39A63E6FF75a02Bc1C00d",       "0x489cb65f6e8B4093762079E8980495CE349bF9E0",  18, MID_ERC20_GASLIMIT)}, //53446
    {"ABT",    std::make_tuple("0xb98d4c97425d9908e66e53a6fdf673acca0be986",       "0x0698424f88F0f52F386F2562F57dEC8f3a1D12Cb",  18, MID_ERC20_GASLIMIT)}, //53113
    {"PPT",    std::make_tuple("0xd4fa1460f537bb9085d22c7bccb5dd450ef28e3a",       "0xE8a4Bc84EbdE4912BAae28C58f9c95EEed000617",   8, MID_ERC20_GASLIMIT)},  //52653
    {"FSN",    std::make_tuple("0xd0352a019e9ab9d757776f532377aaebd36fd541",       "0x6deC13A6E7d90Ff6eC26aA2740272aB93D8278C7",  18, MID_ERC20_GASLIMIT)}, //52447
    {"REQ",    std::make_tuple("0x8f8221afbb33998d8584a2b05749ba73c37a938a",       "0x685CdE0e8eb60100E8F0E0b2889483298731132a",  18, MID_ERC20_GASLIMIT)}, //52448
    {"TNT",    std::make_tuple("0x08f5a9235b08173b7569f83645d2c7fb55e8ccd8",       "0x167C507DaFDcB37A30232635d3995c6841043f5D",   8, MID_ERC20_GASLIMIT)}, //52316

    {"LINK" ,   std::make_tuple("0x514910771af9ca656af840dff83e8264ecf986ca",    "0xA73a509456CeEA0a4E82Dc38F6FB480c48bAadB0",  18, MID_ERC20_GASLIMIT)}, //51541
    {"ZIL"  ,   std::make_tuple("0x05f4a42e251f2d52b8ed15e9fedaacfcef1fad27",    "0x3cDB7ef06726a5c530Bd9b05b748481528fe1720",  12, MID_ERC20_GASLIMIT)}, //此Token已经停止
    {"ELF"  ,   std::make_tuple("0xbf2179859fc6d5bee9bf9158632dc51678a4100e",    "0x7c220fdDfec15b60D5b35012784c99c9eA61ce7E",  18, MID_ERC20_GASLIMIT)}, //53316
    {"XMX"  ,   std::make_tuple("0x0f8c45b896784a1e408526b9300519ef8660209c",    "0xf78f5bAe94CA84023177c1c0fa7128672069EB0D",   8, MID_ERC20_GASLIMIT)}, //53181
    {"TNB"  ,   std::make_tuple("0xf7920b0768ecb20a123fac32311d07d193381d6f",    "0x65BDB39Ec96Af430041fD1232bD67590f52c7433",  18, "120000")}, //117082

    {"ECNY",    std::make_tuple("0x6dece8681928fb7feccf4c36c1f66852bf53285f",   "0xc2f4df10e7fcde92df821ae12569824316381a86",   18, MID_ERC20_GASLIMIT)}, //53470
    {"SPQQ",    std::make_tuple("0x4b351C83c121CBbF337dBd52B104b014E4999237",   "0xA5B9b49A50ed529DE97D57b320355EB0A71f68D2",   18, MID_ERC20_GASLIMIT)}, //51144
    {"BJC",     std::make_tuple("0xb618a25e74d77a51d4b71ec1dff80c6686be9d0a",   "0x9aa64145f63678756db7be37a34017e09394090b",   18, MID_ERC20_GASLIMIT)}, //51144
    {"BEI",     std::make_tuple("0xe4189a5d4174ac6177b406aae8c79228a4ebe02f",   "0xcc4ec193ef87a5b02a25f3d8fc0fdeacf3255ca9",   18, MID_ERC20_GASLIMIT)}, //51144
    {"HT" ,     std::make_tuple("0x6f259637dcd74c767781e37bc6133cd6a68aa161",   "0xd8f102e3a9ef15099daaac1481ee262ee5515d87",   18, MID_ERC20_GASLIMIT)}, //未开放
    {"BNB",     std::make_tuple("0xb8c77482e45f1f44de1745f52c74426c631bdd52",   "0xe30d2b6f144e6ead802414c09a3f519b72ff12f3",   18, MID_ERC20_GASLIMIT)}, //未开放
    {"LEO",     std::make_tuple("0x2af5d2ad76741191d15dfe7bf6ac92d4bd912ca3",   "0x5a2cbd017beeed9983bbc94ce8898bd7764db916",   18, MID_ERC20_GASLIMIT)}, //154328  未开放
    {"YQB",     std::make_tuple("",                                             "0x130fc2749d35fe026f32427f60dd3f2ecb6c2f33",   18, MID_ERC20_GASLIMIT)}, //无
    {"MBJC",    std::make_tuple("",                                             "0x92ae9a4a93018477e4fbe3eea921c9e1b5338215",   18, MID_ERC20_GASLIMIT)}, //无
};

const std::map<QString/*代币简称(必须大写)*/, std::tuple<QString/*主网合约地址*/, QString/*测试网合约地址*/, int/*小数位*/> >  gc_map_HRC20_tokens_config = {
    {"HRC20-AJC",std::make_tuple("",                                                "htdf1nkkc48lfchy92ahg50akj2384v4yfqpm4hsq6y",   18)},
    {"BWC",std::make_tuple("htdf1j8apljndmma2vjxvqtz4rd6dk3fe0yuyyakhun",           "htdf12dvguqedrvgfrdl35hcgfmz4fz6rm6chrvf96g",   18)},
    {"HFH",std::make_tuple("htdf1vek5hjdqhcgj8ljqewgcex8y9eg9agwa3f69ez",           "htdf1y2tmw3aa65dvlgcpsz2pd9vqyz3587vncjlmld",   18)},

    //源自: https://www.htdfscan.com/index.html#/token
    {"HRC20-HET" ,std::make_tuple("htdf1war5w36s3ym6qv2vnfd7k803wu3lkuwksh8md8",    "htdf178qsds3kzu37zlpd2ff7gulmk4z2gupwd8qm3p",   18)},
    {"HRC20-USDP",std::make_tuple("htdf14mc59zt5r2v0u40vdjec64323ypp08lslqc9uv",    "htdf1ahm5zclt52pd58msy9vdtd5059f8cukwlkcwnm",   18)},
    {"HRC20-BEI" ,std::make_tuple("htdf1rcl9x2akjuy3tyqyxwk7sataldzmxy6jm06hq4",    "htdf1djt7ffg4pdlma7q70v7c4vx4h7uukl7e5y4rvf",   18)},
    {"SJC" ,std::make_tuple("htdf1f23nuwptf7rqns3f73lu9xpseql0je8twn2zuk",          "htdf1hpvz7jl0mvtpjc4hs4gku858gj47q6nva606jc",   18)},
    {"AQC" ,std::make_tuple("htdf1lsj9xuxyczru272je6wjd9aeg8vkg23qae5ug2",          "htdf1lnn5k8p080dyhmtkcrd6936yd86um4g6r5ms46",   18)},

    {"BTU" ,std::make_tuple("htdf1w43aazq9sjlcrwj4y7nnsck7na5zljzu4x5nrq",          "htdf1vw4dq4teurls7yg8254pz5esn0gpg0492yvt95",   18)},
    {"SVU" ,std::make_tuple("htdf18y0ks24fat9unc4ezf83zvqu470dtrsvldyev7",          "htdf19cwnd3xwnlcce56cjd848d42xsp8gcg5fsm7pa",   18)},
    {"BKL" ,std::make_tuple("htdf15xsj43lssr26kw0pxn9qrl3yau2n5wu4dhk5rp",          "htdf132atpl4xs9hd5qhjctyhtsc70nzanv6vmlagzq",   18)},

    {"JXC" ,std::make_tuple("htdf1u9cwqeq4tcnar8y8xk8udn5mll3dt5t8xlxqt0",          "htdf1ywr835q6n03jv0th47l6k6qjv93nvlygkcxjnf",   18)},

    {"AGG" ,std::make_tuple("htdf18xq0p7h8rwxykfyrfytylhmrtusmzfwst4s6jx",          "htdf1jacum30uyygdk0lr483nzg4xfk6l276966ugtk",   18)},
    {"HHG" ,std::make_tuple("htdf16kz9ts938wramc4gtn8fz8stnha6nm64w5tra8",          "htdf18d6eeve6a2p06mn55wf957r476udhw6utq6x8v",   18)},

     // 2020-08-20  第1批次
    {"HTD", std::make_tuple("htdf1le2awsf84s37ar4fx9wc8ruwsljgrrzz8agzsn", 		"htdf1pn5kxnekyns4mneuzylyka3fyhz9hnppdruwt4" , 18)},
    {"KML", std::make_tuple("htdf1ehhd5qugg7y02rdnvlxcy92qg02m6dqhttauvh", 		"htdf1d5na3tf0pun72mmprl2ezdn72lv4uzm64nsmj0" , 18)},
    {"MSL", std::make_tuple("htdf1ldkdf84lr0qeydcp6la90hvcww5hx0z4u5zvja", 		"htdf19xzu8pnkryfmww3l5z4mcmll9qxnt8dla0yyn3" , 18)},
    {"DFQ", std::make_tuple("htdf10yxugqes29eahqk227zv0uayt0qedvzaxl24jy", 		"htdf1fx09nq997app7knaztm75tddm73vwwc8l66kye" , 18)},
    {"TTB", std::make_tuple("htdf1kg362pxynmkvvr5cutwf5ksp5qh7lhpzrxesp9", 		"htdf1hkqxrp2w3222edgv3g535exzfw2yls7qm5dmxv" , 18)},
};

//////////////////////////////////////////////////

static bool s_bIsBlockChainMainnet = false;
//static std::string s_cstrJavaIP = "";
//static std::string s_cstrJavaPort = "";
static std::string s_cstrWalletIP = "";
static std::string s_cstrWalletPort =  "";
static bool s_bEnableCharge = false;
static bool s_bEnableWithdraw = false;
static std::string s_cstrUSDT_NODE_URL = "";

static int ReadConfigFile(QString &strErrMsg)
{
    try
    {
        using namespace boost::property_tree;
        ptree ptreeAll;
        QString strCfgFilePath = G_STR_CONFIG_FILE_PATH;
        QFile cfgFile(strCfgFilePath);
        if(false == cfgFile.exists())
        {
            //TODO:弹出对话提示,配置文件不存在
            strErrMsg = "config.ini not exists.";
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg ;
            return -1;
        }

        ini_parser::read_ini(strCfgFilePath.toStdString(), ptreeAll);

        //读取Java后台IP和端口
        basic_ptree<std::string, std::string> ptreeSetting = ptreeAll.get_child("setting");

        //区块链主网测试网络
        int iMainnet = ptreeSetting.get<int>("blockchain_mainnet");
        s_bIsBlockChainMainnet = (0 == iMainnet) ? (false) : (true);


        //TODO:检查端口号
        std::string strHttpJavaIpPort =  ptreeSetting.get<std::string>("java_http_ip_port");
        g_qstr_JavaHttpIpPort = QString::fromStdString(strHttpJavaIpPort );

        //TODO:检查端口号  读取钱包服务端IP和端口
        std::string strWalletHttpIpPort = ptreeSetting.get<std::string>("wallet_http_ip_port");
        g_qstr_WalletHttpIpPort = QString::fromStdString( strWalletHttpIpPort);
        //充币配置
        int iEnableCharge = ptreeSetting.get<int>("enable_charge");
        s_bEnableCharge = (0 == iEnableCharge) ? (false) :(true);

        //充币配置
        int iEnableWithdraw = ptreeSetting.get<int>("enable_withdraw");
        s_bEnableWithdraw = (0 == iEnableWithdraw ) ? (false) : (true);

        //btc自动签名依赖的后台进程
        int iEnableBitcoindExe = ptreeSetting.get<int>("enable_bitcoind_exe");
        g_bEnableBitcoindExe = (0 == iEnableBitcoindExe) ? (false) : (true);

        //usdt自动签名依赖的后台进程
        int iEnableOmnicoredExe = ptreeSetting.get<int>("enable_omnicored_exe");
        g_bEnableOmnicoredExe = (0 == iEnableOmnicoredExe) ? (false) : (true);

        //ltc自动签名依赖的后台进程
        int iEnableLitecoindExe = ptreeSetting.get<int>("enable_litecoind_exe");
        g_bEnableLitecoindExe = (0 == iEnableLitecoindExe) ? (false) : (true);

        //bch自动签名依赖的后台进程
        int iEnableBchcoindExe = ptreeSetting.get<int>("enable_bchcoind_exe");
        g_bEnableBchcoindExe = (0 == iEnableBchcoindExe) ? (false) : (true);

        //bsv自动签名依赖的后台进程
        int iEnableBsvcoindExe = ptreeSetting.get<int>("enable_bsvcoind_exe");
        g_bEnableBsvcoindExe = (0 == iEnableBsvcoindExe) ? (false) : (true);

        //dash自动签名依赖的后台进程
        int iEnableDashdExe = ptreeSetting.get<int>("enable_dashd_exe");
        g_bEnableDashdExe = (0 == iEnableDashdExe) ? (false) : (true);

        //enable_rippled_exe
        int iEnableRippledExe = ptreeSetting.get<int>("enable_rippled_exe");
        g_bEnableRippledExe = (0 == iEnableRippledExe) ? (false) : (true);


        //monero-wallet-rpc.exe
        int iEnableMoneroWalletRpcExe = ptreeSetting.get<int>("enable_monero_wallet_rpc_exe");
        g_bEnableMoneroWalletRpcExe = (0 == iEnableMoneroWalletRpcExe) ? (false) : (true);


        //获取所有支持的ERC20 Tokens
        {
        g_vctERC20Tokens.clear();
        std::string cstrTokens = "";
        if (s_bIsBlockChainMainnet){
            cstrTokens = ptreeSetting.get<std::string>("erc20_main");
        }else{
            cstrTokens = ptreeSetting.get<std::string>("erc20_test");
        }

        if(cstrTokens.empty()) g_vctERC20Tokens.clear();
        QString strTokens = QString::fromStdString(cstrTokens);
        QStringList strlstTokens = strTokens.split(";");
        for(QString strToken : strlstTokens )
        {
            if(!strToken.trimmed().isEmpty())
                g_vctERC20Tokens.push_back(strToken.trimmed());
        }
        }

        //获取所有 HRC20 代币币种列表
        {
        g_vct_HRC20_Tokens.clear();
        std::string cstrTokens = "";
        if (s_bIsBlockChainMainnet){
            cstrTokens = ptreeSetting.get<std::string>("hrc20_main");
        }else{
            cstrTokens = ptreeSetting.get<std::string>("hrc20_test");
        }

        if(cstrTokens.empty()) g_vct_HRC20_Tokens.clear();
        QString strTokens = QString::fromStdString(cstrTokens);
        QStringList strlstTokens = strTokens.split(";");
        for(QString strToken : strlstTokens )
        {
            if(!strToken.trimmed().isEmpty())
                g_vct_HRC20_Tokens.push_back(strToken.trimmed());
        }
        }



        //设置所有erc20币种的充币请求URL
        g_mapERC20Url.clear();
        for (auto it : g_vctERC20Tokens)
        {
            g_mapERC20Url.insert(it, QString("/%1/crawltransactions?blknumber=").arg(it.toLower()));
        }


        //设置所有 HRC20 币种的充币请求URL
        g_map_HRC20_Url.clear();
        for(auto it : g_vct_HRC20_Tokens)
        {
            g_map_HRC20_Url.insert(it, QString("/%1/crawltransactions?blknumber=").arg(it.toLower()));
        }

    }
    catch(std::exception &e)
    {
        //TODO: 处理读取配置异常的情况
        strErrMsg = QString::fromStdString(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        return -1;
    }

    return 0;
}

static int InitGlobalVariable(QString &strErrMsg)
{
    strErrMsg = "";

    //区块链  主网  测试网
    g_bBlockChainMainnet = s_bIsBlockChainMainnet;


    //提币配置
    g_bEnableWithdraw = s_bEnableWithdraw;

    //充币配置
    g_bEnableCharge = s_bEnableCharge;


    std::string cstrTmpUrl = "";

    //提币   TODO:检查URL的合法性
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_JavaHttpIpPort.toStdString() + std::string(STR_web_btb_btbOrder) + std::string(STR_selectWithdrawOrderInfo);
    g_qstr_JAVA_selectWithdrawOrderInfo = QString::fromStdString(cstrTmpUrl);

    cstrTmpUrl.clear();
    cstrTmpUrl =  g_qstr_JavaHttpIpPort.toStdString() + std::string(STR_web_btb_btbOrder) + std::string(STR_upWithdrawOrderStatus);
    g_qstr_JAVA_upWithdrawOrderStatus = QString::fromStdString(cstrTmpUrl);


    //充币 TODO:检查URL的合法性


    //用户资产(用户在交易所用户的各种数字货币的数量)
    cstrTmpUrl.clear();
    cstrTmpUrl =  g_qstr_JavaHttpIpPort.toStdString() + std::string(STR_web_btb_btbDetailLog) + std::string(STR_getMemberBtbCoin);
    g_qstr_JAVA_getUserAsset = QString::fromStdString(cstrTmpUrl);

    //用户流水
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_JavaHttpIpPort.toStdString() + std::string(STR_web_btb_btbDetailLog) + std::string(STR_getBtbDetailLog);
    g_qstr_JAVA_getUserFlow = QString::fromStdString(cstrTmpUrl);

    //钱包接口  createrawtransaction
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_btc_createrawtransaction);
    g_qstr_WalletAPI_btc_createrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_ltc_createrawtransaction);
    g_qstr_WalletAPI_ltc_createrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_bch_createrawtransaction);
    g_qstr_WalletAPI_bch_createrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_bsv_createrawtransaction);
    g_qstr_WalletAPI_bsv_createrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_dash_createrawtransaction);
    g_qstr_WalletAPI_dash_createrawtransaction = QString::fromStdString(cstrTmpUrl);

    //钱包接口 sendrawtransaction
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_btc_sendrawtransaction);
    g_qstr_WalletAPI_btc_sendrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_ltc_sendrawtransaction);
    g_qstr_WalletAPI_ltc_sendrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_bch_sendrawtransaction);
    g_qstr_WalletAPI_bch_sendrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_bsv_sendrawtransaction);
    g_qstr_WalletAPI_bsv_sendrawtransaction = QString::fromStdString(cstrTmpUrl);
    cstrTmpUrl.clear();
    cstrTmpUrl = g_qstr_WalletHttpIpPort.toStdString() + std::string(STR_dash_sendrawtransaction);
    g_qstr_WalletAPI_dash_sendrawtransaction = QString::fromStdString(cstrTmpUrl);


    return 0;
}

int StartOutsideExe(QString &strErrMsg)
{
    strErrMsg = "";

    //如果要开启btc的后台进程
    g_qstr_Btc_OffSig_Port =  (g_bBlockChainMainnet) ? ("8332") : ("18332");
    if(true == g_bEnableBitcoindExe && NULL != g_ptr_qprocess_Bitcoind)
    {
        QString strBitcoind = ".\\daemon\\bitcoind.exe";
        QStringList strListArgs;
        strListArgs.clear();
        strListArgs << "-datadir=.\\daemon\\btc_data" <<  "-dbcache=10240" << "-txindex=1" << "-rpcuser=btc" << "-rpcpassword=btc2018";//"-conf=./daemon/bitcoin.conf";
        strListArgs << "-server=1" << "-rpcallowip=0.0.0.0/0" << "-maxconnections=0";
        if(true == g_bBlockChainMainnet)
        {
            strListArgs << "-rpcbind=127.0.0.1:8332" << "-testnet=0";
        }
        else
        {
            strListArgs << "-rpcbind=127.0.0.1:18332" << "-testnet=1";
        }

        g_ptr_qprocess_Bitcoind->start(strBitcoind, strListArgs);
    }

    //如果要开启usdt的后台进程
    g_qstr_Usdt_OffSig_Port = (g_bBlockChainMainnet) ? ("8335") : ("18335");
    if(true == g_bEnableOmnicoredExe && NULL != g_ptr_qprocess_Omnicored)
    {
        QString strOmnicored = ".\\daemon\\omnicored.exe";
        QStringList strListArgs;
        strListArgs.clear();
        strListArgs << "-datadir=.\\daemon\\usdt_data" <<  "-dbcache=10240" << "-txindex=1" << "-rpcuser=btc" << "-rpcpassword=btc2018";//"-conf=./daemon/bitcoin.conf";
        strListArgs << "-server=1" << "-rpcallowip=0.0.0.0/0" << "-maxconnections=0";

        if(true == g_bBlockChainMainnet)
        {
            strListArgs << "-rpcbind=127.0.0.1:8335" << "-testnet=0";
        }
        else
        {
            strListArgs << "-rpcbind=127.0.0.1:18335" << "-testnet=1";
        }
        g_ptr_qprocess_Omnicored->start(strOmnicored, strListArgs);
    }

    //如果要开启ltc的后台进程
    g_qstr_Ltc_OffSig_Port = (g_bBlockChainMainnet) ? ("8089") : ("18089");
    if(true == g_bEnableLitecoindExe && NULL != g_ptr_qprocess_Litecoind )
    {
        QString strLiteCoincored = ".\\daemon\\litecoind.exe";
        QStringList strListArgs;
        strListArgs.clear();
        strListArgs << "-datadir=.\\daemon\\ltc_data" <<  "-dbcache=10240" << "-txindex=1" << "-rpcuser=ltc" << "-rpcpassword=ltc2018";//"-conf=./daemon/litecoind.conf";
        strListArgs << "-server=1" << "-rpcallowip=0.0.0.0/0" << "-maxconnections=0";

        if(true == g_bBlockChainMainnet)
        {
            strListArgs << "-rpcbind=127.0.0.1:8089" << "-testnet=0";
        }
        else
        {
            strListArgs << "-rpcbind=127.0.0.1:18089" << "-testnet=1";
        }
        g_ptr_qprocess_Litecoind->start(strLiteCoincored, strListArgs);
    }


    //如果要开启bch的后台进程
    g_qstr_Bch_OffSig_Port = (g_bBlockChainMainnet) ? ("8336") : ("18336");
    if(true == g_bEnableBchcoindExe && NULL != g_ptr_qprocess_Bchcoind)
    {
        QString strBchCoincored = ".\\daemon\\bchcoind.exe";
        QStringList strListArgs;
        strListArgs.clear();
        strListArgs << "-datadir=.\\daemon\\bch_data" <<  "-dbcache=10240" << "-txindex=1" << "-rpcuser=bch" << "-rpcpassword=bch2018";//"-conf=./daemon/litecoind.conf";
        strListArgs << "-server=1" << "-rpcallowip=0.0.0.0/0" << "-maxconnections=0";

        if(true == g_bBlockChainMainnet)
        {
            strListArgs << "-rpcbind=127.0.0.1:8336" << "-testnet=0";
        }
        else
        {
            strListArgs << "-rpcbind=127.0.0.1:18336" << "-testnet=1";
        }
        g_ptr_qprocess_Bchcoind->start(strBchCoincored, strListArgs);
    }

    // BSV 直接使用 BCH的后台进程进行签名即可
    //如果要开启bsv的后台进程
    /*g_qstr_Bsv_OffSig_Port = (g_bBlockChainMainnet) ? ("8337") : ("18337");
    if(true == g_bEnableBsvcoindExe && NULL != g_ptr_qprocess_Bsvcoind)
    {
        QString strBsvCoincored = ".\\daemon\\bsvcoind.exe";
        QStringList strListArgs;
        strListArgs.clear();
        strListArgs << "-datadir=.\\daemon\\bsv_data" <<  "-dbcache=10240" << "-txindex=1" << "-rpcuser=bsv" << "-rpcpassword=bsv2018";//"-conf=./daemon/litecoind.conf";
        strListArgs << "-server=1" << "-rpcallowip=0.0.0.0/0" << "-maxconnections=0";

        if(true == g_bBlockChainMainnet)
        {
            strListArgs << "-rpcbind=127.0.0.1:8337" << "-testnet=0";
        }
        else
        {
            strListArgs << "-rpcbind=127.0.0.1:18337" << "-testnet=1";
        }
        g_ptr_qprocess_Bsvcoind->start(strBsvCoincored, strListArgs);
    }*/

    //如果要开启dash的后台进程
    g_qstr_Dash_OffSig_Port = (g_bBlockChainMainnet) ? ("9998") : ("19998");
    if(true == g_bEnableDashdExe && NULL != g_ptr_qprocess_Dashd)
    {
        QString strDashCoincored = ".\\daemon\\dashd.exe";
        QStringList strListArgs;
        strListArgs.clear();
        strListArgs << "-datadir=.\\daemon\\dash_data" <<  "-dbcache=10240" << "-txindex=1" << "-rpcuser=dash" << "-rpcpassword=dash2018";//"-conf=./daemon/litecoind.conf";
        strListArgs << "-server=1" << "-rpcallowip=0.0.0.0/0" << "-maxconnections=0";

        if(true == g_bBlockChainMainnet)
        {
            strListArgs << "-rpcbind=127.0.0.1:9998" << "-testnet=0";
        }
        else
        {
            strListArgs << "-rpcbind=127.0.0.1:19998" << "-testnet=1";
        }
        g_ptr_qprocess_Dashd->start(strDashCoincored, strListArgs);
    }

    //如果要开启ripple的后台进程, ripple不区分主网和测试网
    //.\daemon\xrp\rippled.exe --net --silent --conf .\daemon\xrp\rippled.cfg
    if(true == g_bEnableRippledExe && NULL != g_ptr_qprocess_Rippled)
    {
        //QString strRippled = ".\\daemon\\xrp\\rippled.exe";
        //QStringList strListArgs;
        //strListArgs.clear();
        //strListArgs << "--net" << "--silent" << "--conf .\\daemon\\xrp\\rippled.cfg";
        //g_ptr_qprocess_Rippled->start(strRippled, strListArgs);
        //qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "hello1710";

        //使用 参数列表, 进程起不来, 直接执行命令是可以的
        QString strCmd = ".\\daemon\\xrp\\rippled.exe --net --silent --conf .\\daemon\\xrp\\rippled.cfg";
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strCmd;
        g_ptr_qprocess_Rippled->start(strCmd);
    }


    //开启 monero-wallet-rpc.exe 进程,
    //测试网:  monero-wallet-rpc --stagenet --wallet-file=./manual_withdraw/cold_wallet --confirm-external-bind --rpc-bind-ip 0.0.0.0 --rpc-bind-port 48089 --password 123456 --disable-rpc-login  --offline
    //主网: monero-wallet-rpc  --wallet-file=./manual_withdraw/cold_wallet --confirm-external-bind --rpc-bind-ip 0.0.0.0 --rpc-bind-port 48089 --password 123456 --disable-rpc-login  --offline
    g_qstr_Xmr_OffSig_Port  = (g_bBlockChainMainnet) ? ("18088") : ("48088"); //使用 48088 端口 , 防止测试环境下 AM 和 WM在同一个机器上, 端口占用
    if( true == g_bEnableMoneroWalletRpcExe && NULL != g_ptr_qprocess_MoneroWalletRpc )
    {
        //TODO: 钱包文件密码由管理员输入
        QString strCmd = ".\\daemon\\xmr\\monero-wallet-rpc  --wallet-file=.\\daemon\\xmr\\manual_withdraw\\cold_wallet --confirm-external-bind --rpc-bind-ip 0.0.0.0  --password 123456 --disable-rpc-login  --offline ";
        strCmd += (g_bBlockChainMainnet) ? ("  ") : ( " --stagenet ");
        strCmd +=  "  --rpc-bind-port  " + g_qstr_Xmr_OffSig_Port + "  " ;
        qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strCmd;
        g_ptr_qprocess_MoneroWalletRpc->start(strCmd);

        //TODO: 进行测试

        /*
        strErrMsg = g_ptr_qprocess_MoneroWalletRpc->errorString();
        if(!strErrMsg.isEmpty())
        {
            strErrMsg = QString("启动monero-wallet-rpc.exe 失败: ") + strErrMsg;
            return -1;
        }
        */
    }


    return 0;
}



int KillDaemon(QString &strErrMsg)
{

#define __TASKKILL(proccessname)\
do{\
    QString strCmd = QString( "taskkill /im %1 /f").arg(proccessname);\
    QProcess p(0);\
    p.execute(strCmd);\
    p.close();\
}while(0);

    __TASKKILL("bitcoind.exe");
    __TASKKILL("litecoind.exe");
    __TASKKILL("dashd.exe");
    __TASKKILL("omnicored.exe");
    __TASKKILL("bchcoind.exe");
    __TASKKILL("rippled.exe");
    //__TASKKILL("monero-wallet-rpc.exe"); //TODO: 这样直接杀不优雅!
    if( g_bBlockChainMainnet )
    {
        __TASKKILL("monero-wallet-rpc.exe"); //TODO: 这样直接杀不优雅!
    }

    strErrMsg = "";

    return 0;
}


//初始化程序运行环境
int InitEnvironment(QString &strErrMsg)
{
    int iRet = -1;


    //1.读取配置
    iRet =  ReadConfigFile(strErrMsg);
    if(0 != iRet)
    {
        return iRet;
    }

    

    //2.初始化全局变量
    iRet = InitGlobalVariable(strErrMsg);
    if(0 != iRet)
    {
        return iRet;
    }

    KillDaemon(strErrMsg);

    //3.启动bitcoind.exe    omnicored.exe
    iRet =  StartOutsideExe(strErrMsg);
    if(0 != iRet)
    {
        return iRet;
    }


    return 0;
}












