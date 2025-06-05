# QBTC - Quantum-Resistant Bitcoin

**QBTC** is a quantum-resistant cryptocurrency based on Bitcoin Core, replacing ECDSA secp256k1 with post-quantum CRYSTALS-Dilithium3 cryptography.

## 🔐 Quantum-Resistant Features

- **Post-Quantum Cryptography**: Uses NIST-standardized CRYSTALS-Dilithium3
- **192-bit Security Level**: Equivalent to AES-192 
- **Quantum Attack Resistance**: Protects against Shor's algorithm
- **Lattice-Based Security**: Built on Learning With Errors problem

## 🚀 Key Improvements Over Bitcoin

| Component | Bitcoin | QBTC | Increase |
|-----------|---------|------|----------|
| Private Key | 32 bytes | 4,032 bytes | 126x |
| Public Key | 33 bytes | 1,952 bytes | 59x |
| Signature | ~72 bytes | 3,309 bytes | 46x |
| Hash Functions | SHA-256 (quantum-resistant) | SHA-256 (unchanged) | - |

## 🏗️ Architecture

### Core Classes

- **CQKey**: Quantum-resistant private keys using Dilithium
- **CQPubKey**: Quantum-resistant public keys using Dilithium
- **QKeyPair**: Advanced key operations for Schnorr-style signatures
- **QXOnlyPubKey**: Taproot-compatible x-only public keys
- **CQExtKey/CQExtPubKey**: BIP32 extended keys for hierarchical derivation

### Integrated Components

QBTC includes a complete implementation with:
- **CRYSTALS-Dilithium3**: Integrated post-quantum signature library
- **Bitcoin Core**: Modified quantum-resistant version
- **Build System**: Simplified Makefile for easy compilation
- **Test Suite**: Comprehensive cryptographic testing

### Project Structure

```
qbitcoin/
├── src/
│   ├── dilithium/          # Integrated CRYSTALS-Dilithium3 library
│   ├── key.h/key.cpp       # Quantum-resistant key implementation
│   ├── crypto/             # Cryptographic utilities
│   └── ...                 # Full Bitcoin Core codebase (modified)
├── test/                   # Test framework and test cases
├── Makefile               # Build system
├── test_qbtc_basic.cpp    # Demonstration test
└── README.md              # This file
```

### API Compatibility

QBTC maintains full API compatibility with Bitcoin Core:
- All existing Bitcoin function signatures preserved
- Seamless drop-in replacement for existing applications
- Graceful handling of unsupported operations (e.g., compact signatures)

## 🛠️ Building

### Prerequisites

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential cmake git
```

### Build Process

```bash
# Clone the repository
git clone https://github.com/GyberExperiment/qbitcoin.git
cd qbitcoin

# Install dependencies (Ubuntu/Debian)
make install-deps

# Build and test QBTC
make test
```

## 🔬 Cryptographic Details

### CRYSTALS-Dilithium3 Parameters

- **Algorithm**: FIPS 204 standardized lattice-based signatures
- **Security Level**: NIST Level 3 (192-bit equivalent)
- **Key Sizes**: 
  - Secret Key: 4,032 bytes
  - Public Key: 1,952 bytes
  - Signature: 3,293 bytes (average)

### Hash Functions

- **Primary**: SHA-256 (quantum-resistant)
- **Address**: RIPEMD-160 (quantum-resistant)
- **Merkle Trees**: SHA-256 (unchanged from Bitcoin)

## 🧪 Testing

```bash
# Build and run all tests
make test

# Just build without running
make

# Clean build artifacts
make clean

# Show build help
make help
```

### Expected Test Output

When you run `make test`, you should see output similar to:

```
=== QBTC - Quantum-Resistant Bitcoin Test ===

✅ QBTC sanity check passed

Generating quantum-resistant key pair...
✅ Private key generated (size: 4032 bytes)
✅ Public key derived (size: 1952 bytes)
✅ Test message created
✅ Message signed (signature size: 3309 bytes)
✅ Signature verified successfully

=== QBTC vs Bitcoin Key Size Comparison ===
Component        | Bitcoin    | QBTC       | Increase
-----------------|------------|------------|----------
Private Key      | 32 bytes   |   4032 bytes |  126x
Public Key       | 33 bytes   |   1952 bytes |   59x
Signature        | ~72 bytes  |   3309 bytes |   46x

🔐 QBTC provides post-quantum security using CRYSTALS-Dilithium3
🛡️ Protected against both classical and quantum computer attacks
⚛️ NIST Level 3 security (192-bit equivalent)

✅ All tests passed! QBTC is working correctly.
```

## 📈 Performance Considerations

- **Transaction Size**: ~46x larger signatures impact block size
- **Verification Time**: Dilithium verification is slower than ECDSA
- **Storage Requirements**: Significant increase in blockchain size
- **Network Bandwidth**: Higher data transmission requirements

## 🔒 Security Guarantees

- **Post-Quantum Security**: Resistant to both classical and quantum attacks
- **NIST Standardized**: Uses officially approved algorithms
- **Conservative Parameters**: Dilithium3 provides high security margins
- **Future-Proof**: Designed for long-term resistance to quantum computers

## 🌐 Network Protocol

QBTC extends Bitcoin's protocol to handle larger cryptographic primitives:
- Increased message size limits
- Optimized serialization for large keys/signatures
- Backwards compatibility considerations

## 🎯 Roadmap

- [x] Core cryptographic implementation
- [x] Key management system
- [x] Basic signing/verification
- [ ] Transaction system integration
- [ ] Network protocol updates
- [ ] Wallet interface development
- [ ] Performance optimization
- [ ] Mainnet deployment

## 🤝 Contributing

We welcome contributions to QBTC! Please see our contribution guidelines:

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Add tests for new functionality
5. Submit a pull request

## 📄 License

QBTC is released under the MIT License - see [LICENSE](LICENSE) for details.

## 🔗 References

- [CRYSTALS-Dilithium](https://pq-crystals.org/dilithium/)
- [NIST Post-Quantum Cryptography](https://csrc.nist.gov/projects/post-quantum-cryptography)
- [Bitcoin Core](https://github.com/bitcoin/bitcoin)
- [FIPS 204 Standard](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.204.pdf)

## ⚠️ Disclaimer

QBTC is experimental software. Do not use for production or mainnet transactions without thorough security review.

---

**QBTC: Securing cryptocurrency for the quantum age** 🛡️⚛️
