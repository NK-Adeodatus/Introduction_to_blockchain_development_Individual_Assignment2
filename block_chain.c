#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include "block_chain.h"
#include "crypto.h"

/**
 * calculate_hash - Computes the SHA-256 hash of a block's full data
 * @block: The block to be hashed
 *
 * Description: Generates a digest spanning all block fields, including
 * the previously generated cryptographic signature.
 *
 * Return: A newly allocated hexadecimal string representing the hash
 */
char *calculate_hash(const Block *block)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return NULL;

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    
    EVP_DigestUpdate(ctx, &block->index, sizeof(block->index));
    EVP_DigestUpdate(ctx, &block->timestamp, sizeof(block->timestamp));
    EVP_DigestUpdate(ctx, block->book_id, sizeof(block->book_id));
    EVP_DigestUpdate(ctx, block->book_title, sizeof(block->book_title));
    EVP_DigestUpdate(ctx, block->member_id, sizeof(block->member_id));
    EVP_DigestUpdate(ctx, block->member_name, sizeof(block->member_name));
    EVP_DigestUpdate(ctx, block->action, sizeof(block->action));
    EVP_DigestUpdate(ctx, block->previous_hash, sizeof(block->previous_hash));
    EVP_DigestUpdate(ctx, block->signature, sizeof(block->signature));
    EVP_DigestUpdate(ctx, &block->sig_len, sizeof(block->sig_len));
    EVP_DigestUpdate(ctx, &block->token_reward, sizeof(block->token_reward));
    EVP_DigestUpdate(ctx, block->reward_tx_id, sizeof(block->reward_tx_id));
    EVP_DigestUpdate(ctx, &block->nonce, sizeof(block->nonce));

    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);

    char *hash_str = malloc((digest_len * 2) + 1);
    if (!hash_str) return NULL;

    for (unsigned int i = 0; i < digest_len; i++) {
        sprintf(hash_str + (i * 2), "%02x", digest[i]);
    }
    return hash_str;
}

/**
 * is_book_on_loan - Checks if a book is currently borrowed
 * @blockchain: The ledger containing transaction history
 * @book_id: The identifier of the book to check
 *
 * Description: Scans backwards through the blockchain to find the latest
 * transaction for a given book and evaluates its status.
 *
 * Return: 1 if the book is currently on loan, 0 otherwise
 */
int is_book_on_loan(const Blockchain *blockchain, const char *book_id)
{
    if (!blockchain || !book_id) return 0;

    for (int i = blockchain->num_blocks - 1; i >= 0; i--) {
        Block *b = &blockchain->blocks[i];
        if (strcmp(b->book_id, book_id) == 0) {
            if (strcmp(b->action, "BORROWED") == 0) return 1;
            if (strcmp(b->action, "RETURNED") == 0) return 0;
        }
    }
    return 0;
}

/**
 * find_outstanding_borrow - Finds the active borrow record for a book
 * @blockchain: The ledger containing transaction history
 * @book_id: The identifier of the book
 *
 * Return: Pointer to the corresponding Block, or NULL if none exists
 */
Block* find_outstanding_borrow(const Blockchain *blockchain, const char *book_id)
{
    if (!blockchain || !book_id) return NULL;

    for (int i = blockchain->num_blocks - 1; i >= 0; i--) {
        Block *b = &blockchain->blocks[i];
        if (strcmp(b->book_id, book_id) == 0) {
            if (strcmp(b->action, "BORROWED") == 0) return b;
            if (strcmp(b->action, "RETURNED") == 0) return NULL;
        }
    }
    return NULL;
}

/**
 * add_block - Creates and securely appends a new transaction to the ledger
 * @blockchain: The blockchain to append to
 * @book: The book involved in the transaction
 * @member: The member performing the transaction
 * @action: The transaction type ("BORROWED" or "RETURNED")
 * @keypair: The ECDSA keypair used to digitally sign the transaction
 *
 * Return: 1 on success, 0 on failure
 */
int add_block(Blockchain *blockchain, const Book *book, const Member *member, const char *action, KeyPair *keypair)
{
    if (!blockchain || !book || !member || !action || !keypair) return 0;

    Block *new_blocks = realloc(blockchain->blocks, (blockchain->num_blocks + 1) * sizeof(Block));
    if (!new_blocks) return 0;
    blockchain->blocks = new_blocks;

    Block *block = &blockchain->blocks[blockchain->num_blocks];
    memset(block, 0, sizeof(Block)); 

    block->index = blockchain->num_blocks;
    block->timestamp = time(NULL);
    strncpy(block->book_id, book->book_id, sizeof(block->book_id) - 1);
    strncpy(block->book_title, book->title, sizeof(block->book_title) - 1);
    strncpy(block->member_id, member->member_id, sizeof(block->member_id) - 1);
    strncpy(block->member_name, member->full_name, sizeof(block->member_name) - 1);
    strncpy(block->action, action, sizeof(block->action) - 1);

    if (blockchain->num_blocks > 0) {
        strncpy(block->previous_hash, blockchain->blocks[blockchain->num_blocks - 1].hash, 64);
    } else {
        memset(block->previous_hash, '0', 64);
    }
    block->previous_hash[64] = '\0';

    if (!sign_block(block, keypair)) return 0;

    char *hash = calculate_hash(block);
    if (!hash) return 0;
    strncpy(block->hash, hash, 64);
    block->hash[64] = '\0';
    free(hash);

    blockchain->num_blocks++;
    return 1;
}

/**
 * create_genesis_block - Initializes a new ledger with the Genesis Block
 * @blockchain: The empty blockchain struct to initialize
 * @keypair: The keypair used to sign the genesis block
 *
 * Return: 1 on success, 0 on failure
 */
int create_genesis_block(Blockchain *blockchain, KeyPair *keypair)
{
    if (!blockchain || !keypair) return 0;

    blockchain->blocks = malloc(sizeof(Block));
    if (!blockchain->blocks) return 0;

    Block *genesis = &blockchain->blocks[0];
    memset(genesis, 0, sizeof(Block));

    genesis->index = 0;
    genesis->timestamp = time(NULL);
    strncpy(genesis->action, "GENESIS", sizeof(genesis->action) - 1);
    memset(genesis->previous_hash, '0', 64);
    genesis->previous_hash[64] = '\0';

    if (!sign_block(genesis, keypair)) return 0;

    char *hash = calculate_hash(genesis);
    if (!hash) return 0;
    strncpy(genesis->hash, hash, 64);
    genesis->hash[64] = '\0';
    free(hash);

    blockchain->num_blocks = 1;
    return 1;
}

/**
 * is_chain_valid - Cryptographically verifies the integrity of the full ledger
 * @blockchain: The blockchain ledger to validate
 * @keypair: The public key used to verify all digital signatures
 *
 * Description: Recalculates hashes and verifies signatures to ensure
 * no block data has been tampered with since creation.
 *
 * Return: 1 if the entire chain is valid, 0 if any block is corrupted
 */
int is_chain_valid(const Blockchain *blockchain, KeyPair *keypair)
{
    if (!blockchain || !keypair || blockchain->num_blocks == 0) return 0;

    for (int i = 0; i < blockchain->num_blocks; i++) {
        Block *block = &blockchain->blocks[i];

        if (i > 0) {
            if (strcmp(block->previous_hash, blockchain->blocks[i - 1].hash) != 0) {
                return 0; /* previous_hash mismatch */
            }
        }

        if (!verify_block_signature(block, keypair)) {
            return 0; /* invalid signature */
        }

        char *hash = calculate_hash(block);
        if (strcmp(hash, block->hash) != 0) {
            free(hash);
            return 0; /* hash mismatch */
        }
        free(hash);
    }
    return 1;
}

/**
 * save_blockchain - Serializes the blockchain array to a binary file
 * @blockchain: The blockchain to persist
 * @filename: Path to the destination file
 *
 * Return: 1 on success, 0 on failure
 */
int save_blockchain(const Blockchain *blockchain, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;
    if (fwrite(&blockchain->num_blocks, sizeof(int), 1, f) != 1) {
        fclose(f); return 0;
    }
    if (blockchain->num_blocks > 0) {
        if (fwrite(blockchain->blocks, sizeof(Block), blockchain->num_blocks, f) != (size_t)blockchain->num_blocks) {
            fclose(f); return 0;
        }
    }
    fclose(f);
    return 1;
}

/**
 * load_blockchain - Deserializes the blockchain array from a binary file
 * @blockchain: The struct to populate with the loaded blocks
 * @filename: Path to the source file
 *
 * Return: 1 on success, 0 on failure
 */
int load_blockchain(Blockchain *blockchain, const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    int num;
    if (fread(&num, sizeof(int), 1, f) != 1) {
        fclose(f); return 0;
    }

    if (num > 0) {
        Block *blocks = malloc(num * sizeof(Block));
        if (fread(blocks, sizeof(Block), num, f) != (size_t)num) {
            free(blocks); fclose(f); return 0;
        }
        blockchain->blocks = blocks;
    } else {
        blockchain->blocks = NULL;
    }
    blockchain->num_blocks = num;
    fclose(f);
    return 1;
}
