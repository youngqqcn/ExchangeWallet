#include <string>
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <memory>
#include <vector>
#include "openssl/sha.h"  //openssl sha256
#include "openssl/evp.h"
#include "secp256k1/secp256k1.h"
#include "secp256k1/secp256k1_recovery.h"


#define MY_DLL_API __declspec(dllexport) 
#include "eos_tx_sign.h"

#define  UINT_PRIV_KEY_LEN     32

namespace eos
{
	using namespace std;



	static const char * const ALPHABET =
				"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
	static const char ALPHABET_MAP[128] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1,
		-1,  9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1,
		22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
		-1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46,
		47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1
	};

	static int Base58Encode(const std::string input, int len, unsigned char result[]) 
	{
		unsigned char const* bytes = (unsigned const char*)(input.c_str());
		unsigned char *digits = new unsigned char[len * 137 / 100];
		memset(digits, 0, len * 137 / 100);

		int digitslen = 1;
		for (int i = 0; i < len; i++) 
		{
			unsigned int carry = (unsigned int)bytes[i];
			for (int j = 0; j < digitslen; j++) 
			{
				carry += (unsigned int)(digits[j]) << 8;
				digits[j] = (unsigned char)(carry % 58);
				carry /= 58;
			}
			while (carry > 0) 
			{
				digits[digitslen++] = (unsigned char)(carry % 58);
				carry /= 58;
			}
		}

		int resultlen = 0;
		// leading zero bytes
		for (; resultlen < len && bytes[resultlen] == 0;)
			result[resultlen++] = '1';
		// reverse
		for (int i = 0; i < digitslen; i++)
			result[resultlen + i] = ALPHABET[digits[digitslen - 1 - i]];
		result[digitslen + resultlen] = 0;

		delete[] digits;
		return digitslen + resultlen;
	}

	static int Base58Decode(const std::string input, int len, unsigned char *result) 
	{
		unsigned char const* str = (unsigned const char*)(input.c_str());
		result[0] = 0;
		int resultlen = 1;
		for (int i = 0; i < len; i++) 
		{
			unsigned int carry = (unsigned int)ALPHABET_MAP[str[i]];
			for (int j = 0; j < resultlen; j++)
			{
				carry += (unsigned int)(result[j]) * 58;
				result[j] = (unsigned char)(carry & 0xff);
				carry >>= 8;
			}
			while (carry > 0) 
			{
				result[resultlen++] = (unsigned int)(carry & 0xff);
				carry >>= 8;
			}
		}

		for (int i = 0; i < len && str[i] == '1'; i++)
			result[resultlen++] = 0;

		for (int i = resultlen - 1, z = (resultlen >> 1) + (resultlen & 1); i >= z; i--) 
		{
			int k = result[i];
			result[i] = result[resultlen - i - 1];
			result[resultlen - i - 1] = k;
		}
		return resultlen;
	}


	inline secp256k1_context const* getCtx()
	{
		static std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> s_ctx{
			secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY),
			&secp256k1_context_destroy
		};
		return s_ctx.get();
	}


	inline  std::string Bin2HexStr(const unsigned char *pBin, unsigned int len)
	{
		std::string  strHex;
		for (int i = 0; i < len; i++)
		{
			char buf[10] = { 0 };
			memset(buf, 0, sizeof(buf));
			sprintf_s(buf, "%02x", pBin[i]);
			strHex += buf;

		}
		return strHex;
	}

	inline  std::vector<unsigned char>  Bin2ByteArray(const unsigned char *pBin, unsigned int len)
	{
		std::vector<unsigned char>  vctBytes;

		for (unsigned int i = 0; i < len; i++)
		{
			vctBytes.push_back(pBin[i]);
		}
		return vctBytes;
	}

	inline  std::vector<unsigned char>  Bin2ByteArray(const std::string &strTmp, unsigned int len)
	{
		std::vector<unsigned char>  vctBytes;

		for (unsigned int i = 0; i < strTmp.size(); i++)
		{
			vctBytes.push_back(strTmp[i]);
		}
		return vctBytes;

	}

	inline void PrintBytesArray(std::vector<unsigned char> &vctBytes)
	{
		for (size_t i = 0; i < vctBytes.size(); i++)
		{
			//std::cout << vctBytes[i] << ",";
			printf("%d,", vctBytes[i]);
		}
		std::cout << endl;

	}




	inline string HexToBin(const string &strHexIn)
	{
		if (strHexIn.size() % 2 != 0) return "";

		string strHex;
		if (std::string::npos == strHexIn.find("0x"))
			strHex = strHexIn;
		else
			strHex = strHexIn.substr(2);


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




	inline int GetDigest(
		const EVP_MD *type, 
		const char *pszInData, 
		const int iInDataLen,
		unsigned char *pszOutData, 
		unsigned int *pOutDataLen ) 
	{
		int i, ret = 0;
		EVP_MD_CTX ctx;
		EVP_MD_CTX_init(&ctx);
		EVP_DigestInit_ex(&ctx, type, NULL);
		EVP_DigestUpdate(&ctx, pszInData, iInDataLen);
		EVP_DigestFinal_ex(&ctx, pszOutData, pOutDataLen);
		EVP_MD_CTX_cleanup(&ctx);
		return ret;
	}




	
	//************************************
	// Method:    SigCheckEncode        �����������У��Ͳ�����
	// FullName:  EOS::SigCheckEncode
	// Access:    public 
	// Returns:   std::string
	// Qualifier:
	// Parameter: const unsigned char * pszIRS     ECDSA��ǩ�����  [i][r][s]
	// Parameter: const unsigned int uDataLen      ǩ������ĳ��� 65�ֽ�
	//************************************
	inline int SigCheckEncode(const unsigned char *pszIRS,  const unsigned int uDataLen, std::string &strRet)
	{
		//�����ж�
		if (65 != uDataLen)
		{
			printf("args error: uDataLen != 65");
			return -1;
		}

		//����1:  ׷�� "K1"
		std::string strCheck = Bin2HexStr(pszIRS, uDataLen);
		std:string strForRIP160= HexToBin(strCheck);
		strForRIP160.append(1, 0x4b); //'K'
		strForRIP160.append(1, 0x31); //'1'

		//����2: ����Ripemd160��ϣ
		unsigned char uszRIP160[256] = { 0 };
		unsigned int uRIP160Len = 0;
		GetDigest(EVP_ripemd160(), strForRIP160.data(), strForRIP160.size(), uszRIP160, &uRIP160Len);

		//����3: ȡǰ4�ֽ�
		std::string strChkSum;
		strChkSum.append((const char *)uszRIP160, 4); 
		std::string strForBase58 = std::string((char *)pszIRS, uDataLen) + strChkSum;

		//����4: ���� Base58����
		unsigned char szBase58Sig[256] = { 0 };
		memset(szBase58Sig, 0, sizeof(szBase58Sig));
		Base58Encode(strForBase58,  strForBase58.size(), szBase58Sig);
		printf("%s\n", szBase58Sig); // "StV1DL6CwTryKyV"

		//����5: ����"SIG_K1" �ַ���ǰ׺
		strRet = std::string("SIG_K1_") + std::string((char *)szBase58Sig);
		return 0;
	}


	/*

	def Base58CheckEncode(version, payload):
	'''
	:param version: �汾ǰ׺  , ������������ �� ��������
	:param payload:
	:return:
	'''
	s = chr(version) + payload
	checksum = hashlib.sha256(hashlib.sha256(s).digest()).digest()[0:4]  #����sha256, ��ǰ4�ֽ���ΪУ���
	result = s + checksum
	leadingZeros = CountLeadingChars(result, '\0')
	return '1' * leadingZeros + Base58encode(Base256decode(result))

	*/

	//************************************
	// Returns:   int          0:�ɹ�     ��0: ʧ��
	// Parameter: const char * pszWIFKey           WIF��ʽ��˽Կ
	// Parameter: unsigned char * pDecodedKey      ������˽Կ
	// Parameter: unsigned int * puDecodedKeyLen   ������˽Կ����(32�ֽ�)
	//************************************
	int DecodeWIFPrivKey(
		const char *pszWIFKey,
		unsigned char *puszDecodedKey,
		unsigned int *puDecodedKeyLen
	)
	{

		//�����ж�
		std::string strWifKey(pszWIFKey);
		unsigned char uszDecodePrivKey[1024] = { 0 };
		memset(uszDecodePrivKey, 0, sizeof(uszDecodePrivKey));

		int iRetLen = Base58Decode(strWifKey, strWifKey.length(), uszDecodePrivKey);
		if (1 + UINT_PRIV_KEY_LEN + 4 != iRetLen) // ver(1) + priv(32) + chksum(4)
		{
			printf("decode err: base58 decoded retlen < 37 ");
			return -2;
		}

		std::cout << Bin2HexStr(uszDecodePrivKey, iRetLen) << std::endl;
		memcpy(puszDecodedKey, uszDecodePrivKey + 1, UINT_PRIV_KEY_LEN);   //
		*puDecodedKeyLen = UINT_PRIV_KEY_LEN;

		std::cout << "privKey:" << Bin2HexStr(puszDecodedKey, UINT_PRIV_KEY_LEN) << std::endl;
		return 0;
	}




	//************************************
	// Method:    EosTxSign           EOS����ǩ��
	// FullName:  EosTxSign
	// Access:    public 
	// Returns:    0:�ɹ�   ��0: ʧ��
	// Qualifier:
	// Parameter: const unsigned char * pszPrivKey     ˽Կ(WIF����ת������ֽ���ʽ)
	// Parameter: const unsigned int uPrivKeyLen     ˽Կ���� (������32�ֽ�)
	// Parameter: const unsigned char * pInData      Ҫǩ��������
	// Parameter: const unsigned int uInDataLen      Ҫǩ�������ݵĳ���(������64�ֽ�)
	// Parameter: unsigned char * pOutData			 ǩ�����(����base58������ַ���, �� SIG_K1_xxxxx
	// Parameter: unsigned int * puOutDataLen        ǩ������ĳ���(65�ֽ�)
	//************************************
	int  EosTxSign(
		const unsigned char *pszPrivKey,
		const unsigned int uPrivKeyLen,
		const unsigned char *pInData,
		const unsigned int uInDataLen,
		char *pOutData, 
		unsigned int *puOutDataLen
	)
	{
		//0.�������
		if (NULL == pszPrivKey || UINT_PRIV_KEY_LEN != uPrivKeyLen )
		{
			printf("args error: pszPrivKey is null or uPrivKeyLen != 32\n");
			return -1;
		}
		if (NULL == pInData || 32 != uInDataLen)
		{
			printf("args error: pInData is null or uInDataLen != 32\n");
			return -1;
		}
		if (NULL == pOutData || NULL == puOutDataLen)
		{
			printf("args error: pOutData is null or puOutDataLen is null.\n");
			return -1;
		}


		unsigned char szSignData[32] = { 0 };
		memset(szSignData, 0, sizeof(szSignData));
		memcpy(szSignData, pInData, uInDataLen) ;
		unsigned char * const pSignData = szSignData;

		unsigned char uszCustomNonceData[32] = { 0 }; //�Զ��������������Դ(RFC6979)
		memset(uszCustomNonceData, 0, sizeof(uszCustomNonceData));

		for (unsigned int uRightZeroPadCount = 0; ; uRightZeroPadCount++)
		{
			//1.����sha256��ϣ
			memset(uszCustomNonceData, 0, sizeof(uszCustomNonceData));
			std::string  strTmpBinForSha256((const char *)pSignData, uInDataLen);;
			strTmpBinForSha256.append(uRightZeroPadCount, 0); //�Ҳ���
			SHA256((unsigned char *)strTmpBinForSha256.c_str(), strTmpBinForSha256.size(), uszCustomNonceData);


			//2.����ǩ��
			auto ctx = getCtx();
			secp256k1_ecdsa_recoverable_signature rawSig;
			memset(&rawSig.data, 0, 65);
			if (!secp256k1_ecdsa_sign_recoverable(ctx, &rawSig, pSignData, pszPrivKey, nullptr, uszCustomNonceData))
			{
				std::cout << "secp256k1_ecdsa_sign_recoverable failed" << std::endl;
				return -2;
			}

			//3.����ECDSAǩ�����ؽ��
			int iRecoverId = 0;
			unsigned char uszSigData[65] = { 0 }; 
			memset(uszSigData, 0, sizeof(uszSigData));
			secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, uszSigData, &iRecoverId, &rawSig);


			//תΪDER��ʽ��ǩ����������ϸ���,�Խ������: 'is_canonical( c ): signature is not canonical'
			if (true)
			{

				secp256k1_ecdsa_signature rsSig;
				memcpy(rsSig.data, rawSig.data, 64);

				unsigned char uszDER_R_S[100];
				memset(uszDER_R_S, 0, sizeof(uszDER_R_S));
				size_t uOutLen = 100;
				secp256k1_ecdsa_signature_serialize_der(ctx, uszDER_R_S, &uOutLen, &rsSig);

				size_t nLenR = uszDER_R_S[3];
				size_t nLenS = uszDER_R_S[5 + nLenR];

				if (!(32 == nLenR && 32 == nLenS))
				{
					continue;
				}
			}


			string strR = Bin2HexStr(uszSigData, 32);
			string strS = Bin2HexStr(uszSigData + 32, 32);
			std::cout << "r:" << Bin2HexStr(uszSigData, 32) << std::endl;
			std::cout << "s:" << Bin2HexStr(uszSigData + 32, 32) << std::endl;
			std::cout << "------------------------------------" << std::endl;

			//4.У��  r, s  �Ƿ���ϱ�׼��, ���������, ����� uRightZeroPadCount
			//Ҫ��  int(r[:2]) <= 0x7F7F
			//Ҫ��  int(s[:2]) <= 0x7F7F
			/*if ( ! (true
				    && 0 == (uszSigData[0] & 0x80) 
				    && (! ((0 == uszSigData[0] && uszSigData[1]  ) ))
				    && 0 == (uszSigData[32] & 0x80) 
				    && 0 == (uszSigData[33] & 0x80)
				   )
			   )
			{
				continue;
			}*/


			//5.��ǩ�����  i, r, s,����ƴװ
			unsigned char usz_I_R_S[65] = { 0 };
			memset(usz_I_R_S, 0, sizeof(usz_I_R_S));
			memset(usz_I_R_S, iRecoverId + 4 + 27, 1 ); // i 
			memcpy(usz_I_R_S + 1, uszSigData, 64); // r, s
			std::cout <<"irs: " << Bin2HexStr(usz_I_R_S, sizeof(usz_I_R_S)) << std::endl;

			//6.�� i,r,s ����У��Ͳ����н���base58����, �õ����յ�ǩ����� ���� SIG_K1_xxxxxx
			std::string strCheckEncodeSig;
			SigCheckEncode(usz_I_R_S, sizeof(usz_I_R_S), strCheckEncodeSig);
			std::cout << strCheckEncodeSig << std::endl;

			//7.����ǩ���ַ���,���ַ�������
			strcpy(pOutData, strCheckEncodeSig.c_str());
			*puOutDataLen = strlen(strCheckEncodeSig.c_str()) ;
			break;
		}

		return 0;
	}





	int  EosTxSignWithWIFKey(
		const char *pszWIFPrivKey,
		const unsigned char *pInData,
		const unsigned int uInDataLen,
		char *pOutData,
		unsigned int *puOutDataLen
	)
	{
		//1.�����ж�
		if (NULL == pszWIFPrivKey ||  51 != strlen(pszWIFPrivKey))
		{
			printf("args error: pszPrivKey is null or uPrivKeyLen != 32\n");
			return -1;
		}
		if (NULL == pInData || 32 != uInDataLen)
		{
			printf("args error: pInData is null or uInDataLen != 32\n");
			return -1;
		}
		if (NULL == pOutData || NULL == puOutDataLen)
		{
			printf("args error: pOutData is null or puOutDataLen is null.\n");
			return -1;
		}


		//2.����WIF��ʽ˽Կ
		unsigned char uszDecodedPrivKey[UINT_PRIV_KEY_LEN + 1024] = { 0 };
		memset(uszDecodedPrivKey, 0, sizeof(uszDecodedPrivKey));

		unsigned int uDecodeLen = 0;
		int iRet = DecodeWIFPrivKey(pszWIFPrivKey, uszDecodedPrivKey, &uDecodeLen);
		if (0 != iRet)
		{
			return iRet;
		}

		EosTxSign(uszDecodedPrivKey, UINT_PRIV_KEY_LEN, pInData, uInDataLen, pOutData, puOutDataLen );




		return 0;
	}






};
