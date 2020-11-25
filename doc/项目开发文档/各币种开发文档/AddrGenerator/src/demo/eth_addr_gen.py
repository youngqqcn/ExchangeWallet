#!coding:utf8

#author:yqq
#date:2019/3/6 0006 17:00
#description:  以太坊私钥,公钥,地址生成



import ecdsa
import os
import sha3
import hashlib


def GenEthAddr():

    privKey = os.urandom(32).encode('hex')
    sk = ecdsa.SigningKey.from_string(privKey.decode('hex'), curve=ecdsa.SECP256k1) #通过私钥生成密钥对
    pubKey = (sk.verifying_key.to_string()).encode('hex')   #获取公钥

    # keccak = sha3.keccak_256()   # keccak_256哈希运算
    # keccak.update(pubKey)
    # addr = "0x" + keccak.hexdigest()[24:]  #截取后面40字符
    rawHash = hashlib.sha256(sk.verifying_key.to_string()).hexdigest()
    print("raw:", rawHash)

    addr = rawHash[24:]

    print(privKey)
    print(pubKey)
    print(addr)

    return (str(privKey), str(pubKey), str(addr))


def GenMultiAddr(nAddrCount = 1, isTestnet = True):
    listRet = []
    for i in range(nAddrCount):
        listRet.append(GenEthAddr())
    return listRet

def main():
    GenMultiAddr(1)

if __name__ == '__main__':

    main()
