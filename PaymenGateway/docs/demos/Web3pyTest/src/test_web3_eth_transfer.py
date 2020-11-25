#!coding:utf8

#author:yqq
#date:2020/5/8 0008 21:27
#description: 测试使用web3进行转账


import unittest
from binascii import unhexlify, hexlify
from pprint import pprint

import web3
from eth_account.datastructures import SignedTransaction
from eth_typing import URI, HexStr, BlockNumber, Address
from eth_utils import denoms, to_checksum_address
from web3.contract import ContractFunctions

from src.abi import EIP20_ABI


class TestWeb3Transfer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        pass

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        pass

    def tearDown(self):
        pass


    def test_eth_getTransactions(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider= HTTPProvider( endpoint_uri= URI('http://192.168.10.199:18545')) )
        tx = myweb3.eth.getTransaction(transaction_hash=HexStr('0x482c20818622a146f2c723636f1329f1ddf3c4f640f1c40d602df34fc2f1af7b'))
        pprint(tx , indent=4 )

        tx_receipt = myweb3.eth.getTransactionReceipt(transaction_hash= HexStr('0x482c20818622a146f2c723636f1329f1ddf3c4f640f1c40d602df34fc2f1af7b') )
        tx_receipt_dict = {}
        for key, value in tx_receipt.items():
            tx_receipt_dict[key] = value

        pprint( tx_receipt_dict, indent=4 , depth=5)

        print('--------------------')


        #https://web3py.readthedocs.io/en/stable/middleware.html#geth-style-proof-of-authority
        #如果是测试 Rinkeby(基于POA共识算法)  则需要特殊处理
        from web3.middleware import geth_poa_middleware

        myweb3.middleware_onion.inject(element=geth_poa_middleware, layer=0)
        block =  myweb3.eth.getBlock(block_identifier=BlockNumber(6394437),
                                     full_transactions=True  #是否展开所有交易, 如果是False则只显示txhash , 如果是True则显示和 getTransactionByHash 一样的交易详情
                                     )
        pprint(dict(block), indent=4)


        balance =  myweb3.eth.getBalance(account=Address(unhexlify('C4d2e23807d176441221248fCbC03170e40B37d1')), block_identifier=None )
        ether_balance = myweb3.fromWei(balance, 'ether')  #将wei转为  ether
        print(ether_balance)






        pass

    def test_web3_eth_transaction(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:18545')))
        # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction
        # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction#sign-a-transaction
        from web3.auto import w3
        nonce = myweb3.eth.getTransactionCount(Address('0xdf88522B56B85d4F0Bb08a7494b97E017BC6CB31'), block_identifier='pending')
        private_key = 'D4D4381FF873DDD994BFDEB6B9994E1ED58B3BC1890EBE3FA28B6963BC4B85E7'



        transaction = {
            'to' : '0xC4d2e23807d176441221248fCbC03170e40B37d1',
            'value': Web3.toWei(number=0.01234, unit='ether'),
            'gas' : 21000,
            'gasPrice': Web3.toWei(20, 'gwei') ,
            'nonce' : nonce
        }
        print(transaction)


        signed_tx =  w3.eth.account.sign_transaction(transaction_dict=transaction, private_key=private_key)
        assert isinstance(signed_tx, SignedTransaction)
        print(hexlify( signed_tx.rawTransaction) )

        print(hexlify( signed_tx.hash) )

        txhash_ret =  myweb3.eth.sendRawTransaction(signed_tx.rawTransaction)
        print(txhash_ret)



        pass


    def test_web3_erc20_transaction(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:18545')))


        #https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction#sign-a-contract-transaction
        # from web3.auto import w3
        # myweb3.eth.contract()
        checksum_address = to_checksum_address("0xEca059F3d6De135E520E789CDfEeCBf5CECa3770")

        contract  = myweb3.eth.contract(address=checksum_address, abi=EIP20_ABI)

        nonce = myweb3.eth.getTransactionCount(Address('0x954d1a58c7abd4ac8ebe05f59191Cf718eb0cB89'), block_identifier='pending')

        # assert isinstance(contract.functions, ContractFunctions)
        token_recipient =  '0xC4d2e23807d176441221248fCbC03170e40B37d1'
        contract_tx = contract.functions.transfer(
            token_recipient,

            #注意需要根据 不同代币的 精度来转换,
            myweb3.toWei(123, 'mwei'))\
            .buildTransaction({
            'chainId': 4, # 4: rinkeby SUPPORTED_CHAIN_IDS
            'gas':210000,
            'gasPrice':myweb3.toWei(20,  'gwei'),
            'nonce' : nonce
        })

        print('-----contract_tx----')
        pprint(contract_tx, indent=4)
        print('-----------------')

        private_key = 'DBBAD2A5682517E4FF095F948F721563231282CA4179AE0DFEA1C76143BA9607'


        signed_tx =  myweb3.eth.account.sign_transaction(transaction_dict=contract_tx, private_key=private_key)
        assert isinstance(signed_tx, SignedTransaction)
        print(hexlify( signed_tx.rawTransaction) )

        print(hexlify( signed_tx.hash) )

        txhash_ret =  myweb3.eth.sendRawTransaction(signed_tx.rawTransaction)
        print(txhash_ret)



        reciept = myweb3.eth.waitForTransactionReceipt(
            transaction_hash=txhash_ret,
            timeout=20,  # 20 秒 , 没查到就抛异常 web3.exceptions.TimeExhausted
            poll_latency=0.1  #每隔 0.1 秒 请求一次
        )

        print(reciept)


        #测试成功: https://blockexplorer.one/eth/rinkeby/tx/0x49f0512e1d10fd2ae968ed355b76a6ef2fa5bb4bba5e2a916ecdc87c3ac8a421


        pass


    def test_erc20_query(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:18545')))

        # https://web3py.readthedocs.io/en/stable/web3.eth.account.html?highlight=transaction#sign-a-contract-transaction
        # from web3.auto import w3

        contract = myweb3.eth.contract(address="0xEca059F3d6De135E520E789CDfEeCBf5CECa3770", abi=EIP20_ABI)

        #https://web3py.readthedocs.io/en/stable/examples.html?highlight=erc20#working-with-an-erc20-token-contract
        name = contract.functions.name().call()
        print(name)

        symbol = contract.functions.symbol().call()
        print(symbol)

        decimals = contract.functions.decimals().call()
        print(decimals)

        totalSupply = contract.functions.totalSupply().call()
        print(totalSupply)


        addr = '0x954d1a58c7abd4ac8ebe05f59191Cf718eb0cB89'
        balance = contract.functions.balanceOf(addr).call()

        print(balance)


    def test_waitForTransactionReceipt(self):

        from web3 import Web3, HTTPProvider
        from web3.auto import w3
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:18545')))
        reciept =  myweb3.eth.waitForTransactionReceipt(
                    transaction_hash=HexStr('0x482c20818622a146f2c723636f1329f1ddf3c4f640f1c40d602df34fc2f1af7b'),
                    timeout=20,
                    poll_latency=0.1
                )

        print(reciept)



        pass


    def test_bloom_filter(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:18545')))

        # https://web3py.readthedocs.io/en/stable/middleware.html#geth-style-proof-of-authority
        # 如果是测试 Rinkeby(基于POA共识算法)  则需要特殊处理
        from web3.middleware import geth_poa_middleware

        myweb3.middleware_onion.inject(element=geth_poa_middleware, layer=0)
        block = myweb3.eth.getBlock(block_identifier=BlockNumber(6394437),
                                    full_transactions=True
                                    # 是否展开所有交易, 如果是False则只显示txhash , 如果是True则显示和 getTransactionByHash 一样的交易详情
                                    )
        pprint(dict(block), indent=4)

        # logs_bloom = block.logsBloom
        transfer_event = unhexlify('ddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef')


        from eth_utils.conversions import to_int,  to_hex
        from eth_bloom.bloom import BloomFilter
        bloom_filter = BloomFilter(  to_int(hexstr=to_hex(block.logsBloom)))


        if  transfer_event in bloom_filter:
            print('in')
        else:
            print('not in')

        contract_data = unhexlify('eca059f3d6de135e520e789cdfeecbf5ceca3770')

        if  contract_data in bloom_filter:
            print('in')
        else:
            print('not in')

        pass


    def test_getgasprice(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:8545')))

        # https://web3py.readthedocs.io/en/stable/middleware.html#geth-style-proof-of-authority
        # 如果是测试 Rinkeby(基于POA共识算法)  则需要特殊处理
        # from web3.middleware import geth_poa_middleware
        # myweb3.middleware_onion.inject(element=geth_poa_middleware, layer=0)
        gp  = myweb3.eth.gasPrice
        gw = Web3.toWei(500, 'gwei')
        print(gp)

        pass


    def test_estimate_gas(self):
        from web3 import Web3, HTTPProvider
        myweb3 = Web3(provider=HTTPProvider(endpoint_uri=URI('http://192.168.10.199:8545')))

        g = myweb3.eth.estimateGas({
            "from": to_checksum_address("0xc6306fae81aeba42038c6e1a9bfa3ce936b241e5"),
            "to": to_checksum_address("0xdac17f958d2ee523a2206206994597c13d831ec7"),
            "data": "0xa9059cbb0000000000000000000000003f956c6f0ff3402bee735393dc8ac42c7261e5650000000000000000000000000000000000000000000000000000000000034f2a"
        })
        print(g)
        pass



def main():
    suite = unittest.TestSuite()

    suite.addTests([
        TestWeb3Transfer('test_eth_transfer'),
    ])

    runner = unittest.TextTestRunner()
    runner.run(test=suite)


    pass


if __name__ == '__main__':

    main()