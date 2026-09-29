#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "block_chain.h"

/**
 * struct KeyPair - Wrapper for an OpenSSL cryptographic keypair
 * @pkey: Pointer to the OpenSSL EVP_PKEY structure
 *
 * Description: Holds the private and public keys used for generating
 * and verifying ECDSA digital signatures on the blockchain.
 */
typedef struct KeyPair {
    EVP_PKEY *pkey;
} KeyPair;

KeyPair *generate_keypair(void);
int sign_block(Block *block, KeyPair *keypair);
int verify_block_signature(const Block *block, KeyPair *keypair);
void free_keypair(KeyPair *keypair);

#endif /* CRYPTO_H */
