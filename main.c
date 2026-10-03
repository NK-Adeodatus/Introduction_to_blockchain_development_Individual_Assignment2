#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "registry.h"
#include "crypto.h"
#include "block_chain.h"
#include "transaction.h"
#include "mining.h"

int tx_model = 1; // 1 = UTXO, 0 = Account

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void print_records(const Blockchain *chain, KeyPair *keypair) {
    printf("\n--- Blockchain Records ---\n");
    for (int i = 0; i < chain->num_blocks; i++) {
        Block *b = &chain->blocks[i];
        printf("Block [%d] | Time: %ld | Action: %s\n", b->index, (long)b->timestamp, b->action);
        if (i > 0) {
            printf("  Book: %s (%s)\n", b->book_title, b->book_id);
            printf("  Member: %s (%s)\n", b->member_name, b->member_id);
            printf("  Reward: %d | TxID: %s\n", b->token_reward, b->reward_tx_id);
            printf("  Nonce: %u\n", b->nonce);
        }
        int sig_valid = verify_block_signature(b, keypair);
        printf("  Sig Valid: %s\n", sig_valid ? "YES" : "NO");
    }
    printf("--------------------------\n");
}

int main() {
    srand((unsigned)time(NULL));
    Book *books = NULL;
    int num_books = 0;
    if (!load_books("books.txt", &books, &num_books)) {
        printf("ERROR: books.txt is missing or empty.\n");
        return 1;
    }

    Member *members = NULL;
    int num_members = 0;
    if (!load_members("members.txt", &members, &num_members)) {
        printf("ERROR: members.txt is missing or empty.\n");
        free(books);
        return 1;
    }

    KeyPair *keypair = generate_keypair();
    if (!keypair) {
        printf("ERROR: Failed to generate keypair.\n");
        return 1;
    }

    Blockchain chain = {NULL, 0};
    PendingPool pool = {NULL, 0};
    init_transaction_models();

    // Pre-register all members so they appear in balances from the start
    for (int i = 0; i < num_members; i++) {
        initialize_accounts(members[i].member_id);
    }

    if (!load_blockchain(&chain, "blockchain.dat")) {
        printf("No existing blockchain found. Creating genesis block...\n");
        if (!create_genesis_block(&chain, keypair)) {
            printf("ERROR: Failed to create genesis block.\n");
            return 1;
        }
    }

    int choice;
    char book_id[20], member_id[20];

    printf("Select Transaction Model:\n1. UTXO Model\n2. Account-Based Model\nChoice: ");
    if (scanf("%d", &choice) == 1) {
        if (choice == 2) tx_model = 0;
    }
    clear_input();

    while (1) {
        printf("\n1. Borrow Book\n2. Return Book\n3. View Chain\n4. View Pending Pool\n5. Mine Pending Pool\n6. View Balances\n7. Transfer Tokens\n8. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1) { clear_input(); continue; }
        clear_input();

        if (choice == 1) {
            printf("Enter Book ID: ");
            scanf("%19s", book_id);
            printf("Enter Member ID: ");
            scanf("%19s", member_id);

            Book *b = find_book(books, num_books, book_id);
            Member *m = find_member(members, num_members, member_id);

            if (!b || !m) {
                printf("ERROR: Book or Member not found.\n");
            } else if (is_book_on_loan(&chain, book_id)) {
                printf("ERROR: Book is already on loan.\n");
            } else {
                Block new_block;
                memset(&new_block, 0, sizeof(Block));
                new_block.timestamp = time(NULL);
                strncpy(new_block.book_id, b->book_id, 19);
                strncpy(new_block.book_title, b->title, 79);
                strncpy(new_block.member_id, m->member_id, 19);
                strncpy(new_block.member_name, m->full_name, 49);
                strncpy(new_block.action, "BORROWED", 9);
                new_block.token_reward = 0;
                
                Block *new_blocks = realloc(pool.blocks, (pool.num_blocks + 1) * sizeof(Block));
                if (new_blocks) {
                    pool.blocks = new_blocks;
                    pool.blocks[pool.num_blocks++] = new_block;
                    printf("SUCCESS: Borrow request added to pending pool.\n");
                }
            }
        } else if (choice == 2) {
            printf("Enter Book ID: ");
            scanf("%19s", book_id);
            printf("Enter Member ID: ");
            scanf("%19s", member_id);

            Book *b = find_book(books, num_books, book_id);
            Member *m = find_member(members, num_members, member_id);

            if (!b || !m) {
                printf("ERROR: Book or Member not found.\n");
            } else {
                Block *outstanding = find_outstanding_borrow(&chain, book_id);
                if (!outstanding) {
                    printf("ERROR: Book was never borrowed or already returned.\n");
                } else {
                    time_t now = time(NULL);
                    int reward = 0;
                    if (now - outstanding->timestamp <= MAX_BORROW_SECONDS) {
                        reward = 10;
                    } else if (now - outstanding->timestamp > MAX_BORROW_SECONDS && now - outstanding->timestamp < MAX_BORROW_SECONDS * 2) {
                        reward = 5;
                    } // Overdue gives 0

                    Block new_block;
                    memset(&new_block, 0, sizeof(Block));
                    new_block.timestamp = now;
                    strncpy(new_block.book_id, b->book_id, 19);
                    strncpy(new_block.book_title, b->title, 79);
                    strncpy(new_block.member_id, m->member_id, 19);
                    strncpy(new_block.member_name, m->full_name, 49);
                    strncpy(new_block.action, "RETURNED", 9);
                    
                    new_block.token_reward = reward;
                    credit_reward(m->member_id, reward, new_block.reward_tx_id, tx_model);
                    
                    Block *new_blocks = realloc(pool.blocks, (pool.num_blocks + 1) * sizeof(Block));
                    if (new_blocks) {
                        pool.blocks = new_blocks;
                        pool.blocks[pool.num_blocks++] = new_block;
                        printf("SUCCESS: Return request added to pending pool. Reward: %d\n", reward);
                    }
                }
            }
        } else if (choice == 3) {
            print_records(&chain, keypair);
        } else if (choice == 4) {
            printf("Pending Blocks: %d\n", pool.num_blocks);
        } else if (choice == 5) {
            if (pool.num_blocks == 0) {
                printf("Pool is empty.\n");
                continue;
            }
            int sim_choice, diff;
            printf("Mining Simulation:\n1. Solo\n2. Pool\n3. Cloud\nChoice: ");
            scanf("%d", &sim_choice);
            printf("Enter Difficulty (1-4): ");
            scanf("%d", &diff);
            
            if (sim_choice == 1) mine_solo(&pool, &chain, keypair, diff);
            else if (sim_choice == 2) mine_pool(&pool, &chain, keypair, diff, 5);
            else if (sim_choice == 3) mine_cloud(&pool, &chain, keypair, 3);
            
            save_blockchain(&chain, "blockchain.dat");
        } else if (choice == 6) {
            print_balances(tx_model);
        } else if (choice == 7) {
            char recipient[20];
            int amount;
            printf("Enter Sender Member ID: ");
            scanf("%19s", member_id);
            printf("Enter Recipient Member ID: ");
            scanf("%19s", recipient);
            printf("Enter Amount: ");
            scanf("%d", &amount);
            
            if (transfer_tokens(member_id, recipient, amount, tx_model)) {
                printf("SUCCESS: Tokens transferred.\n");
            } else {
                printf("ERROR: Transfer failed (insufficient funds or account not found).\n");
            }
        } else if (choice == 8) {
            break;
        }
    }

    free(books);
    free(members);
    if (chain.blocks) free(chain.blocks);
    if (pool.blocks) free(pool.blocks);
    free_keypair(keypair);
    return 0;
}
