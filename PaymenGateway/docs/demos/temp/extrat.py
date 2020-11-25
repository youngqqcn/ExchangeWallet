#!coding:utf8

#author:yqq
#date:2020/8/20 0020 15:41
#description:

import requests

from pprint import  pprint


sequecial = ["HTD",
"KML",
"MSL",
"DFQ",
"TTB",
"SNA",
"DBD",
"KLI",
"HHA",
"HBB",
"KYL",
"SKL",
"LTU",
"FTL",
"HGL",
"HRB",
"XYB",
"BYL",
"GZC",
"QLL",
"SFB",
"SDB",
"MYL",
"TZL",
"DFW",
"SMB"]


testnet_token_dict = {
"BTCC":"htdf1y6vd8f3zkv3g9nkxsnhmm7stpv53dms6ej4p4w",
"BTCD":"htdf18yna6vth6uq68dx9nrg6j2agy4h2h828ltl0p8",
"CBH":"htdf1f4e3wenvusxatyee4trsetzrs82rvjwvzccrgj",
"DBD":"htdf164swhdp70nnzr7vly3us83y0n2qrr28m82920l",
"DDV":"htdf1gwkg3xhcxnyg0h4umd6c8rt62deaxxpq5quq87",
"DFQ":"htdf1fx09nq997app7knaztm75tddm73vwwc8l66kye",
"DFW":"htdf1yu40uswsh2lpdtd4mkr2d95vxfajnld47m93ku",
"FTL":"htdf1lr36wdt67m06wrwug90rtatcwwczedql4gh3xe",
"GZC":"htdf1vkn4td8cql59mmwxhu3g2t32fx77nd3d7avsrn",
"HBB":"htdf15ppgdc3r94hvcalf6w5w9ftnrr36paqphq3hs8",
"HGL":"htdf1kmypcnhuraz9nac6at7h4vvkm6yzu8r40uzds0",
"HHA":"htdf1wx53gepe4qer7nmtw7ne0yeq4yrrmyytjv8khr",
"HTD":"htdf1pn5kxnekyns4mneuzylyka3fyhz9hnppdruwt4",
"KLI":"htdf19q8up7ym3nwapcdqr47e3np0xjm3v6vft7l9nd",
"KML":"htdf1d5na3tf0pun72mmprl2ezdn72lv4uzm64nsmj0",
"KYL":"htdf1qqrl9tc33hhcrc2hlkhc2t7z4tl89wxdnwx9g4",
"LS":"htdf1qy8h9zp4gady7e3726rgetgyyycfednthuu4vn",
"LTU":"htdf1k3p8w75427fqstyyfadf3dfhzspq2ztd7rttqp",
"MSL":"htdf19xzu8pnkryfmww3l5z4mcmll9qxnt8dla0yyn3",
"MYL":"htdf15p76k6n2tcdnjnvk9k2ddxsmgxa88uqwqwudwm",
"QLL":"htdf17upummyex8ejhu8es0ucj0y8geqe02m4a4u20x",
"SDB":"htdf18vtl90ys92pvlmlue4ecaxm07xvdckrr47pgau",
"SKL":"htdf1u7f2376gs3np8h0hhdy2ym44r8uc9f8c3jafxk",
"SMB":"htdf1flh3jxqaxl72yd392xdtcqgmewkkn3m6z3eqf9",
"SNA":"htdf1xzfz5ls7tf02re4jk9hw46xg3c24l055aw5sy4",
"TTB":"htdf1hkqxrp2w3222edgv3g535exzfw2yls7qm5dmxv",
"TZL":"htdf1vret3pdtm03emr8pjstljwxxz4a8h0n03535xd",
"UKC":"htdf1kjk8nuzc9g25zrml302w6kzkxkyspqdslkv09e",
"XL":"htdf1937ctgjryuw38t9qmmjytscqvzdx9j9yqxyucc",
"XYB":"htdf10flty3dz6sqr8240rkazrtjw7znwjwzfl3xz09",
}


def mainnet():
    token_dict = dict()
    for i in range(1, 7):
        url = f'https://www.htdfscan.com/-/blockchain/htdf/server/api/hrclist?page={i}&limit=10'
        rsp = requests.get(url=url)
        r = rsp.json()
        r = r['data']
        for t in r:
            # print(f"{t['en_name']} : {t['address']}")
            intr = t['introduce']
            name = intr[0: intr.find('（')].strip()
            token_dict[name] = t['address']

    print(f'len: {len(sequecial)}')
    for t in sequecial:
        # print(f" {t} : {token_dict[t]} ")
        print( '\t"'+ token_dict[t] + '" : ' + '{"decimal":18, "symbol":"' + t + '"},')


def testnet():
    print(f'len: {len(sequecial)}')
    for t in sequecial:
        try:
            print( '\t"'+ testnet_token_dict[t] + '" : ' + '{"decimal":18, "symbol":"' + t + '"},')
        except:
            print(f"{t} 没有")


def main():
    # mainnet()
    testnet()


    pass


if __name__ == '__main__':

    main()