// ETHDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//author:yqq
//description: 以太坊离线签名

#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>

#include "keccak.h" //2019-04-16  yqq 使用 keccak256 算法对交易rlp数据进行hash, 而不是使用 sha3_256

#include "Transaction.h"
using namespace std;
using namespace eth;



//未签名交易 rlp编码格式测试
int RLP_UnsignedTx_Test()
{
	Transaction tx;
	tx.m_addrTo = Address(fromUserHex("0xba1099cc91acdf45771d0a0c6e3b80e8e880c684"));
	tx.m_u256Nonce = 1;
	tx.m_u256GasStart = 4; //21000; //
	tx.m_u256GasPrice = 3;// 50000000000L;
	tx.m_u256Value = 0x2;//100000000000000000L;
	tx.m_bytesData.push_back(0x01);
	std::string strHexTmp;
	for (auto b : tx.RLP_Encode(false))
	{
		char buf[10] = { 0 };
		memset(buf, 0, sizeof(buf)) ;
		sprintf_s(buf, "%02x", b);
		strHexTmp += buf;
	}
	std::cout << strHexTmp << std::endl;

	if ("da01030494ba1099cc91acdf45771d0a0c6e3b80e8e880c6840201" == strHexTmp)
	{
		std::cout <<__FUNCTION__ <<"测试成功!" << std::endl;
	}
	else
	{
		std::cout  << __FUNCTION__ << "测试失败!" << std::endl;
		return -1;
	}

	return 0;
}

int RLP_UnsignedTx_Test2()
{
	Transaction tx;
	tx.m_addrTo = Address(fromUserHex("0xba1099cc91acdf45771d0a0c6e3b80e8e880c684"));
	tx.m_u256Nonce = 0;
	tx.m_u256GasStart = 21000; //
	tx.m_u256GasPrice = 50000000000L;
	tx.m_u256Value = 100000000000000000L;
	//tx.data.push_back();
	std::string strHexTmp;
	for (auto b : tx.RLP_Encode(false))
	{
		char buf[10] = { 0 };
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "%02x", b);
		strHexTmp += buf;
	}
	std::cout << strHexTmp << std::endl;

	if ("e980850ba43b740082520894ba1099cc91acdf45771d0a0c6e3b80e8e880c68488016345785d8a000080" == strHexTmp)
	{
		std::cout << __FUNCTION__ << "测试成功!" << std::endl;
	}
	else
	{
		std::cout << __FUNCTION__ << "测试失败!" << std::endl;
		return -1;
	}

	return 0;
}



//签名交易 测试用例
int RLP_SignRawTx_Test()
{
	Transaction tx(ETHChainID::None);
	tx.m_addrTo = Address(fromUserHex("0xba1099cc91acdf45771d0a0c6e3b80e8e880c684"));
	tx.m_u256Nonce = 0;
	tx.m_u256GasStart = 21000; //
	tx.m_u256GasPrice = 50000000000L;
	tx.m_u256Value = 100000000000000000L;


	eth::Secret privKey(fromUserHex("0x1be48a93cb149ea2cbc3e85db6056c83a37a1d7aafcee079c266dd05af2e7c31"));
	std::cout << "unsigned rlp: " << RLP2HexStr(tx.RLP_Encode(false)) << std::endl; 

	h256 rawHash =  tx.SHA3_Keccak256(false);
	std::cout << "----------->rawhash :" << rawHash;
	tx.Sign(privKey);


	std::string strHexTmp = RLP2HexStr(tx.RLP_Encode(true));
	std::cout << "signed rlp: " << strHexTmp << std::endl;

	//tx.data.push_back();
	

	if ("f86c80850ba43b740082520894ba1099cc91acdf45771d0a0c6e3b80e8e880c68488016345785d8a0000801ba0292c67ea3e190227a37f698afde85fd3fd32e51d8d7f54fe3d9104cdba78352da064dbd18e52ce50ec037aee21c5eb2c8db661b188d2e2469c33a2b2c75476647e" == strHexTmp)
	{
		std::cout << __FUNCTION__ << "测试成功!" << std::endl;
	}
	else
	{
		std::cout << __FUNCTION__ << "测试失败!" << std::endl;
		return -1;
	}


	return 0;
}



//2019-04-16 yqq
//测试空内容的sha3
//使用的是keccak256
//http://www.bluestep.cc/demos/tools/endecode/keccak_256.html
int TestSHA3()
{
	h256 ret;
	bytesRef retRef(&ret[0], 32);

	byte *tmp = nullptr;
	CryptoPP::Keccak_256 ctx;
	ctx.Update(tmp, 0);
	assert(retRef.size() >= 32);
	ctx.Final(retRef.data());

	std::cout <<"空串哈希:" << ret << std::endl;


	std::string strHexTmp;
	for (auto b : ret.asBytes())
	{
		char buf[10] = { 0 };
		memset(buf, 0, sizeof(buf));
		sprintf_s(buf, "%02x", b);
		strHexTmp += buf;
	}
	std::cout << strHexTmp << std::endl;


	if ("c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470" == strHexTmp)
	{
		std::cout << "测试sha3算法成功" << std::endl;
	}
	else
	{
		std::cout << "测试sha3算法失败" << std::endl;
		return -1;
	}

	return 0;
}



//@ref: 参考:https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
//@brief: 测试签名
int SignTest()
{
	Transaction tx(ETHChainID::Mainnet);
	tx.m_addrTo = Address(fromUserHex("0x3535353535353535353535353535353535353535"));
	tx.m_u256Nonce = 9;
	tx.m_u256GasStart = 21000; //
	tx.m_u256GasPrice = 20000000000;
	tx.m_u256Value = 1000000000000000000;

	eth::Secret privKey(fromUserHex("0x4646464646464646464646464646464646464646464646464646464646464646"));
	std::cout << "unsigned rlp: " << RLP2HexStr(tx.RLP_Encode(true)) << std::endl;

	h256 rawHash = tx.SHA3_Keccak256(true);
	std::cout << "rawhash:" << rawHash << std::endl;
	tx.Sign( privKey);


	std::string strHexTmp = RLP2HexStr(tx.RLP_Encode(true));
	std::cout << "signed rlp: " << strHexTmp << std::endl;


	if ("f86c098504a817c800825208943535353535353535353535353535353535353535880de0b6b3a76400008025a028ef61340bd939bc2195fe537567866003e1a15d3c71ff63e1590620aa636276a067cbe9d8997f761aecb703304b3800ccf555c9f3dc64214b297fb1966a3b6d83"
		!= strHexTmp)
	{
		std::cout << "交易签名测试失败" << std::endl;
		return -1;
	}
	std::cout << "交易签名测试成功" << std::endl;


	return 0;
}


//@brief: rinkeby真实交易
int MakeSignedRawTransaction_Rinkeby()
{
	Transaction tx(ETHChainID::Rinkeby); //Rinkeby测试网络
	tx.m_addrTo = Address(fromUserHex("0x954d1a58c7abd4ac8ebe05f59191Cf718eb0cB89"));
	tx.m_u256Nonce = 3;
	tx.m_u256GasStart = 21000; //
	tx.m_u256GasPrice = 1000000000; //20Gwei       1Gwei = 10^9 wei
	tx.m_u256Value = 5 * 100000000000000000; //0.5 eth

	//地址:0xe4bcfb9b6cd697d662cb485ee13e8f0f93b10241
	//私钥: e148bd556b2a43fe8b949fb353c2adba46c8f479b8b0eaafb904085874686f6c
	eth::Secret  privKey(fromUserHex("e148bd556b2a43fe8b949fb353c2adba46c8f479b8b0eaafb904085874686f6c"));
	std::cout << "unsigned rlp: " << RLP2HexStr(tx.RLP_Encode(true)) << std::endl;
	tx.Sign(privKey);

	std::string strHexTmp = RLP2HexStr(tx.RLP_Encode(true));
	std::cout << "创建的已签名的交易: " << strHexTmp << std::endl;



	return 0;
}

//@brief: 新建一个交易, nonce值必须通过节点获取  eth.getTransactionCount
//覆盖一笔交易, nonce值必须是与被覆盖的交易的nonce值相同, 且 手续费 大于原来那笔交易的 110%
int MakeSignedRawTransaction_2_Rinkeby()
{
	Transaction tx(ETHChainID::Rinkeby); //Rinkeby测试网络
	tx.m_addrTo = Address(fromUserHex("0xe4bcfb9b6cd697d662cb485ee13e8f0f93b10241"));
	tx.m_u256Nonce = 47;
	tx.m_u256GasStart = 21000; //
	tx.m_u256GasPrice = 1000000000; //20Gwei       1Gwei = 10^9 wei
	tx.m_u256Value = 3 * 100000000000000000; //0.5 eth

	//地址:0xe4bcfb9b6cd697d662cb485ee13e8f0f93b10241
	//私钥: e148bd556b2a43fe8b949fb353c2adba46c8f479b8b0eaafb904085874686f6c
	eth::Secret  privKey(fromUserHex("DBBAD2A5682517E4FF095F948F721563231282CA4179AE0DFEA1C76143BA9607"));
	std::cout << "unsigned rlp: " << RLP2HexStr(tx.RLP_Encode(true)) << std::endl;
	tx.Sign(privKey);

	std::string strHexTmp = RLP2HexStr(tx.RLP_Encode(true));
	std::cout << "创建的已签名的交易: " << strHexTmp << std::endl;



	return 0;
}

int TestBoostMultiPrecision()
{
	//u256 u256Test = 1000000000 * 10000000000000000000000000000;
	u256 u256Test("100000000000000000000000000000000000000");
	u256 u256Test2("1423423423");
	std::cout << u256Test << std::endl;
	std::cout << u256Test2 << std::endl;
	std::cout << u256Test + u256Test2 << std::endl;
	std::cout << u256Test - u256Test2 << std::endl;
	std::cout << u256Test * u256Test2 << std::endl;
	//std::cout << std::setprecision(std::numeric_limits<float>::digits10) << u256Test / u256Test2 << std::endl;


	return 0;
}


//template <class _T, class _Out>
//inline void ToBigEndian(_T _val, _Out& o_out)
//{
//	for (auto i = 0; i-- != 0; _val >>= 8)
//		o_out[i] = (typename _Out::value_type)(uint8_t)_val;
//}

int TestBigEndian()
{
	//u256 ubigTmp("329482293492834923498923423");
	u256 ubigTmp("9999999999999");

	std::string strOut;
	strOut = toBigEndianString(ubigTmp);

	

	std::cout << "----------------" << std::endl;
	std::string strTmp = "";
	for (int i = 0; i < strOut.size(); i++)
	{
		char buf[100] = { 0 };
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%02x", strOut[i]);
		//std::cout << buf;
		strTmp += buf;
	}
	std::cout << strTmp << std::endl;
	std::cout << "----------------" << std::endl;
	

	return 0;
}



int MainTest()
{
	map<string, int> mapCode;
	int iRet = -1;
#if 0
	std::cout << "-----------------------------------------" << std::endl;

	iRet = TestSHA3();
	mapCode.insert(make_pair("TestSHA3", iRet));

	std::cout << "-----------------------------------------" << std::endl;

	iRet = RLP_UnsignedTx_Test();
	mapCode.insert(make_pair("RLP_UnsignedTx_Test", iRet));

	std::cout << "-----------------------------------------" << std::endl;

	iRet = RLP_UnsignedTx_Test2();
	mapCode.insert(make_pair("RLP_UnsignedTx_Test2", iRet));

	std::cout << "-----------------------------------------" << std::endl;
	iRet = RLP_SignRawTx_Test();
	mapCode.insert(make_pair("RLP_SignRawTx_Test", iRet));
	std::cout << "-----------------------------------------" << std::endl;


	std::cout << "-----------------------------------------" << std::endl;
	iRet = SignTest();
	mapCode.insert(make_pair("SignTest", iRet));
	std::cout << "-----------------------------------------" << std::endl;

	

	std::cout << "-----------------------------------------" << std::endl;
	iRet = MakeSignedRawTransaction_Rinkeby();
	mapCode.insert(make_pair("MakeSignedRawTransaction_Rinkeby", iRet));
	std::cout << "-----------------------------------------" << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	iRet = MakeSignedRawTransaction_2_Rinkeby();
	mapCode.insert(make_pair("MakeSignedRawTransaction_Rinkeby", iRet));
	std::cout << "-----------------------------------------" << std::endl;

	std::cout << "-----------------------------------------" << std::endl;
	iRet = TestBoostMultiPrecision();
	mapCode.insert(make_pair("TestBoostMultiPrecision", iRet));
	std::cout << "-----------------------------------------" << std::endl;
#endif


	std::cout << "-----------------------------------------" << std::endl;
	iRet = TestBigEndian();
	mapCode.insert(make_pair("TestBigEndian", iRet));
	std::cout << "-----------------------------------------" << std::endl;

	std::cout << std::endl << std::endl << std::endl;


	std::cout << "==============================测试结果======================================" << std::endl;
	for (auto item : mapCode)
	{
		std::cout << item.first << " : " << (0 == item.second ? "测试成功" : "测试失败") << std::endl;
	}
	std::cout << "====================================================================" << std::endl;


	return 0;
}


int main()
{
	MainTest();



	system("pause");
	return 0;
}
