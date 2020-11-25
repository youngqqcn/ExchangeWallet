#!encoding:utf8

import random
import ecdsa
import hashlib
import os

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
    sk = ecdsa.SigningKey.from_string(s.decode('hex'), curve=ecdsa.SECP256k1)
    vk = sk.verifying_key
    # return ('\04' + sk.verifying_key.to_string()).encode('hex')
    return ('\04' + sk.verifying_key.to_string()).encode('hex')


#https://en.bitcoin.it/wiki/List_of_address_prefixes
#    00 : p2pkh
#    80 : WIF


def pubKeyToAddr(s):
    ripemd160 = hashlib.new('ripemd160')
    ripemd160.update(hashlib.sha256(s.decode('hex')).digest())
    # return base58CheckEncode(0, ripemd160.digest())  #00  p2pkh
    return base58CheckEncode(0x6f, ripemd160.digest())  #00  p2pkh
    # return base58CheckEncode(0x6f, ripemd160.digest())  #00  p2pkh


def KeyToAddr(s):
    return pubKeyToAddr(privateKeyToPublicKey(s))



def good():
    private_key = os.urandom(32).encode('hex')
    print("privkey : " + private_key)
    # a = '18E14A7B6A307F426A94F8114701E7C8E774E7F9A47E2C2035DB29A206321725'
    pubKey = privateKeyToPublicKey(private_key)
    print("pubkey : " + pubKey)
    addr = pubKeyToAddr( pubKey )
    print("addr : " + addr)
    # s = '0450863AD64A87AE8A2FE83C1AF1A8403CB53F53E486D8511DAD8A04887E5B23522CD470243453A299FA9E77237716103ABC11A1DF38855ED6F2EE187E9C582BA6'
    # print pubKeyToAddr(s)



def main():
    good()

if __name__ == '__main__':

    main()


