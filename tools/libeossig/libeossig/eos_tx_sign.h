#ifndef  __EOS_TX_SIGN__
#define  __EOS_TX_SIGN__



#ifdef MY_DLL_API
#define MY_DLL_API __declspec(dllexport)
#else
#define MY_DLL_API __declspec(dllimport)
#endif


namespace eos
{
#ifdef __cplusplus
	extern "C" {
#endif


		//************************************
		// Method:    EosTxSign
		// FullName:  EosTxSign
		// Access:    public 
		// Returns:    0:�ɹ�   ��0: ʧ��
		// Qualifier:
		// Parameter: const unsigned char * pPrivKey     ˽Կ(WIF����ת������ֽ���ʽ)
		// Parameter: const unsigned int uPrivKeyLen     ˽Կ���� (������32�ֽ�)
		// Parameter: const unsigned char * pInData      Ҫǩ��������
		// Parameter: const unsigned int uInDataLen      Ҫǩ�������ݵĳ���(������64�ֽ�)
		// Parameter: unsigned char * pOutData			 ǩ�����
		// Parameter: unsigned int * puOutDataLen        ǩ������ĳ���(65�ֽ�)
		//************************************
		/*
		int  EosTxSign(
			const unsigned char *pszPrivKey,
			const unsigned int uPrivKeyLen,
			const unsigned char *pInData,
			const unsigned int uInDataLen,
			char *pOutData,
			unsigned int *puOutDataLen
		);
		*/



		//************************************
		// Method:    EosTxSign
		// FullName:  EosTxSign
		// Access:    public 
		// Returns:    0:�ɹ�   ��0: ʧ��
		// Qualifier:
		// Parameter: const unsigned char * pPrivKey     WIF��ʽ��˽Կ
		// Parameter: const unsigned char * pInData      Ҫǩ��������
		// Parameter: const unsigned int uInDataLen      Ҫǩ�������ݵĳ���(������64�ֽ�)
		// Parameter: unsigned char * pOutData			 ǩ�����
		// Parameter: unsigned int * puOutDataLen        ǩ������ĳ���(65�ֽ�)
		//************************************
		MY_DLL_API int  EosTxSignWithWIFKey(
			const char *pszWIFPrivKey,
			const unsigned char *pInData,
			const unsigned int uInDataLen,
			char *pOutData,
			unsigned int *puOutDataLen
		);


		//************************************
		// Returns:   int          0:�ɹ�     ��0: ʧ��
		// Parameter: const char * pszWIFKey           WIF��ʽ��˽Կ
		// Parameter: unsigned char * pDecodedKey      ������˽Կ
		// Parameter: unsigned int * puDecodedKeyLen   ������˽Կ����(32�ֽ�)
		//************************************
		/*int DecodeWIFPrivKey(
			const char *pszWIFKey,
			unsigned char *pDecodedKey,
			unsigned int *puDecodedKeyLen
		);*/


#ifdef __cplusplus
	}
#endif



};


#endif // __EOS_TX_SIGN__
