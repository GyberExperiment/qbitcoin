// Copyright (c) 2025 QBTC developers
// Basic test demonstrating QBTC quantum-resistant functionality

#include <iostream>
#include <vector>
#include <iomanip>

// External dilithium function declarations
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

int main() {
    std::cout << "=== QBTC - Quantum-Resistant Bitcoin Test ===" << std::endl;
    std::cout << std::endl;

    try {
        // Sizes from params.h for DILITHIUM_MODE=3
        const size_t pk_size = 1952;  // CRYPTO_PUBLICKEYBYTES
        const size_t sk_size = 4000;  // CRYPTO_SECRETKEYBYTES  
        const size_t sig_size = 3309; // CRYPTO_BYTES
        
        std::vector<unsigned char> pk(pk_size);
        std::vector<unsigned char> sk(sk_size);
        std::vector<unsigned char> sig(sig_size);
        
        std::cout << "Testing key generation..." << std::endl;
        int ret = qbtc_dilithium3_keypair(pk.data(), sk.data());
        if (ret != 0) {
            std::cout << "❌ Key generation failed: " << ret << std::endl;
            return 1;
        }
        std::cout << "✅ Key generation successful" << std::endl;
        
        // Test message
        std::string message = "Hello, quantum world!";
        
        std::cout << "Testing signature..." << std::endl;
        size_t siglen = sig_size;
        ret = qbtc_dilithium3_signature(sig.data(), &siglen,
                                        reinterpret_cast<const unsigned char*>(message.c_str()), 
                                        message.length(),
                                        nullptr, 0, sk.data());
        if (ret != 0) {
            std::cout << "❌ Signature failed: " << ret << std::endl;
            return 1;
        }
        std::cout << "✅ Signature successful (size: " << siglen << " bytes)" << std::endl;
        
        std::cout << "Testing verification..." << std::endl;
        ret = qbtc_dilithium3_verify(sig.data(), siglen,
                                     reinterpret_cast<const unsigned char*>(message.c_str()),
                                     message.length(),
                                     nullptr, 0, pk.data());
        if (ret != 0) {
            std::cout << "❌ Verification failed: " << ret << std::endl;
            return 1;
        }
        std::cout << "✅ Verification successful" << std::endl;

        // Display key statistics
        std::cout << std::endl << "=== QBTC vs Bitcoin Key Size Comparison ===" << std::endl;
        std::cout << "Component        | Bitcoin    | QBTC       | Increase" << std::endl;
        std::cout << "-----------------|------------|------------|----------" << std::endl;
        std::cout << "Private Key      | 32 bytes   | " << std::setw(6) << sk_size << " bytes | " 
                  << std::setw(4) << (sk_size / 32) << "x" << std::endl;
        std::cout << "Public Key       | 33 bytes   | " << std::setw(6) << pk_size << " bytes | " 
                  << std::setw(4) << (pk_size / 33) << "x" << std::endl;
        std::cout << "Signature        | ~72 bytes  | " << std::setw(6) << siglen << " bytes | " 
                  << std::setw(4) << (siglen / 72) << "x" << std::endl;

        std::cout << std::endl << "🔐 QBTC provides post-quantum security using CRYSTALS-Dilithium3" << std::endl;
        std::cout << "🛡️ Protected against both classical and quantum computer attacks" << std::endl;
        std::cout << "⚛️ NIST Level 3 security (192-bit equivalent)" << std::endl;

        std::cout << std::endl << "✅ All tests passed! QBTC is working correctly." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
} 