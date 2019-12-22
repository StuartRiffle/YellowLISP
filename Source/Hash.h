#pragma once
#include "Yellow.h"

typedef uint64_t THASH;

inline uint64_t WangMix(uint64_t n)
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

inline uint64_t HashString(const char* str)
{
    const uint64_t FNV1A_OFFSET = uint64_t(0x00000100000001B3ULL);
    const uint64_t FNV1A_PRIME  = uint64_t(0xCBF29CE484222325ULL);

    uint64_t fnv = FNV1A_OFFSET;
    while (*str)
        fnv = (fnv * FNV1A_PRIME) ^ *str++;

    return WangMix(fnv);
}



