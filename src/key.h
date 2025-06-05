// Copyright (c) 2025 QBTC developers
// Distributed under the MIT software license

#ifndef QBITCOIN_QKEY_H
#define QBITCOIN_QKEY_H

#include <common/signmessage.h>
#include <hash.h>
#include <pubkey.h>
#include <serialize.h>
#include <span.h>
#include <support/allocators/secure.h>
#include <uint256.h>
#include <dilithium/api.h>

// Wrapper functions for use in C++
extern "C" {
    int qbtc_dilithium3_keypair(unsigned char *pk, unsigned char *sk);
    int qbtc_dilithium3_signature(unsigned char *sig, size_t *siglen,
                                  const unsigned char *m, size_t mlen,
                                  const unsigned char *ctx, size_t ctxlen,
                                  const unsigned char *sk);
    int qbtc_dilithium3_verify(const unsigned char *sig, size_t siglen,
                               const unsigned char *m, size_t mlen,
                               const unsigned char *ctx, size_t ctxlen,
                               const unsigned char *pk);
}

#include <stdexcept>
#include <vector>
#include <array>
#include <optional>

/**
 * CQPrivKey is a serialized quantum-resistant private key using Dilithium3
 */
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CQPrivKey;

/** Size of Dilithium3 keys and signatures */
constexpr static size_t DILITHIUM_PUBLICKEY_SIZE = 1952;  // CRYPTO_PUBLICKEYBYTES for mode 3
constexpr static size_t DILITHIUM_SECRETKEY_SIZE = 4000;  // CRYPTO_SECRETKEYBYTES for mode 3  
constexpr static size_t DILITHIUM_SIGNATURE_SIZE = 3309;  // CRYPTO_BYTES for mode 3

// Forward declarations
class CQPubKey;
class QKeyPair;

/** A reference to a CQKey: the Hash160 of its serialized public key */
class CQKeyID : public uint160
{
public:
    CQKeyID() : uint160() {}
    explicit CQKeyID(const uint160& in) : uint160(in) {}
};

/** An encapsulated quantum-resistant private key using Dilithium. */
class CQKey
{
public:
    /**
     * Dilithium3 key sizes:
     */
    static const unsigned int SIZE = DILITHIUM_SECRETKEY_SIZE;
    static const unsigned int COMPRESSED_SIZE = DILITHIUM_SECRETKEY_SIZE; // Always "compressed"
    static const unsigned int SIGNATURE_SIZE = DILITHIUM_SIGNATURE_SIZE;
    static const unsigned int COMPACT_SIGNATURE_SIZE = 0; // Not supported

    // Quantum-resistant key sizes for Dilithium
    using KeyType = std::array<unsigned char, DILITHIUM_SECRETKEY_SIZE>;

private:
    //! Whether the public key corresponding to this private key is (to be) compressed.
    bool fCompressed{true}; // Always compressed for Dilithium

    //! The actual byte data. nullptr for invalid keys.
    secure_unique_ptr<KeyType> keydata;

    //! Check whether the keydata is valid.
    bool static Check(const unsigned char* vch);

    void MakeKeyData()
    {
        if (!keydata) keydata = make_secure_unique<KeyType>();
    }

    void ClearKeyData()
    {
        keydata.reset();
    }

public:
    CQKey() noexcept = default;
    CQKey(CQKey&&) noexcept = default;
    CQKey& operator=(CQKey&&) noexcept = default;

    CQKey& operator=(const CQKey& other)
    {
        if (this != &other) {
            if (other.keydata) {
                MakeKeyData();
                *keydata = *other.keydata;
            } else {
                ClearKeyData();
            }
            fCompressed = other.fCompressed;
        }
        return *this;
    }

    CQKey(const CQKey& other) { *this = other; }

    friend bool operator==(const CQKey& a, const CQKey& b)
    {
        return a.fCompressed == b.fCompressed &&
            a.size() == b.size() &&
            memcmp(a.data(), b.data(), a.size()) == 0;
    }

    //! Initialize using begin and end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn)
    {
        if (size_t(pend - pbegin) != std::tuple_size_v<KeyType>) {
            ClearKeyData();
        } else if (Check(UCharCast(&pbegin[0]))) {
            MakeKeyData();
            memcpy(keydata->data(), (unsigned char*)&pbegin[0], keydata->size());
            fCompressed = fCompressedIn;
        } else {
            ClearKeyData();
        }
    }

    //! Simple read-only vector-like interface.
    unsigned int size() const { return keydata ? keydata->size() : 0; }
    const std::byte* data() const { return keydata ? reinterpret_cast<const std::byte*>(keydata->data()) : nullptr; }
    const std::byte* begin() const { return data(); }
    const std::byte* end() const { return data() + size(); }

    //! Check whether this private key is valid.
    bool IsValid() const { return !!keydata; }

    //! Check whether the public key corresponding to this private key is (to be) compressed.
    bool IsCompressed() const { return fCompressed; }

    //! Generate a new private key using Dilithium key generation.
    void MakeNewKey(bool fCompressed = true);

    /**
     * Convert the private key to a CQPrivKey (serialized private key data).
     */
    CQPrivKey GetPrivKey() const;

    /**
     * Compute the public key from a private key using Dilithium.
     */
    CQPubKey GetPubKey() const;

    /**
     * Create a Dilithium signature.
     * Note: grind and test_case parameters are kept for API compatibility but ignored
     */
    bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, bool grind = true, uint32_t test_case = 0) const;

    /**
     * Create a compact signature (API compatibility - Dilithium doesn't support compact signatures)
     * Returns false as this is not supported with post-quantum cryptography
     */
    bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;

    /**
     * Create a Dilithium signature (for Schnorr API compatibility)
     */
    bool SignSchnorr(const uint256& hash, unsigned char* sig, size_t sig_len, const uint256* merkle_root, const uint256& aux) const;

    /**
     * Verify thoroughly whether a private key and a public key match.
     */
    bool VerifyPubKey(const CQPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(const CQPrivKey& privkey, const CQPubKey& vchPubKey, bool fSkipCheck);

    /**
     * Compute a QKeyPair (for advanced signature operations)
     */
    QKeyPair ComputeKeyPair(const uint256* merkle_root) const;

    //! BIP32 key derivation (simplified for quantum-resistant keys)
    [[nodiscard]] bool Derive(CQKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;
};

CQKey GenerateRandomQKey(bool compressed = true) noexcept;

/** Quantum-resistant public key using Dilithium. */
class CQPubKey
{
public:
    /**
     * Dilithium3 key and signature sizes:
     */
    static constexpr unsigned int SIZE = DILITHIUM_PUBLICKEY_SIZE;
    static constexpr unsigned int COMPRESSED_SIZE = DILITHIUM_PUBLICKEY_SIZE;
    static constexpr unsigned int SIGNATURE_SIZE = DILITHIUM_SIGNATURE_SIZE;
    static constexpr unsigned int COMPACT_SIGNATURE_SIZE = 0; // Not supported

private:
    /**
     * Store the serialized Dilithium public key data.
     */
    unsigned char vch[SIZE];

    //! Set this key data to be invalid
    void Invalidate()
    {
        memset(vch, 0xFF, SIZE);
    }

public:
    bool static ValidSize(const std::vector<unsigned char> &vch) {
        return vch.size() == SIZE;
    }

    //! Construct an invalid public key.
    CQPubKey()
    {
        Invalidate();
    }

    //! Initialize a public key using begin/end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend)
    {
        int len = pend - pbegin;
        if (len == SIZE)
            memcpy(vch, (unsigned char*)&pbegin[0], len);
        else
            Invalidate();
    }

    //! Construct a public key using begin/end iterators to byte data.
    template <typename T>
    CQPubKey(const T pbegin, const T pend)
    {
        Set(pbegin, pend);
    }

    //! Construct a public key from a byte vector.
    explicit CQPubKey(const std::vector<uint8_t>& _vch)
    {
        Set(_vch.begin(), _vch.end());
    }

    //! Simple read-only vector-like interface to the pubkey data.
    unsigned int size() const { return SIZE; }
    const unsigned char* data() const { return vch; }
    const unsigned char* begin() const { return vch; }
    const unsigned char* end() const { return vch + size(); }
    const unsigned char& operator[](unsigned int pos) const { return vch[pos]; }

    //! Comparator implementation.
    friend bool operator==(const CQPubKey& a, const CQPubKey& b)
    {
        return memcmp(a.vch, b.vch, SIZE) == 0;
    }
    friend bool operator!=(const CQPubKey& a, const CQPubKey& b)
    {
        return !(a == b);
    }
    friend bool operator<(const CQPubKey& a, const CQPubKey& b)
    {
        return memcmp(a.vch, b.vch, SIZE) < 0;
    }
    friend bool operator>(const CQPubKey& a, const CQPubKey& b)
    {
        return memcmp(a.vch, b.vch, SIZE) > 0;
    }

    //! Implement serialization, as if this was a byte vector.
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        unsigned int len = size();
        ::WriteCompactSize(s, len);
        s.write((char*)vch, len);
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        const unsigned int len(::ReadCompactSize(s));
        if (len == SIZE) {
            s.read((char*)vch, len);
        } else {
            // invalid pubkey, skip available data
            s.ignore(len);
            Invalidate();
        }
    }

    //! Get the KeyID of this public key (hash of its serialization)
    CQKeyID GetID() const
    {
        return CQKeyID(Hash160(std::span<const unsigned char>(vch, SIZE)));
    }

    //! Get the 256-bit hash of this public key.
    uint256 GetHash() const
    {
        return Hash(std::span<const unsigned char>(vch, SIZE));
    }

    /*
     * Check syntactic correctness.
     */
    bool IsValid() const
    {
        // Check if not all 0xFF (invalid marker)
        for (unsigned int i = 0; i < SIZE; i++) {
            if (vch[i] != 0xFF) return true;
        }
        return false;
    }

    /** Check if a public key is syntactically valid (for API compatibility). */
    bool IsValidNonHybrid() const noexcept
    {
        return IsValid(); // Dilithium keys don't have hybrid format
    }

    //! Verify a Dilithium signature
    bool Verify(const uint256& hash, const std::vector<unsigned char>& vchSig) const;

    //! Check if signature is low-S (API compatibility - not applicable to Dilithium)
    static bool CheckLowS(const std::vector<unsigned char>& vchSig);

    //! Recover public key from compact signature (not supported for Dilithium)
    bool RecoverCompact(const uint256& hash, const std::vector<unsigned char>& vchSig);

    //! Decompress public key (not applicable - Dilithium keys are always "compressed")
    bool Decompress();

    //! BIP32 public key derivation (simplified for quantum-resistant keys)
    [[nodiscard]] bool Derive(CQPubKey& pubkeyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;

    //! Check if this is a valid Dilithium public key (more expensive than IsValid())
    bool IsFullyValid() const;
};

/** Quantum-resistant X-only public key (simplified version for API compatibility) */
class QXOnlyPubKey
{
private:
    uint256 m_keydata;

public:
    /** Construct a null xonly pubkey. */
    QXOnlyPubKey() = default;

    /** Test whether this is the 0 key (the result of default construction). */
    bool IsNull() const { return m_keydata.IsNull(); }

    /** Construct an x-only pubkey from exactly 32 bytes (hash of full Dilithium key). */
    explicit QXOnlyPubKey(const unsigned char* bytes) 
    {
        memcpy(m_keydata.begin(), bytes, 32);
    }

    /** Construct an x-only pubkey from a full CQPubKey (takes hash of the key). */
    explicit QXOnlyPubKey(const CQPubKey& pubkey);

    /** Verify a signature (for API compatibility - uses full key verification). */
    bool VerifySchnorr(const uint256& msg, const unsigned char* sigbytes, size_t sig_len) const;

    /** Check if the full key has the expected x-only key and parity. */
    bool CheckTapTweak(const QXOnlyPubKey& internal, const uint256& merkle_root, bool parity) const;

    /** Construct a full CQPubKey with the even parity (API compatibility). */
    CQPubKey GetEvenCorrespondingCQPubKey() const;

    const unsigned char& operator[](int pos) const { return *(m_keydata.begin() + pos); }
    static constexpr size_t size() { return 32; }
    const unsigned char* data() const { return m_keydata.begin(); }
    const unsigned char* begin() const { return m_keydata.begin(); }
    const unsigned char* end() const { return m_keydata.end(); }

    bool operator==(const QXOnlyPubKey& other) const { return m_keydata == other.m_keydata; }
    bool operator!=(const QXOnlyPubKey& other) const { return m_keydata != other.m_keydata; }
    bool operator<(const QXOnlyPubKey& other) const { return m_keydata < other.m_keydata; }

    //! Serialization support
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s << m_keydata;
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        s >> m_keydata;
    }
};

/** QKeyPair - wrapper for advanced quantum-resistant key operations */
class QKeyPair
{
public:
    static constexpr size_t SIZE = DILITHIUM_SECRETKEY_SIZE;

private:
    secure_unique_ptr<std::array<unsigned char, SIZE>> m_keypair;

    void MakeKeyPairData()
    {
        if (!m_keypair) m_keypair = make_secure_unique<std::array<unsigned char, SIZE>>();
    }

    void ClearKeyPairData()
    {
        m_keypair.reset();
    }

public:
    QKeyPair() = default;
    QKeyPair(QKeyPair&&) = default;
    QKeyPair& operator=(QKeyPair&&) = default;
    QKeyPair(const QKeyPair&) = delete;
    QKeyPair& operator=(const QKeyPair&) = delete;

    //! Construct from secret key data
    QKeyPair(const unsigned char* seckey, size_t key_len);

    //! Sign a message using the keypair
    [[nodiscard]] bool SignSchnorr(const uint256& hash, unsigned char* sig, size_t sig_len, const uint256& aux) const;

    //! Check if keypair is valid
    bool IsValid() const { return !!m_keypair; }
};

/** Extended key structures for BIP32 compatibility */
struct CQExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CQKey key;

    friend bool operator==(const CQExtKey& a, const CQExtKey& b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(a.vchFingerprint, b.vchFingerprint, sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            a.key == b.key;
    }

    CQExtKey() = default;

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    [[nodiscard]] bool Derive(CQExtKey& out, unsigned int nChild) const;
    void SetSeed(const std::byte* seed, size_t seed_len);
};

struct CQExtPubKey {
    unsigned char version[4];
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CQPubKey pubkey;

    friend bool operator==(const CQExtPubKey &a, const CQExtPubKey &b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(a.vchFingerprint, b.vchFingerprint, sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            memcmp(a.pubkey.data(), b.pubkey.data(), a.pubkey.size()) == 0;
    }

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    void DecodeWithVersion(const unsigned char code[BIP32_EXTKEY_WITH_VERSION_SIZE]);
    [[nodiscard]] bool Derive(CQExtPubKey& out, unsigned int nChild) const;
};

bool QBTC_InitSanityCheck();

#endif // QBITCOIN_QKEY_H 