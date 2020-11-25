#!coding:utf8

#author:yqq
#date:2020/4/28 0028 18:30
#description:

import pytest
from eth_wallet import Wallet
import ecdsa
from binascii import hexlify, unhexlify
import sha3
import json

from eth_wallet.utils import generate_mnemonic


def test_from_seed_1():

    wallet = Wallet()

    print(generate_mnemonic())

    # wallet.from_seed(seed='000102030405060708090a0b0c0d0e0f')
    wallet.from_seed(seed='000102030405060708090a0b0c0d0e09')
    # print(wallet.mnemonic())
    # wallet.from_index(index=1)
    # print( json.dumps( wallet.dumps() , indent=4) )

    # reference:   https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vector-1
    xpub = 'xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8'
    xpriv = 'xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi'

    assert xpriv ==  wallet.extended_key(private_key=True, encoded=True)
    assert xpub == wallet.extended_key(private_key=False, encoded=True)


    # path :    m/0H     其中 H下标  表示  harden(硬化)
    wallet.from_index(index=0, harden=True)
    # print( json.dumps( wallet.dumps() , indent=4) )
    xpub = 'xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw'
    xpriv = 'xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rGL5hj6KCesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7'
    assert xpriv ==  wallet.extended_key(private_key=True, encoded=True)
    assert xpub == wallet.extended_key(private_key=False, encoded=True)




    pass


def test_from_seed_2():

    pass



def main():
    test_from_seed_1()


    pass


if __name__ == '__main__':

    main()