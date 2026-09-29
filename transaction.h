#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "block_chain.h"

/* --- UTXO Model Structures --- */
typedef struct UTXO {
    char txid[65];
    int output_index;
    char owner_id[20];
    int amount;
    int is_spent;
} UTXO;

/* --- Account Model Structures --- */
typedef struct Account {
    char member_id[20];
    int balance;
    int nonce;
} Account;

typedef struct AccountTx {
    char sender[20];
    char recipient[20];
    int amount;
    int fee;
    int nonce;
    struct AccountTx *next;
} AccountTx;

/* API */
void init_transaction_models(void);
void credit_reward(const char *member_id, int reward, char *out_tx_id, int is_utxo);
int transfer_tokens(const char *sender, const char *recipient, int amount, int is_utxo);
void print_balances(int is_utxo);

#endif /* TRANSACTION_H */
