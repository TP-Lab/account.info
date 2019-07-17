## Functions

### 1.transfer and update the account information：

```
transfer(name from, name to, asset quantity, string memo)
update(name account_name,string title,string avatar,string desc,name modifier,string url)
```

#### set the account information firstly：

The price should be 0.1000 EOS at first time

contract account on junle net is  'metadatatptp'

```
cleos push action eosio.token transfer '["huoyantest11","metadatatptp","0.1000 EOS","huoyantest12"]' -p huoyantest11
cleos push action metadatatptp update '["huoyantest12","火焰神","http://www.huoyan.jpg","这是火焰之家","huoyantest11","\"web\":\"123\""]' -p huoyantest11
```

#### update the account information after fist time：

The price should be 1.5 times of last's

```
cleos push action eosio.token transfer '["huoyantest13","metadatatptp","0.1500 EOS","huoyantest12"]' -p huoyantest13
cleos push action metadatatptp update '["huoyantest12","火焰神13","http://www.huoyan.jpg","这是火焰之家13","huoyantest13","\"web\":\"13\""]' -p huoyantest13
```

#### the account update information of itself at first time：

The price should be 0.5 times of last's
```
cleos push action eosio.token transfer '["huoyantest12","metadatatptp","0.0750 EOS","huoyantest12"]' -p huoyantest12
cleos push action metadatatptp update '["huoyantest12","火焰神本人","http://www.huoyan.jpg","这是火焰之家本人","huoyantest12","\"web\":\"本人\""]' -p huoyantest12
```

#### the account update information of itself after first time：

Just update and don't need to pay again.
```
cleos push action metadatatptp update '["huoyantest12","火焰神本人2","http://www.huoyan.jpg","这是火焰之家本人2","huoyantest12","\"web\":\"本人2\""]' -p huoyantest12
```

### 2. Verification

#### add verifier

```
cleos push action metadatatptp addverifier '["chendatony44"]' -p metadatatptp
cleos push action metadatatptp addverifier '["metadatatptp"]' -p metadatatptp

```


#### apply for verfication

```
applyverify(name account_name,string memo)
cleos push action metadatatptp applyverify '["huoyantest12","请确认身份"]' -p huoyantest12
```

#### verify

```
verify(name account_name)
cleos push action metadatatptp verify '["huoyantest12","metadatatptp"]' -p metadatatptp
```

#### add or delete black list

```
增加黑名单
addblack(name account_name,name verifier)
cleos push action metadatatptp addblack '["huoyantest12""metadatatptp"]' -p metadatatptp
去除黑名单
delblack(name account_name,name verifier)
cleos push action metadatatptp delblack '["huoyantest12""metadatatptp"]' -p metadatatptp
```

### 3.Inquiry

#### Inquire verifier

```
cleos get table metadatatptp metadatatptp verifiers

```


#### Inquire account information

```
 struct [[eosio::table]] accounts {
            name account_name;
            string title;
            string avatar;
            string desc;
            name modifier;
            uint64_t status; //0:inital value 1:paid; 2:modified; 3:modified by the account self
            uint64_t verified;
            string url;
            asset price;
            uint64_t primary_key() const { return account_name.value; }
            uint64_t get_price()const {return price.amount; }
            uint64_t get_modifier()const {return modifier.value; }
            EOSLIB_SERIALIZE(accounts, (account_name)(title)(avatar)(desc)(modifier)(status)(verified)(url)(price))
        };
        typedef eosio::multi_index<"accounts"_n, accounts,
                        indexed_by<"price"_n,const_mem_fun<accounts,uint64_t,&accounts::get_price>>, //价格 索引
                        indexed_by<"modifier"_n,const_mem_fun<accounts,uint64_t,&accounts::get_modifier>>> account_table;  //修改者索引

cleos get table metadatatptp metadatatptp accounts

```

#### Inquire verification application

```
struct [[eosio::table]] investigate {
            name account_name;
            string memo;
            uint64_t propose_time;
            uint64_t primary_key() const { return account_name.value; }
            uint64_t get_propose_time() const { return propose_time; }
            EOSLIB_SERIALIZE(investigate, (account_name)(memo)(propose_time))
        };
        typedef eosio::multi_index<"investigate"_n, investigate,
                        indexed_by<"time"_n,const_mem_fun<investigate,uint64_t,&investigate::get_propose_time>>> investigate_table; //apply time index

cleos get table metadatatptp metadatatptp investigate

```

#### Inquire black list

```
struct [[eosio::table]] black {
            name black_account;
            uint64_t primary_key() const { return black_account.value; }
            EOSLIB_SERIALIZE(black, (black_account))
        };

cleos get table metadatatptp metadatatptp black
```

## eosjs

### account information

accounts

``` javascript
{
 account_name: "chendatony44",
 avatar: "https://tp-statics.tokenpocket.pro/website-token/1562040672940-tp-lab.png",
 desc: "小明的账号1",
 modifier: "chendatony44",
 price: "0.1000 EOS",
 status: 3,
 title: "小明",
 url: '{"website":"https://www.baidu.com","telegram":"tokenPocket_en","twitter":"TokenPocket_en","wechat":"TP-robot"}',
 verified: 0
}
```


### update account information

``` javascript

const CONTRACT_NAME = account.info;

let url = {
    website: 'https://www.baidu.com',
    telegram: 'tokenPocket_en',
    twitter: 'TokenPocket_TP',
    wechat: 'TP-robot'
}

let actions = [{
   "account": "eosio.token",
   "name": "transfer",
   "authorization": [{
       "actor": 'youraccount', 
       "permission": 'active'
   }],
   "data": {
       "from": 'youraccount', // the current account,which is used to update other account's information
       "memo": 'itokenpocket', // the account whose information will be update
       "quantity": '0.1000 EOS', // initial price is 0.1EOS, the price will be 1.5 times on each update
       "to": CONTRACT_NAME  // the contract account
   }
}, {
   "account": CONTRACT_NAME,
   "name": "update",
   "authorization": [{
       "actor": 'youraccount', 
       "permission": 'active'
   }],
   "data": {
       "account_name": 'itokenpocket', // the account whose information will be update
       "avatar": 'https://a.com/a.jpg', // avtar url
       "desc": '介绍信息', 
       "modifier": 'youraccount',
       "title": 'TokenPocket官方账号',  // name
       "url": JSON.stringify(url)
   }
}]

```


