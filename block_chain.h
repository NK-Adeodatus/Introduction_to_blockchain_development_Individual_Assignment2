#ifndef BLOCK_CHAIN_H
#define BLOCK_CHAIN_H

#include <time.h>
#include "registry.h"

/* Forward declare KeyPair to avoid circular dependency */
typedef struct KeyPair KeyPair;

/**
 * struct Block - Represents a single block in the library blockchain
 * @index: The block's sequential position in the chain (0 = Genesis)
 * @timestamp: Unix timestamp indicating when the block was created
 * @book_id: ID of the book being transacted
 * @book_title: Title of the book at the time of transaction
 * @member_id: ID of the member performing the transaction
 * @member_name: Name of the member at the time of transaction
 * @action: The transaction type ("BORROWED", "RETURNED", or "OVERDUE")
 * @previous_hash: The SHA-256 hash of the preceding block
 * @signature: The ECDSA digital signature verifying the block data
 * @sig_len: The actual byte length of the variable-length ECDSA signature
 * @hash: The SHA-256 hash representing the entire block, including signature
 *
 * Description: Stores an immutable record of a library lending action.
 */
typedef struct Block {
    int index;
    time_t timestamp;
    char book_id[20];
    char book_title[80];
    char member_id[20];
    char member_name[50];
    char action[10];
    char previous_hash[65];
    unsigned char signature[72];
    int sig_len;
    char hash[65];
} Block;

/**
 * struct Blockchain - Represents the complete library transaction ledger
 * @blocks: A dynamically allocated array of Block structures
 * @num_blocks: The total number of blocks currently in the chain
 *
 * Description: Holds the sequential sequence of library events,
 * allowing full auditing and verification.
 */
typedef struct Blockchain {
    Block *blocks;
    int num_blocks;
} Blockchain;

char *calculate_hash(const Block *block);
int is_book_on_loan(const Blockchain *blockchain, const char *book_id);
Block *find_outstanding_borrow(const Blockchain *blockchain, const char *book_id);
int add_block(Blockchain *blockchain, const Book *book, const Member *member, const char *action, KeyPair *keypair);
int create_genesis_block(Blockchain *blockchain, KeyPair *keypair);
int is_chain_valid(const Blockchain *blockchain, KeyPair *keypair);
int save_blockchain(const Blockchain *blockchain, const char *filename);
int load_blockchain(Blockchain *blockchain, const char *filename);

#endif /* BLOCK_CHAIN_H */
