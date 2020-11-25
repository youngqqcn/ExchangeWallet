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


def base256decode(s):
    result = 0
    for c in s:
        result = result * 256 + ord(c)
    return result


def countLeadingChars(s, ch):
    count = 0
    for c in s:
        if c == ch:
            count += 1
        else:
            break
    return count


def base58CheckEncode(version, payload):
    s = chr(version) + payload
    checksum = hashlib.sha256(hashlib.sha256(s).digest()).digest()[0:4]
    result = s + checksum
    leadingZeros = countLeadingChars(result, '\0')
    return '1' * leadingZeros + base58encode(base256decode(result))


def privateKeyToPublicKey(s):
    sk = ecdsa.SigningKey.from_string(s.decode('hex') , curve=ecdsa.SECP256k1)
    vk = sk.verifying_key
    return ('\04' + sk.verifying_key.to_string()).encode('hex')


# private_key = os.urandom(32).encode('hex')
private_key = '9d5b32d935db0241f010410e481181646eb4249073fab726a0c40de3dbbfbbe1'
# s = '8db3f7525d54067698e6f45ffd9d101ebdfd5aa7f56c3ff9a8f07a04b9fc3220'
# print base58CheckEncode(0x80, s.decode('hex'))
print( base58CheckEncode(0x80, private_key.decode('hex') + '\01')  )
print( base58CheckEncode(0xEF, private_key.decode('hex') + '\01')  )
