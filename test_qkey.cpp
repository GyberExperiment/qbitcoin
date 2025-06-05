// Test program for QBTC quantum-resistant keys
#include <iostream>
#include <iomanip>

#include "src/qkey.h"
#include "src/uint256.h"
#include "src/random.h"

int main() {
    std::cout << "=== QBTC Quantum-Resistant Key Test ===" << std::endl;
    
    try {
        // Generate a new quantum-resistant key pair
        std::cout << "Generating new QKey..." << std::endl;
        QKey key = GenerateRandomQKey();
        
        if (!key.IsValid()) {
            std::cerr << "Error: Failed to generate valid QKey" << std::endl;
            return 1;
        }
        
        std::cout << "✓ QKey generated successfully" << std::endl;
        std::cout << "Private key size: " << key.size() << " bytes" << std::endl;
        
        // Get corresponding public key
        std::cout << "Deriving public key..." << std::endl;
        QPubKey pubkey = key.GetPubKey();
        
        if (!pubkey.IsValid()) {
            std::cerr << "Error: Failed to derive valid QPubKey" << std::endl;
            return 1;
        }
        
        std::cout << "✓ QPubKey derived successfully" << std::endl;
        std::cout << "Public key size: " << pubkey.size() << " bytes" << std::endl;
        
        // Test signing and verification
        std::cout << "Testing signature..." << std::endl;
        
        // Create a test message hash
        uint256 test_hash;
        GetRandBytes(test_hash.begin(), 32);
        
        // Sign the hash
        std::vector<unsigned char> signature;
        bool sign_result = key.Sign(test_hash, signature);
        
        if (!sign_result) {
            std::cerr << "Error: Failed to create signature" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Signature created successfully" << std::endl;
        std::cout << "Signature size: " << signature.size() << " bytes" << std::endl;
        
        // Verify the signature
        bool verify_result = pubkey.Verify(test_hash, signature);
        
        if (!verify_result) {
            std::cerr << "Error: Signature verification failed" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Signature verified successfully" << std::endl;
        
        // Test verification with wrong message (should fail)
        uint256 wrong_hash;
        GetRandBytes(wrong_hash.begin(), 32);
        bool wrong_verify = pubkey.Verify(wrong_hash, signature);
        
        if (wrong_verify) {
            std::cerr << "Error: Signature verified with wrong message!" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Wrong message correctly rejected" << std::endl;
        
        // Display size comparison with Bitcoin
        std::cout << "\n=== Size Comparison with Bitcoin ===" << std::endl;
        std::cout << "Bitcoin secp256k1:" << std::endl;
        std::cout << "  Private key: 32 bytes" << std::endl;
        std::cout << "  Public key:  33 bytes (compressed)" << std::endl;
        std::cout << "  Signature:   ~72 bytes" << std::endl;
        
        std::cout << "\nQBTC Dilithium3:" << std::endl;
        std::cout << "  Private key: " << key.size() << " bytes" << std::endl;
        std::cout << "  Public key:  " << pubkey.size() << " bytes" << std::endl;
        std::cout << "  Signature:   " << signature.size() << " bytes" << std::endl;
        
        std::cout << "\nSize increase factors:" << std::endl;
        std::cout << "  Private key: " << std::fixed << std::setprecision(1) 
                  << (double)key.size() / 32.0 << "x" << std::endl;
        std::cout << "  Public key:  " << std::fixed << std::setprecision(1) 
                  << (double)pubkey.size() / 33.0 << "x" << std::endl;
        std::cout << "  Signature:   " << std::fixed << std::setprecision(1) 
                  << (double)signature.size() / 72.0 << "x" << std::endl;
        
        std::cout << "\n✅ All tests passed! QBTC quantum-resistant keys are working!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 