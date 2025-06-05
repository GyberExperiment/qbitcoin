# QBTC - Quantum-Resistant Bitcoin Makefile
# Simplified build system for demonstration

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -I. -Isrc
LDFLAGS = -Wl,-force_load,src/dilithium/libdilithium.a

# Dilithium library path
DILITHIUM_PATH = src/dilithium
DILITHIUM_LIB = $(DILITHIUM_PATH)/libdilithium.a

# Source files
QBTC_SOURCES = src/key.cpp \
               src/random.cpp \
               src/support/cleanse.cpp \
               src/support/lockedpool.cpp \
               src/crypto/sha256.cpp \
               src/crypto/sha512.cpp \
               src/logging.cpp \
               src/randomenv.cpp \
               src/crypto/sha256_sse4.cpp \
               src/crypto/chacha20.cpp \
               src/util/time.cpp \
               src/util/threadnames.cpp \
               src/util/fs.cpp \
               src/util/syserror.cpp \
               src/util/check.cpp \
               src/clientversion.cpp \
               src/util/translation_qbtc.cpp \
               src/dilithium/aggregation.cpp
QBTC_OBJECTS = $(QBTC_SOURCES:.cpp=.o)

# Test executable
TEST_TARGET = test_qbtc_basic
TEST_SOURCES = test_qbtc_basic.cpp

# Object files for QBTC
QBTC_OBJS = src/key.o src/random.o src/support/cleanse.o src/support/lockedpool.o \
           src/crypto/sha256.o src/crypto/sha512.o src/logging.o src/randomenv.o \
           src/crypto/sha256_sse4.o src/crypto/chacha20.o src/util/time.o \
           src/util/threadnames.o src/util/fs.o src/util/syserror.o src/util/check.o \
           src/clientversion.o src/util/translation_qbtc.o src/dilithium/aggregation.o

# Script engine objects
SCRIPT_OBJS = src/script/script.o src/script/interpreter.o src/primitives/transaction.o \
             src/hash.o src/uint256.o src/crypto/ripemd160.o src/crypto/hmac_sha512.o \
             src/pubkey.o src/util/strencodings.o src/crypto/sha1.o src/crypto/hex_base.o

# Basic objects for QBTC tests 
BASIC_OBJS = src/key.o src/random.o src/support/cleanse.o src/support/lockedpool.o \
             src/crypto/sha256.o src/crypto/sha512.o src/logging.o src/randomenv.o \
             src/crypto/sha256_sse4.o src/crypto/chacha20.o src/util/time.o \
             src/util/threadnames.o src/util/fs.o src/util/syserror.o src/util/check.o \
             src/clientversion.o src/util/translation_qbtc.o src/crypto/ripemd160.o \
             src/crypto/hmac_sha512.o

# Default target
all: test_qbtc_basic test_script_integration test_aggregation_production

# Build Dilithium library
$(DILITHIUM_LIB):
	@echo "Building CRYSTALS-Dilithium library..."
	@if [ -d "$(DILITHIUM_PATH)" ]; then \
		cd $(DILITHIUM_PATH) && ar rcs libdilithium.a *.o; \
	else \
		echo "❌ Dilithium directory not found. Please ensure Dilithium is in $(DILITHIUM_PATH)"; \
		exit 1; \
	fi

# Build QBTC objects
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test executable
test_qbtc_basic: test_qbtc_basic.cpp $(BASIC_OBJS)
	@echo "Building QBTC test..."
	$(CXX) $(CXXFLAGS) test_qbtc_basic.cpp $(BASIC_OBJS) $(LDFLAGS) -o test_qbtc_basic
	@echo "✅ QBTC test built successfully!"

# Build production aggregation test
test_aggregation_production: test_aggregation_integration.cpp $(QBTC_OBJECTS) $(SCRIPT_OBJS)
	@echo "Building QBTC Production Dilithium Aggregation Test..."
	$(CXX) $(CXXFLAGS) test_aggregation_integration.cpp $(QBTC_OBJECTS) $(SCRIPT_OBJS) $(LDFLAGS) -o test_aggregation_production
	@echo "✅ Production aggregation test built successfully!"

# Run tests
test: $(TEST_TARGET)
	@echo "Running QBTC tests..."
	./$(TEST_TARGET)

# Run production aggregation test
test-aggregation: test_aggregation_production
	@echo "Running QBTC Production Dilithium Aggregation Test..."
	./test_aggregation_production

# Build script integration test
test_script_integration: $(QBTC_OBJECTS) $(SCRIPT_OBJS) test_script_integration.cpp
	@echo "Building QBTC script integration test..."
	$(CXX) $(CXXFLAGS) test_script_integration.cpp $(QBTC_OBJECTS) $(SCRIPT_OBJS) $(LDFLAGS) -o test_script_integration
	@echo "✅ QBTC script integration test built successfully!"

# COMPRESSED QUANTUM KEYS INTEGRATION - боевая реализация с реальной secp256k1
test_compressed_quantum_keys: test_compressed_quantum_keys.cpp src/compressed_quantum_keys.cpp src/compressed_quantum_keys.h $(DILITHIUM_LIB)
	@echo "Building Compressed Quantum Keys test with real secp256k1..."
	$(CXX) -std=c++20 -O2 -I. -Isrc -Isrc/secp256k1/include \
		-DHAVE_CONFIG_H \
		test_compressed_quantum_keys.cpp \
		src/compressed_quantum_keys.cpp \
		src/dilithium/aggregation.cpp \
		src/key_original.cpp \
		src/key.cpp \
		src/pubkey.cpp \
		src/hash.cpp \
		src/random.cpp \
		src/util/strencodings.cpp \
		src/crypto/hex_base.cpp \
		src/bech32.cpp \
		src/script/script.cpp \
		src/primitives/transaction.cpp \
		src/crypto/hmac_sha512.cpp \
		src/crypto/chacha20.cpp \
		src/crypto/sha256.cpp \
		src/crypto/sha256_sse4.cpp \
		src/logging.cpp \
		src/support/cleanse.cpp \
		src/crypto/sha512.cpp \
		src/crypto/ripemd160.cpp \
		src/uint256.cpp \
		src/util/time.cpp \
		src/util/fs.cpp \
		src/util/syserror.cpp \
		src/util/threadnames.cpp \
		src/randomenv.cpp \
		src/util/check.cpp \
		src/clientversion.cpp \
		src/support/lockedpool.cpp \
		src/util/translation_qbtc.cpp \
		-Lsrc/secp256k1/build/lib -lsecp256k1 \
		-Wl,-force_load,src/dilithium/libdilithium.a \
		-o test_compressed_quantum_keys
	@echo "✅ Compressed Quantum Keys test built successfully with real secp256k1!"

# Production ready system build
build_quantum_system: src/compressed_quantum_keys.cpp src/compressed_quantum_keys.h
	@echo "🔧 Building complete Compressed Quantum Keys system..."
	$(CXX) -std=c++20 -O2 -I. -Isrc -c src/compressed_quantum_keys.cpp -o compressed_quantum_keys.o
	@echo "✅ Compressed Quantum Keys system compiled successfully"

# Clean quantum builds
clean_quantum:
	rm -f test_compressed_quantum_keys compressed_quantum_keys.o test_quantum_wallet.dat

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(QBTC_OBJECTS) $(SCRIPT_OBJS)
	rm -f $(TEST_TARGET) test_script_integration test_aggregation_production
	@echo "✅ Clean completed"

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing build dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential git cmake

# Help target
help:
	@echo "QBTC - Quantum-Resistant Bitcoin Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build everything (default)"
	@echo "  test         - Build and run tests"
	@echo "  clean        - Clean build artifacts"
	@echo "  install-deps - Install build dependencies (Ubuntu/Debian)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Example usage:"
	@echo "  make               # Build everything"
	@echo "  make test          # Build and test"
	@echo "  make clean         # Clean build"

.PHONY: all test clean install-deps help 