#!coding:utf8

#author:yqq
#date:2019/3/26 0026 10:52
#description:

import os



def ParseAddrFile(strFilePath, strFiledName):

    file  = open(strFilePath, 'r')

    lstPrivKeys = []
    for line in file.readlines():

        lstSplit = line.split('\t')
        if len(lstSplit) == 3:
            if strFiledName == "privkey":
                lstPrivKeys.append( lstSplit[2].strip() )
            elif strFiledName == "addr":
                lstPrivKeys.append( lstSplit[1].strip() )
            else:
                raise Exception("error strFiledName")

    return lstPrivKeys


def ImportPrivKeys(lstPrivKeys):
    # for privKey in lstPrivKeys:
    #     print(privKey)
    print("importing   %d" % len(lstPrivKeys))
    print('--------------------')
    strUnlockWalletCmd = "bitcoin-cli -conf=/root/.bitcoin/bitcoin-test.conf walletpassphrase 123456 120"
    iRet = os.system(strUnlockWalletCmd)
    if 0 == iRet:
        print("unlock wallet success")
        for privKey in lstPrivKeys:
            strCmd = "bitcoin-cli -conf=/root/.bitcoin/bitcoin-test.conf importprivkey %s false" % (privKey)
            # strCmd = "ipconfig"
            iRet = os.system(strCmd)
            if (0 == iRet):
                print("import ok")
            # break
    else:
        print("unlock wallet failed please check password!")
    pass


def  ImportAddrOnly(lstAddrs):
    print("importing   %d  addr" % len(lstAddrs))
    print('--------------------')
    # strUnlockWalletCmd = "bitcoin-cli -conf=/root/.bitcoin/bitcoin-test.conf walletpassphrase 123456 120"
    # iRet = os.system(strUnlockWalletCmd)
    iRet = 0
    if 0 == iRet:
        print("unlock wallet success")
        for addr in lstAddrs:
            strCmd = 'bitcoin-cli -conf=/root/.bitcoin/bitcoin-test.conf importaddress %s "user" false' % (addr)
            # strCmd = "ipconfig"
            iRet = os.system(strCmd)
            if (0 == iRet):
                print("import address  %s ok" % (addr))
            break
    else:
        print("unlock wallet failed please check password!")
    pass




def main():
    try:
        lstPrivKeys = ParseAddrFile("addr_admin.bin", "privkey")
        lstAddrs = ParseAddrFile("addr_admin.bin", "addr")
    except :
        print("importprivkey error")
        return

    #ImportPrivKeys(lstPrivKeys)
    try:
        ImportAddrOnly(lstAddrs)
    except:
        print("importaddr error")
    pass


if __name__ == '__main__':

    main()