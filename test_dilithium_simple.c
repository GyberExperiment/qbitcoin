#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/dilithium/api.h"

// External wrapper function declarations
extern int qbtc_dilithium3_keypair(unsigned char *pk, unsigned char *sk);
extern int qbtc_dilithium3_signature(unsigned char *sig, size_t *siglen,
                                      const unsigned char *m, size_t mlen,
                                      const unsigned char *ctx, size_t ctxlen,
                                      const unsigned char *sk);
extern int qbtc_dilithium3_verify(const unsigned char *sig, size_t siglen,
                                   const unsigned char *m, size_t mlen,
                                   const unsigned char *ctx, size_t ctxlen,
                                   const unsigned char *pk);

int main() {
    printf("=== Simple Dilithium Test ===\n");
    
    // Sizes from params.h for DILITHIUM_MODE=3
    const size_t pk_size = 1952;  // CRYPTO_PUBLICKEYBYTES
    const size_t sk_size = 4000;  // CRYPTO_SECRETKEYBYTES  
    const size_t sig_size = 3309; // CRYPTO_BYTES
    
    unsigned char *pk = malloc(pk_size);
    unsigned char *sk = malloc(sk_size);
    unsigned char *sig = malloc(sig_size);
    
    if (!pk || !sk || !sig) {
        printf("❌ Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing key generation...\n");
    int ret = qbtc_dilithium3_keypair(pk, sk);
    if (ret != 0) {
        printf("❌ Key generation failed: %d\n", ret);
        return 1;
    }
    printf("✅ Key generation successful\n");
    
    // Test message
    const char *message = "Hello, quantum world!";
    size_t msg_len = strlen(message);
    
    printf("Testing signature...\n");
    size_t siglen = sig_size;
    ret = qbtc_dilithium3_signature(sig, &siglen, 
                                    (unsigned char*)message, msg_len,
                                    NULL, 0, sk);
    if (ret != 0) {
        printf("❌ Signature failed: %d\n", ret);
        return 1;
    }
    printf("✅ Signature successful (size: %zu bytes)\n", siglen);
    
    printf("Testing verification...\n");
    ret = qbtc_dilithium3_verify(sig, siglen,
                                 (unsigned char*)message, msg_len,
                                 NULL, 0, pk);
    if (ret != 0) {
        printf("❌ Verification failed: %d\n", ret);
        return 1;
    }
    printf("✅ Verification successful\n");
    
    free(pk);
    free(sk);
    free(sig);
    
    printf("🎉 All tests passed!\n");
    return 0;
} 