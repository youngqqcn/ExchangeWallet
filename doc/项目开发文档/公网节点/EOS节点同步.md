```
nodeos -d /data1/eos_data/node_data --config-dir /data1/eos_data/node_data  --plugin eosio::chain_api_plugin 



nodeos -d /data1/eos_data/node_data --config-dir /data1/eos_data/node_data    --genesis-json /data1/eos_data/genesis.json --delete-all-blocks


nodeos -d /data1/eos_data/node_data --config-dir /data1/eos_data/node_data --delete-all-blocks --snapshot /data1/eos_data/snapshots/snapshot-04b402f545955a6c355dbd1ab1a8557d1999cbb6e987a02f427b0640183d4800.bin



/v1/chain/abi_bin_to_json
/v1/chain/abi_json_to_bin
/v1/chain/get_abi
/v1/chain/get_account
/v1/chain/get_block
/v1/chain/get_block_header_state
/v1/chain/get_code
/v1/chain/get_code_hash
/v1/chain/get_currency_balance
/v1/chain/get_currency_stats
/v1/chain/get_info
/v1/chain/get_producer_schedule
/v1/chain/get_producers
/v1/chain/get_raw_abi
/v1/chain/get_raw_code_and_abi
/v1/chain/get_required_keys
/v1/chain/get_scheduled_transactions
/v1/chain/get_table_by_scope
/v1/chain/get_table_rows
/v1/chain/get_transaction_id
/v1/chain/push_block
/v1/chain/push_transaction
/v1/chain/push_transactions
ory_api_plugin
/v1/history/get_actions
/v1/history/get_controlled_accounts
/v1/history/get_key_accounts
/v1/history/get_transaction
```







```
Mainnet API nodes - Source

https://eos.greymass.com - operated by greymass   (chain history都可用)
https://mainnet.eoscalgary.io - operated by EOS Cafe  (不可访问)
https://api.eosnewyork.io - operated by EOS New York  (history不可用)
https://api.eosdetroit.io - operated by EOS Detroit  (history不可用)
http://api.hkeos.com - operated by HK EOS  (history不可用)
https://bp.cryptolions.io - operated by CryptoLions  (history不可用)
http://dc1.eosemerge.io:8888 - operated by EOS Emerge Poland  (不可访问)
https://dc1.eosemerge.io:5443 - operated by EOS Emerge Poland (不可访问)
https://api1.eosdublin.io - operated by EOS Dublin  (chain可用  history不可用)
https://api2.eosdublin.io - operated by EOS Dublin (chain可用  history不可用)
https://mainnet.eoscannon.io - operated by EOS Cannon  (不可用)
https://eos-api.privex.io - operated by Privex (privexinceos) (chain可用  history不可用 )
https://eosapi.blockmatrix.network - operated by Block Matrix  (chain可用 history不可用)
https://user-api.eoseoul.io - operated by EOSeoul   (chain可用  history不可用)
https://api.eos.bitspace.no - operated by Bitspace  (不可用)
https://node.eosflare.io - operated by EOS Flare  (不可用)
https://api-eos.blckchnd.com - operated by BLCKCHND  (chain可用  history不可用)
https://mainnet.eosimpera.com - operated by EOS IMPERA  (不可用)
https://api.franceos.fr - operated by franceos   (不可用)



Testnet API nodes - Source. To get the link, click on the HTTP column values.

http://jungle2.cryptolions.io:80
https://jungle2.cryptolions.io:443
https://jungle.eosio.cr:443
https://api.jungle.alohaeos.com:443
http://145.239.133.201:8888
http://35.237.40.93:8888
http://jungle.eosgen.io:80
http://51.15.224.168:8888
http://jungle2.eosdac.io:8882
http://eos.eosza.io:8888
http://5.9.19.86:28888
https://testnet.blockgenesys.com:443
http://jm1234.ddns.net:8888
http://172.93.54.194:8888
http://173.255.195.20:8888
http://ubeos.astrsk.org:7779
http://jungle.eosbcn.com:8080
```





https://monitor.jungletestnet.io/#home



Public Key: EOS8VkGmC1W7jdLRej6CRq8hYMTethCoLHCB25yWX6aQTCeYCXeGt
Private key: 5JfUC7k6yGs5RCoHeX464TqZnPWgdqrFfsETBzYGPB9ipDfNyzw Public





[1] active : [1] EOS8VkGmC1W7jdLRej6CRq8hYMTethCoLHCB25yWX6aQTCeYCXeGt
[1] owner : [1] EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

yangqingqin1



```

http://jungle2.cryptolions.io:80
https://jungle2.cryptolions.io:443
https://api.jungle.alohaeos.com:443
http://145.239.133.201:8888
http://jungle.eoscafeblock.com:8888
http://jungle-2.eosgen.io:80
http://jungle2.eosdac.io:8882
https://jungle.eosn.io:443
http://jungle.eosbcn.com:8080
http://jungle.atticlab.net:8888
https://jungleapi.eossweden.se:443
https://jungle.eosdac.io:443
https://jungle.eosphere.io:443
http://jungle2.cryptolions.io:8888
http://jungle-2.eosgen.io:80

```



/v1/chain/abi_json_to_bin

```
//requests
{
    "code":"eosio.token",
    "action":"transfer",
    "args":{
        "from":"yangqingqin1",
        "to":"yangqingqing",
        "quantity":"0.1234 EOS",
        "memo":"EOS to the moon"
    }
}

//response
{"binargs":"10a6b36c3acba6f1c0a6b36c3acba6f1d20400000000000004454f53000000000f454f5320746f20746865206d6f6f6e"}
```





/v1/chain/get_info

获取 last_irreversible_block_num

```

{
    "server_version":"7116e887",
    "chain_id":"e70aaab8997e1dfce58fbfac80cbbb8fecec7b99cf982a9444273cbc64c41473",
    "head_block_num":50443753,
    "last_irreversible_block_num":50443428,
    "last_irreversible_block_id":"0301b4a4fc69cb2b7af5d299abe64213ed80559ca78c6fc3e8e553680271ee8c",
    "head_block_id":"0301b5e95a29e614ff143f58f28fbbb446d83bfdf84719b8e3d78881ed07a10b",
    "head_block_time":"2019-09-19T11:25:31.500",
    "head_block_producer":"alohaeostest",
    "virtual_block_cpu_limit":200000000,
    "virtual_block_net_limit":524288000,
    "block_cpu_limit":199900,
    "block_net_limit":524288,
    "server_version_string":"v1.8.3",
    "fork_db_head_block_num":50443753,
    "fork_db_head_block_id":"0301b5e95a29e614ff143f58f28fbbb446d83bfdf84719b8e3d78881ed07a10b"
}

```





/v1/chain/get_block

获取 last_irreversible_block_num 区块信息

```
//req
{"block_num_or_id": 50443428}


//res
{
    "timestamp":"2019-09-19T11:22:47.500",
    "producer":"funnyhamster",
    "confirmed":0,
    "previous":"0301b4a3638ff81cb11f0afa63834aee2b113f9d63f79ff58ae75501e29a9673",
    "transaction_mroot":"0000000000000000000000000000000000000000000000000000000000000000",
    "action_mroot":"af641668954a451ea3de481d52208dd662b62d8c03b5fabf0f815c45ddbe1916",
    "schedule_version":270,
    "new_producers":null,
    "header_extensions":[

    ],
    "producer_signature":"SIG_K1_JzvN2NyGaosD7o47UQVsBkuSPBuLaANex41eHJG12VjNsvbjbU56CpPNEZEE5z1YB3oLmZuwMEVPkAnv2jgudp1cNPXsbG",
    "transactions":[

    ],
    "block_extensions":[

    ],
    "id":"0301b4a4fc69cb2b7af5d299abe64213ed80559ca78c6fc3e8e553680271ee8c",
    "block_num":50443428,
    "ref_block_prefix":2580739450
}
```





```

class Transaction(BaseObject) :
    def __init__(self, d, chain_info, lib_info) :
        ''' '''
        # add defaults
        if 'expiration' not in d :
            d['expiration'] = str((dt.datetime.utcnow() + dt.timedelta(seconds=30)).replace(tzinfo=pytz.UTC))   //时间
        if 'ref_block_num' not in d :
            d['ref_block_num'] = chain_info['last_irreversible_block_num'] & 0xFFFF
        if 'ref_block_prefix' not in d :
            d['ref_block_prefix'] = lib_info['ref_block_prefix']
        # validate
        self._validator = TransactionSchema()
        super(Transaction, self).__init__(d)
        # parse actions
        self.actions = self._create_obj_array(self.actions, Action)


```







/v1/chain/push_transaction 

```
//req

{
    "compression":"none",
    "transaction":{
        "expiration":"2019-09-19T11:26:30.869870+00:00",
        "ref_block_num":46244,
        "ref_block_prefix":2580739450,
        "net_usage_words":0,
        "max_cpu_usage_ms":0,
        "delay_sec":0,
        "context_free_actions":[

        ],
        "actions":[
            {
                "account":"eosio.token",
                "name":"transfer",
                "authorization":[
                    {
                        "actor":"yangqingqin1",
                        "permission":"active"
                    }
                ],
                "data":"10a6b36c3acba6f1c0a6b36c3acba6f1d20400000000000004454f53000000000f454f5320746f20746865206d6f6f6e"
            }
        ],
        "transaction_extensions":[

        ]
    },
    "signatures":[
        "SIG_K1_Kg1kBcYsi5WF22TTVcPWrkRFodMTqqbaGaaRAg8rcYAzHjsNd43SmNkGcrZN1wdvRmTKjaTWFubJUQr16QTSAabrKznT64"
    ]
}


//res

{
    "transaction_id":"a806d4b55e463ca14a3da263d1eefbfc69d24ec2ce269134ae82ac0782952c1f",
    "processed":{
        "id":"a806d4b55e463ca14a3da263d1eefbfc69d24ec2ce269134ae82ac0782952c1f",
        "block_num":50443757,
        "block_time":"2019-09-19T11:25:33.500",
        "producer_block_id":null,
        "receipt":{
            "status":"executed",
            "cpu_usage_us":247,
            "net_usage_words":18
        },
        "elapsed":247,
        "net_usage":144,
        "scheduled":false,
        "action_traces":[
            {
                "action_ordinal":1,
                "creator_action_ordinal":0,
                "closest_unnotified_ancestor_action_ordinal":0,
                "receipt":{
                    "receiver":"eosio.token",
                    "act_digest":"a7e57cb706c7f4867394f94106386bee23095f27faa7f3468b8f0ef2cfa2aac7",
                    "global_sequence":486057045,
                    "recv_sequence":76377299,
                    "auth_sequence":[
                        [
                            "yangqingqin1",
                            10
                        ]
                    ],
                    "code_sequence":5,
                    "abi_sequence":4
                },
                "receiver":"eosio.token",
                "act":{
                    "account":"eosio.token",
                    "name":"transfer",
                    "authorization":[
                        {
                            "actor":"yangqingqin1",
                            "permission":"active"
                        }
                    ],
                    "data":{
                        "from":"yangqingqin1",
                        "to":"yangqingqing",
                        "quantity":"0.1234 EOS",
                        "memo":"EOS to the moon"
                    },
                    "hex_data":"10a6b36c3acba6f1c0a6b36c3acba6f1d20400000000000004454f53000000000f454f5320746f20746865206d6f6f6e"
                },
                "context_free":false,
                "elapsed":88,
                "console":"",
                "trx_id":"a806d4b55e463ca14a3da263d1eefbfc69d24ec2ce269134ae82ac0782952c1f",
                "block_num":50443757,
                "block_time":"2019-09-19T11:25:33.500",
                "producer_block_id":null,
                "account_ram_deltas":[

                ],
                "except":null,
                "error_code":null,
                "inline_traces":[
                    {
                        "action_ordinal":2,
                        "creator_action_ordinal":1,
                        "closest_unnotified_ancestor_action_ordinal":1,
                        "receipt":{
                            "receiver":"yangqingqin1",
                            "act_digest":"a7e57cb706c7f4867394f94106386bee23095f27faa7f3468b8f0ef2cfa2aac7",
                            "global_sequence":486057046,
                            "recv_sequence":6,
                            "auth_sequence":[
                                [
                                    "yangqingqin1",
                                    11
                                ]
                            ],
                            "code_sequence":5,
                            "abi_sequence":4
                        },
                        "receiver":"yangqingqin1",
                        "act":{
                            "account":"eosio.token",
                            "name":"transfer",
                            "authorization":[
                                {
                                    "actor":"yangqingqin1",
                                    "permission":"active"
                                }
                            ],
                            "data":{
                                "from":"yangqingqin1",
                                "to":"yangqingqing",
                                "quantity":"0.1234 EOS",
                                "memo":"EOS to the moon"
                            },
                            "hex_data":"10a6b36c3acba6f1c0a6b36c3acba6f1d20400000000000004454f53000000000f454f5320746f20746865206d6f6f6e"
                        },
                        "context_free":false,
                        "elapsed":3,
                        "console":"",
                        "trx_id":"a806d4b55e463ca14a3da263d1eefbfc69d24ec2ce269134ae82ac0782952c1f",
                        "block_num":50443757,
                        "block_time":"2019-09-19T11:25:33.500",
                        "producer_block_id":null,
                        "account_ram_deltas":[

                        ],
                        "except":null,
                        "error_code":null,
                        "inline_traces":[

                        ]
                    },
                    {
                        "action_ordinal":3,
                        "creator_action_ordinal":1,
                        "closest_unnotified_ancestor_action_ordinal":1,
                        "receipt":{
                            "receiver":"yangqingqing",
                            "act_digest":"a7e57cb706c7f4867394f94106386bee23095f27faa7f3468b8f0ef2cfa2aac7",
                            "global_sequence":486057047,
                            "recv_sequence":6,
                            "auth_sequence":[
                                [
                                    "yangqingqin1",
                                    12
                                ]
                            ],
                            "code_sequence":5,
                            "abi_sequence":4
                        },
                        "receiver":"yangqingqing",
                        "act":{
                            "account":"eosio.token",
                            "name":"transfer",
                            "authorization":[
                                {
                                    "actor":"yangqingqin1",
                                    "permission":"active"
                                }
                            ],
                            "data":{
                                "from":"yangqingqin1",
                                "to":"yangqingqing",
                                "quantity":"0.1234 EOS",
                                "memo":"EOS to the moon"
                            },
                            "hex_data":"10a6b36c3acba6f1c0a6b36c3acba6f1d20400000000000004454f53000000000f454f5320746f20746865206d6f6f6e"
                        },
                        "context_free":false,
                        "elapsed":2,
                        "console":"",
                        "trx_id":"a806d4b55e463ca14a3da263d1eefbfc69d24ec2ce269134ae82ac0782952c1f",
                        "block_num":50443757,
                        "block_time":"2019-09-19T11:25:33.500",
                        "producer_block_id":null,
                        "account_ram_deltas":[

                        ],
                        "except":null,
                        "error_code":null,
                        "inline_traces":[

                        ]
                    }
                ]
            }
        ],
        "account_ram_delta":null,
        "except":null,
        "error_code":null
    }
}
```







