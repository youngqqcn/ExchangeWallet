
# BTC 通用API服务 


## electr

- API后台服务(Rust编写): https://github.com/Blockstream/electrs 
- 使用electr作为后台的区块链浏览器: https://blockstream.info/
- electr后台搭建: https://github.com/romanz/electrs/blob/master/doc/usage.md
- API文档: https://github.com/blockstream/esplora/blob/master/API.md



## 部署

```shell

#主网节点

cargo run  --release --bin electrs -- --network=mainnet --cookie=btc:btc2018 --db-dir=/database/btcmain  --daemon-rpc-addr=127.0.0.1:8332  -vvvv --daemon-dir=/data/bitcoin/ --http-addr=0.0.0.0:3000


```



## API

```shell
alias curlreg='curl  --user user:helloworld -X POST  http://192.168.10.199:18443/'

curlreg  -d '{"jsonrpc": "1.0", "id":"curltest", "method": "sendrawtransaction", "params": ["0100000002c717dc0c7183e7d6ff9c60c4e566836f31b19f00074017496ab00ae1e385620f000000006b48304502210097c99dc2f156e72bd08d3d88290aeeb1ce5e2edfecf9ca2c8fb7421e20cca34c0220401c0cac4ad6e1706b4744d21a7a7b39bcbd3b0ba878cad1fbdd450d78f01eb50121029524dfcad2f01838269171096fea6bc427e9351177802cb40f0801f1584ff4d1ffffffffbca5f526dd8279cfca3712efdb0ff2465f424c93655f87070fc9e7d3a63a35b0000000006a47304402206edd370544043cd7499a6c068d84e2144e12146ff7eb0598063310285638dfb502206257fdf635e3180dd5f5c5556ecd032990aa3e18d49220062f30c9a3f5206b670121029524dfcad2f01838269171096fea6bc427e9351177802cb40f0801f1584ff4d1ffffffff0118e00b54020000001976a91453f5300d6374e03b5277d7d5e9054ea9e2959d7b88ac00000000"] }' | json


curlreg  -d '{"jsonrpc": "1.0", "id":"curltest", "method": "getblockchaininfo", "params": [] }'  | json

curlreg -d '{"jsonrpc": "1.0", "id":"curltest", "method": "gettransaction", "params": ["aa2adaed959c22a363ec4f251f5443743b149ee26cfa6ded046e26f24d6328ce"] }' | json


#只能查钱包里面的地址的utxo, 不能查其他地址的utxo, 除非导入
curlreg -d '{"jsonrpc": "1.0", "id":"curltest", "method": "listunspent", "params": [0, 9999999, ["mjGRnCSyan333FdQVKonTFTmNqESaHUJmt"] , true, { "minimumAmount": 0.005 } ] }'  | json

#使用 REST API


# 可以重复广播相同的(只要上一笔交易没有被确认)
curl -X POST  http://192.168.10.199:3002/tx   -d '0100000002c717dc0c7183e7d6ff9c60c4e566836f31b19f00074017496ab00ae1e385620f000000006b48304502210097c99dc2f156e72bd08d3d88290aeeb1ce5e2edfecf9ca2c8fb7421e20cca34c0220401c0cac4ad6e1706b4744d21a7a7b39bcbd3b0ba878cad1fbdd450d78f01eb50121029524dfcad2f01838269171096fea6bc427e9351177802cb40f0801f1584ff4d1ffffffffbca5f526dd8279cfca3712efdb0ff2465f424c93655f87070fc9e7d3a63a35b0000000006a47304402206edd370544043cd7499a6c068d84e2144e12146ff7eb0598063310285638dfb502206257fdf635e3180dd5f5c5556ecd032990aa3e18d49220062f30c9a3f5206b670121029524dfcad2f01838269171096fea6bc427e9351177802cb40f0801f1584ff4d1ffffffff0118e00b54020000001976a91453f5300d6374e03b5277d7d5e9054ea9e2959d7b88ac00000000' 

curl   http://192.168.10.199:3002/tx/aa2adaed959c22a363ec4f251f5443743b149ee26cfa6ded046e26f24d6328ce | json


curl http://192.168.10.199:3002/address/mjGRnCSyan333FdQVKonTFTmNqESaHUJmt/utxo | json



# 包含多个重复的未区块确认的(输出)utxo(如果多次广播相同的交易)
# 此接口没有分页
# input的utxo查不多,防止双花
curl http://192.168.10.199:3002/address/moAt6v6gpfJhSBYSmS2AzanW9565kakujW/utxo | json

```


获取地址信息
curl http://192.168.10.199:3002/address/moAt6v6gpfJhSBYSmS2AzanW9565kakujW | json

```json
{
  "address": "mjGRnCSyan333FdQVKonTFTmNqESaHUJmt",
  "chain_stats": {
    "funded_txo_count": 117,
    "funded_txo_sum": 584900001000,
    "spent_txo_count": 3,
    "spent_txo_sum": 15000000000,
    "tx_count": 118
  },
  "mempool_stats": {
    "funded_txo_count": 0,
    "funded_txo_sum": 0,
    "spent_txo_count": 0,
    "spent_txo_sum": 0,
    "tx_count": 0
  }
}
```


获取区块数据

curl http://192.168.10.199:3002/block/6aec13fae91d54b6ff5956426d4741c62be4712d090ab5b909fd23099baa09fc  | json


获取区块中的所有交易的txid

curl http://192.168.10.199:3002/block/6aec13fae91d54b6ff5956426d4741c62be4712d090ab5b909fd23099baa09fc/txids  | json


获取区块中交易详情(默认25个)
GET /block/:hash/txs[/:start_index]


curl http://192.168.10.199:3002/block/6aec13fae91d54b6ff5956426d4741c62be4712d090ab5b909fd23099baa09fc/txs  | json




获取区块状态
curl http://192.168.10.199:3002/block/6aec13fae91d54b6ff5956426d4741c62be4712d090ab5b909fd23099baa09fc/status  | json















