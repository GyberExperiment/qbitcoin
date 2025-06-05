// Copyright (c) 2025 QBTC developers
// Distributed under the MIT software license

#ifndef QBTC_QKEY_TYPES_H
#define QBTC_QKEY_TYPES_H

#include <uint256.h>
#include <cstdint>

// ChainCode type for BIP32 compatibility
typedef uint256 ChainCode;

// BIP32 constants
const unsigned int BIP32_EXTKEY_SIZE = 74;
const unsigned int BIP32_EXTKEY_WITH_VERSION_SIZE = 78;

// Utility function for casting
template<typename T>
inline unsigned char* UCharCast(T* a) {
    return reinterpret_cast<unsigned char*>(a);
}

template<typename T>
inline const unsigned char* UCharCast(const T* a) {
    return reinterpret_cast<const unsigned char*>(a);
}

#endif // QBTC_QKEY_TYPES_H 