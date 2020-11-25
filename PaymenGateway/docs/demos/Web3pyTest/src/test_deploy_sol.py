#!coding:utf8

#author:yqq
#date:2020/7/17 0017 10:17
#description:



import unittest
from binascii import unhexlify, hexlify
from pprint import pprint

import web3
from eth_account.datastructures import SignedTransaction
from eth_typing import URI, HexStr, BlockNumber, Address
from eth_utils import denoms, to_checksum_address
from web3.contract import ContractFunctions

# from src.abi import EIP20_ABI

def  foo1():
    from web3 import Web3, HTTPProvider
    myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:18545')))
    # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction
    # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction#sign-a-transaction

    from web3.auto import w3
    nonce = myweb3.eth.getTransactionCount(Address('0x954d1a58c7abd4ac8ebe05f59191Cf718eb0cB89'),
                                           block_identifier='pending')
    private_key = 'dbbad2a5682517e4ff095f948f721563231282ca4179ae0dfea1c76143ba9607'

    transaction = {
        # 'to': '0xC4d2e23807d176441221248fCbC03170e40B37d1',
        # 'value': Web3.toWei(number=0.01234, unit='ether'),
        'value': 0,
        'gas': 210000,
        'gasPrice': Web3.toWei(20, 'gwei'),
        'nonce': nonce,
        'data': '60806040523480156100115760006000fd5b50610017565b610123806100266000396000f3fe608060405234801560105760006000fd5b5060043610602c5760003560e01c806306fdde0314603257602c565b60006000fd5b603860b1565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101560775780820151818401525b602081019050605d565b50505050905090810190601f16801560a35780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6040518060400160405280600481526020017f74657374000000000000000000000000000000000000000000000000000000008152602001508156fea264697066735822122000e71aeb9ba8a38e0eed738e7166c63ebd92a74d654506a0cf11eba5e067221d64736f6c634300060b0033'
    }
    print(transaction)

    signed_tx = w3.eth.account.sign_transaction(transaction_dict=transaction, private_key=private_key)
    assert isinstance(signed_tx, SignedTransaction)
    print(hexlify(signed_tx.rawTransaction))

    print(hexlify(signed_tx.hash))

    txhash_ret = myweb3.eth.sendRawTransaction(signed_tx.rawTransaction)
    print(txhash_ret)

    pass



    pass



def main():
    foo1()

    pass


if __name__ == '__main__':

    main()