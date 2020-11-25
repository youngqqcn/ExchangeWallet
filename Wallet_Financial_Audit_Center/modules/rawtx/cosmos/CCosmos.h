/************************************************************************
作者: yqq
日期: 2019-05-08  14:25
说明: Cosmos类币的签名库  支持 (USDP, HTDF)
************************************************************************/

#ifndef _CCOSMOS_H_
#define  _CCOSMOS_H_
#pragma execution_character_set("utf-8")


#include <iostream>
#include <string>
#include <vector>

//#include "cryptopp/base64.h"  //base64   能不用cryptopp就不用cryptopp
#include "secp256k1/secp256k1.h"
#include "secp256k1/secp256k1_recovery.h"

#include "openssl/sha.h"  //openssl sha256


//base64编码解码
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

//字符串工具
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;

namespace cosmos
{




#define  UINT_MAX_FEE_AMOUNT	(2000000000)//最大交易手续费
#define  UINT_MIN_FEE_AMOUNT	(10)			//最小交易手续费
#define  UINT_MAX_GAS_AMOUNT	(200000000) //最大gaslimit
#define  UINT_MIN_GAS_AMOUNT	(30000)		//最小gaslimit
#define  UINT_ADDR_LEN(cstrAddr)    (( boost::starts_with(cstrAddr, "0x"))?(41):(43))		//地址长度
#define  UINT_PRIV_KEY_LEN		(32)		//公钥的字节数
#define  UINT_PUB_KEY_LEN		(33)		//公钥长度(33字节)
#define  UINT_SIG_RS_LEN		(64)		//128 (r , s) 签名信息, 组装后的长度
#define  UINT_TXID_CHAR_LEN          (64)        //txid的字符串长度

#define  STR_MAINCHAIN			"mainchain"
#define  STR_TESTCHAIN			"testchain"

#define  STR_USDP				"usdp"
#define  STR_HTDF				"htdf"
#define  STR_HET				"het"
#define  STR_SATOSHI            "satoshi"

#define  STR_BROADCAST_TYPE		"auth/StdTx"
#define  STR_BROADCAST_MSG_TYPE "htdfservice/send"
#define  STR_BROADCAST_MSG_TYPE_HET "hetservice/send"
#define  STR_BROADCAST_PUB_KEY_TYPE "tendermint/PubKeySecp256k1"



	enum ErrorCode
	{
		NO_ERROR  = 0,
		ARGS_ERROR = 1001,
		ECCSIGN_STEP1_ERROR = 1005,
		ECCSIGN_STEP2_ERROR = 1006
	};



typedef unsigned long long int uint64_t;
typedef unsigned int uint32_t;




inline std::string Base64Encode(const char *input, int length, bool with_new_line)
{
    BIO * bmem = NULL;
    BIO * b64 = NULL;
    BUF_MEM * bptr = NULL;

    b64 = BIO_new(BIO_f_base64());
    if (!with_new_line) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char * buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;
    BIO_free_all(b64);


    std::string  strRet(buff);
    free(buff); buff = NULL;
    return strRet;
}

inline std::string Base64Decode(const char * input, int length, bool with_new_line)
{
    BIO * b64 = NULL;
    BIO * bmem = NULL;
    char * buffer = (char *)malloc(length);
    memset(buffer, 0, length);

    b64 = BIO_new(BIO_f_base64());
    if (!with_new_line) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new_mem_buf((void *)input, length);
    bmem = BIO_push(b64, bmem);
    int iDecodeLen = BIO_read(bmem, buffer, length);

    BIO_free_all(bmem);

    std::string strRet;
    strRet.append(buffer, iDecodeLen);

    free(buffer); buffer = NULL;
    return strRet;
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




inline string HexToBin(const string &strHexIn)
{
    if (strHexIn.size() % 2 != 0) return "";

    std::string strHex = (boost::starts_with(strHexIn, "0x")) ? (strHexIn.substr(2)) : (strHexIn);

    string strBin;
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
                return "";
            }
        }
        strBin[i] = cTemp;
    }

    return strBin;
}



inline secp256k1_context const* getCtx()
{
    static std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> s_ctx{
        secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY),
        &secp256k1_context_destroy
    };
    return s_ctx.get();
}




//输入: 十六进制字符串形式的私钥
//输出: 十六进制字符串形式的公钥
inline int PrivKeyToCompressPubKey(const std::string &_strPrivKey, std::string &strPubKey)
{
    std::string strPrivKey = HexToBin(_strPrivKey);

    secp256k1_pubkey  pubkey;
    memset(pubkey.data, 0, sizeof(pubkey.data));

    static std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> s_ctx{
        secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY),
        &secp256k1_context_destroy
    };

    auto* ctx = getCtx();

    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, (unsigned char *)strPrivKey.data()))
    {
        return 1;
    }

    unsigned char output[1024] = { 0 }; memset(output, 0, sizeof(output));
    size_t  outputlen = 33;
    secp256k1_ec_pubkey_serialize(ctx, output, &outputlen, &pubkey, SECP256K1_EC_COMPRESSED);

    if (33 != outputlen)
    {
        return 1;
    }

    strPubKey = Bin2HexStr(output, outputlen);
    return 0;
}


	
/*要签名的数据
{
	"account_number": "12",		//需从节点获取
	"chain_id": "mainchain",	//testchain或mainchain
	"fee": {
		"amount": [{
			"amount": "20",		//手续费金额
            "denom": "satoshi"		//币种
		}],
		"gas": "200000"			//gasprice??
	},
	"memo": "",					//备注
	"msgs": [{	
		"Amount": [{
			"amount": "66",		//转账金额
            "denom": "satoshi"		//币种
		}],
		"From": "usdp15havmvnt4ygfyfuqm32aecth7p8800f6pkf652",	//源地址
		"To": "usdp1vmsfd7rly9chhjpaalr55q5886lnva99ghlesg"		//目的地址
	}],
	"sequence": "4"				//交易次数? 
}


//以下是  2020版  HTDF 普通交易的结构
{
	"account_number": "11",
	"chain_id": "testchain",
	"fee": {
		"gas_price": "100",
		"gas_wanted": "200000"
	},
	"memo": "yqq",
	"msgs": [{
		"Amount": [{
			"amount": "1000000000",
			"denom": "satoshi"
		}],
		"Data": "", //这里填写 HRC20 转账信息,  4字节函数签名 + 32字节代币接收方地址(hexstring形式) + 32字节代币转账金额
		"From": "htdf1jrh6kxrcr0fd8gfgdwna8yyr9tkt99ggmz9ja2",
		"GasPrice": 100,
		"GasWanted": 200000,
		"To": "htdf1m5phsvgrwpxdsmah5cqkvd6ffz9xzrc3e0jkr2"
	}],
	"sequence": "6"
}

*/
struct CsRawTx
{
	uint64_t	uAccountNumber;
	char		szChainId[128];
	uint64_t	uFeeAmount;
	char		szFeeDenom[128];
	uint64_t	uGas;
	char		szMemo[1024];
	uint64_t	uMsgAmount;
	char		szMsgDenom[128];
	char		szMsgFrom[128];
	char		szMsgTo[128];
	uint64_t	uSequence;

	char        szData[1024]; // Data字段, 支持 HRC20 代币转账  2020-04-15 yqq

	CsRawTx()
	{
		uAccountNumber = INTMAX_MAX; //((0x1 << 32) - 1);
		memset(szChainId, 0, sizeof(szChainId));
		uFeeAmount = 0;
		memset(szFeeDenom, 0, sizeof(szFeeDenom));
		uGas = 0;
		memset(szMemo, 0, sizeof(szMemo));
		uMsgAmount = 0;
		memset(szMsgDenom, 0, sizeof(szMsgDenom));
		memset(szMsgFrom, 0, sizeof(szMsgFrom));
		memset(szMsgTo, 0, sizeof(szMsgTo));
		uSequence = INTMAX_MAX;

		memset(szData, 0, sizeof(szData));
	}

	CsRawTx(const CsRawTx & other)
	{
		uAccountNumber = other.uAccountNumber;
		memcpy(szChainId, other.szChainId, sizeof(szChainId));
		uFeeAmount = other.uFeeAmount;
		memcpy(szFeeDenom, other.szFeeDenom, sizeof(szFeeDenom));
		uGas = other.uGas;
		memcpy(szMemo, other.szMemo, sizeof(szMemo));
		uMsgAmount = other.uMsgAmount;

		memcpy(szMsgDenom, other.szMsgDenom, sizeof(szMsgDenom));
		memcpy(szMsgFrom, other.szMsgFrom, sizeof(szMsgFrom));
		memcpy(szMsgTo, other.szMsgTo, sizeof(szMsgTo));
		uSequence = other.uSequence;

		memcpy(szData, other.szData, sizeof(szData));
	}

	CsRawTx& operator = (const CsRawTx &other)
	{
	    if(this == &other) return *this;

		uAccountNumber = other.uAccountNumber;
		memcpy(szChainId, other.szChainId, sizeof(szChainId));
		uFeeAmount = other.uFeeAmount;
		memcpy(szFeeDenom, other.szFeeDenom, sizeof(szFeeDenom));
		uGas = other.uGas;
		memcpy(szMemo, other.szMemo, sizeof(szMemo));
		uMsgAmount = other.uMsgAmount;

		memcpy(szMsgDenom, other.szMsgDenom, sizeof(szMsgDenom));
		memcpy(szMsgFrom, other.szMsgFrom, sizeof(szMsgFrom));
		memcpy(szMsgTo, other.szMsgTo, sizeof(szMsgTo));
		uSequence = other.uSequence;

        memcpy(szData, other.szData, sizeof(szData));
		return *this;
	}

    //老版本, 2020之前的
	bool ToString( std::string &strOut)
	{
		if (false == this->ParamsCheck(strOut))
		{
			return false;
		}
		strOut.clear();


		std::string  strJson;

		strJson += "{"; //root

		char buf[1024] = { 0 };

		//account_number
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"account_number\":\"%llu\"", this->uAccountNumber);
		strJson += buf;
		strJson += ",";

		//chain_id
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"chain_id\":\"%s\"", this->szChainId);
		strJson += buf;
		strJson += ",";

		//fee
		strJson += "\"fee\":{"; 
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"amount\":[{\"amount\":\"%llu\",\"denom\":\"%s\"}],\"gas\":\"%llu\"", 
						this->uFeeAmount, this->szFeeDenom, this->uGas);
		strJson += buf;
		strJson += "}"; 
		strJson += ",";


		//memo
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"memo\":\"%s\"", this->szMemo);
		strJson += buf;
		strJson += ",";

		//msgs  注意是 msgs  不是 msg
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"msgs\":[{\"Amount\":[{\"amount\":\"%llu\",\"denom\":\"%s\"}],\"From\":\"%s\",\"To\":\"%s\"}]", 
					this->uMsgAmount, this->szMsgDenom, this->szMsgFrom, this->szMsgTo);
		strJson += buf;
		strJson += ",";

		//sequence
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"sequence\":\"%llu\"", this->uSequence);
		strJson += buf;


		strJson += "}"; //root 

		strOut = strJson;
		return true;
	}



    //2020版
	bool ToString_2020( std::string &strOut)
	{
		if (false == this->ParamsCheck(strOut))
		{
			return false;
		}
		strOut.clear();


		std::string  strJson;

		strJson += "{"; //root

		char buf[1024] = { 0 };

		//account_number
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"account_number\":\"%llu\"", this->uAccountNumber);
		strJson += buf;
		strJson += ",";

		//chain_id
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"chain_id\":\"%s\"", this->szChainId);
		strJson += buf;
		strJson += ",";

        //fee
		strJson += "\"fee\":{";
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"gas_price\":\"%llu\",\"gas_wanted\":\"%llu\"", this->uFeeAmount, this->uGas);
		strJson += buf;
		strJson += "}";
		strJson += ",";


		//memo
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"memo\":\"%s\"", this->szMemo);
		strJson += buf;
		strJson += ",";

        //msgs  注意是 msgs  不是 msg
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"msgs\":[{\"Amount\":[{\"amount\":\"%llu\",\"denom\":\"%s\"}],\"Data\":\"%s\",\"From\":\"%s\",\"GasPrice\":%llu,\"GasWanted\":%llu,\"To\":\"%s\"}]",
			this->uMsgAmount, this->szMsgDenom, this->szData ,this->szMsgFrom, this->uFeeAmount, this->uGas,this->szMsgTo);
		strJson += buf;
		strJson += ",";

		//sequence
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "\"sequence\":\"%llu\"", this->uSequence);
		strJson += buf;


		strJson += "}"; //root

		strOut = strJson;
		return true;
	}



	bool  ParamsCheck( std::string &strErrMsg  )
	{
		if (INTMAX_MAX == this->uAccountNumber || 0 == this->uAccountNumber)
		{
			strErrMsg = boost::str(boost::format("invalid `account_number`: %llu , must between %llu and %llu.") % this->uAccountNumber % 0 % INTMAX_MAX);
			return false;
		}

		if(!(0 == strcmp(STR_MAINCHAIN, this->szChainId) || 0 == strcmp(STR_TESTCHAIN, this->szChainId) ))
		{
			strErrMsg = boost::str( boost::format("invalid `chain_id`: %s, must be '%s'or '%s' . ") % this->szChainId % STR_MAINCHAIN % STR_TESTCHAIN );
			return false;
		}

		if (UINT_MAX_FEE_AMOUNT < this->uFeeAmount || this->uFeeAmount < UINT_MIN_FEE_AMOUNT)
		{
			strErrMsg =  boost::str(boost::format("invalid `fee amount`, must between %d and %d.") % UINT_MIN_FEE_AMOUNT % UINT_MAX_FEE_AMOUNT  );
			return false;
		}

		if (UINT_MAX_GAS_AMOUNT < this->uGas || this->uGas < UINT_MIN_GAS_AMOUNT)
		{
			strErrMsg = boost::str(boost::format("invalid `fee gas` : %llu, must between %llu and %llu.") % this->uGas  % UINT_MIN_GAS_AMOUNT % UINT_MAX_GAS_AMOUNT );
			return false;
		}

        //单位使用 satoshi,  不再使用 usdp作为单位    1 usdp = 10^8 satoshi
        /*if (!(0 == strcmp(STR_USDP, this->szFeeDenom) || 0 == strcmp(STR_HTDF, this->szFeeDenom)))
        {
			strErrMsg = boost::str(boost::format("invalid `fee denom` : %s, must be `%s` or `%s`.") % this->szFeeDenom % STR_HTDF % STR_USDP );
			return false;
        }*/
        if (!(0 == strcmp(STR_SATOSHI, this->szFeeDenom)))
        {
            strErrMsg = boost::str(boost::format("invalid `fee denom` : %s, must be `%s`.") % this->szFeeDenom % STR_SATOSHI );
            return false;
        }

        //同上
        /*if (!(0 == strcmp(STR_USDP, this->szMsgDenom) || 0 == strcmp(STR_HTDF, this->szMsgDenom)))
        {
			strErrMsg = boost::str(boost::format("invalid `msgs amount denom` : %s, must be `%s` or `%s`.") % this->szMsgDenom % STR_HTDF % STR_USDP );
			return false;
        }*/
        if (!(0 == strcmp(STR_SATOSHI, this->szMsgDenom) ))
        {
            strErrMsg = boost::str(boost::format("invalid `msgs amount denom` : %s, must be `%s`.") % this->szMsgDenom % STR_SATOSHI );
            return false;
        }

		//TODO: 利用 bech32解码地址, 如果bech32解码失败, 则地址错误   //2019-05-09
        int nAddrLen = UINT_ADDR_LEN(std::string(this->szMsgFrom));
		if ( nAddrLen != strlen(this->szMsgFrom))
		{
			strErrMsg = boost::str(boost::format("invalid address `msg From`:%s, address length must be %d.") % this->szMsgFrom % nAddrLen );
			return false;
		}

		if ( nAddrLen != strlen(this->szMsgTo))
		{
			strErrMsg = boost::str(boost::format("invalid address `msg To`:%s, address length must be %d.") % this->szMsgTo % nAddrLen );
			return false;
		}

		//检查是否是 'usdp1'开头  或者 'htdf1'开头
		//if (!(this->szMsgFrom == strstr(this->szMsgFrom, this->szMsgDenmo)))
        if( !(true == boost::starts_with(this->szMsgFrom, std::string(STR_USDP) + "1")
              || true == boost::starts_with(this->szMsgFrom, std::string(STR_HTDF) + "1")
              || true == boost::starts_with(this->szMsgFrom, "0x1" /*HET地址开头*/)))
        {
			strErrMsg = boost::str(boost::format( "invalid address `From`:%s.") % this->szMsgFrom);
			return false;
		}

		//检查是否是 'usdp'开头  或者 'htdf'开头
        if( !(true == boost::starts_with(this->szMsgTo, std::string(STR_USDP) + "1")
              || true == boost::starts_with(this->szMsgTo, std::string(STR_HTDF) + "1")
              || true == boost::starts_with(this->szMsgTo, "0x1"/*HET地址开头*/)))
        {
			strErrMsg = boost::str(boost::format( "invalid address `To`:%s.") % this->szMsgTo);
			return false;
		}

        //源地址和目的地址 必须是同一类型地址
        if( !((true == boost::starts_with(this->szMsgFrom, std::string(STR_USDP) + "1") && true == boost::starts_with(this->szMsgTo, std::string(STR_USDP) + "1"))
            || (true == boost::starts_with(this->szMsgFrom, std::string(STR_HTDF) + "1") && true == boost::starts_with(this->szMsgTo, std::string(STR_HTDF) + "1"))
            || (true == boost::starts_with(this->szMsgFrom, "0x1") && true == boost::starts_with(this->szMsgTo, "0x1"))/*HET地址开头*/
              )
            )
        {
            strErrMsg = boost::str(boost::format("invalid address: `To` is diffrent type with `From`. `To`:`%s`, `From`:`%s`.") % this->szMsgTo % this->szMsgFrom);
            return false;
        }



		if (INTMAX_MAX <= uSequence)
		{
			strErrMsg = "invalid `sequence` , TOO LARGE, this is tx number of the address in the node.";
			return false;
		}

		return true;
	}


};







/*
{
	"type": "auth/StdTx",
	"value": {
		"fee": {
			"amount": [{
				"amount": "20",
                "denom": "satoshi"
			}],
			"gas": "200000"
		},
		"memo": "",
		"msg": [{
			"type": "htdfservice/send",   //写死的??
			"value": {
				"Amount": [{
					"amount": "66",
                    "denom": "satoshi"
				}],
				"From": "usdp15havmvnt4ygfyfuqm32aecth7p8800f6pkf652",
				"To": "usdp1vmsfd7rly9chhjpaalr55q5886lnva99ghlesg"
			}
		}],
		"signatures": [{
			"pub_key": {
				"type": "tendermint/PubKeySecp256k1",
				"value": "A9wn8etkGR2CeeEzkHqGq17gxFtWajDa/aqmPCUVt9cN"    //base64(pubKey)
			},
			"signature": "pnwGIegSXmDHnWHYtA9jBFkFBadmdx9mjNijeuRrwMQ+OJBphHlH/+i2bl/7OUT3usX1P1jR00vB43SS7IgPeQ=="    //base64(r[:] + s[:])
		}]
	}
}


//以下是  2020版本

{
	"type": "auth/StdTx",
	"value": {
		"msg": [{
			"type": "htdfservice/send",
			"value": {
				"From": "htdf1jrh6kxrcr0fd8gfgdwna8yyr9tkt99ggmz9ja2",
				"To": "htdf1m5phsvgrwpxdsmah5cqkvd6ffz9xzrc3e0jkr2",
				"Amount": [{
					"denom": "satoshi",
					"amount": "1000000000"
				}],
				"Data": "",  //这里填写 HRC20 转账信息,  4字节函数签名 + 32字节代币接收方地址(hexstring形式) + 32字节代币转账金额
				"GasPrice": "100",
				"GasWanted": "200000"
			}
		}],
		"fee": {
			"gas_wanted": "200000",
			"gas_price": "100"
		},
		"signatures": [{
			"pub_key": {
				"type": "tendermint/PubKeySecp256k1",
				"value": "AynNwicLJpg9SSyojXQ04vGcd2DYe4663ynQFrk5Mp6d"
			},
			"signature": "ELigkcJbQwA9hW6H3qnySjkjB2kgGhB0g1TD6GZDOFVvM2NNHcjWLQC93lSgCq5M5LJ1Xcpg/74lAoNc9mqNww=="
		}],
		"memo": "yqq"
	}
}

*/
struct CsBroadcastTx
{
	std::string					strType;		//"auth/StdTx"

	CsRawTx						csRawTx;
	std::string					strMsgType;		//"htdfservice/send"
	std::string					strPubKeyType;	//type
	std::string					strPubkeyValue;	//value
	std::string					strSignature;	//signature



	CsBroadcastTx()
	{
		strType				= STR_BROADCAST_TYPE; //"auth/StdTx";
		csRawTx				= CsRawTx();
		strMsgType			= STR_BROADCAST_MSG_TYPE;// "htdfservice/send";
		strPubKeyType		= STR_BROADCAST_PUB_KEY_TYPE ;//"tendermint/PubKeySecp256k1";
		strPubkeyValue		= "";
		strSignature		= "";


	}

	CsBroadcastTx(const CsBroadcastTx &other)
	{
		strType				= other.strType;
		csRawTx				= other.csRawTx;
		strMsgType			= other.strMsgType;
		strPubKeyType		= other.strPubKeyType;
		strPubkeyValue		= other.strPubkeyValue;
		strSignature		= other.strSignature;
	}


	CsBroadcastTx& operator = (const CsBroadcastTx &other)
	{
		strType = other.strType;
		csRawTx = other.csRawTx;
		strMsgType = other.strMsgType;
		strPubKeyType = other.strPubKeyType;
		strPubkeyValue = other.strPubkeyValue;
		strSignature = other.strSignature;

		return *this;
	}



	//转为json, 
	//如果参数检查失败返回 false, strRet携带错误信息
	//如果成功返回 true, strRet是json字符串(不含任何空格,除memo的value外) 且顺序固定不能变
	bool ToString( std::string &strRet )
	{
		std::string strErrMsg;
		if (false == this->ParamsCheck(strErrMsg))
		{
			strRet = strErrMsg;
			return false;
		}

		std::string strJson;
		strJson += "{"; //root

		//type
		strJson += "\"type\":\"" + strType + "\",";
		boost::trim(strJson);

		//value
		strJson += "\"value\":{";//value


		//msg
		strJson += boost::str( boost::format("\
			\"msg\":[{\
				\"type\":\"%s\",\
				\"value\":{\
					\"From\":\"%s\",\
					\"To\":\"%s\",\
					\"Amount\":[{\
						\"denom\":\"%s\",\
						\"amount\":\"%llu\"\
					}]\
				  }\
			}],") % this->strMsgType % this->csRawTx.szMsgFrom % this->csRawTx.szMsgTo \
				% this->csRawTx.szMsgDenom % this->csRawTx.uMsgAmount );


		//fee
		strJson += boost::str( boost::format( "\
				\"fee\":{\
					\"amount\":[{\
						\"denom\":\"%s\",\
						\"amount\":\"%llu\"\
					}],\
					\"gas\":\"%llu\"\
				}," ) % this->csRawTx.szFeeDenom % this->csRawTx.uFeeAmount % this->csRawTx.uGas);


		//signatures
		strJson += boost::str(boost::format("\
			\"signatures\":[{\
				\"pub_key\":{\
					\"type\":\"%s\",\
					\"value\":\"%s\"\
				},\
				\"signature\":\"%s\"\
			}],") % this->strPubKeyType % this->strPubkeyValue % this->strSignature );


		strJson = boost::algorithm::erase_all_copy(strJson, " ");  //去掉空格
		strJson = boost::algorithm::erase_all_copy(strJson, "\t" ); //去掉空格
		strJson = boost::algorithm::erase_all_copy(strJson, "\n"); //去掉空格
		

		//memo 中的空格不能去掉
		strJson += boost::str(boost::format("\"memo\":\"%s\"") % this->csRawTx.szMemo);


		strJson += "}"; //value


		strJson += "}"; //root


		strRet = strJson;
		return true;
	}




	//转为json,
	//如果参数检查失败返回 false, strRet携带错误信息
	//如果成功返回 true, strRet是json字符串(不含任何空格,除memo的value外) 且顺序固定不能变
	bool ToString_2020( std::string &strRet )
	{
		std::string strErrMsg;
		if (false == this->ParamsCheck(strErrMsg))
		{
			strRet = strErrMsg;
			return false;
		}

		std::string strJson;
		strJson += "{"; //root

		//type
		strJson += "\"type\":\"" + strType + "\",";
		boost::trim(strJson);

		//value
		strJson += "\"value\":{";//value


		//msg
		strJson += boost::str(boost::format("\
			\"msg\":[{\
				\"type\":\"%s\",\
				\"value\":{\
					\"From\":\"%s\",\
					\"To\":\"%s\",\
					\"Amount\":[{\
						\"denom\":\"%s\",\
						\"amount\":\"%llu\"\
					}],\
					\"Data\":\"%s\",\
					\"GasPrice\":\"%llu\",\
					\"GasWanted\":\"%llu\"\
				  }\
			}],") % this->strMsgType % this->csRawTx.szMsgFrom % this->csRawTx.szMsgTo \
			% this->csRawTx.szMsgDenom % this->csRawTx.uMsgAmount % this->csRawTx.szData % this->csRawTx.uFeeAmount % this->csRawTx.uGas);


        //fee
		strJson += boost::str(boost::format("\
				\"fee\":{\
					\"gas_price\":\"%llu\",\
					\"gas_wanted\":\"%llu\"\
				},") %  this->csRawTx.uFeeAmount % this->csRawTx.uGas);


		//signatures
		strJson += boost::str(boost::format("\
			\"signatures\":[{\
				\"pub_key\":{\
					\"type\":\"%s\",\
					\"value\":\"%s\"\
				},\
				\"signature\":\"%s\"\
			}],") % this->strPubKeyType % this->strPubkeyValue % this->strSignature );


		strJson = boost::algorithm::erase_all_copy(strJson, " ");  //去掉空格
		strJson = boost::algorithm::erase_all_copy(strJson, "\t" ); //去掉空格
		strJson = boost::algorithm::erase_all_copy(strJson, "\n"); //去掉空格


		//memo 中的空格不能去掉
		strJson += boost::str(boost::format("\"memo\":\"%s\"") % this->csRawTx.szMemo);


		strJson += "}"; //value


		strJson += "}"; //root


		strRet = strJson;
		return true;
	}



	//参数检查,
	//返回值: 
	//   成功: true  失败: false
	//   strErrMsg 携带错误信息
	bool ParamsCheck(std::string &strErrMsg)
	{

		if (false == this->csRawTx.ParamsCheck(strErrMsg))
		{
			strErrMsg += "CsBroadcastTx::ParamsCheck:";
			return false;
		}

		if ( STR_BROADCAST_MSG_TYPE != this->strMsgType && STR_BROADCAST_MSG_TYPE_HET != this->strMsgType)
		{
			strErrMsg = boost::str(boost::format("invalid `msg type` : '%s', must be '%s' or '%s'.")
				                      % this->strMsgType % STR_BROADCAST_MSG_TYPE % STR_BROADCAST_MSG_TYPE_HET);
			return false;
		}

		if ( STR_BROADCAST_PUB_KEY_TYPE != this->strPubKeyType )
		{
			strErrMsg = boost::str(boost::format("invalid `pub_key type` : '%s', must be '%s'.") 
				                    % this->strPubKeyType% STR_BROADCAST_PUB_KEY_TYPE);
			return false;
		}

		if (strPubkeyValue.empty())
		{
			
			strErrMsg = boost::str(boost::format("invalid `pub_key value` is empty, must be base64(pubkey).") 
									% this->strPubKeyType % STR_BROADCAST_PUB_KEY_TYPE);
			return false;
		}

		string strTmpDecode;
        strTmpDecode = cosmos::Base64Decode(strPubkeyValue.data(), strPubkeyValue.size() , false);
		if (UINT_PUB_KEY_LEN != strTmpDecode.size())
		{
            strErrMsg = boost::str(boost::format("invalid `pub_key value` length %d is not %d. After base64 decode, pubkey's length must be %d.")
                                    %strTmpDecode.size() % UINT_PUB_KEY_LEN % UINT_PUB_KEY_LEN);
			return false;
		}


		strTmpDecode.clear();
        strTmpDecode =  cosmos::Base64Decode(this->strSignature.data(), this->strSignature.size(), false);
		if (UINT_SIG_RS_LEN != strTmpDecode.size())
		{
			strErrMsg = boost::str(boost::format("invalid `signature` length is not %d. After base64 decode, signature's length must be %d.")
				% UINT_SIG_RS_LEN % UINT_SIG_RS_LEN);
			return false;
		}

		return true;
	}


	//转为十六进制字符串,用于调用服务端的广播接口
	//返回值:
	//    成功:  true
	//    失败: 参数检查失败, strOut携带错误信息
	bool ToHexStr(std::string &strOut)
	{
		std::string strErrMsg;
		if (false == this->ParamsCheck(strErrMsg))
		{
			strOut = strErrMsg;
			return false;
		}

		std::string strRet;
		std::string strHex;
		if (false == this->ToString(strErrMsg)) //测试是否成功
		{
			strOut = strErrMsg;
			return false;
		}

		this->ToString(strHex);

		strRet = "";
		for (size_t i  = 0; i < strHex.size(); i++)
		{
			strRet += boost::str(boost::format("%02x") % ((int)strHex[i]));
		}
		strOut = strRet;

		return true;
	}

    //转为十六进制字符串,用于调用服务端的广播接口
	//返回值:
	//    成功:  true
	//    失败: 参数检查失败, strOut携带错误信息
	bool ToHexStr_2020(std::string &strOut)
	{
		std::string strErrMsg;
		if (false == this->ParamsCheck(strErrMsg))
		{
			strOut = strErrMsg;
			return false;
		}

		std::string strRet;
		std::string strHex;
		if (false == this->ToString_2020(strErrMsg)) //测试是否成功
		{
			strOut = strErrMsg;
			return false;
		}

		this->ToString_2020(strHex);

		strRet = "";
		for (size_t i  = 0; i < strHex.size(); i++)
		{
			strRet += boost::str(boost::format("%02x") % ((int)strHex[i]));
		}
		strOut = strRet;

		return true;
	}


};


//************************************
// 说明: 对交易信息进行签名   输出  r,s 共 64字节
// Method:    ECDSA_SignRawTxWithPrivKey
// FullName:  cosmos::ECDSA_SignRawTxWithPrivKey
// Access:    public 
// Returns:   int   错误码:  成功:0    失败:-1
// Qualifier:  
// Parameter: unsigned char * pszIn     数据输入
// Parameter: unsigned int uInLen      输入数据的长度
// Parameter: unsigned char *pszPrivKey 私钥
// Parameter: unsigned int uPrivKeyLen   私钥长度   
// Parameter: unsigned char * pszOut   输出数据
// Parameter: unsigned int uOutBufLen   输出buf的长度
// Parameter: unsigned int * puOutDataLen  输出数据的实际长度
// Parameter: char * pszErrMsg          错误信息
//************************************

int ECDSA_Sign(
	unsigned char *pszIn, 
	unsigned int uInLen, 
	unsigned char *pszPrivKey,
	unsigned int  uPrivKeyLen,
	unsigned char *pszOut,
	unsigned int uOutBufLen, 
	unsigned int *puOutDataLen, 
	char *pszErrMsg);


//SHA256(pJsonBuf, uBufSize, pSha256Out);

//int OpenSSL_SHA256()





}
#endif // _CCOSMOS_H_
