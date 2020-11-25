#!coding:utf8

#author:yqq
#date:2020/8/6 0006 18:06
#description:


import os

from eth_typing import URI, Address

import web3
from eth_account.datastructures import SignedTransaction
from eth_typing import URI, HexStr, BlockNumber, Address
from binascii import unhexlify, hexlify

from eth_utils import to_checksum_address


def main():

    key = ''
    with open('D:\\tmppp', 'r') as infile:
        key = infile.readline().strip()

    from web3 import Web3, HTTPProvider
    myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:8545')))
    # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction
    # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction#sign-a-transaction
    from web3.auto import w3
    nonce = myweb3.eth.getTransactionCount(Address( to_checksum_address('这里填源地址')))
    private_key = key

    transaction = {
        'to': to_checksum_address( '这里填目的地址'),
        'value': Web3.toWei(number=0.00971, unit='ether'),
        'gas': 21000,
        'gasPrice': Web3.toWei(42, 'gwei'),
        'nonce': nonce
    }
    print(transaction)

    signed_tx = w3.eth.account.sign_transaction(transaction_dict=transaction, private_key=private_key)
    assert isinstance(signed_tx, SignedTransaction)
    print(hexlify(signed_tx.rawTransaction))

    print(hexlify(signed_tx.hash))

    txhash_ret = myweb3.eth.sendRawTransaction(signed_tx.rawTransaction)
    print(txhash_ret)




    pass


if __name__ == '__main__':

    main()