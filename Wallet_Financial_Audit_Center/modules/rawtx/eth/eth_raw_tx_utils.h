#ifndef ETH_RAW_TX_UTILS_H
#define ETH_RAW_TX_UTILS_H

#include <QObject>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include "raw_tx_utils_base.h"
#include "eth/Transaction.h"
#include "rawtx_comman.h"

using namespace boost::multiprecision;
using ubigint = boost::multiprecision::uint128_t;
//typedef number<backends::cpp_bin_float<200> > cpp_bin_float_200;
using float100 = boost::multiprecision::cpp_bin_float_100;


namespace rawtx
{

//前置声明
bool CheckChainId(uint uChainId);
Implement ETH_ExportRawTxItem;
Implement ETH_ExportRawTxParam;
//导入交易数据的接口参数类
using ETH_ImportRawTxItem = ETH_ExportRawTxItem;
using ETH_ImportRawTxParam = ETH_ExportRawTxParam;



struct ETH_CreateRawTxParam : public IRawTxUtilsParamBase
{
    explicit ETH_CreateRawTxParam() { Init(); }

    virtual void Init() override
    {
        strCoinType                 = "";

        //初始化传入参数
        uChainId                    = eth::ETHChainID::None;
        uNonce                      = ULLONG_MAX;
        float100Value               = 0.0;
        ubigintGasPrice             = 0;
        ubigintGasStart             = 0;
        uDataLen                    = 0;
        strAddrTo                   = "";
        strPrivKey                  = "";

        memset(szData, 0, sizeof(szData));
        //初始化接收返回参数
        memset(szRetBuf, 0, sizeof(szRetBuf));
        uBufSize = sizeof(szRetBuf);
        uRetDataLen = 0;
    }

    virtual bool ParamsCheck(QString *pStrErr) override
    {
        if(ULLONG_MAX == this->uNonce  || this->uNonce > INT_MAX)
        {
            if(NULL != pStrErr) *pStrErr =  QString("nonce to large!!");
            return false;
        }

        /*if( this->float100Value <= 0.00000001  || this->float100Value >= 9999)
        {
            if(NULL != pStrErr) *pStrErr = QString("value is not at range. too small or too large");
            return false;
        }*/

        if(42 != this->strAddrTo.length() ) //包含 '0x'前缀一共42个字符
        {
            if(NULL != pStrErr) *pStrErr = QString("pszAddrTo  is null, or addr's length is error");
            return false;
        }

        if(42 != this->strAddrFrom.length() ) //包含 '0x'前缀一共42个字符
        {
            if(NULL != pStrErr) *pStrErr = QString("pszAddrFrom  is null, or addr's length is error");
            return false;
        }


        if(64 != this->strPrivKey.length() )
        {
            if(NULL != pStrErr) *pStrErr = QString("privkey is null, or privkey's length is error");
            return false;
        }

        //if(this->ubigintGasPrice < 1000 || this->ubigintGasPrice > ubigint("200000000000"))
        if(this->ubigintGasPrice < ubigint(MIN_GASPRICE) || this->ubigintGasPrice > ubigint(MAX_GASPRICE))
        {
            std::string strTmp = ubigintGasPrice.str();
            if(NULL != pStrErr) *pStrErr = QString("ubigintGasPrice %1 is too small or too large, reference value is 21000").arg(strTmp.c_str());
            return false;
        }

        if(this->ubigintGasStart < 1000 || this->ubigintGasStart >   21000*1000)
        {
            if(NULL != pStrErr) *pStrErr = QString("ubigintGasStart is too small or too large, reference value is 1000000000(1Gwei)");
            return false;
        }

        if(false == CheckChainId(uChainId))
        {
            if(NULL != pStrErr ) *pStrErr = QString("uChainId is invalid");
            return false;
        }

        return true;
    }

public:
    QString                         strCoinType;

public: //输入参数
    eth::ETHChainID                 uChainId;               //chain Id
    uint64_t                        uNonce;                 //nonce
    float100                        float100Value;          //单位是, eth
    ubigint                         ubigintGasPrice;        //单位是, wei
    ubigint                         ubigintGasStart;        //单位是, wei
    unsigned char                   szData[2048];           //input data,  ERC20代币交易
    unsigned int                    uDataLen;               //input data长度, 为0即可
    std::string                     strAddrTo;              //目的地址
    std::string                     strPrivKey;             //私钥

public: //额外的参数
    std::string                     strAddrFrom;            //源地址

public: //接收返回值
    unsigned char                   szRetBuf[2048];         //存放签名后的交易的buffer
    unsigned int                    uBufSize;               //buffer的size
    unsigned int                    uRetDataLen;            //返回数据的长度
};


//导出交易数据的接口参数类
struct ETH_ExportRawTxItem
{
    explicit ETH_ExportRawTxItem () { Init(); }
    virtual void Init()
    {
        uChainId                    = eth::ETHChainID::None;
        strNonce                    = "";
        strAddrFrom                 = "";
        strAddrTo                   = "";
        strValue                    = "";
        strGasPrice                 = "";
        strGasStart                 = "";
        strERC20TokenValue          = "";
        strERC20Recipient           = "";

        strSignedRawTxHex           = "";
        strOrderId                  = "";
        bComplete                   = false;
        strSymbol                   = ""; //symbol
    }

    virtual bool ParamsCheck(QString *pStrErr )
    {
        if(false == CheckChainId(uChainId))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uChainId` is invalid. current value is %1").arg(uChainId);
            return false;
        }


        if(strOrderId.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("strOrderId is empty");
            return false;
        }

        if(strNonce.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("strNonce is empty.");
            return false;
        }

        if(strAddrFrom.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("strAddrFrom is empty.");
            return false;
        }

        if(strAddrTo.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("strAddrTo is empty.");
            return false;
        }

        if(42 != strAddrTo.length())
        {
            if(NULL != pStrErr) *pStrErr = QString("strAddrTo length error, not 42 char");
            return false;
        }

        if(42 != strAddrFrom.length())
        {
            if(NULL != pStrErr) *pStrErr = QString("strAddrFrom length error, not 42 char");
            return false;
        }

        if(strValue.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("strValue is empty.");
            return false;
        }

        if(strGasPrice.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("strGasPrice is empty.");
            return false;
        }

        // 防止 gasprice 太大
        bool ok = false;
        quint64 gasprice = strGasPrice.toULongLong(&ok);
        if(!ok || gasprice > MAX_GASPRICE)
        {
            if(NULL != pStrErr ) *pStrErr = QString("strGasPrice: %1 is too big. ").arg(gasprice);
            return false;
        }

        return true;
    }

public:
    QString                         strOrderId;             //订单编号
    bool                            bComplete;              //是否已完成
    QString                         strSymbol;              //币种代号,  ETH为 ETH , 如果是 ERC20为 Token的symbol

public:
    eth::ETHChainID                 uChainId;               //chaindId
    QString                         strNonce;               //nonce  此字段需要通过服务端接口获取,后期考虑是否通过数据库维护此nonce值
    QString                         strAddrFrom;            //from 源地址,此字段不参与交易的签名和创建,用于其他用途
    QString                         strAddrTo;              //to  目的地址
    QString                         strValue;               //value 转账金额,使用浮点数
    QString                         strGasPrice;            //gasprice 在资产管理中设置默认即可
    QString                         strGasStart;            //gasstart 在资产管理默认即可

    QString                         strERC20TokenValue;     //ERC20 代币的金额
    QString                         strERC20Recipient;       //ERC20 代币的接收方, 注意: ERC20代币的to地址是合约地址, recipient才是接收方地址

public: //
    QString                         strSignedRawTxHex;      //创建完成的(已签名)交易数据的十六进制字符串
};



Implement ETH_ExportRawTxParam : public IRawTxUtilsParamBase
{
public:
    explicit ETH_ExportRawTxParam () { Init(); }
    /*explicit*/ ETH_ExportRawTxParam (const ETH_ExportRawTxParam &param)
    {
        this->operator =(param);
    }

    virtual void Init() override
    {
        __m_vctItems.clear();
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        /*
        if(__m_vctItems.empty())
        {
            if(NULL != pStrErr) *pStrErr = QString("params vector is empty");
            return false;
        }
        */
        if(m_strCoinType.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("strCoinType is empty.");
            return false;
        }

        if(m_strFilePath.isEmpty())
        {
            if(NULL != pStrErr ) *pStrErr = QString("file path is empty.");
            return false;
        }

        for(size_t i = 0; i < __m_vctItems.size(); i++)
        {
            if(false ==  __m_vctItems[i].ParamsCheck(pStrErr))
            {
                *pStrErr = QString("params[%1] is invalid:").arg(i) + *pStrErr;
                return false;
            }
        }

        return true;
    }

    void push_back(const ETH_ExportRawTxItem &param) { __m_vctItems.push_back(param); }
    void clear() { __m_vctItems.clear(); }
    const ETH_ExportRawTxItem& operator [] (size_t pos) const { return __m_vctItems[pos]; }
    ETH_ExportRawTxItem& operator [] (size_t pos)
    {
        auto it = __m_vctItems.begin();
        for(size_t i = 0 ; i < pos; i++, it++) { }
        return (*it);
    }
    size_t size() const { return __m_vctItems.size(); }



    ETH_ExportRawTxParam& operator = (const ETH_ExportRawTxParam &params)
    {
        __m_vctItems.clear();
        for(size_t i = 0; i < params.size(); i++)
        {
            __m_vctItems.push_back(params[i]);
        }
        this->m_strCoinType = params.m_strCoinType;
        this->m_strFilePath = params.m_strFilePath;
        this->m_bIsCollection = params.m_bIsCollection;
        return (*this);
    }

public:
    QString                                 m_strCoinType;
    QString                                 m_strFilePath;
    bool        m_bIsCollection;
private:
    std::vector<ETH_ExportRawTxItem>        __m_vctItems;
};



Implement ETH_BroadcastRawTxParam : public IRawTxUtilsParamBase
{
    explicit ETH_BroadcastRawTxParam () { Init(); }
    virtual void Init() override
    {
        strCoinType             = "";

        uChainId                = eth::ETHChainID::None;
        strAddrFrom             = "";
        strNonce                = "";
        strSignedRawTxHex       = "";
        strOrderId              = "";

        strTxid                 = "";
        strErrMsg               = "";
    }

    virtual bool ParamsCheck(QString *pStrErr ) override
    {
        if(strSignedRawTxHex.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString(" `strSignedRawTxHex` is empty.");
            return false;
        }

        if(false == CheckChainId(uChainId))
        {
            if(NULL != pStrErr) *pStrErr = QString("`uChainId` is invalid. current value is %1").arg(uChainId);
            return false;
        }

        if(42 != strAddrFrom.length())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strFromAddr` is invalid. strFromAddr=%1").arg(strAddrFrom);
            return false;
        }

        if( strNonce.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strNonce` is empty. strNonce=%1.").arg(strNonce);
            return false;
        }

        if(strOrderId.isEmpty())
        {
            if(NULL != pStrErr) *pStrErr = QString("`strOrderId` is empty");
            return false;
        }


        return true;
    }

public:
    QString                 strCoinType;        //币种, 区分  ETH 和  ETC

public: //入参
    eth::ETHChainID         uChainId;
    QString                 strAddrFrom;
    QString                 strNonce;
    QString                 strSignedRawTxHex;  //已签名的交易十六进制字符串
    QString                 strOrderId;
public://结果
    QString                 strTxid;            //txid
    QString                 strErrMsg;          //错误信息
};




Implement CETHRawTxUtilsImpl : public IRawTxUtils
{
public:
    explicit CETHRawTxUtilsImpl(){}

public:
    //实现, 创建裸交易(并签名)的接口, 如果需要进行离线签名的创建,
    //只需要在资产管理导出转账信息即可(包含nonce,nonce需要在线获取),
    //然后在签名端调用此函数进行创建已签名交易
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override ;

    //实现,导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam) noexcept(false) override;

    //实现导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

    //实现广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam)noexcept(false) override;

public:
    //获取nonce值
    //根据以太坊源码数据结构, nonce值是  uint64
    int GetNonce(const QString &strAddr,  uint64_t &Nonce, QString strCoinType);

    //获取地址的余额
    //int GetBalance(const QString &strAddr, std::map<QString, float100> &mapBalances);
    int GetBalance(const QString &strAddr, std::map<QString, double> &mapBalances, QString strCoinType );

    //获取实时gasprice
    QString GetGasPrice() noexcept(false) ;

public:

    //创建标准 ERC20 代币的 transfer
    static int MakeStdERC20TransferData(const QString &strTo, const QString &strHexValueInWei, std::string &cstrRetData) noexcept(false);

    //将ERC20的 以最小单位字符串表示
    static int ToWeiStr(double dAmount , int nDecimals, std::string &cstrRet) noexcept(false);

};

inline  std::string UBigint2String(ubigint &ubigintValue)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << ubigintValue;
    return  ss.str();
}

inline  std::string Float2UBigintString(float100 &floatValue)
{
    float100 floatTmpValue = floatValue ;
    floatTmpValue *= GWEI;
    ubigint ubigintTmpValue    = floatTmpValue.convert_to<ubigint>();
    ubigintTmpValue *= GWEI;
    return UBigint2String(ubigintTmpValue);
}

 //检查ChainId的有效性
inline bool CheckChainId(uint uChainId)
{
    if( !(eth::ETHChainID::Mainnet == uChainId || eth::ETHChainID::Rinkeby == uChainId || eth::ETHChainID::Ropsten == uChainId
        || eth::ETHChainID::Goerli == uChainId || eth::ETHChainID::Kovan == uChainId
        || eth::ETHChainID::EthereumClassic == uChainId ))
    {
        return false;
    }
    return true;
}

inline eth::ETHChainID UInt2ChainId(uint uValue)
{
    switch (uValue) {
    case eth::ETHChainID::Mainnet: return  eth::ETHChainID::Mainnet; break;
    case eth::ETHChainID::Rinkeby: return eth::ETHChainID::Rinkeby; break;
    case eth::ETHChainID::Kovan: return eth::ETHChainID::Kovan; break;
    case eth::ETHChainID::Goerli: return eth::ETHChainID::Goerli; break;
    case eth::ETHChainID::Ropsten: return eth::ETHChainID::Ropsten; break;
    case eth::ETHChainID::EthereumClassic: return eth::ETHChainID::EthereumClassic; break;
    default: return eth::ETHChainID::None; break;
    }
    return eth::ETHChainID::None;
}


inline std::string Bin2HexString(const unsigned char *pBinData, unsigned int uLen, bool b0xPrefix = true)
{
    if(NULL == pBinData) return "";

    std::string strRet;
    for(unsigned int i = 0; i < uLen; i++)
    {
        char buf[5] = {0};
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%02x", pBinData[i]);
        strRet += buf;
    }
    return  (b0xPrefix) ? ("0x" + strRet): (strRet);
}



template <class _T, class _Out>
inline void ToBigEndian(_T _val, _Out& o_out)
{
	for (auto i = o_out.size(); i-- != 0; _val >>= 8)
		o_out[i] = (typename _Out::value_type)(uint8_t)_val;
}

inline std::string ToBigEndianString(ubigint _val)
{
    std::string ret(32, '\0');
    ToBigEndian(_val, ret);
    return ret;
}




















}

#endif // ETH_RAW_TX_UTILS_H
