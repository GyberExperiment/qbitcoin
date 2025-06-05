#ifndef BITCOIN_HYBRID_CRYPTO_H
#define BITCOIN_HYBRID_CRYPTO_H

/**
 * QBTC Hybrid Cryptography System
 * 
 * Provides ECDSA context initialization for Bitcoin address compatibility
 * while maintaining Dilithium quantum-resistant signatures.
 * 
 * Architecture:
 * - ECDSA: For Bitcoin address generation and wallet compatibility
 * - Dilithium: For quantum-resistant transaction signatures
 * - Deterministic: Both keys derived from same seed
 */

// Forward declaration of ECCVerifyHandle from key_original.cpp
class ECCVerifyHandle;

// RAII wrapper for QBTC hybrid cryptography
class QBTCHybridCrypto {
private:
    ECCVerifyHandle* ecc_handle;
    
public:
    QBTCHybridCrypto();
    ~QBTCHybridCrypto();
    
    // Check if ECDSA context is initialized
    bool IsInitialized() const;
};

#endif // BITCOIN_HYBRID_CRYPTO_H 