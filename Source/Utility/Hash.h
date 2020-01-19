// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"

typedef uint64_t hash64_t;

inline constexpr uint64_t WangMix64(uint64_t n)
{
    n = ~n + (n << 21);
    n =  n ^ (n >> 24);
    n =  n + (n << 3) + (n << 8);
    n =  n ^ (n >> 14);
    n =  n + (n << 2) + (n << 4);
    n =  n ^ (n >> 28);
    n =  n + (n << 31);
         
    return n;
}

template<int A, int B, int C>
inline constexpr uint64_t Xorshift64(uint64_t n)
{
    n ^= (n << A);
    n ^= (n >> B);
    n ^= (n << C);

    return n;
}

inline uint64_t HashString64(const char* str)
{
    // This function keeps 128 bits of internal state. The seed
    // values are arbitrary, but should have [roughly] the same 
    // number of zeroes and ones to kickstart the avalache. The
    // mix function used to generate them is constexpr, and can 
    // be evaluated at compile time.

    uint64_t a = WangMix64('hash');
    uint64_t b = WangMix64('init');

    while (*str)
    {
        const uint64_t FNV1A_PRIME = 0x100000001B3ULL;
        uint64_t fnv = ((a - b) ^ *str++) * FNV1A_PRIME;

        // The Xorshift parameters below are from Marsaglia's paper
        // (in the Reference folder) and have a period of 2^64-1. 
        // The two calculations are independent and should pipeline well.

        a ^= Xorshift64<13,  7, 17>(fnv);
        b ^= Xorshift64<49, 15, 61>(fnv);
    }

    return (a ^ b);
}


