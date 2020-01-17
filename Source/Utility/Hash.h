// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"

typedef uint64_t STRINGHASH;

inline uint32_t WangMix32(uint32_t n)
{
    n = n + ~(n << 15);
    n = n ^  (n >> 10);
    n = n +  (n << 3);
    n = n ^  (n >> 6);
    n = n + ~(n << 11);
    n = n ^  (n >> 16);

    return n;
}

inline uint64_t WangMix64(uint64_t n)
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
inline uint64_t XorShift64(uint64_t n)
{
    n ^= (n << A);
    n ^= (n >> B);
    n ^= (n << C);

    return n;
}

inline uint64_t HashString64(const char* str)
{
    const uint64_t FRAC_SQRT_2  = 0x6A09E667F3BCC908ULL;
    const uint64_t FNV1A_OFFSET = 0xCBF29CE484222325ULL;
    const uint64_t FNV1A_PRIME  = 0x00000100000001B3ULL;

    uint64_t a = FNV1A_OFFSET;
    uint64_t b = FRAC_SQRT_2;

    while (*str)
    {
        a ^= *str++;
        a *= FNV1A_PRIME;
        a ^= b;
        b -= XorShift64<18, 31, 11>(a);
    }

    return (b);
}


