#!coding:utf8

#author:yqq
#date:2020/4/28 0028 16:20
#description: 测试 使用BIP44规范生成 ETH 地址


import pytest
from eth_wallet import Wallet
import ecdsa
from binascii import hexlify, unhexlify
import sha3
import json


def gen_addr_by_privkey(privkey : bytes):
    sk = ecdsa.SigningKey.from_string(unhexlify(privkey), curve=ecdsa.SECP256k1)  # 通过私钥生成密钥对
    pubKey = hexlify(sk.verifying_key.to_string())  # 获取公钥

    keccak = sha3.keccak_256()  # keccak_256哈希运算
    keccak.update(unhexlify(pubKey))
    addr = "0x" + keccak.hexdigest()[24:]  # 截取后面40字符
    return addr


# @pytest.fixture(scope='function')
def test_eth_sub_addr_gen():

    MNEMONIC = 'mother citizen apart father resemble coral section pony floor brother fuel lottery'
    wallet = Wallet()


    test_vector = [
        '0x954d1a58c7abd4ac8ebe05f59191Cf718eb0cB89', #0
        '0xc6a6FdBcab9eA255eDEE2e658E330a62f793B74E', #1
        '0xdf88522B56B85d4F0Bb08a7494b97E017BC6CB31', #2
        '0x99F239694CbF9753B8ad649E34AcF4359cb5caF0', #3
        '0xD2F8f3fBc27745b9fa4f5299c1812f4b95aC9F91', #4
    ]

    for i in range(len(test_vector)):
        wallet.from_mnemonic(mnemonic=MNEMONIC)
        addr_path = f"m/44'/60'/0'/0/{i}"
        wallet.from_path(addr_path)
        # print( wallet.public_key() )
        # print(wallet.private_key())
        # print( json.dumps( wallet.dumps() , indent=4) )
        addr = gen_addr_by_privkey(wallet.private_key())
        assert test_vector[i].lower() == addr.lower()

    pass


def test_eth_sub_account_addr_gen():

    MNEMONIC = 'mother citizen apart father resemble coral section pony floor brother fuel lottery'
    wallet = Wallet()


    test_vector = [
        '0x2bdc01efe26c8a71d920472a6c7c6f2cfffe888a', #0
        '0x59ccc11420dea66bef173aaf5819abfcbf0b91ab', #1
        '0x3b8a2b1bcbdf4d9814902e1992d980d3ad27c07f', #2
        '0xaa6dd8015fadd860ee9a4c9ff036c490ed364f83', #3
        '0x881c5a50b294747a3b04dfa1e9708f67ff5ecbd4', #4
    ]

    for i in range(len(test_vector)):
        wallet.from_mnemonic(mnemonic=MNEMONIC)
        addr_path = f"m/44'/60'/1'/0/{i}"
        wallet.from_path(addr_path)
        # print( wallet.public_key() )
        # print(wallet.private_key())
        # print( json.dumps( wallet.dumps() , indent=4) )
        addr = gen_addr_by_privkey(wallet.private_key())
        print(addr)
        assert test_vector[i].lower() == addr.lower()

    pass


def test_sub_priv():
    # MNEMONIC = 'mother citizen apart father resemble coral section pony floor brother fuel lottery'
    # wallet = Wallet()

    pass


if __name__ =="__main__":
    test_eth_sub_addr_gen()
    test_eth_sub_account_addr_gen()
    # pytest.main()