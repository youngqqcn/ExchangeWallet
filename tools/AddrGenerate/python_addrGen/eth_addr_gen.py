#!coding:utf8

#author:yqq
#date:2019/3/6 0006 17:00
#description:  以太坊私钥,公钥,地址生成



import ecdsa
import os
import sha3


#2019-05-15  添加私钥限制范围
g_nMaxPrivKey = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364140 - 0x423423843  #私钥最大值 (差值是自定义的)
g_nMinPrivKey = 0x0000000000000000000000000000000000000000000000000000000000000001 + 0x324389329  #私钥最小值 (增值是自定义的)


def GenPrivKey():
    '''
    生成私钥, 使用 os.urandom (底层使用了操作系统的随机函数接口, 取决于CPU的性能,各种的硬件的数据指标)
    :return:私钥(16进制编码)
    '''

    #2019-05-15 添加私钥范围限制
    while True:
        privKey = os.urandom(32).encode('hex')    #生成 256位 私钥
        if  g_nMinPrivKey < int(privKey, 16) <   g_nMaxPrivKey:
            return privKey

def GenEthAddr():

    privKey = GenPrivKey() #os.urandom(32).encode('hex')

    # sk = ecdsa.SigningKey.from_string(privKey.decode('hex'), curve=ecdsa.SECP256k1)
    sk = ecdsa.SigningKey.from_string(privKey.decode('hex'), curve=ecdsa.SECP256k1) #通过私钥生成密钥对
    pubKey = (sk.verifying_key.to_string()).encode('hex')   #获取公钥

    keccak = sha3.keccak_256()   # keccak_256哈希运算
    keccak.update(pubKey.decode('hex'))
    addr = "0x" + keccak.hexdigest()[24:]  #截取后面40字符

    # print(privKey)
    # print(pubKey)
    # print(addr)

    return (str(privKey), str(pubKey), str(addr))


def GenMultiAddr(nAddrCount = 1, isTestnet = True):
    listRet = []
    for i in range(nAddrCount):
        listRet.append(GenEthAddr())
    return listRet

# def main():
#     print(GenMultiAddr(1))
#
# if __name__ == '__main__':
#
#     main()
