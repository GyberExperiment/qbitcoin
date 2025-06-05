#ifndef SIMPLE_UINT256_H
#define SIMPLE_UINT256_H

#include <cstdint>
#include <vector>
#include <array>
#include <functional>

/**
 * Simplified uint256 implementation for aggregation prototype
 * Does not require the full QBTC codebase dependencies
 */
class uint256 {
private:
    std::array<uint64_t, 4> data;

public:
    uint256() : data{0, 0, 0, 0} {}
    
    uint256(uint64_t val) : data{val, 0, 0, 0} {}
    
    uint256(const uint256& other) : data(other.data) {}
    
    uint256& operator=(const uint256& other) {
        data = other.data;
        return *this;
    }
    
    // Basic operations needed for aggregation prototype
    uint64_t GetUint64(int index) const {
        if (index >= 0 && index < 4) {
            return data[index];
        }
        return 0;
    }
    
    unsigned char* begin() {
        return reinterpret_cast<unsigned char*>(data.data());
    }
    
    const unsigned char* begin() const {
        return reinterpret_cast<const unsigned char*>(data.data());
    }
    
    unsigned char* end() {
        return begin() + 32;
    }
    
    const unsigned char* end() const {
        return begin() + 32;
    }
    
    size_t size() const {
        return 32;
    }
    
    // Generate a simple hash for testing
    static uint256 FromString(const std::string& str) {
        uint256 result;
        std::hash<std::string> hasher;
        size_t hash_val = hasher(str);
        result.data[0] = hash_val;
        return result;
    }
    
    // Comparison operators
    bool operator==(const uint256& other) const {
        return data == other.data;
    }
    
    bool operator!=(const uint256& other) const {
        return !(*this == other);
    }
    
    // Convert to string for debugging
    std::string ToString() const {
        std::string result;
        for (int i = 0; i < 32; ++i) {
            unsigned char byte = begin()[i];
            result += "0123456789abcdef"[byte >> 4];
            result += "0123456789abcdef"[byte & 0xF];
        }
        return result;
    }
};

#endif // SIMPLE_UINT256_H 