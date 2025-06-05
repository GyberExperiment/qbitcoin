// Copyright (c) 2025 QBTC developers
// Distributed under the MIT software license

#ifndef QBTC_QKEY_H
#define QBTC_QKEY_H

#include <hash.h>
#include <serialize.h>
#include <support/allocators/secure.h>
#include <uint256.h>
#include <span.h>
#include <qkey_types.h>
#include <dilithium/api.h>

#include <stdexcept>
#include <vector>
#include <array>
#include <optional>

/**
 * QPrivKey is a serialized quantum-resistant private key using Dilithium3
 */
typedef std::vector<unsigned char, secure_allocator<unsigned char> > QPrivKey;

/** Size of Dilithium3 keys and signatures */
constexpr static size_t DILITHIUM_PUBLICKEY_SIZE = pqcrystals_dilithium3_PUBLICKEYBYTES;
constexpr static size_t DILITHIUM_SECRETKEY_SIZE = pqcrystals_dilithium3_SECRETKEYBYTES;
constexpr static size_t DILITHIUM_SIGNATURE_SIZE = pqcrystals_dilithium3_BYTES;

// Forward declarations
class QPubKey;
class QKeyPair;

/** A reference to a QKey: the Hash160 of its serialized public key */
class QKeyID : public uint160
{
public:
    QKeyID() : uint160() {}
    explicit QKeyID(const uint160& in) : uint160(in) {}
};

/** An encapsulated quantum-resistant private key using Dilithium. */
class QKey
{
public:
    /**
     * Dilithium3 key sizes:
     */
    static const unsigned int SIZE = DILITHIUM_SECRETKEY_SIZE;
    static const unsigned int COMPRESSED_SIZE = DILITHIUM_SECRETKEY_SIZE; // Always "compressed"

private:
    /** Internal data container for private key material. */
    using KeyType = std::array<unsigned char, DILITHIUM_SECRETKEY_SIZE>;

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
    QKey() noexcept = default;
    QKey(QKey&&) noexcept = default;
    QKey& operator=(QKey&&) noexcept = default;

    QKey& operator=(const QKey& other)
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

    QKey(const QKey& other) { *this = other; }

    friend bool operator==(const QKey& a, const QKey& b)
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
     * Convert the private key to a QPrivKey (serialized private key data).
     */
    QPrivKey GetPrivKey() const;

    /**
     * Compute the public key from a private key using Dilithium.
     */
    QPubKey GetPubKey() const;

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
     * Verify thoroughly whether a private key and a public key match.
     */
    bool VerifyPubKey(const QPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(const QPrivKey& privkey, const QPubKey& vchPubKey, bool fSkipCheck);

    /**
     * Compute a QKeyPair (for advanced signature operations)
     */
    QKeyPair ComputeKeyPair() const;

    //! BIP32 key derivation (simplified for quantum-resistant keys)
    [[nodiscard]] bool Derive(QKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;
};

QKey GenerateRandomQKey(bool compressed = true) noexcept;

/** Quantum-resistant public key using Dilithium. */
class QPubKey
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
    QPubKey()
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
    QPubKey(const T pbegin, const T pend)
    {
        Set(pbegin, pend);
    }

    //! Construct a public key from a byte vector.
    explicit QPubKey(std::span<const uint8_t> _vch)
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
    friend bool operator==(const QPubKey& a, const QPubKey& b)
    {
        return memcmp(a.vch, b.vch, SIZE) == 0;
    }
    friend bool operator!=(const QPubKey& a, const QPubKey& b)
    {
        return !(a == b);
    }
    friend bool operator<(const QPubKey& a, const QPubKey& b)
    {
        return memcmp(a.vch, b.vch, SIZE) < 0;
    }
    friend bool operator>(const QPubKey& a, const QPubKey& b)
    {
        return memcmp(a.vch, b.vch, SIZE) > 0;
    }

    //! Implement serialization, as if this was a byte vector.
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        unsigned int len = size();
        ::WriteCompactSize(s, len);
        s << std::span{vch, len};
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        const unsigned int len(::ReadCompactSize(s));
        if (len == SIZE) {
            s >> std::span{vch, len};
        } else {
            // invalid pubkey, skip available data
            s.ignore(len);
            Invalidate();
        }
    }

    //! Get the KeyID of this public key (hash of its serialization)
    QKeyID GetID() const
    {
        return QKeyID(Hash160(std::span{vch, SIZE}));
    }

    //! Get the 256-bit hash of this public key.
    uint256 GetHash() const
    {
        return Hash(std::span{vch, SIZE});
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
    [[nodiscard]] bool Derive(QPubKey& pubkeyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;

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
    constexpr QXOnlyPubKey() = default;

    /** Test whether this is the 0 key (the result of default construction). */
    bool IsNull() const { return m_keydata.IsNull(); }

    /** Construct an x-only pubkey from exactly 32 bytes (hash of full Dilithium key). */
    constexpr explicit QXOnlyPubKey(std::span<const unsigned char> bytes) : m_keydata{bytes} {}

    /** Construct an x-only pubkey from a full QPubKey (takes hash of the key). */
    explicit QXOnlyPubKey(const QPubKey& pubkey);

    /** Verify a signature (for API compatibility - uses full key verification). */
    bool VerifySchnorr(const uint256& msg, std::span<const unsigned char> sigbytes) const;

    /** Check if the full key has the expected x-only key and parity. */
    bool CheckTapTweak(const QXOnlyPubKey& internal, const uint256& merkle_root, bool parity) const;

    /** Construct a full QPubKey with the even parity (API compatibility). */
    QPubKey GetEvenCorrespondingQPubKey() const;

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
    QKeyPair(std::span<const unsigned char> seckey);

    //! Sign a message using the keypair
    [[nodiscard]] bool SignSchnorr(const uint256& hash, std::span<unsigned char> sig, const uint256& aux) const;

    //! Check if keypair is valid
    bool IsValid() const { return !!m_keypair; }
};

/** Extended key structures for BIP32 compatibility */
struct QExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    QKey key;

    friend bool operator==(const QExtKey& a, const QExtKey& b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(a.vchFingerprint, b.vchFingerprint, sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            a.key == b.key;
    }

    QExtKey() = default;

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    [[nodiscard]] bool Derive(QExtKey& out, unsigned int nChild) const;
    void SetSeed(std::span<const std::byte> seed);
};

struct QExtPubKey {
    unsigned char version[4];
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    QPubKey pubkey;

    friend bool operator==(const QExtPubKey &a, const QExtPubKey &b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(a.vchFingerprint, b.vchFingerprint, sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            a.pubkey == b.pubkey;
    }

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    void DecodeWithVersion(const unsigned char code[BIP32_EXTKEY_WITH_VERSION_SIZE]);
    [[nodiscard]] bool Derive(QExtPubKey& out, unsigned int nChild) const;
};

//! Initialize quantum-resistant cryptography sanity checks
bool QCC_InitSanityCheck();

#endif // QBTC_QKEY_H 