#!coding:utf8

#author:yqq
#date:2019/11/12 0004 14:35
#description:  TRON地址生成算法
#参考: https://github.com/iexbase/tron-api-python/blob/master/tronapi/common/account.py

import hashlib
import ecdsa
import os
import sha3


#2019-11-12 根据官方定义修改  有限域
# http://www.secg.org/sec2-v2.pdf#page=9&zoom=100,0,249
# 关于 有限域的定义 请参考
# 0xEFFFFFC2F = 2**32 - 2**9 - 2**8 - 2**7 - 2**6 - 2**4 - 1
g_nFactor = 0xEFFFFFC2F + 0x23492397 #增值自定义
g_nMaxPrivKey = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364140 - g_nFactor #私钥最大值 (差值是自定义的)
g_nMinPrivKey = 0x0000000000000000000000000000000000000000000000000000000000000001 + g_nFactor #私钥最小值 (增值是自定义的)


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



g_b58 = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'


def Base58encode(n):
    '''
    base58编码
    :param n: 需要编码的数
    :return: 编码后的
    '''
    result = ''
    while n > 0:
        result = g_b58[n % 58] + result
        n /= 58
    return result


def Base256decode(s):
    '''
    base256编码
    :param s:
    :return:
    '''
    result = 0
    for c in s:
        result = result * 256 + ord(c)
    return result


def CountLeadingChars(s, ch):
    '''
    计算一个字符串开头的字符的次数
    比如:  CountLeadingChars('000001234', '0')  结果是5
    :param s:字符串
    :param ch:字符
    :return:次数
    '''
    count = 0
    for c in s:
        if c == ch:
            count += 1
        else:
            break
    return count


def Base58CheckEncode(version, payload):
    '''

    :param version: 版本前缀  , 用于区分主网 和 测试网络
    :param payload:
    :return:
    '''
    s = chr(version) + payload
    checksum = hashlib.sha256(hashlib.sha256(s).digest()).digest()[0:4]  #两次sha256, 区前4字节作为校验和
    result = s + checksum
    leadingZeros = CountLeadingChars(result, '\0')
    return '1' * leadingZeros + Base58encode(Base256decode(result))


def PrivKeyToPubKey(privKey):
    '''
    私钥-->公钥
    :param privKey: 共65个字节:  0x04   +  x的坐标  +   y的坐标
    :return:
    '''
    sk = ecdsa.SigningKey.from_string(privKey.decode('hex'), curve=ecdsa.SECP256k1)
    # vk = sk.verifying_key
    return ('\04' + sk.verifying_key.to_string()).encode('hex')


def PubKeyToAddr(pubKey, isTestnet = False):
    '''
    公钥 --> 地址
    :param pubKey: 公钥
    :param isTestnet:  是否是地址
    :return:  地址字符串
    '''

    addr1 = sha3.keccak_256(pubKey.decode('hex')[1:]).digest()

    addr2 = addr1[11 + 1: ]


    #不区分主网测试网
    # if isTestnet:
    #     return Base58CheckEncode(0xA0, addr2)  #test
    return Base58CheckEncode(0x41, addr2)  #main


def GenAddr(isTestnet=False):
    '''
    此函数用于C++调用,
    :param isTestnet: 是否是测试网络
    :return:  (私钥, 公钥, 地址)
    '''
    privKey = GenPrivKey()
    # print("privkey : " + privKey)
    # print("privkey WIF:" + PrivKeyToWIF(privKey, isTestnet))
    pubKey = PrivKeyToPubKey(privKey)
    # print("pubkey : " + pubKey)
    addr = PubKeyToAddr(pubKey, isTestnet)
    # print("addr : " + addr)
    return str(privKey), str(pubKey), str(addr)



def GenMultiAddr(nAddrCount = 1, isTestnet=True):
    '''
    生成多个地址
    :param nAddrCount:
    :param isTestnet:
    :return:
    '''
    # return [("1111", "2222", "3333"), ("4444", "55555", "66666")]
    # return [1, 2, 3, 4]
    # return ["1111", "2222", "3333", "4444"]

    lstRet = []
    for i in range(nAddrCount):
        lstRet.append(GenAddr(isTestnet))
    return lstRet




def TronAddrGen2(isTestnet=False):
    # 测试 sha3.keccak256
    # print('sha256(\'hello\'): {}'.format( sha3.keccak_256(bytes('hello')).digest().encode('hex')))

    prikey = 'f43ebcc94e6c257edbe559183d1a8778b2d5a08040902c0f0a77a3343a1d0ea5'
    pubkey = PrivKeyToPubKey(prikey)
    print(pubkey)
    addr =  PubKeyToAddr(pubkey, isTestnet)
    return  addr

#
#
# def main():
#     # TronAddrGen(False)
#
#     str_addr_mainnet =  TronAddrGen2(False)
#     # str_addr_testnet = TronAddrGen2(True)
#
#     print(str_addr_mainnet)
#     # print(str_addr_testnet)
#
#     assert  str(str_addr_mainnet) == 'TWVRXXN5tsggjUCDmqbJ4KxPdJKQiynaG6'
#     # assert  str(str_addr_testnet) == '27jbj4qgTM1hvReST6hEa8Ep8RDo2AM8TJo'
#
#     # good()
#     # for i in range(1):
#     #     print(GenAddr(True))
#
#
# def main2():
#
#     # addrs =  GenMultiAddr(10, False)
#     # print(addrs)
#
#     pass
#
#
# if __name__ == '__main__':
#
#     # main()
#     main2()

