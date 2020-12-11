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
/** @file Transaction.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

/*
author: yqq    
email:youngqqcn@gmail.com  
github: github.com/youngqqcn
date:2019-04-18
descriptions: 
	implement ehtereum transaction offline signature
	实现ethereum交易离线签名
*/


//#include <secp256k1.h>
#include "secp256k1/secp256k1.h"
#include "secp256k1/secp256k1_recovery.h"
#include "secp256k1/secp256k1_ecdh.h"
#include "vector_ref.h"
#include "Exceptions.h"
#include "Transaction.h"
using namespace std;
using namespace eth;

static const u256 c_secp256k1n("115792089237316195423570985008687907852837564279074904382605163141518161494337");

inline secp256k1_context const* getCtx()
{
	static std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> s_ctx{
		secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY),
		&secp256k1_context_destroy
	};
	return s_ctx.get();
}



Transaction::Transaction(bytesConstRef _rlpData)
{
	RLP rlp(_rlpData);
	m_u256Nonce = rlp[pos_nonce].toInt<u256>();
	m_addrTo = rlp[pos_receive_addr].toHash<Address>();
	m_u256Value = rlp[pos_value].toInt<u256>();
	m_u256GasStart = rlp[pos_gaslimit].toInt<u256>();
	m_u256GasPrice = rlp[pos_gasprice].toInt<u256>();
	m_bytesData.reserve(rlp[pos_data].itemCountStrict());
	for (auto const& i: rlp[pos_data])
	{
		//data.push_back(i.toInt<u256>()); 
		m_bytesData.push_back(i.toInt<byte>()); // 2019-04-15 yqq fix bug
	}
	m_vrsSig = VRS_SIG{ rlp[pos_v].toInt<byte>(), rlp[pos_r].toInt<u256>(), rlp[pos_s].toInt<u256>() };
}

eth::Transaction::Transaction(uint uChainId /*= ETHChainID::None*/)
{
	m_uChainId = uChainId;
	m_u256Nonce = 0; 
	m_u256Value = 0;
	m_u256GasPrice = 0;
	m_u256GasStart = 0;
	m_vrsSig = VRS_SIG{ (byte)m_uChainId, 0, 0 };
}




int eth::Transaction::_Sign(h256 rawHash, Secret _priv)
{
	h256 _hash = rawHash;
	std::cout << "--------------" << std::endl;
	std::cout << "sha3:" << _hash << std::endl;
	std::cout << "--------------" << std::endl;
	Secret& _k = _priv;


	auto* ctx = getCtx();

	secp256k1_ecdsa_recoverable_signature rawSig;
	memset(&rawSig.data, 0, 65 );
	if (!secp256k1_ecdsa_sign_recoverable(ctx, &rawSig, _hash.data(), _k.data(), nullptr, nullptr))
	{
		std::cout << "secp256k1_ecdsa_sign_recoverable 失败" << std::endl;
		return ERRCODE::ERR_SECP256K1_ECDSA_SIGN_RECOVERABLE_faild;
	}


	std::string strRawSig1 = Bin2HexStr((unsigned char *)&rawSig, 65);
	std::cout << "rawsig1 hex:" << strRawSig1 << std::endl;


	Signature_yqq s;
	int iRecid = 0;
	memset((unsigned char *)s.data(), 0, 65);
	secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, s.data(), &iRecid, &rawSig);

	SignatureStruct& ss = *reinterpret_cast<SignatureStruct*>(&s);
	ss.v = static_cast<byte>(iRecid);
	if (ss.s > c_secp256k1n / 2)
	{
		ss.v = static_cast<byte>(ss.v ^ 1);
		ss.s = h256(c_secp256k1n - u256(ss.s));
	}
	assert(ss.s <= c_secp256k1n / 2);

	m_vrsSig.r = (u256)(ss.r);
	m_vrsSig.s = (u256)(ss.s);
	if (ETHChainID::None == m_uChainId)
		m_vrsSig.v = 27;
	else
		m_vrsSig.v = (byte)ss.v + 8 + m_vrsSig.v * 2 + 27;


#if ETH_ADDRESS_DEBUG
	std::cout << "-------------------------- 签名信息 -------------------------------" << endl;
	cout << "RawHash: " << rawHash << endl;
	std::cout << "nonce: " << m_u256Nonce << endl;
	std::cout << "私钥: " << _priv << endl;
	std::cout << "r: " << m_vrsSig.r << std::endl;
	std::cout << "s: " << m_vrsSig.s << std::endl;
	std::cout << "v: " << (int)m_vrsSig.v << std::endl;
	std::cout << "-----------------------------------------------------------------" << endl;
#endif


	if (false == ss.isValid())
	{
		std::cout << "签名无效" << std::endl;
		return ERRCODE::ERR_INVALID_SIG;
	}
	std::cout << "签名有效" << std::endl;

	return ERRCODE::NO_ERROR;
}



int eth::Transaction::Sign(Secret privKey)
{
	h256 rawHash;
	if (ETHChainID::None == m_uChainId)
	{
		rawHash = this->SHA3_Keccak256(false);
	}
	else if(ETHChainID::Mainnet == m_uChainId
		|| ETHChainID::Rinkeby == m_uChainId
		|| ETHChainID::Goerli == m_uChainId
		|| ETHChainID::Ropsten == m_uChainId
		|| ETHChainID::Kovan == m_uChainId)
	{
		rawHash = this->SHA3_Keccak256(true);
	}
	else
	{
		std::cout << "错误的chainId" << std::endl;
		return ERRCODE::ERR_ErrChainID;
	}

	this->_Sign(rawHash, privKey);
	return ERRCODE::NO_ERROR;
}

eth::h256 eth::Transaction::SHA3_Keccak256(bool _sig /*= true*/) const
{
	RLPStream s; RLP_Encode(s, _sig);
	return eth::sha3(s.out());
}

eth::bytes eth::Transaction::SHA3_Keccak256_Bytes(bool _sig /*= true*/) const
{
	RLPStream s; RLP_Encode(s, _sig);
	return eth::sha3Bytes(s.out());
}

void Transaction::RLP_Encode(RLPStream& _s, bool _sig) const
{
	_s.appendList(_sig ? 9 : 6);
	_s << m_u256Nonce << m_u256GasPrice << m_u256GasStart << m_addrTo << m_u256Value<< m_bytesData;

	if (_sig)
		_s << m_vrsSig.v << m_vrsSig.r << m_vrsSig.s;
}

bool eth::SignatureStruct::isValid() const noexcept
{
	static const h256 s_max{fromUserHex(  "0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141")};
	static const h256 s_zero;

	return (v <= 1 && r > s_zero && s > s_zero && r < s_max && s < s_max);
}



//Address Transaction::sender() const
//{
////#if 0
////	secp256k1_start();
////
////	h256 sig[2] = { vrs.r, vrs.s };
////	h256 msg = sha3(false);
////
////	byte pubkey[65];
////	int pubkeylen = 65;
////	if (!secp256k1_ecdsa_recover_compact(msg.data(), 32, sig[0].data(), pubkey, &pubkeylen, 0, (int)vrs.v - 27))
////		throw InvalidSignature();
////
////	// TODO: check right160 is correct and shouldn't be left160.
////	auto ret = right160(eth::sha3(bytesConstRef(&(pubkey[1]), 64)));
////
////#if ETH_ADDRESS_DEBUG
////	cout << "---- RECOVER -------------------------------" << endl;
////	cout << "MSG: " << msg << endl;
////	cout << "R S V: " << sig[0] << " " << sig[1] << " " << (int)(vrs.v - 27) << "+27" << endl;
////	cout << "PUB: " << asHex(bytesConstRef(&(pubkey[1]), 64)) << endl;
////	cout << "ADR: " << ret << endl;
////#endif
////	return ret;
////#endif
//	return Address();
//}


//#if 0
//void Transaction::sign(Secret _priv)
//{
//	int v = 0;
//
//	secp256k1_start();
//
//	h256 msg = sha3(false);
//	h256 sig[2];
//	h256 nonce = kFromMessage(msg, _priv);
//
//	if (!secp256k1_ecdsa_sign_compact(msg.data(), 32, sig[0].data(), _priv.data(), nonce.data(), &v))
//		throw InvalidSignature();
//#if ETH_ADDRESS_DEBUG
//	cout << "---- SIGN -------------------------------" << endl;
//	cout << "MSG: " << msg << endl;
//	cout << "SEC: " << _priv << endl;
//	cout << "NON: " << nonce << endl;
//	cout << "R S V: " << sig[0] << " " << sig[1] << " " << v << "+27" << endl;
//#endif
//
//	vrs.v = (byte)(v + 27);
//	vrs.r = (u256)sig[0];
//	vrs.s = (u256)sig[1];
//}
//#endif

//
//#if 0
//void Transaction::sign(Secret _priv)
//{
//
//	h256 _hash = sha3(false);
//	//h256 _hash(fromUserHex("fef2880beadfff65a6e0e8206c19f81e5d8a3b2158f6a3418db2b95985d42000"));
//	std::cout << "--------------" << std::endl;
//	std::cout << "sha3:" << _hash << std::endl;
//	std::cout << "--------------" << std::endl;
//	Secret& _k = _priv;
//
//
//	auto* ctx = getCtx();
//	secp256k1_ecdsa_recoverable_signature rawSig;
//	if (!secp256k1_ecdsa_sign_recoverable(ctx, &rawSig, _hash.data(), _k.data(), nullptr, nullptr))
//	{
//		std::cout << "secp256k1_ecdsa_sign_recoverable 失败";
//		return;
//	}
//
//
//	Signature_yqq s;
//	int v = 0;
//	secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, s.data(), &v, &rawSig);
//
//	SignatureStruct& ss = *reinterpret_cast<SignatureStruct*>(&s);
//	ss.v = static_cast<byte>(v);
//	if (ss.s > c_secp256k1n / 2)
//	{
//		ss.v = static_cast<byte>(ss.v ^ 1);
//		ss.s = h256(c_secp256k1n - u256(ss.s));
//	}
//	assert(ss.s <= c_secp256k1n / 2);
//
//	vrs.r = (u256)(ss.r);
//	vrs.s = (u256)(ss.s);
//	vrs.v = ss.v + 27;
//
//
//#if ETH_ADDRESS_DEBUG
//	cout << "---- SIGN -------------------------------" << endl;
//	//cout << "MSG: " << msg << endl;
//	cout << "SEC: " << _priv << endl;
//	cout << "NON: " << nonce << endl;
//	cout << "R: " << vrs.r << std::endl;
//	cout << "S: " << vrs.s << std::endl;
//	cout << "V: " << (int)vrs.v << std::endl;
//	cout << "---- SIGN -------------------------------" << endl;
//#endif
//
//
//	if (ss.isValid())
//	{
//		std::cout << "签名有效" << std::endl;
//	}
//	else
//	{
//		std::cout << "签名无效" << std::endl;
//	}
//
//	return ;
//
//}
//#endif



//// If the h256 return is an integer, store it in bigendian (i.e. u256 ret; ... return (h256)ret; )
//h256 Transaction::kFromMessage(h256 _msg, h256 _priv)
//{
//	// TODO!
//	/*
//	v = '\x01' * 32
//	k = '\x00' * 32
//	priv = encode_privkey(priv,'bin')
//	msghash = encode(hash_to_int(msghash),256,32)
//	k = hmac.new(k, v+'\x00'+priv+msghash, hashlib.sha256).digest()
//	v = hmac.new(k, v, hashlib.sha256).digest()
//	k = hmac.new(k, v+'\x01'+priv+msghash, hashlib.sha256).digest()
//	v = hmac.new(k, v, hashlib.sha256).digest()
//	return decode(hmac.new(k, v, hashlib.sha256).digest(),256)
//	*/
//	//secp256k1_ecdsa_sign_recoverable();
//	//secp256k1_scalar_clear();
//	return _msg ^ _priv;
//}

