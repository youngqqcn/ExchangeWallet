#!coding:utf8
#author:yqq
#date:2020/6/16 0016 9:37
#description: shbao支付网关demo
#
# 运行环境:  python3.5及以上 , Linux,Unix, Mac OS, Windows(需要安装C++编译环境,如:visual studio 2015),
# 安装依赖: pip3 install requests , ed25519
# 启动: python3 demo.py


import json
import time
import requests
from ed25519 import SigningKey, VerifyingKey



def test_ed25519():
    # 测试 ED25519签名算法, 不同编程语言可以通过此测试用例,达成签名算法的一致
    # 温馨提示: data 仅仅是测试用例的数据, 仅仅用来测试ED255199签名算法的正确性, 并无实际含义, 不要猜疑
    # 不同编程语言可以通过此测试用例,达成签名的一致
    # 即测试以下内容:
    # 1. json对象转为json字符串时根据key按照字典排序, 并且去掉分隔的空白符
    # 2. json对象转为json字符串时, 对 None 或 null 的处理(不同编程语言可能不同)
    # 3. json对象bool 类型转为 json字符串时的情况
    # 4. json对象中内嵌 数组, 对象  的情况

    pg_client_sign_key = '7d9c911ca987e85c90af66511f0ba31ea95996ba7a095b5afcf58df82ae0016c'
    pg_client_verify_key = '77dbfd30dedf746fb6088017cf5fdcbe59411686784bd5a27ca40cef26cab4f7'
    api_name = 'this_is_api_name'
    data = {
        'book': 'book name',
        'apple': 'apple name',
        'cat': 'cat name',
        'null_field': None,
        'list_field': ['bbbbb', 'ccccc', 'aaaaaa'],
        'object_field': {
            'ccc': '0000',
            'bb': '11111',
            'aaa': '22222'
        },
        'is_ok': True,
    }

    try:
        data_json_str = json.dumps(data, separators=(',', ':'), sort_keys=True)  # 按照key字母顺序排序,并且去掉分隔的空白符
        print('json_str:{}'.format(data_json_str))

        # 进行断言
        right_data_json_str = '{"apple":"apple name","book":"book name","cat":"cat name","is_ok":true,"list_field":["bbbbb","ccccc","aaaaaa"],"null_field":null,"object_field":{"aaa":"22222","bb":"11111","ccc":"0000"}}'
        assert data_json_str == right_data_json_str, 'data_json_str 不正确!'

        timestamp_str = '1590156401029'
        print('tmpstamp_str:{}'.format(timestamp_str))

        join_str = '|'.join([timestamp_str, api_name, data_json_str])
        print('join_str:{}'.format(join_str))

        # 进行断言
        right_join_str = '1590156401029|this_is_api_name|{"apple":"apple name","book":"book name","cat":"cat name","is_ok":true,"list_field":["bbbbb","ccccc","aaaaaa"],"null_field":null,"object_field":{"aaa":"22222","bb":"11111","ccc":"0000"}}'
        assert join_str == right_join_str, "join_str 不正确!"


        #以下操作包含很多字节数组的操作, 不同语言有所不同, 请自行处理
        #转为字节数组,
        sign_msg_bytes = join_str.encode('utf8')
        print('sign_msg_bytes:{}'.format(sign_msg_bytes))
        right_sign_msg_bytes = b'1590156401029|this_is_api_name|{"apple":"apple name","book":"book name","cat":"cat name","is_ok":true,"list_field":["bbbbb","ccccc","aaaaaa"],"null_field":null,"object_field":{"aaa":"22222","bb":"11111","ccc":"0000"}}'

        #断言字节数组的长度和内容
        assert  len(sign_msg_bytes) == len(right_sign_msg_bytes) == 217 , 'sign_msg_bytes 长度不正确!'
        assert sign_msg_bytes == right_sign_msg_bytes , "sign_msg_bytes 不正确!"


        sk = SigningKey(sk_s=pg_client_sign_key.encode('utf8'), prefix='', encoding='hex')
        signature_bytes = sk.sign(msg=sign_msg_bytes, prefix='', encoding='hex')
        print('signature_bytes:{}'.format(signature_bytes))

        #断言签名 的字节数组的长度和内容
        right_signature_bytes = b'b49f98bb45152191eef0d10a44e726638e295aaf6f3cc6abcc80fe521db392fb06ea72b99a6a59c58661dd660984f24b958d4c907e1e776c4304f9dc045a4504'
        assert len(signature_bytes) == len(right_signature_bytes) ==  128 , "signature_bytes 长度不正确!"
        assert signature_bytes == right_signature_bytes , "signature_bytes 不正确!"

        # 进行断言
        signature_str = signature_bytes.decode('utf8')
        print('signature_str:{}'.format(signature_str))
        right_signature_str = 'b49f98bb45152191eef0d10a44e726638e295aaf6f3cc6abcc80fe521db392fb06ea72b99a6a59c58661dd660984f24b958d4c907e1e776c4304f9dc045a4504'
        assert len(signature_str) == len(right_signature_str) == 128 , 'signature_str 长度不正确!'
        assert signature_str == right_signature_str, 'signature_str 不正确!'


        #使用公钥进行 验证签名,  即 自签自验,  如果没有抛异常,则说明验签成功
        vk = VerifyingKey(vk_s=pg_client_verify_key, prefix='', encoding='hex')
        vk.verify(sig=signature_str, msg=sign_msg_bytes, prefix='', encoding='hex')
        print('验签成功')

        print('ED25519签名测试用例, 测试通过!')
    except Exception as e:
        print('ED25519签名测试用例,测试失败! {}'.format(e))
    pass


############################## 以下是测试支付网关API的测试用例 #############################################
GLOBAL_PRO_ID = 1
GLOBAL_API_KEY = 'fa50750ddf3f24d108875c5799d773111dcdd70c7e5343b91e69c07f57bb9795'  # API KEY
GLOBAL_CLIENT_SIGN_KEY = 'ea011302192d4c7adff602d4d0d189a93465b7ec45ee487630b30373b0bcc801'  #客户端私钥
GLOBAL_HOST = 'http://192.168.10.179'

def comman_sign_and_request(req_body: dict, api_name: str, sign_key: str, api_key: str, api_url: str) :
    """
    签名函数
    :param req_data: 请求体
    :param api_name: 接口名
    :param timestamp_in_ms_str:  时间戳字符串(毫秒)
    :param sign_key: 客户端私钥
    :return: 签名字符串
    """

    #注意 签名用的时间戳字符串 要和  请求头里面的时间戳字符串 相同
    timestamp_in_ms_str = str(int(time.time() * 1000))
    print("时间戳(毫秒级)字符串:{}".format(timestamp_in_ms_str))

    json_str = json.dumps(req_body, separators=(',', ':'), sort_keys=True)  # 按照key字母顺序排序
    print('json字符串(key按照字母顺序排序):{}'.format(json_str))


    join_str = '|'.join([timestamp_in_ms_str, api_name, json_str])
    print('拼接的[签名消息体]:{}'.format(join_str))

    msg_bytes = join_str.encode('utf8')
    print('签名消息体的字节数组:{}'.format(msg_bytes))

    sk = SigningKey(sk_s=sign_key.encode('latin1'), prefix='', encoding='hex')
    sig_bytes = sk.sign(msg=msg_bytes, prefix='', encoding='hex')
    print("签名的字节数组:{}".format(sig_bytes))

    sig_str = sig_bytes.decode('utf8')
    print('签名的字符串形式:{}'.format(sig_str))


    ########## 开始请求 ############

    header = {
        'PG_API_KEY': api_key,
        'PG_API_TIMESTAMP': timestamp_in_ms_str, #注意 签名用的时间戳字符串 要和  请求头里面的时间戳字符串 相同
        'PG_API_SIGNATURE': sig_str
    }

    rsp = requests.post(url=api_url, json=req_body, headers=header)
    rsp_data = rsp.json()
    print(rsp_data)

    pass


def test_addaddresses():

    api_url =  GLOBAL_HOST + '/api/wallet/addaddresses'
    api_name = 'addaddresses'
    req_body = {
        'pro_id': GLOBAL_PRO_ID, #项目方id
        'address_count': 1, #申请地址数量
        'token_name': 'HTDF',   # 申请的币种名  HTDF 或 ETH.  注意: USDT共用ETH的地址,不需要单独申请
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print('test_addaddresses() 测试结束')

    pass


def test_queryaddress():

    api_url = GLOBAL_HOST + '/api/wallet/queryaddresses'
    api_name = 'queryaddresses'

    req_body = {
        'pro_id': GLOBAL_PRO_ID,
        'token_name': 'HTDF', # HTDF 或 ETH , 注意: USDT共用ETH的地址,不需要单独申请
        'page_index':1,
        'page_size':20,
        # 'order_id':'202006081010508973077'  #可选参数,
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print('test_queryaddress() 测试结束')
    pass




def test_queryaddaddressorder():

    api_url = GLOBAL_HOST + '/api/wallet/queryaddaddressesorder'
    api_name = 'queryaddaddressesorder'

    req_body = {
        'pro_id': GLOBAL_PRO_ID,
        'order_id':'202005281828093277212'  #申请充币地址的订单号
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print('test_queryaddaddressorder() 测试结束')
    pass



def test_getdepositdata():

    api_url = GLOBAL_HOST + '/api/deposit/getdepositdata'
    api_name = 'getdepositdata'

    #方式1: 根据区块高度查询(推荐)
    # req_body = {
    #     'pro_id': GLOBAL_PRO_ID,
    #     'token_name': 'HTDF',   # HTDF 或 ETH  或 USDT
    #     'start_height': 0,   #初始情况下设置为0即可. 后期根据自己库里面的最大高度减去一个常量(如:3000) 作为起始高度即可
    #     'end_height': 999999999,  #如果不是范围查询, 此值设置为 亿级别即可
    #     'page_index': 1,
    #     'page_size': 2,
    # }

    # 方式2: 根据时间戳(秒级)查询(做对账时, 可以使用此种查询方式)
    req_body = {
        'pro_id': 1,
        'token_name': 'ETH',
        'start_blocktime': 0,
        'end_blocktime': 99999999999,
        'page_index': 1,
        'page_size': 2,
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print('test_getdepositdata() 测试结束')
    pass




def test_getcollectiondata():

    api_url =  GLOBAL_HOST + '/api/collection/getcollectiondata'
    api_name = 'getcollectiondata'

    req_body = {
        'pro_id': GLOBAL_PRO_ID,
        'token_name': 'HTDF',   # HTDF 或 ETH  或 USDT
        'start_time' : 0,
        'end_time' : 9999999999,
        'page_index':1,
        'page_size':20
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print('test_getcollectiondata() 测试结束')
    pass


def test_withraw():

    api_url =  GLOBAL_HOST + "/api/withdraw/withdraw"
    api_name = 'withdraw'

    #生成唯一的order_id(长度大于10小于20即可), 此例仅供参考.  项目方需根据自己业务规则生成order_id
    order_id = (str(int(time.time()*1000000)) + str(int(time.time()*1000000)))[0:20]

    req_body = {
        "pro_id": GLOBAL_PRO_ID,
        "token_name": "HTDF",   # HTDF 或 ETH  或 USDT
        "order_id": order_id, #此order_id是由项目方自己生成唯一的订单号,注意和其他接口的order_id进行区分
        "from_address": 'htdf1vhq6c38demm58cnevc4sntc77z8ppvl85mj0a6',
        "to_address": 'htdf1up029t56rn9hfg72kp0f2mkn05j57a7t8422jx',
        "amount": '0.12345678',  #保留8位小数, 注意:超过8位小数则进行截取!
        "callback_url": "http://192.168.10.29:8001/notify/withdraw"  #回调接口的url, 由项目方实现,具体说明请看文档.
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)
    print('test_withraw() 测试结束')
    pass



def test_query_withdraw_order():

    api_url  = GLOBAL_HOST + "/api/withdraw/querywithdraworder"
    api_name = 'querywithdraworder'

    req_body = {
        'pro_id': GLOBAL_PRO_ID,
        'serial_id' : '202006081349179432754'   #此serial_id 是 withdraw接口返回数据中的 serial_id, 即订单的唯一标识
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print("test_query_withdraw_order() 测试结束")
    pass



def test_queryallwithdrawdata():

    api_url = GLOBAL_HOST + "/api/withdraw/queryallwithdrawdata"
    api_name = 'queryallwithdrawdata'

    req_body = {
        'pro_id': GLOBAL_PRO_ID,
        'token_name': 'HTDF',  # HTDF 或 ETH  或 USDT
        'start_time': 0,
        'end_time': 9999999999,
        'page_index': 1,
        'page_size': 20
    }

    comman_sign_and_request(req_body=req_body,
                            api_name=api_name,
                            sign_key=GLOBAL_CLIENT_SIGN_KEY,
                            api_key=GLOBAL_API_KEY,
                            api_url=api_url)

    print("test_queryallwithdrawdata() 测试结束")
    pass



def main():

    # test_ed25519()
    try:
        test_queryaddress()
    except Exception as e:
        print('error---------> {}'.format( e))
    # test_queryaddaddressorder()
    # test_addaddresses()
    # test_getdepositdata()
    # test_withraw()
    # test_queryallwithdrawdata()
    # test_query_withdraw_order()
    # test_getcollectiondata()


    pass


if __name__ == '__main__':

    main()