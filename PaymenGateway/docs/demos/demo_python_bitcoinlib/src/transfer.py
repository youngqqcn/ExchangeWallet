#!coding:utf8

#author:yqq
#date:2020/7/8 0008 15:05
#description:

# Copyright (C) 2014 The python-bitcoinlib developers
#
# This file is part of python-bitcoinlib.
#
# It is subject to the license terms in the LICENSE file found in the top-level
# directory of this distribution.
#
# No part of python-bitcoinlib, including this file, may be copied, modified,
# propagated, or distributed except according to the terms contained in the
# LICENSE file.

"""Low-level example of how to spend a standard pay-to-pubkey-hash (P2PKH) txout"""

import sys
if sys.version_info.major < 3:
    sys.stderr.write('Sorry, Python 3.x required by this example.\n')
    sys.exit(1)

import hashlib

from bitcoin import SelectParams
from bitcoin.core import b2x, lx, COIN, COutPoint, CMutableTxOut, CMutableTxIn, CMutableTransaction, Hash160
from bitcoin.core.script import CScript, OP_DUP, OP_HASH160, OP_EQUALVERIFY, OP_CHECKSIG, SignatureHash, SIGHASH_ALL
from bitcoin.core.scripteval import VerifyScript, SCRIPT_VERIFY_P2SH
from bitcoin.wallet import CBitcoinAddress, CBitcoinSecret, CBech32BitcoinAddress, P2WSHBitcoinAddress

# SelectParams('mainnet')
SelectParams('regtest')


def transfer():
    """
    测试转账
    :return:
    """

    # Create the (in)famous correct brainwallet secret key.
    # h = hashlib.sha256(b'correct horse battery staple').digest()
    # seckey = CBitcoinSecret.from_secret_bytes(h)
    seckey = CBitcoinSecret(s = 'cVNHD7FCKEpm3yafwNjusAjz1oqm9e2nHpQJzhmHyCMZLbckCNbg')

    # Same as the txid:vout the createrawtransaction RPC call requires
    #
    # lx() takes *little-endian* hex and converts it to bytes; in Bitcoin
    # transaction hashes are shown little-endian rather than the usual big-endian.
    # There's also a corresponding x() convenience function that takes big-endian
    # hex and converts it to bytes.
    txid = lx('1f4525339145655a8c8f27f2cf85101e4aa4cbc679ea848cbaf0fe7c48194875')
    vout = 0

    # Create the txin structure, which includes the outpoint. The scriptSig
    # defaults to being empty.
    txin = CMutableTxIn(COutPoint(txid, vout))

    # We also need the scriptPubKey of the output we're spending because
    # SignatureHash() replaces the transaction scriptSig's with it.
    #
    # Here we'll create that scriptPubKey from scratch using the pubkey that
    # corresponds to the secret key we generated above.
    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    # Create the txout. This time we create the scriptPubKey from a Bitcoin
    # address.
    txout = CMutableTxOut(12.3456 * COIN, CBitcoinAddress('moAt6v6gpfJhSBYSmS2AzanW9565kakujW').to_scriptPubKey())
    txout_2 = CMutableTxOut(37.6543 * COIN, CBitcoinAddress('mjGRnCSyan333FdQVKonTFTmNqESaHUJmt').to_scriptPubKey())

    # Create the unsigned transaction.
    tx = CMutableTransaction([txin], [txout, txout_2])

    # Calculate the signature hash for that transaction.
    sighash = SignatureHash(txin_scriptPubKey, tx, 0, SIGHASH_ALL)

    # Now sign it. We have to append the type of signature we want to the end, in
    # this case the usual SIGHASH_ALL.
    sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])

    # Set the scriptSig of our transaction input appropriately.
    txin.scriptSig = CScript([sig, seckey.pub])

    # Verify the signature worked. This calls EvalScript() and actually executes
    # the opcodes in the scripts to see if everything worked out. If it doesn't an
    # exception will be raised.
    VerifyScript(txin.scriptSig, txin_scriptPubKey, tx, 0, (SCRIPT_VERIFY_P2SH,))

    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))


def transfer_multi_input():
    """
    测试转账
    :return:
    """

    # Create the (in)famous correct brainwallet secret key.
    # h = hashlib.sha256(b'correct horse battery staple').digest()
    # seckey = CBitcoinSecret.from_secret_bytes(h)
    seckey = CBitcoinSecret(s = 'cVNHD7FCKEpm3yafwNjusAjz1oqm9e2nHpQJzhmHyCMZLbckCNbg')

    # Same as the txid:vout the createrawtransaction RPC call requires
    #
    # lx() takes *little-endian* hex and converts it to bytes; in Bitcoin
    # transaction hashes are shown little-endian rather than the usual big-endian.
    # There's also a corresponding x() convenience function that takes big-endian
    # hex and converts it to bytes.
    txid = lx('27c2b68ad77fbcbe8a0ff4f3d9a96adc5aff9a922a6a22b0e5f8ac81ed2e102d')
    vout = 0

    txid_2 = lx('f454948e3f717db1330a40751e69d0cf920577f2ef12e3077a823ce8810cf4c8')
    vout_2 = 0

    # Create the txin structure, which includes the outpoint. The scriptSig
    # defaults to being empty.
    txin = CMutableTxIn(COutPoint(txid, vout))
    txin_2 = CMutableTxIn(COutPoint(txid_2, vout_2))

    # We also need the scriptPubKey of the output we're spending because
    # SignatureHash() replaces the transaction scriptSig's with it.
    #
    # Here we'll create that scriptPubKey from scratch using the pubkey that
    # corresponds to the secret key we generated above.
    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    # Create the txout. This time we create the scriptPubKey from a Bitcoin
    # address.
    txout = CMutableTxOut(90.9999 * COIN, CBitcoinAddress('moAt6v6gpfJhSBYSmS2AzanW9565kakujW').to_scriptPubKey())
    txout_2 = CMutableTxOut(9 * COIN, CBitcoinAddress('mjGRnCSyan333FdQVKonTFTmNqESaHUJmt').to_scriptPubKey())

    # Create the unsigned transaction.
    tx = CMutableTransaction([txin, txin_2], [txout, txout_2])

    # Calculate the signature hash for that transaction.
    sighash = SignatureHash(txin_scriptPubKey, tx, 0, SIGHASH_ALL)
    # sighash = SignatureHash(txin_scriptPubKey, tx, 1, SIGHASH_ALL)

    # Now sign it. We have to append the type of signature we want to the end, in
    # this case the usual SIGHASH_ALL.
    sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])

    # Set the scriptSig of our transaction input appropriately.
    txin.scriptSig = CScript([sig, seckey.pub])

    # Verify the signature worked. This calls EvalScript() and actually executes
    # the opcodes in the scripts to see if everything worked out. If it doesn't an
    # exception will be raised.
    VerifyScript(txin.scriptSig, txin_scriptPubKey, tx, 0, (SCRIPT_VERIFY_P2SH,))



    ###############
    # Calculate the signature hash for that transaction.
    # sighash = SignatureHash(txin_scriptPubKey, tx, 0, SIGHASH_ALL)
    sighash = SignatureHash(txin_scriptPubKey, tx, 1, SIGHASH_ALL)

    # Now sign it. We have to append the type of signature we want to the end, in
    # this case the usual SIGHASH_ALL.
    sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])

    # Set the scriptSig of our transaction input appropriately.
    txin_2.scriptSig = CScript([sig, seckey.pub])

    # Verify the signature worked. This calls EvalScript() and actually executes
    # the opcodes in the scripts to see if everything worked out. If it doesn't an
    # exception will be raised.
    VerifyScript(txin_2.scriptSig, txin_scriptPubKey, tx, 1, (SCRIPT_VERIFY_P2SH,))
    ##############



    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))


def transfer_multi_input_v2():

    seckey = CBitcoinSecret(s='cVNHD7FCKEpm3yafwNjusAjz1oqm9e2nHpQJzhmHyCMZLbckCNbg')


    txin = CMutableTxIn(COutPoint(
        lx('238ec3a7a016e0a1c6205004e3beaad95028b2507e4cf1080acbab03d1101920'),
        n=0
    ))

    txin_2 = CMutableTxIn(COutPoint(
        hash=lx('d54912188a0f05a55566a0ead3d3de62c5e81366a315c58ea29271cf0d1471c0'),
        n=0
    ))

    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    txout = CMutableTxOut(90.9999 * COIN, CBitcoinAddress('moAt6v6gpfJhSBYSmS2AzanW9565kakujW').to_scriptPubKey())
    txout_2 = CMutableTxOut(9 * COIN, CBitcoinAddress('mjGRnCSyan333FdQVKonTFTmNqESaHUJmt').to_scriptPubKey())

    # Create the unsigned transaction.
    tx = CMutableTransaction([txin, txin_2], [txout, txout_2])

    # sign transaction input
    for n in range(len(tx.vin)):
        sighash = SignatureHash(txin_scriptPubKey, tx, n, SIGHASH_ALL)
        sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])
        tx.vin[n].scriptSig = CScript([sig, seckey.pub])
        VerifyScript(tx.vin[n].scriptSig , txin_scriptPubKey, tx, n, (SCRIPT_VERIFY_P2SH,))

    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))
    pass



def transfer_multi_input_v3():


    seckey = CBitcoinSecret(s='cVNHD7FCKEpm3yafwNjusAjz1oqm9e2nHpQJzhmHyCMZLbckCNbg')


    txin = CMutableTxIn(COutPoint(
        lx('6202c48fa2d1c38aead24c6a25fdee0124c1a05932ea88f463715552fb1b2123'),
        n=0
    ))

    txin_2 = CMutableTxIn(COutPoint(
        hash=lx('016f02ed443c77bdf1dda19d07233749f47901ac812fd1d9c16b526c9d912159'),
        n=0
    ))

    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    txout = CMutableTxOut(90.9999 * COIN, CBitcoinAddress('moAt6v6gpfJhSBYSmS2AzanW9565kakujW').to_scriptPubKey())
    txout_2 = CMutableTxOut(9 * COIN, CBitcoinAddress('2N11UaUuvA8dUVTPhCkUqP7yVtVsPQXv6Q1').to_scriptPubKey())  #P2SH地址

    # Create the unsigned transaction.
    tx = CMutableTransaction([txin, txin_2], [txout, txout_2])

    # sign transaction input
    for n in range(len(tx.vin)):
        sighash = SignatureHash(txin_scriptPubKey, tx, n, SIGHASH_ALL)
        sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])
        tx.vin[n].scriptSig = CScript([sig, seckey.pub])
        VerifyScript(tx.vin[n].scriptSig , txin_scriptPubKey, tx, n, (SCRIPT_VERIFY_P2SH,))

    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))
    pass


def transfer_multi_input_v4():

    seckey = CBitcoinSecret(s='cVNHD7FCKEpm3yafwNjusAjz1oqm9e2nHpQJzhmHyCMZLbckCNbg')


    txin = CMutableTxIn(COutPoint(
        lx('1160acc3511f732c89401d98f316dc61b106ef5b9d18b2639a900a3b6a80f568'),
        n=0
    ))

    # txin_2 = CMutableTxIn(COutPoint(
    #     hash=lx('26318cf92585595e8807e7632542b6f997fa172c89a09d23f2dd65251d1866fa'),
    #     n=0
    # # ))

    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    # bech32_addr = CBech32BitcoinAddress.from_scriptPubKey( CBitcoinAddress('tb1qsc6euxdw3r0a2a0ex07er7zn3yd8qp6h7uhjwc').to_scriptPubKey() )
    # SelectParams('testnet')
    # script_pub_key =  CBitcoinAddress('tb1qsc6euxdw3r0a2a0ex07er7zn3yd8qp6h7uhjwc').to_scriptPubKey()
    # # print(bech32_addr)
    # SelectParams('regtest')
    # bech32_addr = CBech32BitcoinAddress.from_scriptPubKey( script_pub_key )
    # print(bech32_addr)


    txout = CMutableTxOut(49.9999 * COIN, CBitcoinAddress('bcrt1qsc6euxdw3r0a2a0ex07er7zn3yd8qp6hu4wle3').to_scriptPubKey()) #隔离见证地址 P2WPKH


    # Create the unsigned transaction.
    tx = CMutableTransaction([txin], [txout])

    # sign transaction input
    for n in range(len(tx.vin)):
        sighash = SignatureHash(txin_scriptPubKey, tx, n, SIGHASH_ALL)
        sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])
        tx.vin[n].scriptSig = CScript([sig, seckey.pub])
        VerifyScript(tx.vin[n].scriptSig , txin_scriptPubKey, tx, n, (SCRIPT_VERIFY_P2SH,))

    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))
    pass



def transfer_multi_input_v5():

    seckey = CBitcoinSecret(s='cVNHD7FCKEpm3yafwNjusAjz1oqm9e2nHpQJzhmHyCMZLbckCNbg')


    txin = CMutableTxIn(COutPoint(
        lx('c37e6092fb7a5a53c17bb1a20b3e2b00d87401b81c41a903195138646dd9f20d'),
        n=0
    ))

    # txin_2 = CMutableTxIn(COutPoint(
    #     hash=lx('26318cf92585595e8807e7632542b6f997fa172c89a09d23f2dd65251d1866fa'),
    #     n=0
    # # ))

    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    # bech32_addr = CBech32BitcoinAddress.from_scriptPubKey( CBitcoinAddress('tb1qsc6euxdw3r0a2a0ex07er7zn3yd8qp6h7uhjwc').to_scriptPubKey() )
    # SelectParams('testnet')
    # script_pub_key =  CBitcoinAddress('tb1qsc6euxdw3r0a2a0ex07er7zn3yd8qp6h7uhjwc').to_scriptPubKey()
    # # print(bech32_addr)
    # SelectParams('regtest')
    # bech32_addr = CBech32BitcoinAddress.from_scriptPubKey( script_pub_key )
    # print(bech32_addr)

    # from bitcoin.core import b2x , x
    # SelectParams('mainnet')
    # sp  =CBitcoinAddress('bc1qc7slrfxkknqcq2jevvvkdgvrt8080852dfjewde450xdlk4ugp7szw5tk9').to_scriptPubKey()
    # SelectParams('regtest')
    # addr = P2WSHBitcoinAddress.from_scriptPubKey(sp)
    # print(addr)

    txout = CMutableTxOut(49.9999 * COIN, CBitcoinAddress('bcrt1qc7slrfxkknqcq2jevvvkdgvrt8080852dfjewde450xdlk4ugp7sclgzes').to_scriptPubKey()) #隔离见证地址


    # Create the unsigned transaction.
    tx = CMutableTransaction([txin], [txout])

    # sign transaction input
    for n in range(len(tx.vin)):
        sighash = SignatureHash(txin_scriptPubKey, tx, n, SIGHASH_ALL)
        sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])
        tx.vin[n].scriptSig = CScript([sig, seckey.pub])
        VerifyScript(tx.vin[n].scriptSig , txin_scriptPubKey, tx, n, (SCRIPT_VERIFY_P2SH,))

    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))
    pass



def transfer_multi_input_v6():
    # BTC	moAt6v6gpfJhSBYSmS2AzanW9565kakujW	cSGCNnp3LxnRHaQnjrs3mRRX8wrSdeck5oDz51MhTyMx1mikrQKd
    seckey = CBitcoinSecret(s='cSGCNnp3LxnRHaQnjrs3mRRX8wrSdeck5oDz51MhTyMx1mikrQKd')

    txin = CMutableTxIn(COutPoint(
        lx('5dc7ac943ece26e86eac28f5aa52cd7e9106b2a861336f0291e2f95fd8921d93'),
        n=0
    ))

    txin_scriptPubKey = CScript([OP_DUP, OP_HASH160, Hash160(seckey.pub), OP_EQUALVERIFY, OP_CHECKSIG])

    txout1 = CMutableTxOut(99999000, CBitcoinAddress('bcrt1qsc6euxdw3r0a2a0ex07er7zn3yd8qp6hu4wle3').to_scriptPubKey()) #隔离见证地址 P2WPKH
    # txout2 = CMutableTxOut(999990000, CBitcoinAddress('moAt6v6gpfJhSBYSmS2AzanW9565kakujW').to_scriptPubKey()) #隔离见证地址 P2WPKH

    # Create the unsigned transaction.
    tx = CMutableTransaction([txin], [txout1])

    # sign transaction input
    for n in range(len(tx.vin)):
        sighash = SignatureHash(txin_scriptPubKey, tx, n, SIGHASH_ALL)
        sig = seckey.sign(sighash) + bytes([SIGHASH_ALL])
        tx.vin[n].scriptSig = CScript([sig, seckey.pub])
        VerifyScript(tx.vin[n].scriptSig , txin_scriptPubKey, tx, n, (SCRIPT_VERIFY_P2SH,))

    # Done! Print the transaction to standard output with the bytes-to-hex
    # function.
    print(b2x(tx.serialize()))
    pass

def main():

    # transfer()
    # transfer_multi_input()
    # transfer_multi_input_v2()
    # transfer_multi_input_v3()
    # transfer_multi_input_v4()
    # transfer_multi_input_v5()
    transfer_multi_input_v6()


    pass


if __name__ == '__main__':

    main()