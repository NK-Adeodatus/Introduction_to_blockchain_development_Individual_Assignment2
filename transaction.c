#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transaction.h"

#define MAX_UTXOS 1000
#define MAX_ACCOUNTS 100

static UTXO utxo_set[MAX_UTXOS];
static int utxo_count = 0;

static Account accounts[MAX_ACCOUNTS];
static int account_count = 0;
static AccountTx *history_head = NULL;

void init_transaction_models(void) {
    utxo_count = 0;
    account_count = 0;
    history_head = NULL;
}

Account* get_account(const char *member_id) {
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].member_id, member_id) == 0) return &accounts[i];
    }
    if (account_count < MAX_ACCOUNTS) {
        strncpy(accounts[account_count].member_id, member_id, 19);
        accounts[account_count].balance = 0;
        accounts[account_count].nonce = 0;
        return &accounts[account_count++];
    }
    return NULL;
}

void initialize_accounts(const char *member_id) {
    get_account(member_id);
}

void credit_reward(const char *member_id, int reward, char *out_tx_id, int is_utxo) {
    if (reward <= 0) {
        strcpy(out_tx_id, "0000000000000000000000000000000000000000000000000000000000000000");
        return;
    }
    
    // Simulate tx hash
    sprintf(out_tx_id, "TX_REWARD_%s_%d", member_id, rand());

    if (is_utxo) {
        if (utxo_count < MAX_UTXOS) {
            strcpy(utxo_set[utxo_count].txid, out_tx_id);
            utxo_set[utxo_count].output_index = 0;
            strncpy(utxo_set[utxo_count].owner_id, member_id, 19);
            utxo_set[utxo_count].amount = reward;
            utxo_set[utxo_count].is_spent = 0;
            utxo_count++;
        }
    } else {
        Account *acc = get_account(member_id);
        if (acc) {
            acc->balance += reward;
            // log to history
            AccountTx *tx = malloc(sizeof(AccountTx));
            strcpy(tx->sender, "SYSTEM");
            strcpy(tx->recipient, member_id);
            tx->amount = reward;
            tx->fee = 0;
            tx->nonce = 0;
            tx->next = history_head;
            history_head = tx;
        }
    }
}

int transfer_tokens(const char *sender, const char *recipient, int amount, int is_utxo) {
    int fee = 1;
    if (is_utxo) {
        int balance = 0;
        for (int i = 0; i < utxo_count; i++) {
            if (!utxo_set[i].is_spent && strcmp(utxo_set[i].owner_id, sender) == 0) {
                balance += utxo_set[i].amount;
            }
        }
        if (balance < amount + fee) return 0;
        
        // Spend UTXOs
        int gathered = 0;
        for (int i = 0; i < utxo_count; i++) {
            if (!utxo_set[i].is_spent && strcmp(utxo_set[i].owner_id, sender) == 0) {
                utxo_set[i].is_spent = 1;
                gathered += utxo_set[i].amount;
                if (gathered >= amount + fee) break;
            }
        }
        
        // Create new UTXOs
        char tx_id[65];
        sprintf(tx_id, "TX_TRANSFER_%d", rand());
        
        if (utxo_count < MAX_UTXOS) {
            strcpy(utxo_set[utxo_count].txid, tx_id);
            utxo_set[utxo_count].output_index = 0;
            strncpy(utxo_set[utxo_count].owner_id, recipient, 19);
            utxo_set[utxo_count].amount = amount;
            utxo_set[utxo_count].is_spent = 0;
            utxo_count++;
        }
        
        // Change
        if (gathered > amount + fee && utxo_count < MAX_UTXOS) {
            strcpy(utxo_set[utxo_count].txid, tx_id);
            utxo_set[utxo_count].output_index = 1;
            strncpy(utxo_set[utxo_count].owner_id, sender, 19);
            utxo_set[utxo_count].amount = gathered - amount - fee;
            utxo_set[utxo_count].is_spent = 0;
            utxo_count++;
        }
        return 1;
    } else {
        Account *s_acc = get_account(sender);
        Account *r_acc = get_account(recipient);
        if (!s_acc || !r_acc) return 0;
        if (s_acc->balance < amount + fee) return 0;
        
        s_acc->balance -= (amount + fee);
        r_acc->balance += amount;
        s_acc->nonce++;
        
        AccountTx *tx = malloc(sizeof(AccountTx));
        strcpy(tx->sender, sender);
        strcpy(tx->recipient, recipient);
        tx->amount = amount;
        tx->fee = fee;
        tx->nonce = s_acc->nonce;
        tx->next = history_head;
        history_head = tx;
        return 1;
    }
}

void print_balances(int is_utxo) {
    printf("\n--- Balances ---\n");
    if (is_utxo) {
        printf("Mode: UTXO\n");
        for (int i = 0; i < utxo_count; i++) {
            if (!utxo_set[i].is_spent) {
                printf("UTXO: %s[%d] | Owner: %s | Amount: %d\n", 
                    utxo_set[i].txid, utxo_set[i].output_index, utxo_set[i].owner_id, utxo_set[i].amount);
            }
        }
    } else {
        printf("Mode: Account\n");
        for (int i = 0; i < account_count; i++) {
            printf("Account: %s | Balance: %d | Nonce: %d\n", accounts[i].member_id, accounts[i].balance, accounts[i].nonce);
        }
        printf("\n--- Account Transaction History ---\n");
        AccountTx *curr = history_head;
        while (curr) {
            printf("Sender: %s -> Recipient: %s | Amount: %d | Fee: %d | Nonce: %d\n", 
                curr->sender, curr->recipient, curr->amount, curr->fee, curr->nonce);
            curr = curr->next;
        }
    }
    printf("----------------\n");
}
