#!coding:utf8

#author:yqq
#date:2019/11/18 0018 20:50
#description:


import requests
import json

import time

def main():


    n = 0
    while True:
    # if True:

        try:
            url = 'https://api.bitaps.com/ltc/testnet/v1/faucet/send/payment'
            headers = {'content-type': 'application/json'}
            # data = {"receivers_list":[{"address":"mk7xvLdQggbzL2o8K4g36PZBBtGUCBXr9V","amount":1000000}]}
            if (n & 0x1) == 0x1:
                data = {"receivers_list": [{"address": "mn9ft9QiM36uZTFJkpU5PV6c1VDoGNCtp3", "amount": 1000000}]}
            else:
                data = {"receivers_list": [{"address": "mk7xvLdQggbzL2o8K4g36PZBBtGUCBXr9V", "amount": 1000000}]}

            rsp = requests.post(url=url, data=json.dumps(data), headers=headers)
            if rsp.status_code != 200:
                print('error:{}'.format(rsp.text))
            else:
                print ('success: {}'.format(rsp.text))

            n += 1
            if n > 10: n = 0

        except Exception as e :
            pass

        time.sleep(360)
        pass




    pass


if __name__ == '__main__':

    main()