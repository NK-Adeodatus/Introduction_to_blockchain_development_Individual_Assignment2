#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mining.h"
#include "crypto.h"

// Helper to check difficulty
static int check_difficulty(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; i++) {
        if (hash[i] != '0') return 0;
    }
    return 1;
}

static int mine_block(Block *b, int difficulty) {
    int attempts = 0;
    b->nonce = 0;
    while (1) {
        char *hash = calculate_hash(b);
        attempts++;
        if (check_difficulty(hash, difficulty)) {
            strcpy(b->hash, hash);
            free(hash);
            break;
        }
        free(hash);
        b->nonce++;
    }
    return attempts;
}

int mine_solo(PendingPool *pool, Blockchain *chain, KeyPair *keypair, int difficulty) {
    int total_attempts = 0;
    for (int i = 0; i < pool->num_blocks; i++) {
        Block *b = &pool->blocks[i];
        
        // Link to chain
        if (chain->num_blocks > 0) {
            strncpy(b->previous_hash, chain->blocks[chain->num_blocks - 1].hash, 64);
        } else {
            memset(b->previous_hash, '0', 64);
        }
        b->previous_hash[64] = '\0';
        
        if (!sign_block(b, keypair)) continue;
        
        int attempts = mine_block(b, difficulty);
        total_attempts += attempts;
        
        // Add to chain
        Block *new_blocks = realloc(chain->blocks, (chain->num_blocks + 1) * sizeof(Block));
        if (new_blocks) {
            chain->blocks = new_blocks;
            chain->blocks[chain->num_blocks] = *b;
            chain->blocks[chain->num_blocks].index = chain->num_blocks;
            chain->num_blocks++;
        }
    }
    pool->num_blocks = 0; // Clear pool
    printf("[Solo Mining] Confirmed blocks. Total hash attempts: %d. Reward claimed!\n", total_attempts);
    return total_attempts;
}

int mine_pool(PendingPool *pool, Blockchain *chain, KeyPair *keypair, int difficulty, int num_miners) {
    int total_attempts = mine_solo(pool, chain, keypair, difficulty);
    printf("\n--- Pool Mining Distribution ---\n");
    int block_reward = 50 * pool->num_blocks; // example reward
    int after_fee = block_reward * 98 / 100; // 2% fee
    for (int i = 0; i < num_miners; i++) {
        int miner_share = rand() % 100; // Random simplified share representation
        printf("Miner %d | Share: %d%% | Reward: %d coins\n", i+1, miner_share, (after_fee * miner_share) / 100);
    }
    return total_attempts;
}

int mine_cloud(PendingPool *pool, Blockchain *chain, KeyPair *keypair, int rounds) {
    int difficulty = 2; // Default for cloud
    mine_solo(pool, chain, keypair, difficulty);
    
    int fee_per_round = 15;
    int reward_per_round = 20; // arbitrary
    
    int gross = reward_per_round * rounds;
    int fees = fee_per_round * rounds;
    int net = gross - fees;
    
    printf("\n--- Cloud Mining Summary ---\n");
    printf("Rounds: %d | Gross: %d | Fees: %d | Net Profit: %d\n", rounds, gross, fees, net);
    if (net < 0) {
        printf("WARNING: Cloud mining was unprofitable!\n");
    }
    return net;
}
