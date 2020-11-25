#ifndef UTILS_H
#define UTILS_H

#pragma execution_character_set("utf-8")

#include "config.h"
#include <exception>
#include <QTimer>
#include <QEventLoop>
#include <QString>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>


//第一次
#define FIRST_ENCODE 'x'
//第二次
#define SECOND_ENCODE 'C'

#define N_MAIN_CONTRACT_ADDR_IDX    0
#define N_TEST_CONTRACT_ADDR_IDX    1
#define N_TOKEN_DECIMALS_IDX        2
#define N_GASLIMIT_IDX              3

//工具箱
namespace utils {


bool Is_HRC20_Token(const QString &strCoinType);


inline bool IsERC20Token(const QString &strCoinType)
{
    QString strTmpCoinType = strCoinType.toUpper().trimmed();
    auto it = std::find(g_vctERC20Tokens.begin(), g_vctERC20Tokens.end(), strTmpCoinType.trimmed());
    if( g_vctERC20Tokens.end() == it)
    {
        it = std::find(g_vctERC20Tokens.begin(), g_vctERC20Tokens.end(), strTmpCoinType.toLower().trimmed());
    }

    //保持config.ini中配置的ERC20代币币种和代码中设置的合约地址一致
    auto itCfg = gc_mapERC20TokensCfg.find(strTmpCoinType);
    if( gc_mapERC20TokensCfg.end() == itCfg )
    {
        itCfg = gc_mapERC20TokensCfg.find(strTmpCoinType.toLower());
    }

    return it != g_vctERC20Tokens.end() && itCfg != gc_mapERC20TokensCfg.end();
}

inline QString GetERC20ContractAddr(const QString &strCoinType) noexcept(false)
{
    QString strErrMsg;

    if(IsERC20Token(strCoinType))
    {
        const std::tuple<QString, QString, int, QString> &tpCfg = gc_mapERC20TokensCfg.at(strCoinType.toUpper().trimmed());
        QString strContractAddr = (g_bBlockChainMainnet) ? ( std::get<N_MAIN_CONTRACT_ADDR_IDX>(tpCfg) ) : ( std::get<N_TEST_CONTRACT_ADDR_IDX>(tpCfg) );

        if(42 != strContractAddr.length())
        {
            strErrMsg  = QString("Invalid %1 contract address %2!").arg(strCoinType).arg(strContractAddr);
            throw std::runtime_error(strErrMsg.toStdString());
        }

        return strContractAddr;
    }

    strErrMsg = QString("GetERC20ContractAddr(): Unknow ERC20 Token %1").arg(strCoinType);
    throw std::runtime_error(strErrMsg.toStdString());
}

inline int GetERC20Decimals(const QString &strCoinType)
{
    QString strErrMsg;
    if(IsERC20Token(strCoinType))
    {
        const std::tuple<QString, QString, int, QString> &tpCfg = gc_mapERC20TokensCfg.at(strCoinType.toUpper().trimmed());
        return std::get<N_TOKEN_DECIMALS_IDX>(tpCfg) ;
    }

    strErrMsg = QString("%is not ERC20 Token").arg(strCoinType);
    throw std::runtime_error(strErrMsg.toStdString());
}

inline QString GetGasLimit(const QString &strCoinType)
{
    QString strErrMsg;
    if(0 == strCoinType.compare("ETH", Qt::CaseInsensitive) ||
        0 == strCoinType.compare("ETC", Qt::CaseInsensitive) )
    {
        return  STR_NORMAL_ETH_TX_GASLIMIT;
    }
    else if(IsERC20Token(strCoinType))
    {
        const std::tuple<QString, QString, int, QString> &tpCfg = gc_mapERC20TokensCfg.at(strCoinType.toUpper().trimmed());
        return std::get<N_GASLIMIT_IDX>(tpCfg) ;
    }

    strErrMsg = QString("%is not ERC20 Token").arg(strCoinType);
    throw std::runtime_error(strErrMsg.toStdString());
}

inline double GetETH_ERC20_CollectionFee(const QString &strCoinType)
{
    double dClcEthFee  = 0.0;
    QString strGasLimit = utils::GetGasLimit(strCoinType);
    bool ok = false;
    double dGasLimit = strGasLimit.toDouble(&ok);
    if(!ok)
    {
        QString strErrMsg = QString("strGasLimit : %1").arg(strGasLimit);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        throw std::runtime_error(strErrMsg.toStdString());
    }
    dClcEthFee = COLLECTION_GASPRICE_GWEI * dGasLimit / 1000000000;
    return dClcEthFee;
}



inline bool IsValidPrivKey(const QString &strCoinType, const QString &strPrivKey, bool bIsMainnet)
{

    if(0 == strCoinType.compare("btc", Qt::CaseInsensitive)
      || 0 == strCoinType.compare("usdt", Qt::CaseInsensitive)
      || 0 == strCoinType.compare("bch", Qt::CaseInsensitive)
      || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)
      || ( !bIsMainnet  && 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) )   //LTC 测试网, 私钥格式和BTC测试网 一样
      )
    {
        if(bIsMainnet)//主网
        {
            //目前只支持 传统格式的 私钥  (compressed WIF格式) 不支持 uncompressed格式
            //TODO: 支持其他格式的私钥, 如 BIP32
            if(!(26 == strPrivKey.length() / 2))
                return false;
            if(!(strPrivKey.startsWith('K', Qt::CaseSensitive) || strPrivKey.startsWith('L', Qt::CaseSensitive)))
                return false;
        }
        else //测试网
        {
            //TODO:支持其他格式  2019-05-24  by yqq
            if(!((26 == strPrivKey.length() / 2) && strPrivKey.startsWith('c', Qt::CaseSensitive)))
                return false;
        }
    }
    else if(bIsMainnet && 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) )
    {
        if(!((26 == strPrivKey.length() / 2) &&  strPrivKey.startsWith('T', Qt::CaseSensitive)))
            return false;
    }
    else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
    {
        if(bIsMainnet)
        {
            if(!((26 == strPrivKey.length() / 2) && strPrivKey.startsWith('X', Qt::CaseSensitive) ))
                return false;
        }
        else
        {
             //TODO:支持其他格式  2019-05-24  by yqq
            if(!((26 == strPrivKey.length() / 2) && strPrivKey.startsWith('c', Qt::CaseSensitive)))
                return false;
        }

    }
    else if(0 == strCoinType.compare("eth", Qt::CaseInsensitive) || IsERC20Token(strCoinType)
            ||  0 == strCoinType.compare("etc", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("usdp", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCoinType.compare("het", Qt::CaseInsensitive)
            ||  utils::Is_HRC20_Token(strCoinType)
            || 0 ==  strCoinType.compare("HRC20FEE", Qt::CaseInsensitive)
            )
    {
        //是否是 32字节(64字符) 长度
        if(32 !=  strPrivKey.length() / 2) return false;

        //是否是有效的十六进制字符串
        if(! boost::all(strPrivKey.toStdString(), boost::is_xdigit())) return false;
    }
    else if( 0 == strCoinType.compare("xrp", Qt::CaseInsensitive))
    {
        if(!( strPrivKey.startsWith('s')
            && 29 == strPrivKey.length()
            && (!strPrivKey.contains('I')  && !strPrivKey.contains('O') && !strPrivKey.contains('l') && !strPrivKey.contains('0') ))
            )
         {
                return false;
         }
    }
    else if( 0 == strCoinType.compare("EOS", Qt::CaseInsensitive))
    {
        if( !(strPrivKey.startsWith('5')  && 51 == strPrivKey.length()
            && (!strPrivKey.contains('I')  && !strPrivKey.contains('O') && !strPrivKey.contains('l') && !strPrivKey.contains('0') )
         ) )
         {
            return false;
         }

    }
    else if( 0 == strCoinType.compare("XLM", Qt::CaseInsensitive) )
    {
        if( !( strPrivKey.startsWith('S') && 56 == strPrivKey.length() ))
        {
            return false;
        }
    }
    else if( 0 == strCoinType.compare("TRX", Qt::CaseInsensitive) )
    {
        //是否是 32字节(64字符) 长度
        if(32 !=  strPrivKey.length() / 2) return false;

        //是否是有效的十六进制字符串
        if(! boost::all(strPrivKey.toStdString(), boost::is_xdigit())) return false;
    }
    else //暂不支持的币种
    {
        return false;
    }


    return true;
}


inline QString  GetXrpAddrFrom_Addr_Tag(const QString &str_Addr_Tag)
{
    //如果后面跟了标签, 需要截取 '_' 前面的作为地址
    if(str_Addr_Tag.contains('_') )
    {
        QStringList strTmpList = str_Addr_Tag.split('_');
        QString strTmpXrpAddr = strTmpList[0];
        return strTmpXrpAddr;
    }

    //如果不存在 '_' 直接返回即可
    return str_Addr_Tag;
}


inline QString  GetTagFrom_Addr_Tag(const QString &str_Addr_Tag)
{
    QString strErrMsg;

    //如果后面跟了标签, 需要截取 '_' 前面的作为地址
    if(!str_Addr_Tag.contains('_') )
    {
        strErrMsg = QString("%1 地址_标签格式错误! 请检查!").arg(str_Addr_Tag);
        throw std::runtime_error(strErrMsg.toStdString());
    }

    QString strTag = str_Addr_Tag.mid( str_Addr_Tag.indexOf('_') + 1);
    return strTag;
}

inline uint32_t  GetXrpTagFrom_Addr_Tag(const QString &str_Addr_Tag)
{
    QString strErrMsg;

    //如果后面跟了标签, 需要截取 '_' 前面的作为地址
    if(!str_Addr_Tag.contains('_') )
    {
        strErrMsg = QString("%1 地址_标签格式错误! 请检查!").arg(str_Addr_Tag);
        throw std::runtime_error(strErrMsg.toStdString());
    }

    QString strTag = str_Addr_Tag.mid( str_Addr_Tag.indexOf('_') + 1);


    uint32_t uRetTag  = 0;
    try
    {
        uRetTag = boost::lexical_cast<uint32_t>(strTag.toStdString());
    }
    catch(boost::bad_lexical_cast &e)
    {
        strErrMsg = QString("%1 标签格式错误! 请检查!  %2").arg(str_Addr_Tag).arg(e.what());
        throw std::runtime_error(strErrMsg.toStdString());
    }

    return  uRetTag;
}


//判断地址是否是有效地址
inline bool IsValidAddr(const QString &strCoinType, const QString &strDstAddr)
{
    if( 0 == strCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCoinType.compare("usdt", Qt::CaseInsensitive)
    )
    {
        if(g_bBlockChainMainnet)//比特币主网地址检查规则
        {
            if(  strDstAddr.startsWith('1', Qt::CaseSensitive)   //BTC BCH BSV USDT 地址是 '1'开头
                 &&(26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }


            if(strDstAddr.startsWith('3', Qt::CaseSensitive) && (34 <= strDstAddr.length() && strDstAddr.length() <= 35))  //p2sh格式地址
                return true;

            if(strDstAddr.startsWith("bc1", Qt::CaseSensitive) && (40 <= strDstAddr.length() && strDstAddr.length() <= 45)) //bech32格式的
                return true;
        }
        else //测试网网络地址检查规则,  //ltc测试网地址和btc测试网地址格式一样
        {
            if(  (strDstAddr.startsWith('m', Qt::CaseSensitive) || strDstAddr.startsWith('n', Qt::CaseSensitive))  //地址是 'm'或 'n'开头
                 && (26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }

            if(strDstAddr.startsWith('2', Qt::CaseSensitive) && (34 <= strDstAddr.length() && strDstAddr.length() <= 35))  //p2sh格式地址
                return true;

            if(strDstAddr.startsWith("tb1q", Qt::CaseSensitive) && (40 <= strDstAddr.length() && strDstAddr.length() <= 45)) //bech32格式的
                return true;
        }
    }
    else if(0 == strCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
    {
        if(g_bBlockChainMainnet)//BCH主网地址检查规则
        {
             if(  strDstAddr.startsWith('1', Qt::CaseSensitive)   //BTC BCH BSV USDT 地址是 '1'开头
                 &&(26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }


            if(strDstAddr.startsWith('3', Qt::CaseSensitive) && (34 <= strDstAddr.length() && strDstAddr.length() <= 35))  //p2sh格式地址
            {
                //bsv 已经不支持 '3'开头的地址
                if( 0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
                    return false;

                return true;
            }

            if(strDstAddr.startsWith("bc1", Qt::CaseSensitive) && (40 <= strDstAddr.length() && strDstAddr.length() <= 45)) //bech32格式的
                return true;

            if(strDstAddr.startsWith("bitcoincash:") && ( 54 == strDstAddr.length()))
                return true;
        }
        else
        {
             if(  (strDstAddr.startsWith('m', Qt::CaseSensitive) || strDstAddr.startsWith('n', Qt::CaseSensitive))  //地址是 'm'或 'n'开头
                 && (26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }

            if(strDstAddr.startsWith('2', Qt::CaseSensitive) && (34 <= strDstAddr.length() && strDstAddr.length() <= 35))  //p2sh格式地址
            {
                //bsv测试网 已经不支持 '2'开头的地址
                if( 0 == strCoinType.compare("bsv", Qt::CaseInsensitive))
                    return false;

                return true;
            }

            if(strDstAddr.startsWith("tb1q", Qt::CaseSensitive) && (40 <= strDstAddr.length() && strDstAddr.length() <= 45)) //bech32格式的
                return true;
        }
    }
    else if(0 == strCoinType.compare("ltc", Qt::CaseInsensitive))
    {
        if(g_bBlockChainMainnet)
        {
            if(  strDstAddr.startsWith('L', Qt::CaseSensitive)   //地址是 'L'开头
                 &&(26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }


            if((strDstAddr.startsWith('3', Qt::CaseSensitive) || strDstAddr.startsWith('M', Qt::CaseSensitive))
                && (34 <= strDstAddr.length() && strDstAddr.length() <= 35))  //p2sh-segwit格式地址
                return true;

            if(strDstAddr.startsWith("ltc1", Qt::CaseSensitive) && (40 <= strDstAddr.length() && strDstAddr.length() <= 45)) //bech32格式的
                return true;

        }
        else
        {
            if(  (strDstAddr.startsWith('m', Qt::CaseSensitive) || strDstAddr.startsWith('n', Qt::CaseSensitive))  //地址是 'm'或 'n'开头
                 && (26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }

            if(strDstAddr.startsWith('Q', Qt::CaseSensitive) && (34 <= strDstAddr.length() && strDstAddr.length() <= 35))  //p2sh格式地址
                return true;

            if(strDstAddr.startsWith("tltc1q", Qt::CaseSensitive) && (40 <= strDstAddr.length() && strDstAddr.length() <= 45)) //bech32格式的
                return true;

        }
    }
    else if(0 == strCoinType.compare("dash", Qt::CaseInsensitive))
    {
        if(g_bBlockChainMainnet)
        {
            if(  strDstAddr.startsWith('X', Qt::CaseSensitive)  //DASH地址是 'X'开头, 大小写敏感
                 &&(26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }
        }
        else
        {
            if(  strDstAddr.startsWith('y', Qt::CaseSensitive)  //地址是 'y'开头
                 && (26 <= strDstAddr.length() && strDstAddr.length() <= 34) //长度在[26,34]区间
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
                 )
            {
                return true;
            }
        }

    }
    else if( 0 == strCoinType.compare("eth", Qt::CaseInsensitive) || IsERC20Token(strCoinType)
         || 0 == strCoinType.compare("etc", Qt::CaseInsensitive)
        )
    {
        //2019-04-28 yqq
        //以太坊测试网 和 以太坊主网的地址格式是一样的
        //必须判断ETH地址的长度, 防止短地址攻击
        if(strDstAddr.startsWith("0x", Qt::CaseInsensitive) && 42 == strDstAddr.length()   // "0x"开头  并且是    42字符长度(包括'0x')
          && boost::all(strDstAddr.mid(2).toStdString(),  boost::is_xdigit())) //是有效的十六进制字符串(不区分大小写)
        {
            return true;
        }
    }
    else if(0 == strCoinType.compare("usdp", Qt::CaseInsensitive))
    {
        if(strDstAddr.startsWith("usdp1", Qt::CaseSensitive) && 43 == strDstAddr.length()) //以 "usdp1"开头(大小写敏感)  并且是 43字符长度(包括"usdp1")
            return true;
    }
    else if(0 == strCoinType.compare("htdf", Qt::CaseInsensitive) || Is_HRC20_Token(strCoinType)
            || 0 == strCoinType.compare("HRC20FEE", Qt::CaseSensitive))
    {
         if(strDstAddr.startsWith("htdf1", Qt::CaseSensitive) && 43 == strDstAddr.length()) //以 "htdf1"开头(大小写敏感)  并且是 43字符长度(包括"htdf1")
            return true;
    }
    else if( 0 == strCoinType.compare("het", Qt::CaseInsensitive))
    {
        //HET 地址的开头
        if(strDstAddr.startsWith("0x1", Qt::CaseSensitive) && 41 == strDstAddr.length()) //以 "0x1"开头(大小写敏感)  并且是 41字符长度(包括"0x1")
            return true;
    }
    else if( 0 == strCoinType.compare("xrp", Qt::CaseInsensitive))
    {
        QString strTmpDstAddr = GetXrpAddrFrom_Addr_Tag(strDstAddr);
        if( strTmpDstAddr.startsWith('r') && (25 <= strTmpDstAddr.length() && strTmpDstAddr.length() <= 35)
            && (!strTmpDstAddr.contains('I' )  && !strTmpDstAddr.contains('O') &&
                !strTmpDstAddr.contains('l') && !strTmpDstAddr.contains('0') )
         )
         {
            return true;
         }
    }
    else if( 0 == strCoinType.compare("EOS", Qt::CaseInsensitive))
    {
        QString strTmpAddr = utils::GetXrpAddrFrom_Addr_Tag(strDstAddr);

        QRegularExpression  reExp("^[1-5a-z]{12}$");
        QRegularExpressionMatch matches = reExp.match( strTmpAddr );
        if( matches.hasMatch() )
        {
            return true;
        }
    }
    else if(  0 == strCoinType.compare("XLM", Qt::CaseInsensitive) )
    {
        QString strTmpDstAddr = GetXrpAddrFrom_Addr_Tag(strDstAddr);
        if( strTmpDstAddr.startsWith('G') && (56 == strTmpDstAddr.length()))
         {
            return true;
         }

    }
    else if(  0 == strCoinType.compare("TRX", Qt::CaseInsensitive) )
    {
        //注意:
        //      仅考虑 TRX 的   Base58格式的地址,  不考虑  hexString 格式的地址
        //      如果个别地方需要进行转换验证, 单独处理即可, 不必调用此函数进行判断
        if(  strDstAddr.startsWith('T', Qt::CaseSensitive)   // 以字母 T 开头
                 &&( strDstAddr.length() == 34) //长度是34字符
                 && (!(strDstAddr.contains('0', Qt::CaseSensitive) || strDstAddr.contains('O', Qt::CaseSensitive))) //不包含 数字0 或 大写O
            )
        {
                return true;
        }
    }
    else if( 0 == strCoinType.compare("XMR", Qt::CaseInsensitive)  )
    {
        //只考虑标准地址, 不考虑支持  intergrated-address
        if(95 == strDstAddr.length()
            && !strDstAddr.contains('I', Qt::CaseSensitive)
            && !strDstAddr.contains('O', Qt::CaseSensitive)
            && !strDstAddr.contains('l', Qt::CaseSensitive)
             )
        {
            // 主网:  '8' 开头的subAddress,  '4'开头的 standardAddress(或称 masterAddress)
            if(g_bBlockChainMainnet && (strDstAddr.startsWith('8') || strDstAddr.startsWith('4')))
                return true;

            // stagenet测试网:  '7' 开头的subAddress,  '5'开头的 standardAddress(或称 masterAddress)
            if(!g_bBlockChainMainnet && (strDstAddr.startsWith('7') || strDstAddr.startsWith('5')))
                return true;
        }

        return false;
    }
    else
    {
        qCritical() << __FUNCTION__ << "暂不支持" << strCoinType <<"的提币操作";
        return false; //暂不支持的
    }

    qCritical() << __FUNCTION__ <<  strCoinType << "地址格式非法, 已拒绝提币, 地址为:" << strDstAddr;

    return false;
}





//生成归集订单号
inline QString GenOrderID()
{
    static std::default_random_engine engine; //静态
    static std::uniform_int_distribution<unsigned> udistribution(0, 9999999); //静态

    QString strOrderID = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    int nRand = udistribution(engine);

    strOrderID += QString::asprintf("%07d", nRand);
    return strOrderID;
}

inline int GenRandomUInt()
{
    static std::default_random_engine engine; //静态
    static std::uniform_int_distribution<int> udistribution(0, 9999999); //静态
    int nRand = udistribution(engine);
    return nRand;
}


//注意此http请求错误函数会抛异常
struct HttpUtils : public QObject
{
    Q_OBJECT

public:
    static int SyncGetData(const QString &strInUrl, QByteArray &byteArray) noexcept(false)
    {
        //此处必须使用shared_ptr , 防止抛异常导致内存泄露
        std::shared_ptr<QNetworkAccessManager> _pNetManager = std::make_shared<QNetworkAccessManager>();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始http get请求";

        QUrl url = strInUrl;
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        SET_HTTPS_SSL_CONFIG(req);

        qDebug()<< "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求链接: " << strInUrl;

        _pNetManager->disconnect();
        auto _pReply = _pNetManager->get(req);

        //异步转为同步
        ASYNC_TO_SYNC_CAN_THROW(_pReply);

        byteArray = _pReply->readAll();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求结果:" << byteArray;

        _pReply->close();

        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "http请求成功";
        return 0;
    }


};



//检查是否能够ping通
inline bool Ping(QString strIP = "192.168.10.1")
{
    //ping 192.168.10.1 -n 1 -w 1000
    //-n  回显次数
    //-w  超时时间(毫秒)

    std::shared_ptr<QProcess> pCmd = std::make_shared<QProcess>( new QProcess);
    QString strArg = "ping " + strIP + " -n 1 -w " + QString::number(1000) ;
    pCmd->start(strArg);
    pCmd->waitForReadyRead();
    pCmd->waitForFinished();

    QString retStr = pCmd->readAll();
    if (-1 != retStr.indexOf("TTL"))
        return true;

    retStr.clear();
    return false;
}

//异或运算
inline QString xorEncode(const QString &strEncodePrivKey, const QChar cKey)
{
    QByteArray src;
    if(strEncodePrivKey.length() > 0)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        src = codec->fromUnicode(strEncodePrivKey);
    }

    for (int i = 0; i < src.size(); i++)
    {
        src[i] = src[i] ^ cKey.toLatin1();
    }

    QString strResult;
    if(src.size() > 0)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        strResult = codec->toUnicode(src);
    }

    //    for(int i = 0; i < strSrc.count(); i++)
    //    {
    //        strSrc[i] = strSrc.at(i).toLatin1() ^ cKey.toLatin1();
    //    }
    //    qDebug() << "异或之后的私钥: " << strResult;
    return strResult;
}
//解密私钥
inline void decodePrivKey(const QString &strEncodePrivKey, QString &strDecodePrivKey)
{
    QByteArray base64PrivKey = strEncodePrivKey.toUtf8().fromBase64(strEncodePrivKey.toUtf8());
    QString strBase64PrivKeyTmp = base64PrivKey;
    QString strSecondXor = xorEncode(strBase64PrivKeyTmp, SECOND_ENCODE);
    QString strFirstXor = xorEncode(strSecondXor, FIRST_ENCODE);
    base64PrivKey = strFirstXor.toUtf8().fromBase64(strFirstXor.toUtf8());
    strDecodePrivKey = base64PrivKey;
}

//左补'0'
inline QString ZeroPadLeft(const QString &strIn, int nDstLen)
{
    QString strRet = strIn;
    int nPad = nDstLen - strIn.size();
    while(nPad > 0)
    {
        strRet = QString('0') + strRet;
        nPad--;
    }
    return strRet;
}

//右补 '0'
inline QString ZeroPadRight(const QString &strIn, int nDstLen)
{
    QString strRet = strIn;
    int nPad = nDstLen - strIn.size();
    while(nPad > 0)
    {
        strRet += QString('0') ;
        nPad--;
    }
    return strRet;
}





inline std::string HexToBin(const std::string &strHexIn)
{
    if (strHexIn.size() % 2 != 0) return "";

    //size_t nfind = strHexIn.find("0x");
    std::string strHex = ( boost::starts_with(strHexIn, "0x")  ) ?(strHexIn.substr(2)) : (strHexIn);

    std::string strBin;
    strBin.resize(strHex.size() / 2);
    for (size_t i = 0; i < strBin.size(); i++)
    {
        uint8_t cTemp = 0;
        for (size_t j = 0; j < 2; j++)
        {
            char cCur = strHex[2 * i + j];
            if (cCur >= '0' && cCur <= '9')
            {
                cTemp = (cTemp << 4) + (cCur - '0');
            }
            else if (cCur >= 'a' && cCur <= 'f')
            {
                cTemp = (cTemp << 4) + (cCur - 'a' + 10);
            }
            else if (cCur >= 'A' && cCur <= 'F')
            {
                cTemp = (cTemp << 4) + (cCur - 'A' + 10);
            }
            else
            {
                throw std::runtime_error("HexToBin():invlaid hex string");
            }
        }
        strBin[i] = cTemp;
    }

    return strBin;
}

inline  std::string Bin2HexStr(const unsigned char *pBin, unsigned int len)
{
    std::string  strHex;
    for (unsigned int i = 0; i < len; i++)
    {
        char buf[10] = { 0 };
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, "%02x", pBin[i]);
        strHex += buf;

    }
    return strHex;
}




inline bool Is_HRC20_Token(const QString &strCoinType)
{
    QString strTmpCoinType = strCoinType.toUpper().trimmed();
    auto it = std::find(g_vct_HRC20_Tokens.begin(), g_vct_HRC20_Tokens.end(), strTmpCoinType.trimmed());
    if( g_vct_HRC20_Tokens.end() == it)
    {
        it = std::find(g_vct_HRC20_Tokens.begin(), g_vct_HRC20_Tokens.end(), strTmpCoinType.toLower().trimmed());
    }

    //保持config.ini中配置的ERC20代币币种和代码中设置的合约地址一致
    auto itCfg = gc_map_HRC20_tokens_config.find(strTmpCoinType);
    if( gc_map_HRC20_tokens_config.end() == itCfg )
    {
        itCfg = gc_map_HRC20_tokens_config.find(strTmpCoinType.toLower());
    }

    return it != g_vct_HRC20_Tokens.end() && itCfg != gc_map_HRC20_tokens_config.end();
}

inline QString Get_HRC20_ContractAddr(const QString &strCoinType) noexcept(false)
{
    QString strErrMsg;

    if(Is_HRC20_Token(strCoinType))
    {
        const std::tuple<QString, QString, int> &tpCfg = gc_map_HRC20_tokens_config.at(strCoinType.toUpper().trimmed());
        QString strContractAddr = (g_bBlockChainMainnet) ? ( std::get<N_MAIN_CONTRACT_ADDR_IDX>(tpCfg) ) : ( std::get<N_TEST_CONTRACT_ADDR_IDX>(tpCfg) );

        if(43 != strContractAddr.length())
        {
            strErrMsg  = QString("Invalid %1 contract address %2!").arg(strCoinType).arg(strContractAddr);
            throw std::runtime_error(strErrMsg.toStdString());
        }

        return strContractAddr;
    }

    strErrMsg = QString("Get_HRC20_ContractAddr(): Unknow HRC20 Token %1").arg(strCoinType);
    throw std::runtime_error(strErrMsg.toStdString());
}

inline int Get_HRC20_Decimals(const QString &strCoinType)
{
    QString strErrMsg;
    if(Is_HRC20_Token(strCoinType))
    {
        const std::tuple<QString, QString, int> &tpCfg = gc_map_HRC20_tokens_config.at(strCoinType.toUpper().trimmed());
        return std::get<N_TOKEN_DECIMALS_IDX>(tpCfg) ;
    }

    strErrMsg = QString("%is not HRC20 Token").arg(strCoinType);
    throw std::runtime_error(strErrMsg.toStdString());
}



}
#endif // UTILS_H
