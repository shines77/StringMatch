
#ifndef SM_BUILTIN_POPCNT_H
#define SM_BUILTIN_POPCNT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

//
// popcount() algorithm
//
// See: http://www.cnblogs.com/Martinium/articles/popcount.html
// See: https://en.wikipedia.org/wiki/Hamming_weight
// See: https://stackoverflow.com/questions/757059/position-of-least-significant-bit-that-is-set
//

static inline
unsigned int __internal_popcnt(unsigned int x)
{ 
    x -=  ((x >> 1U) & 0x55555555U);
    x  = (((x >> 2U) & 0x33333333U) + (x & 0x33333333U));
    x  = (((x >> 4U) + x) & 0x0F0F0F0FU);
    x +=   (x >> 8U);
    x +=   (x >> 16U);
    x  = x & 0x0000003FU;
    assert(x >= 0 && x <= 32);
    return x;
}

static inline
unsigned int __internal_popcnt_slow(unsigned int x)
{
    x = (x & 0x55555555UL) + ((x >>  1U) & 0x55555555UL);
    x = (x & 0x33333333UL) + ((x >>  2U) & 0x33333333UL);
    x = (x & 0x0F0F0F0FUL) + ((x >>  4U) & 0x0F0F0F0FUL);
    x = (x & 0x00FF00FFUL) + ((x >>  8U) & 0x00FF00FFUL);
    x = (x & 0x0000FFFFUL) + ((x >> 16U) & 0x0000FFFFUL);
    assert(x >= 0 && x <= 32);
    return x;
}

static inline
unsigned int __internal_hakmem_popcnt(unsigned int x)
{
    unsigned int tmp;
    tmp = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);
    return (((tmp + (tmp >> 3)) & 030707070707) % 63U);
}

static inline
unsigned int __internal_popcnt64(uint64_t x)
{
#if 1
    x -=  ((x >> 1U) & 0x55555555U);
    x  = (((x >> 2U) & 0x33333333U) + (x & 0x33333333U));
    x  = (((x >> 4U) + x) & 0x0F0F0F0FU);
    x +=   (x >> 8U);
    x +=   (x >> 16U);
    x +=   (x >> 32U);
    x  = x & 0x0000007FU;
    assert(x >= 0 && x <= 64);
    return (unsigned int)x;
#elif 0
    x = (x & 0x5555555555555555ULL) + ((x >>  1U) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >>  2U) & 0x3333333333333333ULL);
    x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >>  4U) & 0x0F0F0F0F0F0F0F0FULL);
    x = (x & 0x00FF00FF00FF00FFULL) + ((x >>  8U) & 0x00FF00FF00FF00FFULL);
    x = (x & 0x0000FFFF0000FFFFULL) + ((x >> 16U) & 0x0000FFFF0000FFFFULL);
    x = (x & 0x00000000FFFFFFFFULL) + ((x >> 32U) & 0x00000000FFFFFFFFULL);
    assert(x >= 0 && x <= 64);
    return (unsigned int)x;
#else
    unsigned int high, low;
    unsigned int n1, n2;
    high = (unsigned int) (x & 0x00000000FFFFFFFFULL);
    low  = (unsigned int)((x & 0xFFFFFFFF00000000ULL) >> 32U);
    n1 = __internal_popcnt(high);
    n2 = __internal_popcnt(low);
    return (n1 + n2);
#endif
}

static inline
int __internal_clz(unsigned int x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return (int)(32U - __internal_popcnt(x));
}

static inline
int __internal_clzll(uint64_t x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return (int)(64U - __internal_popcnt64(x));
}

static inline
int __internal_ctz(unsigned int x)
{
    return (int)__internal_popcnt((x & -(int)x) - 1);
}

static inline
int __internal_ctzll(uint64_t x)
{
    return (int)__internal_popcnt64((x & -(int64_t)x) - 1);
}

#endif // SM_BUILTIN_POPCNT_H
