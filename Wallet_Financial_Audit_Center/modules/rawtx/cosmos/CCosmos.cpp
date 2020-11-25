
/************************************************************************
作者: yqq
日期: 2019-05-08  14:25
说明: Cosmos类币的签名库  支持 (USDP, HTDF)
************************************************************************/
#pragma execution_character_set("utf-8")

#include "CCosmos.h"


//#include <type_traits>
#include "secp256k1/secp256k1.h"
#include "secp256k1/secp256k1_recovery.h"
#include <boost/algorithm/string.hpp>



int cosmos::ECDSA_Sign(
	unsigned char *pszIn, 
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
		return  cosmos::ARGS_ERROR;
	}

	if (NULL == pszIn)
	{
		strcpy(pszErrMsg , "pszIn is null.");
		return  cosmos::ARGS_ERROR;
	}

	if (0 == uInLen)
	{
		strcpy(pszErrMsg, "uInLen is 0.");
		return  cosmos::ARGS_ERROR;
	}

	if (NULL == pszPrivKey)
	{
		strcpy(pszErrMsg, "pszPrivKey is null.");
		return cosmos::ARGS_ERROR;
	}

	if ( UINT_PRIV_KEY_LEN != uPrivKeyLen)
	{
		sprintf(pszErrMsg, "priv-key len is not %d bytes.", UINT_PRIV_KEY_LEN);
		return cosmos::ARGS_ERROR;
	}

	if (NULL == pszOut)
	{
		strcpy(pszErrMsg, "pszOut is null.");
		return  cosmos::ARGS_ERROR;
	}

	if (uOutBufLen < UINT_SIG_RS_LEN)
	{
		sprintf(pszErrMsg, "uOutBufLen less than %d. Must more than %d.", UINT_SIG_RS_LEN, UINT_SIG_RS_LEN);
		return cosmos::ARGS_ERROR;
	}

	if (NULL == puOutDataLen)
	{
		strcpy(pszErrMsg, "puOutDataLen is null");
		return  cosmos::ARGS_ERROR;
	}


	//2.进行签名
	auto* ctx = getCtx();

	secp256k1_ecdsa_recoverable_signature rawSig;
	memset(&rawSig.data, 0, 65);
	if (!secp256k1_ecdsa_sign_recoverable(ctx, &rawSig, pszIn, pszPrivKey, nullptr, nullptr))
	{
        strcpy(pszErrMsg, "secp256k1_ecdsa_sign_recoverable  failed.");
		return cosmos::ECCSIGN_STEP1_ERROR;
	}

	int iRecid = 0;
	unsigned char uszSigRSData[UINT_SIG_RS_LEN] = { 0 }; 
	memset(uszSigRSData, 0, sizeof(uszSigRSData));
	secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, uszSigRSData, &iRecid, &rawSig);


	//返回数据
	memcpy(pszOut, uszSigRSData, UINT_SIG_RS_LEN);
	*puOutDataLen = UINT_SIG_RS_LEN;


	return cosmos::NO_ERROR;
}
