/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	Foobar is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Transaction.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

//author: yqq
//date:2019-04-18
//desc: 以太坊交易



#pragma once

#include "Common.h"
#include "RLP.h"



namespace eth
{

typedef struct _SIG
{
	byte v;
	u256 r;
	u256 s;
}VRS_SIG;

using Signature_yqq = h520;

struct SignatureStruct
{
	SignatureStruct() = default;
	SignatureStruct(Signature_yqq const& _s) { *(h520*)this = _s; }
	SignatureStruct(h256 const& _r, h256 const& _s, byte _v) : r(_r), s(_s), v(_v) {}
	operator Signature_yqq() const { return *(h520 const*)this; }

	/// @returns true if r,s,v values are valid, otherwise false
	bool isValid() const noexcept;

	h256 r;
	h256 s;
	byte v = 0;
};



enum ETHChainID
{
	None = 0,
	Mainnet = 1,
	//EXPANSE = 2,  //暂时未使用
	Ropsten = 3,
	Rinkeby = 4,
	Goerli = 5,
	Kovan = 42,
	Geth_Private_Chains = 1337 //geth私链默认id
};

struct Transaction
{
	enum ERRCODE
	{
		NO_ERROR = 0,

		ERR_ErrChainID = 96, //错误的chainID
		ERR_SECP256K1_ECDSA_SIGN_RECOVERABLE_faild = 97, //函数调用失败
		ERR_INVALID_SIG = 99, //无效签名
	};

	enum FIELD_POS
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
	};

	explicit Transaction(uint uChainId = ETHChainID::None);
	explicit Transaction(bytesConstRef _rlp);
	explicit Transaction(bytes const& _rlp) : Transaction(&_rlp) {}


	//@brief: 对交易数据进行签名
	//@arg : rawHash rlp编码进行keccak哈希
	//@arg : privKey  私钥
	//@return: 错误码    0:成功   非0:失败
protected:
	int _Sign(h256 rawHash, Secret privKey);
public:
	int Sign(Secret privKey);


	//@brief: RLP编码
	//@arg : bSig是否包含签名信息(v,r,s)
	//@return: 
	void RLP_Encode(RLPStream& _s, bool bSig = true) const;
	bytes RLP_Encode(bool bSig = true) const { RLPStream s; RLP_Encode(s, bSig); return s.out(); }


	//@brief: RLP编码
	//@arg : bSig是否包含签名信息(v,r,s)
	//@return: std::string
	std::string RLP_EncodeString(bool bSig = true) const { return asString(RLP_Encode(bSig)); }

	// @brief : sha3
	// @arg bsig : 是否包含签名信息
	// @return :返回keccak256哈希
	h256 SHA3_Keccak256(bool bSig = true) const;


	// @brief : sha3
	// @arg bsig 是否包含签名信息
	// @return :返回keccak256哈希
	bytes SHA3_Keccak256_Bytes(bool bSig = true) const;

public:
	uint	m_uChainId;			//chainid

public://以下内容会进行RLP编码

	u256	m_u256Nonce;	//nonce, 此值应是发送地址的交易集中的最大nonce值加1,
	u256	m_u256Value;	//金额(wei),  1(eth)=10^18(wei)
	u256	m_u256GasPrice;	//gasprice, 一次操作所需gas
	u256	m_u256GasStart; //gasstart,也成 gaslimit, 
	bytes	m_bytesData;	//data,  附加数据,默认为空
	Address	m_addrTo;		//to,  目的地址
	VRS_SIG m_vrsSig;		//v,r,s  签名信息


};

// [ nonce, receiving_address, value, fee, [ data item 0, data item 1 ... data item n ], v, r, s ]
/*
struct Transaction
{
	Transaction() {}
	Transaction(bytesConstRef _rlp);
	Transaction(bytes const& _rlp) : Transaction(&_rlp) {}

	u256 nonce;
	Address receiveAddress;
	u256 value;
	u256 fee;
	u256s data;
	Signature vrs;

	Address sender() const;
	void sign(Secret _priv);

	static h256 kFromMessage(h256 _msg, h256 _priv);

	void fillStream(RLPStream& _s, bool _sig = true) const;
	bytes rlp(bool _sig = true) const { RLPStream s; fillStream(s, _sig); return s.out(); }
	std::string rlpString(bool _sig = true) const { return asString(rlp(_sig)); }
	h256 sha3(bool _sig = true) const { RLPStream s; fillStream(s, _sig); return eth::sha3(s.out()); }
	bytes sha3Bytes(bool _sig = true) const { RLPStream s; fillStream(s, _sig); return eth::sha3Bytes(s.out()); }
};
*/

}


