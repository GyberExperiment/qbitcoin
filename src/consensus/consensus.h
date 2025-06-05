// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

#include <cstdlib>
#include <stdint.h>

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) 
 *  Increased for quantum-resistant signatures (Dilithium3 signatures are ~46x larger than ECDSA) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 184000000; // 4MB * 46 = ~184MB
/** The maximum allowed weight for a block, see BIP 141 (network rule)
 *  Increased for quantum-resistant signatures */
static const unsigned int MAX_BLOCK_WEIGHT = 184000000; // Proportional increase
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 80000; // Keep same - Dilithium verification is similar cost
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

static const int WITNESS_SCALE_FACTOR = 4;

// Adjusted minimum transaction weights for larger quantum-resistant signatures
static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 2760; // ~60 * 46 (Dilithium increase factor)
static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 460; // ~10 * 46

/** Flags for nSequence and nLockTime locks */
/** Interpret sequence numbers as relative lock-time constraints. */
static constexpr unsigned int LOCKTIME_VERIFY_SEQUENCE = (1 << 0);

/**
 * Maximum number of seconds that the timestamp of the first
 * block of a difficulty adjustment period is allowed to
 * be earlier than the last block of the previous period (BIP94).
 */
static constexpr int64_t MAX_TIMEWARP = 600;

#endif // BITCOIN_CONSENSUS_CONSENSUS_H
