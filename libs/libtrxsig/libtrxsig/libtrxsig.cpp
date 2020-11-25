// libtrxsig.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
using namespace std;

#include "TrxSig.h"

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

void SigTest()
{
	char szTxid[] = "fa1d4878612ac088055c8292d50ac93880968d7e4b53c8a610dee67d1f310d93";
	std::string strBinTxid = HexToBin(szTxid);

	char szPrivKey[] = "04f73c190f8143c8abe8b3d85e8c4faf1d148c43f261a89793398c370851e48d";
	std::string strBinPrivKey = HexToBin(szPrivKey);
	

	unsigned char uszOutBuf[1024] = { 0 };
	memset(uszOutBuf, 0, sizeof(uszOutBuf));


	unsigned int  uOutDataLen = 0;

	char szMsg[1024] = { 0 };


	int iRet = trx::TrxUitls::ECDSA_Sign(
		(unsigned char *)strBinTxid.data(),
		strBinTxid.size(),
		(unsigned char *)strBinPrivKey.data(),
		strBinPrivKey.size(),
		uszOutBuf,
		sizeof(uszOutBuf),
		&uOutDataLen,
		szMsg
		);

	//签名结果: d6f51775288e588d2af4440f743bd230b2e6fc765bd52e56927972f4ea1d1d924086a1bf1d9051c762e9ab184647f0f7266d8d33345645fac40a40a98b3e5c741c

 	std::string strHexSig = Bin2HexStr( uszOutBuf, uOutDataLen );

	std::cout << "签名结果: " << strHexSig << std::endl;

	std::string strRightSig = "d6f51775288e588d2af4440f743bd230b2e6fc765bd52e56927972f4ea1d1d924086a1bf1d9051c762e9ab184647f0f7266d8d33345645fac40a40a98b3e5c741c";
	if (0 == strHexSig.compare(strRightSig))
	{
		std::cout << "签名比对成功! " << std::endl;
	}

}

int main()
{

	SigTest();
    std::cout << "Hello World!\n"; 
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
