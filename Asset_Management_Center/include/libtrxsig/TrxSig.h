#ifndef __TRXSIG_H__
#define  __TRXSIG_H__

//author: yqq
//date:2020-03-04
//desc: 
//     TRX ǩ��,    ��  secp256k1 ǩ��

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MY_DLL_API
#define MY_DLL_API __declspec(dllexport)
#else
#define MY_DLL_API __declspec(dllimport)

#endif



#define  UINT_SIG_RSV_LEN		(65)		//130 (r , s, v) ǩ����Ϣ, ��װ���ĳ���
#define  UINT_PRIV_KEY_LEN		(32)

namespace trx
{
	enum ErrorCode
	{
		NO_ERROR = 0,
		ARGS_ERROR = 1001,
		ECCSIGN_STEP1_ERROR = 1005,
		ECCSIGN_STEP2_ERROR = 1006
	};

	struct MY_DLL_API TrxUitls 
	{

	
		//@warning: DLL�ӿڲ�Ҫʹ�� STL, ����Ҫ��(std::string, std::vector, std::list, ...�ȵ�)
		static int ECDSA_Sign(
			unsigned char *pszIn,
			unsigned int uInLen,
			unsigned char *pszPrivKey,
			unsigned int uPrivKeyLen,
			unsigned char *pszOut,
			unsigned int uOutBufLen,
			unsigned int *puOutDataLen,
			char *pszErrMsg);


	};


}

#endif //__TRXSIG_H__


