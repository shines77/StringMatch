
#ifndef SM_BUILTIN_POPCNT_H
#define SM_BUILTIN_POPCNT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"

static inline
unsigned int __native_popcnt(unsigned int x)
{
    unsigned int n;
    n = x - ((x >> 1) &  0x55555555UL);
    n = (n & 0x33333333UL) + ((n >>  2) & 0x33333333UL);
    n = (n & 0x0F0F0F0FUL) + ((n >>  4) & 0x0F0F0F0FUL);
    n = (n & 0x0000FFFFUL) +  (n >> 16);
    n = (n & 0x000000FFUL) +  (n >>  8);
    return (n & 0x0000003FUL);
}

static inline
unsigned int __native_popcnt_2(unsigned int x)
{ 
    x -=  ((x >> 1) & 0x55555555U);
    x  = (((x >> 2) & 0x33333333U) + (x & 0x33333333U));
    x  = (((x >> 4) + x) & 0x0F0F0F0FU);
    x +=   (x >> 8);
    x +=   (x >> 16);
    return (x & 0x0000003FU);
}

static inline
unsigned int __native_popcnt64(uint64_t x)
{ 
    unsigned int high, low;
    unsigned int n1, n2;
    high = (unsigned int) (x & 0x00000000FFFFFFFFULL);
    low  = (unsigned int)((x & 0xFFFFFFFF00000000ULL) >> 32);
    n1 = __native_popcnt(high);
    n2 = __native_popcnt(low);
    return (n1 + n2);
}

static inline
unsigned int __native_clz(unsigned int x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return (32 - __native_popcnt(x));
}

static inline
unsigned int __native_clzll(uint64_t x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return (64 - __native_popcnt64(x));
}

static inline
unsigned int __native_ctz(unsigned int x)
{
    return __native_popcnt((x & -(int)x) - 1);
}

static inline
unsigned int __native_ctzll(uint64_t x)
{
    return __native_popcnt64((x & -(int64_t)x) - 1);
}

#endif // SM_BUILTIN_POPCNT_H
