#!encoding:utf8


import random
import ecdsa
import hashlib
import os
import binascii

b58 = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'


def base58encode(n):
    result = ''
    while n > 0:
        result = b58[n % 58] + result
        n /= 58
    return result


def base58CheckEncode(version, payload):
    s = chr(version) + payload
    print( binascii.b2a_hex(s))
    checksum = hashlib.sha256(hashlib.sha256(s).digest()).digest()[0:4]
    print binascii.b2a_hex(checksum)


def privateKeyToPublicKey(s):
    sk = ecdsa.SigningKey.from_string(s.decode('hex'), curve=ecdsa.SECP256k1)
    vk = sk.verifying_key
    base58CheckEncode(0x80, s.decode('hex'))
    return ('\04' + sk.verifying_key.to_string()).encode('hex')


private_key = ''.join(['%x' % random.randrange(16) for x in range(0, 64)])
s = '8db3f7525d54067698e6f45ffd9d101ebdfd5aa7f56c3ff9a8f07a04b9fc3220'
print("pubkey:", privateKeyToPublicKey(s))
base58CheckEncode(0x80, s.decode('hex'))