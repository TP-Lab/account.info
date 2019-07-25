#include "metadata.hpp"

using namespace meta_data;

struct [[eosio::table, eosio::contract("eosio.system")]] user_resources {
    name          owner;
    asset         net_weight;
    asset         cpu_weight;
    int64_t       ram_bytes = 0;

    uint64_t primary_key()const { return owner.value; }

    // explicit serialization macro is not necessary, used here only to improve compilation time
    EOSLIB_SERIALIZE( user_resources, (owner)(net_weight)(cpu_weight)(ram_bytes) )
};
typedef eosio::multi_index< "userres"_n, user_resources >      user_resources_table;


void metadata::transfer(name from, name to, asset quantity, string memo) {
    eosio_assert(now() >= START_TIME,"game has not started");
    require_auth(from);
    if (!(from != _self && to == _self)) {
        return;
    }
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");
    eosio_assert(quantity.symbol == EOS_SYMBOL,"quantity symbol is not EOS");


    name account;
    name refer;
    bool valid_refer = false;
    auto separator_pos_1 = memo.find('-');
    if(separator_pos_1 != string::npos){
        string account_str = memo.substr(0, separator_pos_1);
        eosio_assert(account_str.size()<=12,"account name string is too long");
        account = name(account_str);
        if(memo.size() > separator_pos_1+1){
            string refer_str = memo.substr(separator_pos_1+1);
            if(refer_str.size()<=12){
                refer = name(refer_str);
                user_resources_table  userres( "eosio"_n, refer.value );
                auto res_itr = userres.find( refer.value );
                if(res_itr != userres.end()) {
                    auto refer_ptr = _account.find(refer.value);
                    if (refer_ptr!=_account.end()){
                        if (refer_ptr->modifier == refer && refer != from) {//邀请人必须编辑过自己的账号；自己不能邀请自己
                            valid_refer = true;
                        }
                    }
                }
            }
        }
    }else{
        account = name(memo);
    }
    user_resources_table  userres( "eosio"_n, account.value );
    auto res_itr = userres.find( account.value );
    eosio_assert( res_itr != userres.end() || account == "eosio.token"_n, "the account doesn't exist");
    auto account_ptr = _account.find(account.value);
    if (account_ptr == _account.end()) {
        eosio_assert(quantity.amount == INIT_PRICE_EOS_AMOUNT,"quantity amount is not correct ");
        _account.emplace(_self,[&](auto&s){
            s.account_name = account;
            s.price = quantity;
            s.modifier = from;
            if (from==account){
                s.status = 3;
            }else{
                s.status = 1;
            }
        });
        if (valid_refer) {
            action(
                    permission_level{_self, "active"_n},
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple(_self, refer, quantity/2, std::string("thank you for sharing the account.info"))
            ).send();
        }
    }else{
        eosio_assert(account_ptr->status != 3,"the account information is not allowed to be modified now");
        eosio_assert((account_ptr->account_name != from && quantity.amount >= account_ptr->price.amount * 15/10) ||
                             (account_ptr->account_name == from && quantity.amount >= account_ptr->price.amount/2),"quantity amount is not correct");
        asset backasset;
        if(account_ptr->account_name == from) {
            backasset = account_ptr->price/2;
        }else{
            backasset = account_ptr->price * 14/10;
        }

        string memo = "The account info of " + account_ptr->account_name.to_string() + " has been updated. Thanks for your attention.";
        //将token返给上一个修改者
        action(
                permission_level{_self, "active"_n},
                "eosio.token"_n,
                "transfer"_n,
                make_tuple(_self, account_ptr->modifier, backasset, std::string(memo))
        ).send();

        //将本次修改者多支付的token返回
        asset change = quantity;
        change.amount = 0;
        if (account_ptr->account_name == from) {
            if (quantity.amount > account_ptr->price.amount/2){
                change.amount = quantity.amount - account_ptr->price.amount/2;
            }
        }else {
            if (quantity.amount > account_ptr->price.amount * 15/10){
                change.amount = quantity.amount - (account_ptr->price.amount * 15/10);
            }
        }

        if (change.amount > 0) {
            action(
                    permission_level{_self, "active"_n},
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple(_self, from, change, std::string("here is your change"))
            ).send();
        }

        if (valid_refer) {
            if (from != account){
                action(
                        permission_level{_self, "active"_n},
                        "eosio.token"_n,
                        "transfer"_n,
                        make_tuple(_self, refer, account_ptr->price * 1/20, std::string("thank you for sharing the account.info"))
                ).send();
            }
        }

        _account.modify(account_ptr,_self,[&](auto&s){
            s.account_name = account;
            s.price = quantity-change;
            s.modifier = from;
            s.status = 1;
        });
    }
    return;
}

void metadata::update(name account_name,string title,string avatar,string desc,name modifier,string url){
    require_auth(modifier);
    auto account_ptr = _account.find(account_name.value);
    eosio_assert(account_ptr != _account.end(),"the user has not pay for the fee");
    if (account_ptr->status != 3) {
        eosio_assert(account_ptr->modifier == modifier,"the modifier is not correct");
        eosio_assert(account_ptr->status==1, "it should pay before update information");
    }else{
        eosio_assert(account_ptr->modifier == modifier && modifier == account_ptr->account_name,"the modifier is not correct");
    }

    _account.modify(account_ptr,modifier,[&](auto&s){
        s.account_name = account_name;
        s.title = title;
        s.avatar = avatar;
        s.desc = desc;
        s.url = url;
        if(account_ptr->status != 3){
            if( account_ptr->account_name == modifier) {
                s.status = 3;
            }else {
                s.status = 2;
            }
        }else{
            s.verified = 0;
        }
    });
}

void metadata::addverifier(name verifier){
    require_auth(_self);
    auto verifier_ptr = _verifier.find(verifier.value);
    eosio_assert(verifier_ptr == _verifier.end(),"the verifier has been added");
    _verifier.emplace(_self,[&](auto&s){
       s.verifier = verifier;
    });
}

void metadata::verify(name account_name, name verifier){
    require_auth(verifier);
    auto verifier_ptr = _verifier.find(verifier.value);
    eosio_assert(verifier_ptr != _verifier.end(),"the verifier has no right to verify the account");
    auto investigate_ptr = _investigate.find(account_name.value);
    eosio_assert(investigate_ptr != _investigate.end(),"the investigate application for the account does not exist");

    auto account_ptr = _account.find(account_name.value);
    eosio_assert(account_ptr!=_account.end(),"the account information is not created yet");
    eosio_assert(account_ptr->status == 3,"the account owner has not update the information");
    eosio_assert(account_ptr->verified != 1,"the account information has been verified");
    _account.modify(account_ptr,same_payer,[&](auto&s){
        s.verified = 1;
    });

    _investigate.erase(investigate_ptr);
}

void metadata:: applyverify(name account_name,string memo) {
    require_auth(account_name);
    auto investigate_ptr = _investigate.find(account_name.value);
    eosio_assert(investigate_ptr == _investigate.end(),"the investigate application for the account has been existed");

    auto account_ptr = _account.find(account_name.value);
    eosio_assert(account_ptr!=_account.end(),"the account information is not created yet");
    eosio_assert(account_ptr->status == 3,"the account owner has not update the information");
    eosio_assert(account_ptr->verified != 1,"the account information has been verified");

    _investigate.emplace(account_name,[&](auto& s){
        s.account_name = account_name;
        s.memo = memo;
        s.propose_time = now();
    });
}

void metadata:: addblack(name account_name,name verifier) {
    require_auth(verifier);
    auto verifier_ptr = _verifier.find(verifier.value);
    eosio_assert(verifier_ptr != _verifier.end(),"the verifier has no right to add black list");

    auto black_ptr = _black.find(account_name.value);
    eosio_assert(black_ptr == _black.end(),"the account has been in black list");
    _black.emplace(_self,[&](auto &s) {
       s.black_account = account_name;
    });
}

void metadata:: delblack(name account_name,name verifier) {
    require_auth(verifier);
    auto verifier_ptr = _verifier.find(verifier.value);
    eosio_assert(verifier_ptr != _verifier.end(),"the verifier has no right to add black list");

    auto black_ptr = _black.find(account_name.value);
    eosio_assert(black_ptr != _black.end(),"the account is not in black list ");
    _black.erase(black_ptr);
}

void metadata:: reset(void) {
    require_auth(_self);
    while(_account.begin() != _account.end()) {
        auto account_ptr = _account.begin();
        _account.erase(account_ptr);
    }

    while(_investigate.begin() != _investigate.end()) {
        auto investigate_ptr = _investigate.begin();
        _investigate.erase(investigate_ptr);
    }
}

void metadata::setbymaster(name account_name,string title,string avatar,string desc,string url){
    require_auth(_self);
    auto account_ptr = _account.find(account_name.value);
    if(account_ptr == _account.end()){
        _account.emplace(_self,[&](auto&s){
            s.account_name = account_name;
            s.title = title;
            s.avatar = avatar;
            s.desc = desc;
            s.url = url;
            s.status = 3;
            s.verified = 0;
            s.modifier = account_name;
            s.price = asset(1000,EOS_SYMBOL);
        });
    }else{
        _account.modify(account_ptr,_self,[&](auto&s){
            s.account_name = account_name;
            s.title = title;
            s.avatar = avatar;
            s.desc = desc;
            s.url = url;
            s.status = 3;
            s.verified = 0;
            s.modifier = account_name;
            s.price = asset(1000,EOS_SYMBOL);
        });
    }
}