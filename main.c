#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registry.h"
#include "crypto.h"
#include "block_chain.h"

/**
 * clear_input - Flushes the standard input buffer
 *
 * Description: Discards remaining characters in stdin until a newline
 * or EOF is encountered, preventing infinite loops during invalid user input.
 *
 * Return: void
 */
void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/**
 * print_records - Outputs all blocks in the blockchain ledger to stdout
 * @chain: The blockchain to iterate over
 * @keypair: The keypair used to live-verify each block's signature
 *
 * Return: void
 */
void print_records(const Blockchain *chain, KeyPair *keypair) {
    printf("\n--- Blockchain Records ---\n");
    for (int i = 0; i < chain->num_blocks; i++) {
        Block *b = &chain->blocks[i];
        printf("Block [%d] | Time: %ld | Action: %s\n", b->index, (long)b->timestamp, b->action);
        if (i > 0) {
            printf("  Book: %s (%s)\n", b->book_title, b->book_id);
            printf("  Member: %s (%s)\n", b->member_name, b->member_id);
        }
        int sig_valid = verify_block_signature(b, keypair);
        printf("  Sig Valid: %s\n", sig_valid ? "YES" : "NO");
    }
    printf("--------------------------\n");
}

/**
 * main - Entry point for the Blockchain Library Tracker
 *
 * Description: Initializes data registries, sets up cryptographic keys,
 * loads the persistent ledger, and runs the interactive CLI loop.
 *
 * Return: 0 on successful execution, 1 on critical failure
 */
int main() {
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
    if (!load_blockchain(&chain, "blockchain.dat")) {
        printf("No existing blockchain found. Creating genesis block...\n");
        if (!create_genesis_block(&chain, keypair)) {
            printf("ERROR: Failed to create genesis block.\n");
            return 1;
        }
    } else {
        printf("Loaded blockchain with %d blocks.\n", chain.num_blocks);
        printf("NOTE: Since keypair is transient in this demo, old signatures will fail validation until you add new blocks.\n");
    }

    int choice;
    char book_id[20], member_id[20];

    while (1) {
        printf("\n1. Borrow Book\n2. Return Book\n3. View Records\n4. Validate Chain\n5. Tamper Demo\n6. Exit\nChoice: ");
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
                if (add_block(&chain, b, m, "BORROWED", keypair)) {
                    printf("SUCCESS: Book borrowed.\n");
                    save_blockchain(&chain, "blockchain.dat");
                } else {
                    printf("ERROR: Failed to create block.\n");
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
                    if (add_block(&chain, b, m, "RETURNED", keypair)) {
                        printf("SUCCESS: Book returned.\n");
                        save_blockchain(&chain, "blockchain.dat");
                    } else {
                        printf("ERROR: Failed to create block.\n");
                    }
                }
            }
        } else if (choice == 3) {
            print_records(&chain, keypair);
        } else if (choice == 4) {
            if (is_chain_valid(&chain, keypair)) {
                printf("SUCCESS: Blockchain is perfectly valid!\n");
            } else {
                printf("ERROR: Blockchain validation failed!\n");
            }
        } else if (choice == 5) {
            if (chain.num_blocks > 0) {
                printf("Tampering with the last block's action...\n");
                strncpy(chain.blocks[chain.num_blocks - 1].action, "HACKED", 9);
                if (is_chain_valid(&chain, keypair)) {
                    printf("Demo failed: chain still valid.\n");
                } else {
                    printf("Demo success: chain validation broke as expected.\n");
                }
            } else {
                printf("No blocks to tamper with.\n");
            }
        } else if (choice == 6) {
            break;
        }
    }

    free(books);
    free(members);
    if (chain.blocks) free(chain.blocks);
    free_keypair(keypair);
    return 0;
}
