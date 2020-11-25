//author: yqq
//date:2019-04-18
//desc: 
//     ��̫�����״�����ǩ��,  �ο� cpp-ethereum



#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>


#ifdef MY_DLL_API
#define MY_DLL_API __declspec(dllexport)
#else
#define MY_DLL_API __declspec(dllimport)

#endif







#define  WEI		(1)
#define  KWEI		(1000 * WEI)
#define  MWEI		(1000 * KWEI)
#define  GWEI		(1000 * MWEI)
#define  MICROETHER (1000 * GWEI)				//΢ether
#define  MILLIETHER (1000 * MICROETHER)			//��ether
#define  ETHER		(1000 * MILLIETHER)			//ether

#define	 STR_GWEI_0_SUFFIX				("000000000")		//
#define  UINT_MAX_WITHDRAW_AMOUNT		(9999)
#define  FLOAT_MIN_WITHDRAW_AMOUNT		(0.00000001)



namespace eth
{
	enum ETHChainID
	{
		None = 0,
		Mainnet = 1,
		//EXPANSE = 2,  //��ʱδʹ��
		Ropsten = 3,
		Rinkeby = 4,
		Goerli = 5,
		Kovan = 42,
		EthereumClassic=61, //ETC

		Geth_Private_Chains = 1337 //geth˽��Ĭ��id
	};


	typedef struct _EthTxData
	{
		explicit _EthTxData()
		{
			uChainId = ETHChainID::None;
			pszNonce = NULL;
			pszGasStart = NULL;
			pszValue = NULL;
			pszGasPrice = NULL;
			pData = NULL;
			pszAddrTo = NULL;
		}

		ETHChainID			uChainId;				//chainId , 
		char				*pszNonce;				//nonce , ͨ���ڵ�� eth.getTransactionCount rpc�ӿڻ�ȡnonce
		char				*pszValue;				//value, ���, ��λ:wei
		char				*pszGasPrice;			//gasprice,  ��λ:wei
		char				*pszGasStart;			//gasstart , ��Ϊ:wei
		unsigned char		*pData;					//data��������, ��ʮ�����Ʊ����, ���ֶ�Ĭ��Ϊ��!
		unsigned int		uDataLen;				//data����
		//std::string		strAddrTo;				//Ŀ�ĵ�ַ    //��Ҫʹ�� STL
		char				*pszAddrTo;				//Ŀ�ĵ�ַ
	}EthTxData;




	struct MY_DLL_API Transaction
	{
		enum ETH_ERRCODE
		{
			ETH_NO_ERROR = 0,

			ETH_ERR_BadRLP = 95,
			ETH_ERR_ErrChainID = 96, //�����chainID
			ETH_ERR_SECP256K1_ECDSA_SIGN_RECOVERABLE_faild = 97, //��������ʧ��
			ETH_ERR_INVALID_SIG = 99, //��Чǩ��

		};

		/*enum FIELD_POS
		{
			pos_nonce = 0,
			pos_receive_addr,
			pos_value,
			pos_gaslimit,
			pos_gasprice,
			pos_data,
			pos_v,
			pos_r,
			pos_s
		};*/


		
		//@warning: DLL�ӿڲ�Ҫʹ�� STL, ����Ҫ��(std::string, std::vector, std::list, ...�ȵ�)
		static int Sign(const EthTxData &ethTxData, const char *pszPrivKey, unsigned char *pOutBuf, unsigned int uBufLen,  unsigned int *puOutLen);


	};




	enum PricisionLevel
	{
		//LEVEL_WEI = 0,
		ETH_LEVEL_GWEI,
		//LEVEL_KWEI,
		//LEVEL_MWEI,
		//LEVEL_GWEI,
		//LEVEL_MICROETHER,
		//LEVEL_MILLIETHER,
		//LEVEL_ETHER
	};


	//��������: ��ether����Ϊ wei
	//ע��:
	//dValue�����ֵ(���ᵼ���������)���㷽��:
	//>>> 1.0 * (2**64 - 1) / (10**9)
	//18446744073.709553      
	//����С�����8λ, �� 0.00000001   , �����������ȥ!
	inline int EtherToWei(double dValue, std::string &strValueWei, PricisionLevel uPrecisionLevel = ETH_LEVEL_GWEI)
	{
		if (dValue > UINT_MAX_WITHDRAW_AMOUNT || dValue < FLOAT_MIN_WITHDRAW_AMOUNT)
		{
			return 1;
		}

		switch (uPrecisionLevel)
		{
		case ETH_LEVEL_GWEI:
		{
			unsigned long long int  ullValue = (unsigned long long int)(dValue * GWEI);
			char buf[100] = { 0 };
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%llu", ullValue);
			strValueWei = std::string(buf) + STR_GWEI_0_SUFFIX;  //��9��0
		}
		break;
		default:
			return 2;
			break;
		}

		if (strValueWei.empty())
		{
			return 3;
		}
		return 0;
	}

}



