#ifndef MINING_H
#define MINING_H

#include "block_chain.h"

int mine_solo(PendingPool *pool, Blockchain *chain, KeyPair *keypair, int difficulty);
int mine_pool(PendingPool *pool, Blockchain *chain, KeyPair *keypair, int difficulty, int num_miners);
int mine_cloud(PendingPool *pool, Blockchain *chain, KeyPair *keypair, int rounds);

#endif /* MINING_H */
