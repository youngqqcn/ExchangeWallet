#!coding:utf8

#author:yqq
#date:2019/12/27 0027 11:15
#description:





import base58
import os
import ecdsa
import re
from binascii import hexlify, unhexlify
# from .utils import sha256, ripemd160, str_to_hex, hex_to_int
# from .signer import Signer
import hashlib
import time
import struct
import array
import os
import random


def check_wif(key):
    if isinstance(key, str):
        try:
            EOSKey(key)
            return True
        except Exception as ex:
            pass
    return False

def sha256(data):
    ''' '''
    return hashlib.sha256(data).hexdigest()

def ripemd160(data):
    ''' '''
    #h = hashlib.new('ripemd160')
    h = hashlib.new('rmd160')
    h.update(data)
    return h.hexdigest()

class EOSKey:
    def __init__(self, private_str=''):
        ''' '''
        prng = self._create_entropy()
        self._sk = ecdsa.SigningKey.generate(curve=ecdsa.SECP256k1, entropy=prng)
        self._vk = self._sk.get_verifying_key()

    def __str__(self):
        return self.to_public()


    def _create_entropy(self):
        ''' '''
        ba = bytearray(os.urandom(32))
        seed = sha256(ba)
        return ecdsa.util.PRNG(seed)

    def _check_encode(self, key_buffer, key_type=None):
        '''    '''
        if isinstance(key_buffer, bytes):
            key_buffer = key_buffer.decode()
        check = key_buffer
        if key_type == 'sha256x2':
            first_sha = sha256(unhexlify(check))
            chksum = sha256(unhexlify(first_sha))[:8]
        else:
            if key_type:
                check += hexlify(bytearray(key_type, 'utf-8')).decode()
            chksum = ripemd160(unhexlify(check))[:8]
        return base58.b58encode(unhexlify(key_buffer + chksum))

    def _check_decode(self, key_string, key_type=None):
        '''    '''
        buffer = hexlify(base58.b58decode(key_string)).decode()
        chksum = buffer[-8:]
        key = buffer[:-8]
        if key_type == 'sha256x2':
            # legacy
            first_sha = sha256(unhexlify(key))
            newChk = sha256(unhexlify(first_sha))[:8]
        else:
            check = key
            if key_type:
                check += hexlify(bytearray(key_type, 'utf-8')).decode()
            newChk = ripemd160(unhexlify(check))[:8]
        # print('newChk: '+newChk)
        if chksum != newChk:
            raise ValueError('checksums do not match: {0} != {1}'.format(chksum, newChk))
        return key

    def _recover_key(self, digest, signature, i):
        ''' Recover the public key from the sig
            http://www.secg.org/sec1-v2.pdf
        '''
        curve = ecdsa.SECP256k1.curve
        G = ecdsa.SECP256k1.generator
        order = ecdsa.SECP256k1.order
        yp = (i % 2)
        r, s = ecdsa.util.sigdecode_string(signature, order)
        x = r + (i // 2) * order
        alpha = ((x * x * x) + (curve.a() * x) + curve.b()) % curve.p()
        beta = ecdsa.numbertheory.square_root_mod_prime(alpha, curve.p())
        y = beta if (beta - yp) % 2 == 0 else curve.p() - beta
        # generate R
        R = ecdsa.ellipticcurve.Point(curve, x, y, order)
        e = ecdsa.util.string_to_number(digest)
        # compute Q
        Q = ecdsa.numbertheory.inverse_mod(r, order) * (s * R + (-e % order) * G)
        # verify message
        if not ecdsa.VerifyingKey.from_public_point(Q, curve=ecdsa.SECP256k1).verify_digest(signature, digest,
                                                                                            sigdecode=ecdsa.util.sigdecode_string):
            return None
        return ecdsa.VerifyingKey.from_public_point(Q, curve=ecdsa.SECP256k1)

    def _recovery_pubkey_param(self, digest, signature):
        ''' Use to derive a number that will allow for the easy recovery
            of the public key from the signature
        '''
        for i in range(0, 4):
            p = self._recover_key(digest, signature, i)
            if (p.to_string() == self._vk.to_string()):
                return i

    def _compress_pubkey(self):
        ''' '''
        order = self._sk.curve.generator.order()
        p = self._vk.pubkey.point
        x_str = ecdsa.util.number_to_string(p.x(), order)
        hex_data = bytearray(chr(2 + (p.y() & 1)), 'utf-8')
        compressed = hexlify(hex_data + x_str).decode()
        return compressed

    def _is_canonical(self, sig):
        print("sig: " + str(sig))
        t1 = (sig[1] & 0x80) == 0
        t2 = not (sig[1] == 0 and ((sig[2] & 0x80) == 0))
        t3 = (sig[33] & 0x80) == 0
        t4 = not (sig[33] == 0 and ((sig[34] & 0x80) == 0))
        return t1 and t2 and t3 and t4

    def to_public(self):
        ''' '''
        cmp = self._compress_pubkey()
        return 'EOS' + self._check_encode(cmp).decode()

    def to_wif(self):
        ''' '''
        pri_key = '80' + hexlify(self._sk.to_string()).decode()
        return self._check_encode(pri_key, 'sha256x2').decode()


def GenAccountName():
    preffix = 'hetbi'
    while True:
        suffix  = ''.join( random.sample('zyxwvutsrqpnmkjhgfedca', 12 - len(preffix)))
        if 'fuck' in suffix: continue
        if 'huobi' in suffix : continue
        if 'ok' in suffix :  continue
        if 'ex' in suffix : continue
        if 'sex' in suffix : continue
        if 'bitch' in suffix: continue
        if 'shit' in suffix : continue
        if 'ass' in suffix : continue
        if 'dead' in suffix: continue
        if 'bb' in suffix: continue
        if 'sb' in suffix: continue
        if 'zb' in suffix: continue
        if 'wc' in suffix: continue
        if 'sz' in suffix: continue
        if 'md' in suffix: continue
        if 'ai' in suffix: continue
        if 'mxc' in suffix: continue


        account_name = preffix + suffix
        return account_name

def GenEosKey():
    key = EOSKey()
    priv_key = key.to_wif()
    pub_key = key.to_public()
    account_name = GenAccountName()
    return  (str(priv_key), str(pub_key), str(account_name))


def GenMultiAddr(nAddrCount = 1, isTestnet = True):
    listRet = []
    for i in range(nAddrCount):
        listRet.append(GenEosKey())
    return listRet

def main():

    # key = EOSKey()
    # priv_key = key.to_wif()
    # pub_key = key.to_public()
    # print('priv key:  {}'.format(priv_key))
    # print('pub key: {}'.format(pub_key))

    retinfo = GenEosKey()
    print(retinfo)


    pass


if __name__ == '__main__':
    main()

