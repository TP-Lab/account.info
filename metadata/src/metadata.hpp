#ifndef WORK_METADATA_HPP
#define WORK_METADATA_HPP
#include <eosiolib/eosio.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/crypto.hpp>
#include <eosiolib/types.h>
#include <eosiolib/multi_index.hpp>


#define EOS_SYMBOL symbol(symbol_code("EOS"),4)
#define INIT_PRICE_EOS_AMOUNT 1000

#define START_TIME 1563280200

namespace meta_data {
    using namespace eosio;
    using eosio::name;
    using eosio::contract;
    using eosio::asset;
    using eosio::symbol_code;
    using std::string;
    using std::vector;

    class [[eosio::contract]] metadata : public eosio::contract {
    public:
        using eosio::contract::contract;
        metadata(name s, name code, eosio::datastream<const char*> ds) : contract(s, code, ds),_account(s,s.value),_investigate(s,s.value),_verifier(s,s.value),_black(s,s.value) {}

        [[eosio::action]]
        void transfer(name from, name to, asset quantity, string memo);
        [[eosio::action]]
        void update(name account_name,string title,string avatar,string desc,name modifier,string url);
        [[eosio::action]]
        void verify(name account_name,name verifier);
        [[eosio::action]]
        void applyverify(name account_name,string memo);
        [[eosio::action]]
        void addverifier(name verifier);
        [[eosio::action]]
        void reset(void);
        [[eosio::action]]
        void addblack(name account_name,name verifier);
        [[eosio::action]]
        void delblack(name account_name,name verifier);

    private:
        struct [[eosio::table]] accounts {
            name account_name;
            string title;
            string avatar;
            string desc;
            name modifier;
            uint64_t status; //0:初值 1:已支付; 2:已修改; 3:account_name本人已经修改
            uint64_t verified;
            string url;
            asset price;
            uint64_t primary_key() const { return account_name.value; }
            uint64_t get_price()const {return 0 - price.amount; }
            uint64_t get_modifier()const {return modifier.value; }
            EOSLIB_SERIALIZE(accounts, (account_name)(title)(avatar)(desc)(modifier)(status)(verified)(url)(price))
        };
        typedef eosio::multi_index<"accounts"_n, accounts,
                indexed_by<"price"_n,const_mem_fun<accounts,uint64_t,&accounts::get_price>>,
                indexed_by<"modifier"_n,const_mem_fun<accounts,uint64_t,&accounts::get_modifier>>> account_table;

        struct [[eosio::table]] investigate {
            name account_name;
            string memo;
            uint64_t propose_time;
            uint64_t primary_key() const { return account_name.value; }
            uint64_t get_propose_time() const { return propose_time; }
            EOSLIB_SERIALIZE(investigate, (account_name)(memo)(propose_time))
        };
        typedef eosio::multi_index<"investigate"_n, investigate,
                indexed_by<"time"_n,const_mem_fun<investigate,uint64_t,&investigate::get_propose_time>>> investigate_table;

        struct [[eosio::table]] verifiers {
            name verifier;
            uint64_t primary_key() const { return verifier.value; }
            EOSLIB_SERIALIZE(verifiers, (verifier))
        };
        typedef eosio::multi_index<"verifiers"_n, verifiers> verifier_table;

        struct [[eosio::table]] black {
            name black_account;
            uint64_t primary_key() const { return black_account.value; }
            EOSLIB_SERIALIZE(black, (black_account))
        };
        typedef eosio::multi_index<"black"_n, black> black_table;

        account_table _account;
        investigate_table _investigate;
        verifier_table _verifier;
        black_table _black;

    };
}

#define EOSIO_DISPATCH_EX(TYPE, MEMBERS) \
            extern "C" { \
                void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
                    auto self = receiver; \
                    if( action == eosio::name("onerror").value) { \
                        /* onerror is only valid if it is for the "enumivo" code account and authorized by "eosio"'s "active permission */ \
                        eosio_assert(code == eosio::name("eosio").value, "onerror action's are only valid from the \"eosio\" system account"); \
                    } \
                    if((code == self && action != eosio::name("transfer").value) || action == eosio::name("onerror").value || (action == eosio::name("transfer").value && code == eosio::name("eosio.token").value)) { \
                            switch( action ) { \
                            EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
                        } \
                    } \
                } \
            } \

EOSIO_DISPATCH_EX(meta_data::metadata, (transfer)(update)(applyverify)(verify)(addverifier)(reset)(addblack)(delblack))

#endif //WORK_METADATA_HPP
