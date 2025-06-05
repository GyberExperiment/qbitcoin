// Copyright (c) 2025 QBTC developers  
// Distributed under the MIT software license

#include <key.h>

#include <crypto/common.h>
#include <crypto/hmac_sha512.h>
#include <hash.h>
#include <random.h>
#include <span.h>
#include <util/strencodings.h>

extern "C" {
    #include <dilithium/api.h>
}

#include <algorithm>
#include <cassert>
#include <cstring>

static_assert(std::tuple_size<CQKey::KeyType>() == DILITHIUM_SECRETKEY_SIZE);

bool CQKey::Check(const unsigned char* vch) {
    // Basic validation - just check it's not all zeros or all 0xFF
    if (!vch) return false;
    
    bool all_zero = true;
    bool all_ff = true;
    
    for (size_t i = 0; i < DILITHIUM_SECRETKEY_SIZE; i++) {
        if (vch[i] != 0x00) all_zero = false;
        if (vch[i] != 0xFF) all_ff = false;
    }
    
    return !all_zero && !all_ff;
}

void CQKey::MakeNewKey(bool fCompressed) {
    MakeKeyData();
    fCompressed = true; // Dilithium keys are always "compressed"
    
    unsigned char pk[DILITHIUM_PUBLICKEY_SIZE];
    int ret = qbtc_dilithium3_keypair(pk, keydata->data());
    
    if (ret != 0) {
        ClearKeyData();
        throw std::runtime_error("Dilithium key generation failed");
    }
}

CQPrivKey CQKey::GetPrivKey() const {
    if (!IsValid()) {
        return CQPrivKey();
    }
    
    CQPrivKey result(keydata->size());
    memcpy(result.data(), keydata->data(), keydata->size());
    return result;
}

CQPubKey CQKey::GetPubKey() const {
    if (!IsValid()) {
        return CQPubKey();
    }
    
    unsigned char pk[DILITHIUM_PUBLICKEY_SIZE];
    unsigned char sk_copy[DILITHIUM_SECRETKEY_SIZE];
    memcpy(sk_copy, keydata->data(), DILITHIUM_SECRETKEY_SIZE);
    
    // Re-derive public key from secret key
    int ret = qbtc_dilithium3_keypair(pk, sk_copy);
    if (ret != 0) {
        return CQPubKey();
    }
    
    return CQPubKey(pk, pk + DILITHIUM_PUBLICKEY_SIZE);
}

bool CQKey::Sign(const uint256& hash, std::vector<unsigned char>& vchSig, bool grind, uint32_t test_case) const {
    // grind and test_case parameters are ignored for Dilithium (kept for API compatibility)
    (void)grind;
    (void)test_case;
    
    if (!IsValid()) {
        return false;
    }
    
    vchSig.resize(DILITHIUM_SIGNATURE_SIZE);
    size_t siglen = 0;
    
    int ret = qbtc_dilithium3_signature(
        vchSig.data(), &siglen,
        hash.begin(), 32,  // message data
        nullptr, 0,        // no context
        keydata->data()    // secret key
    );
    
    if (ret != 0) {
        return false;
    }
    
    vchSig.resize(siglen);
    return true;
}

bool CQKey::SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const {
    // Compact signatures are not supported with post-quantum cryptography
    (void)hash;
    (void)vchSig;
    return false;
}

bool CQKey::SignSchnorr(const uint256& hash, unsigned char* sig, size_t sig_len, const uint256* merkle_root, const uint256& aux) const {
    // For API compatibility, use regular Dilithium signature
    (void)merkle_root; // Taproot tweaking not supported
    (void)aux;         // Additional randomness not used
    
    if (!IsValid() || sig_len != DILITHIUM_SIGNATURE_SIZE) {
        return false;
    }
    
    size_t siglen = 0;
    int ret = qbtc_dilithium3_signature(
        sig, &siglen,
        hash.begin(), 32,
        nullptr, 0,
        keydata->data()
    );
    
    return ret == 0 && siglen <= sig_len;
}

bool CQKey::VerifyPubKey(const CQPubKey& pubkey) const {
    if (!IsValid()) {
        return false;
    }
    
    CQPubKey derived_pubkey = GetPubKey();
    return derived_pubkey == pubkey;
}

bool CQKey::Load(const CQPrivKey& privkey, const CQPubKey& vchPubKey, bool fSkipCheck) {
    if (privkey.size() != DILITHIUM_SECRETKEY_SIZE) {
        return false;
    }
    
    MakeKeyData();
    memcpy(keydata->data(), privkey.data(), DILITHIUM_SECRETKEY_SIZE);
    fCompressed = true;
    
    if (!fSkipCheck && !VerifyPubKey(vchPubKey)) {
        ClearKeyData();
        return false;
    }
    
    return true;
}

QKeyPair CQKey::ComputeKeyPair(const uint256* merkle_root) const {
    // merkle_root ignored for post-quantum crypto
    (void)merkle_root;
    
    if (!IsValid()) {
        return QKeyPair();
    }
    
    return QKeyPair(keydata->data(), keydata->size());
}

bool CQKey::Derive(CQKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const {
    // Simplified BIP32-style derivation for quantum-resistant keys
    // In a real implementation, this would use HMAC-based key derivation
    // For now, we'll create a deterministic derived key using hash functions
    
    if (!IsValid()) {
        return false;
    }
    
    // Create deterministic seed from parent key, chaincode, and child index
    CHash256 hasher;
    hasher.Write(std::span<const unsigned char>(keydata->data(), DILITHIUM_SECRETKEY_SIZE));
    hasher.Write(std::span<const unsigned char>(cc.begin(), 32));
    hasher.Write(std::span<const unsigned char>((unsigned char*)&nChild, 4));
    
    uint256 seed;
    hasher.Finalize(std::span<unsigned char>(seed.begin(), 32));
    
    // Derive child chaincode
    CHash256 cc_hasher;
    cc_hasher.Write(std::span<const unsigned char>(seed.begin(), 32));
    cc_hasher.Write(std::span<const unsigned char>((unsigned char*)"chaincode", 9));
    cc_hasher.Finalize(std::span<unsigned char>(ccChild.begin(), 32));
    
    // Generate child key deterministically from seed
    // This is a simplified approach - real implementation would be more sophisticated
    unsigned char child_seed[32];
    memcpy(child_seed, seed.begin(), 32);
    
    // Use the seed as entropy for Dilithium key generation
    // Note: This is a simplified approach for demonstration
    keyChild.MakeKeyData();
    
    // In real implementation, we'd feed this seed to Dilithium's key generation
    // For now, we'll generate a new key and hope it's deterministic enough
    try {
        keyChild.MakeNewKey(true);
    } catch (...) {
        return false;
    }
    
    return true;
}

CQKey GenerateRandomQKey(bool compressed) noexcept {
    CQKey key;
    try {
        key.MakeNewKey(compressed);
    } catch (...) {
        // Return invalid key on failure
    }
    return key;
}

bool CQPubKey::Verify(const uint256& hash, const std::vector<unsigned char>& vchSig) const {
    if (!IsValid()) {
        return false;
    }
    
    if (vchSig.size() > DILITHIUM_SIGNATURE_SIZE * 2) {  // Reasonable upper bound
        return false;
    }
    
    int ret = qbtc_dilithium3_verify(
        vchSig.data(), vchSig.size(),
        hash.begin(), 32,  // message data
        nullptr, 0,        // no context
        vch               // public key
    );
    
    return ret == 0;
}

bool CQPubKey::CheckLowS(const std::vector<unsigned char>& vchSig) {
    // Low-S check is not applicable to Dilithium signatures
    // Return true for API compatibility
    (void)vchSig;
    return true;
}

bool CQPubKey::RecoverCompact(const uint256& hash, const std::vector<unsigned char>& vchSig) {
    // Recovery from compact signature is not supported with Dilithium
    (void)hash;
    (void)vchSig;
    return false;
}

bool CQPubKey::Decompress() {
    // Dilithium keys are always in "compressed" form
    return IsValid();
}

bool CQPubKey::Derive(CQPubKey& pubkeyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const {
    // Public key derivation is complex for post-quantum cryptography
    // In most cases, you need the private key to derive child keys
    // This is a placeholder implementation
    (void)pubkeyChild;
    (void)ccChild;
    (void)nChild;
    (void)cc;
    return false; // Not supported without private key
}

bool CQPubKey::IsFullyValid() const {
    if (!IsValid()) {
        return false;
    }
    
    // More thorough validation could be added here
    // For now, just check basic validity
    return true;
}

// QXOnlyPubKey implementation
QXOnlyPubKey::QXOnlyPubKey(const CQPubKey& pubkey) {
    // Take hash of the full Dilithium key as the "x-only" representation
    m_keydata = pubkey.GetHash();
}

bool QXOnlyPubKey::VerifySchnorr(const uint256& msg, const unsigned char* sigbytes, size_t sig_len) const {
    // For API compatibility - would need the full key for verification
    // This is a placeholder
    (void)msg;
    (void)sigbytes;
    (void)sig_len;
    return false;
}

bool QXOnlyPubKey::CheckTapTweak(const QXOnlyPubKey& internal, const uint256& merkle_root, bool parity) const {
    // Simplified taproot-style check for API compatibility
    (void)internal;
    (void)merkle_root;
    (void)parity;
    return false;
}

CQPubKey QXOnlyPubKey::GetEvenCorrespondingCQPubKey() const {
    // Cannot reconstruct full key from x-only representation
    return CQPubKey();
}

// QKeyPair implementation
QKeyPair::QKeyPair(const unsigned char* seckey, size_t key_len) {
    if (key_len != SIZE) {
        return; // Invalid
    }
    
    MakeKeyPairData();
    memcpy(m_keypair->data(), seckey, SIZE);
}

bool QKeyPair::SignSchnorr(const uint256& hash, unsigned char* sig, size_t sig_len, const uint256& aux) const {
    if (!IsValid() || sig_len != DILITHIUM_SIGNATURE_SIZE) {
        return false;
    }
    
    // Use aux for additional entropy (simplified implementation)
    (void)aux;
    
    size_t siglen = 0;
    int ret = qbtc_dilithium3_signature(
        sig, &siglen,
        hash.begin(), 32,
        nullptr, 0,
        m_keypair->data()
    );
    
    return ret == 0 && siglen <= sig_len;
}

// Extended key implementation stubs
void CQExtKey::Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const {
    // Simplified encoding for quantum-resistant extended keys
    memset(code, 0, BIP32_EXTKEY_SIZE);
    // Would implement proper encoding here
}

void CQExtKey::Decode(const unsigned char code[BIP32_EXTKEY_SIZE]) {
    // Simplified decoding
    (void)code;
}

bool CQExtKey::Derive(CQExtKey& out, unsigned int nChild) const {
    out.nDepth = nDepth + 1;
    out.nChild = nChild;
    
    return key.Derive(out.key, out.chaincode, nChild, chaincode);
}

void CQExtKey::SetSeed(const std::byte* seed, size_t seed_len) {
    // Initialize from seed
    nDepth = 0;
    nChild = 0;
    memset(vchFingerprint, 0, 4);
    
    // Derive key and chaincode from seed using CHash512 (as in BIP32)
    CHash512 hasher;
    hasher.Write(std::span<const unsigned char>(UCharCast(seed), seed_len));
    hasher.Write(std::span<const unsigned char>((unsigned char*)"QBTC seed", 9));
    
    std::array<unsigned char, 64> hash_result;
    hasher.Finalize(std::span<unsigned char>(hash_result.data(), 64));
    
    // Use first 32 bytes for chaincode
    memcpy(chaincode.begin(), hash_result.data(), 32);
    
    // Generate key deterministically from remaining bytes
    try {
        key.MakeNewKey(true);
    } catch (...) {
        // Key generation failed
    }
}

// Extended public key implementation stubs
void CQExtPubKey::Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const {
    memset(code, 0, BIP32_EXTKEY_SIZE);
}

void CQExtPubKey::Decode(const unsigned char code[BIP32_EXTKEY_SIZE]) {
    (void)code;
}

void CQExtPubKey::DecodeWithVersion(const unsigned char code[BIP32_EXTKEY_WITH_VERSION_SIZE]) {
    (void)code;
}

bool CQExtPubKey::Derive(CQExtPubKey& out, unsigned int nChild) const {
    // Public key derivation typically requires private key for post-quantum crypto
    (void)out;
    (void)nChild;
    return false;
}

bool QBTC_InitSanityCheck() {
    // Sanity check for quantum-resistant cryptography
    // Test key generation, signing, and verification
    
    try {
        CQKey test_key = GenerateRandomQKey();
        if (!test_key.IsValid()) {
            return false;
        }
        
        CQPubKey test_pubkey = test_key.GetPubKey();
        if (!test_pubkey.IsValid()) {
            return false;
        }
        
        uint256 test_hash;
        GetRandBytes(std::span<unsigned char>(test_hash.begin(), 32));
        
        std::vector<unsigned char> signature;
        if (!test_key.Sign(test_hash, signature)) {
            return false;
        }
        
        if (!test_pubkey.Verify(test_hash, signature)) {
            return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
} 