/*
author: yqq    
email:youngqqcn@gmail.com  
github: github.com/youngqqcn
date: 2020-03-04
descriptions: 
*/


//#include <secp256k1.h>
#include "secp256k1/secp256k1.h"
#include "secp256k1/secp256k1_recovery.h"
#include "secp256k1/secp256k1_ecdh.h"


#define MY_DLL_API __declspec(dllexport)

#include "TrxSig.h"
#include <memory>
#include <iostream>
using namespace std;
using namespace trx;


/*
#define  INT_SINGED_TX_FILED_COUNT	(9)
#define  INT_SIG_V_OFFSET			(27)
#define  INT_SIG_INFO_SIZE			(65)  //32(r) + 32(s) + 1(v)
*/


//static const u256 c_secp256k1n("115792089237316195423570985008687907852837564279074904382605163141518161494337");

inline secp256k1_context const* getCtx()
{
	static std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> s_ctx{
		secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY),
		&secp256k1_context_destroy
	};
	return s_ctx.get();
}



int TrxUitls::ECDSA_Sign(
	unsigned char *pszInTxid,
	unsigned int uInLen,
	unsigned char *pszPrivKey,
	unsigned int uPrivKeyLen,
	unsigned char *pszOut,
	unsigned int uOutBufLen,
	unsigned int *puOutDataLen,
	char *pszErrMsg)
{
	//1.检查参数
	if (NULL == pszErrMsg)
	{
		return  trx::ARGS_ERROR;
	}

	if (NULL == pszInTxid)
	{
		strcpy(pszErrMsg, "pszInTxid is null.");
		return  trx::ARGS_ERROR;
	}

	if (0 == uInLen)
	{
		strcpy(pszErrMsg, "uInLen is 0.");
		return  trx::ARGS_ERROR;
	}

	if (NULL == pszPrivKey)
	{
		strcpy(pszErrMsg, "pszPrivKey is null.");
		return trx::ARGS_ERROR;
	}

	if (UINT_PRIV_KEY_LEN != uPrivKeyLen)
	{
		sprintf(pszErrMsg, "priv-key len is not %d bytes.", UINT_PRIV_KEY_LEN);
		return trx::ARGS_ERROR;
	}

	if (NULL == pszOut)
	{
		strcpy(pszErrMsg, "pszOut is null.");
		return  trx::ARGS_ERROR;
	}

	if (uOutBufLen < UINT_SIG_RSV_LEN)
	{
		sprintf(pszErrMsg, "uOutBufLen less than %d. Must more than %d.", UINT_SIG_RSV_LEN, UINT_SIG_RSV_LEN);
		return trx::ARGS_ERROR;
	}

	if (NULL == puOutDataLen)
	{
		strcpy(pszErrMsg, "puOutDataLen is null");
		return  trx::ARGS_ERROR;
	}


	//2.进行签名
	auto* ctx = getCtx();

	secp256k1_ecdsa_recoverable_signature rawSig;
	memset(&rawSig.data, 0, 65);
	if (!secp256k1_ecdsa_sign_recoverable(ctx, &rawSig, pszInTxid, pszPrivKey, nullptr, nullptr))
	{
		strcpy(pszErrMsg, "secp256k1_ecdsa_sign_recoverable  faield.");
		return trx::ECCSIGN_STEP1_ERROR;
	}

	int iRecid = 0;
	unsigned char uszSigRSData[UINT_SIG_RSV_LEN] = { 0 };
	memset(uszSigRSData, 0, sizeof(uszSigRSData));
	secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, uszSigRSData, &iRecid, &rawSig);

	uszSigRSData[UINT_SIG_RSV_LEN - 1] = iRecid + 27;

	//返回数据
	memcpy(pszOut, uszSigRSData, UINT_SIG_RSV_LEN);
	*puOutDataLen = UINT_SIG_RSV_LEN;


	return trx::NO_ERROR;
}

