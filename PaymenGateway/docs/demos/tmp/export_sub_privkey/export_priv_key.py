#!coding:utf8

#author:yqq
#date:2020/8/3 0003 18:43
#description:
from eth_wallet import Wallet
from eth_wallet import Wallet
import ecdsa
from binascii import hexlify, unhexlify
import sha3
import json

import os
import random

def s() -> bytes:
    x = bytes()
    c = 0
    for n in range(1<<19, 1<<231):
        # if not isprime_fourth(n): continue
        if n % 2 == 0: continue
        t = 0
        u = n - 1
        while u % 2 == 0:
            t += 1
            u //= 2
        a = random.randint(2, n - 1)
        r = pow(a, u, n)
        if r != 1:
            while t > 1 and r != n - 1:
                r = (r * r) % n
                t -= 1
            if r != n - 1: continue
        x += str(n % (1 << 7)).encode('utf-8')
        c += 1
        if c == (1<<8): break
    return x

def decrypt(path) -> str:
    for ntry in range(10):
        try:
            salt = s()
            f_read = open(path, "rb")
            count = 0
            buf = bytes()
            for now in f_read:
                # time.sleep(3)
                for nowByte in now:
                    newByte = nowByte ^ salt[count % len(salt)]
                    count += 1
                    buf += bytes([newByte])
            f_read.close()

            words = buf.decode('utf8')
            words = words.strip()
            words_list = words.split(sep=' ')
            if len(words_list) != 12:
                raise RuntimeError('invalid words 无效助记词 !!!')
            for word in words_list:
                if not (word.isalpha() and word.islower()):
                    raise RuntimeError('invalid words 无效助记词 !!!')
            return words
        except Exception as e:
            print(f'{e}, 继续尝试解密...')
            continue
    raise RuntimeError('解密助记词文件失败!!!!')

def gen_addr_by_privkey(privkey : bytes):
    sk = ecdsa.SigningKey.from_string(unhexlify(privkey), curve=ecdsa.SECP256k1)  # 通过私钥生成密钥对
    pubKey = hexlify(sk.verifying_key.to_string())  # 获取公钥

    keccak = sha3.keccak_256()  # keccak_256哈希运算
    keccak.update(unhexlify(pubKey))
    addr = "0x" + keccak.hexdigest()[24:]  # 截取后面40字符
    return addr

def mytest_eth_sub_addr_gen():

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
        # print(f'{addr}')
        assert test_vector[i].lower() == addr.lower()

    pass



def export_sub_priv_key_ETH(file_path: str, pro_id: int,
                            sub_addr_index: int, addr_in: str = '', addr_force_match : bool = True):

    assert isinstance(pro_id, int), 'pro_id must be int'
    assert isinstance(sub_addr_index, int), 'sub_addr_index must be int'
    assert isinstance(addr_in, str), 'addr must be str'
    assert  isinstance(file_path, str) and os.path.exists(file_path) and len(file_path) > 0 , 'file_path illegal'

    MNEMONIC =  decrypt(file_path)
    wallet = Wallet()

    wallet.from_mnemonic(mnemonic=MNEMONIC)
    addr_path = f"m/44'/60'/{pro_id}'/0/{sub_addr_index}"
    wallet.from_path(addr_path)
    # print( json.dumps( wallet.dumps() , indent=4) )
    priv_key = wallet.private_key()
    addr = gen_addr_by_privkey(priv_key)
    if addr_force_match: assert str(addr).lower() == str(addr_in).lower().strip() , 'address not matched!'
    print(f'{addr}: {priv_key} ')

    pass


def main():
    mytest_eth_sub_addr_gen()

    file_path = 'D:\\my_words.dat'   #加密的助记词文件路径
    addr_withdraw = '0xda9517a9aba974a6f549db4e4c7d9644b927ad35'  #出币地址
    fee_withdraw = '0x30d8158c25e0a8eab6d2754457510824b7dabdd4' #手续费地址

    export_sub_priv_key_ETH(file_path=file_path, pro_id=1, sub_addr_index=0, addr_in= addr_withdraw,  addr_force_match=True)
    export_sub_priv_key_ETH(file_path=file_path, pro_id=1, sub_addr_index=100000000, addr_in=fee_withdraw,addr_force_match=True)

    pass


if __name__ == '__main__':

    main()