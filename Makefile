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
               src/util/translation_qbtc.cpp
QBTC_OBJECTS = $(QBTC_SOURCES:.cpp=.o)

# Test executable
TEST_TARGET = test_qbtc_basic
TEST_SOURCES = test_qbtc_basic.cpp

# Script engine objects
SCRIPT_OBJS = src/script/script.o src/script/interpreter.o src/primitives/transaction.o \
             src/hash.o src/uint256.o src/serialize.o

# Default target
all: test_qbtc_basic test_script_integration

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
$(TEST_TARGET): $(QBTC_OBJECTS) $(TEST_SOURCES) $(DILITHIUM_LIB)
	@echo "Building QBTC test..."
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) $(QBTC_OBJECTS) -Wl,-force_load,$(DILITHIUM_LIB) -o $(TEST_TARGET) $(LDFLAGS)
	@echo "✅ QBTC test built successfully!"

# Run tests
test: $(TEST_TARGET)
	@echo "Running QBTC tests..."
	./$(TEST_TARGET)

# Build script integration test
test_script_integration: $(QBTC_OBJECTS) $(SCRIPT_OBJS) test_script_integration.cpp
	@echo "Building QBTC script integration test..."
	$(CXX) $(CXXFLAGS) test_script_integration.cpp $(QBTC_OBJECTS) $(SCRIPT_OBJS) $(LDFLAGS) -o test_script_integration
	@echo "✅ QBTC script integration test built successfully!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(QBTC_OBJECTS) $(SCRIPT_OBJS)
	rm -f $(TEST_TARGET) test_script_integration
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