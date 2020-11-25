
# import sys
# sys.path.append('src/ethwallet/eth_wallet' )

#注意设置设置 PythonPath
# https://blog.csdn.net/hx301416/article/details/66477150?utm_source=blogxgwz2


from eth_wallet import Wallet
from eth_wallet.utils import generate_entropy



import json



#!coding:utf8

#author:yqq
#date:2020/4/28 0028 15:16
#description:


MNEMONIC = 'mother citizen apart father resemble coral section pony floor brother fuel lottery'


def main():
    # 128 strength entropy
    ENTROPY = generate_entropy(strength=128)
    # Secret passphrase
    PASSPHRASE = str('123456')  # str("meherett")
    # Choose language english, french, italian, spanish, chinese_simplified, chinese_traditional & korean
    LANGUAGE = "english"  # default is english

    # Initialize wallet
    wallet = Wallet()
    # Get Ethereum wallet from entropy
    # wallet.from_entropy(entropy=ENTROPY, passphrase=PASSPHRASE, language=LANGUAGE)
    # wallet.from_path()
    wallet.from_mnemonic(mnemonic=MNEMONIC)

    # Derivation from path
    wallet.from_path("m/44'/60'/0'/0/0")
    # Or derivation from index
    # wallet.from_index(44, harden=True)
    # wallet.from_index(60, harden=True)
    # wallet.from_index(0, harden=True)
    # wallet.from_index(0)
    # wallet.from_index(0)

    # print( wallet.seed() )

    # Print all wallet information's
    print(json.dumps(wallet.dumps(), indent=4))

    pass


if __name__ == '__main__':

    main()


