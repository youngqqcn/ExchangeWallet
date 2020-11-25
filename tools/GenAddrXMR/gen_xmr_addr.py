#!coding:utf8

#author:yqq
#date:2020/3/31 0031 10:32
#description:



from monero.address import Address, address, IntegratedAddress, SubAddress
from monero.seed import Seed
from monero.wordlists import list_wordlists
from monero.seed import const
from monero.wallet import *
from monero.account import *
import datetime




def get_address_ex(master_addr, major, minor, private_view_key):
    """
    Calculates sub-address for account index (`major`) and address index within
    the account (`minor`).

    :rtype: :class:`BaseAddress <monero.address.BaseAddress>`
    """
    # ensure indexes are within uint32
    if major < 0 or major >= 2 ** 32:
        raise ValueError('major index {} is outside uint32 range'.format(major))
    if minor < 0 or minor >= 2 ** 32:
        raise ValueError('minor index {} is outside uint32 range'.format(minor))
    master_address = master_addr  #self.address()
    if major == minor == 0:  #如果是  (0, 0) 则直接返回
        return master_address

    # spk = Address(master_addr).spend_key() #根据master address 直接获取 public_key
    #  = Address(master_addr).view_key()

    priv_view_key =  private_view_key  #seed.secret_view_key()
    # print(f'private_view_key: {priv_view_key}')
    master_svk = unhexlify(priv_view_key)

    pub_spend_key =  Address(master_addr).spend_key()
    master_psk = unhexlify( pub_spend_key  )


    # m = Hs("SubAddr\0" || master_svk || major || minor)
    hsdata = b''.join([
        b'SubAddr\0', master_svk,
        struct.pack('<I', major), struct.pack('<I', minor)])
    m = keccak_256(hsdata).digest()


    # print(f'subprivate_view_key: { hexlify(m) }')


    # D = master_psk + m * B
    D = ed25519.edwards_add(
        ed25519.decodepoint(master_psk),
        ed25519.scalarmult_B(ed25519.decodeint(m)))

    # print(f'{D}')
    # print(f'public_spend_key: { hexlify(  ed25519.encodepoint(D) ) }')

    # C = master_svk * D
    C = ed25519.scalarmult(D, ed25519.decodeint(master_svk))


    # print(f'public_view_key: { hexlify(  ed25519.encodepoint(C) ) }')

    netbyte = bytearray([const.SUBADDR_NETBYTES[const.NETS.index(master_address.net)]])
    data = netbyte + ed25519.encodepoint(D) + ed25519.encodepoint(C)
    checksum = keccak_256(data).digest()[:4]
    return address.SubAddress(base58.encode(hexlify(data + checksum)))




def  gen_addrs(master_addr,  private_view_key,  addr_count)->int :


    assert len(master_addr) == 95
    assert len(private_view_key) == 64
    assert  addr_count > 0   and addr_count < 200000


    primary_addr = Address(master_addr)

    timestr = datetime.datetime.now().strftime('%Y-%m-%d_%H:%M:%S')

    filename = f'XMR_主网_子地址_{str(addr_count).strip()}个_{timestr}.txt'

    with open(filename, 'w') as outfile:
        print('starting gen addr write into file......')
        for index in range(1, addr_count):   #从 1 开始,   (0, 0) 是 standard address 不能泄露
            line = f'{index}\t{get_address_ex( master_addr= primary_addr, major=0, minor=index, private_view_key=private_view_key)}\n'
            outfile.write(line)
        print('done')

    return 0



def main():


    sed = 'dozen lazy lucky itinerary egotistic inbound eating deity debut knapsack sedan onslaught atrium uphill dwarf furnished ongoing rated exotic sidekick names budget lazy misery inbound'
    seed = Seed(phrase_or_hex=sed)

    # primary_addr =  seed.public_address(net=const.NET_STAGE)
    primary_addr =  seed.public_address(net=const.NET_MAIN)
    print(f'private_spend_key: {seed.secret_spend_key()}')
    print(f'primary_addr: {primary_addr}')
    print(f'{seed.secret_view_key()}')
    print(f'{seed.public_spend_key()}')
    print(f'{seed.public_view_key()}')
    print('')



    private_view_key = seed.secret_view_key()
    print(type(private_view_key))


    with open('xmr_teststagenet_addrs.txt', 'w') as outfile:
        print('starting gen addr write into file......')
        for index in range(1, 11):   #从 1 开始,   (0, 0) 是 standard address 不能泄露
            line = f'{get_address_ex( master_addr= primary_addr, major=0, minor=index, private_view_key=private_view_key)}\n'
            outfile.write(line)

        print('done')

    pass


def main2():

    master_addr = '46FpjHMeWLyM5mLhxMqUaz5AuQpobGfHScyQKGMMmnZFcSFTj6zJFNDUGyDR5SVadjAmxgBp8qv1u2vZsEs8Vo1T4uK16xE'
    private_view_key = '53a60fee1ef4386838cf7d7c5e0b5a252287590fc544a3fda802e92cce3a880a'
    addr_count = 100

    iret = gen_addrs(master_addr=master_addr, private_view_key=private_view_key, addr_count=addr_count)
    print(f'{iret}')



if __name__ == '__main__':

    # main()
    main2()
