
/**************************************************************************************

 _BitScanForward(index, mask) - (VC) = __builtin_ctz(unsigned int i) - (gcc) = bsf eax - (asm)
 _BitScanReverse(index, mask) - (VC) = __builtin_clz(unsigned int i) - (gcc) = bsr eax - (asm)

  On ARM it would be the CLZ (count leading zeroes) instruction.

 See: https://msdn.microsoft.com/en-us/library/wfd9z0bb.aspx
 See: https://msdn.microsoft.com/en-us/library/fbxyd7zd.aspx

 See: http://www.cnblogs.com/gleam/p/5025867.html

 ¡ª int __builtin_ctz (unsigned int x);
   int __builtin_ctzll (unsigned long long x);

    Returns the number of trailing 0-bits in x, starting at the least significant bit position.
    If x is 0, the result is undefined.   (MSB)

 ¡ª int __builtin_clz (unsigned int x);
   int __builtin_clzll (unsigned long long x);

    Returns the number of leading 0-bits in x, starting at the most significant bit position.
    If x is 0, the result is undefined.   (LSB)

 See: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html

 ¡ª int __builtin_ffs (int x);
   int __builtin_ffsll (long long x);

    Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero. 

 See: https://blog.stephencleary.com/2010/10/implementing-gccs-builtin-functions.html
 See: http://www.cnblogs.com/miloyip/p/4517053.html

 int __builtin_ffs(int x) {
     if (x == 0)
         return 0;
     return __builtin_ctz((unsigned int)x) + 1;
 }

 How to use MSVC intrinsics to get the equivalent of this GCC code?

 See: https://stackoverflow.com/questions/355967/how-to-use-msvc-intrinsics-to-get-the-equivalent-of-this-gcc-code

***************************************************************************************/

#ifndef SM_BITSCAN_FORWARD_H
#define SM_BITSCAN_FORWARD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <assert.h>

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
#define __IS_X86_64      1
#endif // _WIN64

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#include <intrin.h>     // For _BitScanForward(), _BitScanForward64()
#pragma intrinsic(_BitScanForward)
#if __IS_X86_64
#pragma intrinsic(_BitScanForward64)
#endif // _WIN64
#endif // _MSC_VER

//
// See: http://www.cnblogs.com/zyl910/archive/2012/08/27/intrin_table_gcc.html
//
//#include <xmmintrin.h>    // For MMX, SSE instructions
//#include <emmintrin.h>    // For SSE2 instructions, __SSE2__ | -msse2
//#include <avxintrin.h>    // __AVX__  | -mavx     AVX:  Advanced Vector Extensions
//#include <avx2intrin.h>   // __AVX2__ | -mavx2    AVX2: Advanced Vector Extensions 2
//

/* __has_builtin() available in clang */
#ifdef __has_builtin
#  if __has_builtin(__builtin_ctz)
#    define __has_builtin_ctz
#  endif
#  if __has_builtin(__builtin_ctzll)
#    define __has_builtin_ctzll
#  endif
/* __builtin_ctz available beginning with GCC 3.4 */
#elif (__GNUC__ * 100 + __GNUC_MINOR__) >= 304
#  define __has_builtin_ctz
#  define __has_builtin_ctzll
#endif // __has_builtin

// Get the index of the first bit on set to 1.
#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)
// _MSC_VER

#define __BitScanForward(index, mask) \
        _BitScanForward((unsigned long *)&(index), (unsigned long)(mask))

#if __IS_X86_64
    #define __BitScanForward64(index, mask) \
            _BitScanForward64((unsigned long *)&(index), (unsigned long long)(mask))
#endif // __x86_64__

#elif (defined(__GNUC__) && ((__GNUC__ >= 4) \
   || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))) \
   || defined(__clang__) || defined(__MINGW32__) || defined(__CYGWIN__)
// __GNUC__

#define __BitScanForward(index, mask) \
        __builtin_BitScanForward((unsigned long *)&(index), (unsigned long)mask)

#if __IS_X86_64
    #define __BitScanForward64(index, mask) \
            __builtin_BitScanForward64((unsigned long *)&(index), (unsigned long long)mask)
#endif // __x86_64__

#else
    // Not support
    #define __BitScanForward(index, mask) \
            __builtin_BitScanForward((unsigned long *)&(index), (unsigned long)mask)

#if __IS_X86_64
    #define __BitScanForward64(index, mask) \
            __builtin_BitScanForward64((unsigned long *)&(index), (unsigned long long)mask)
#endif // __x86_64__

    // #error "The compiler does not support BitScanForward()."
#endif // BitScanForward()

#include "support/popcnt.h"

static inline
unsigned char
__builtin_BitScanForward(unsigned long * index, unsigned long mask)
{
    int trailing_zeros;
#if defined(__has_builtin_ctz) || defined(__linux__)
    trailing_zeros = __builtin_ctz((unsigned int)mask);
#else
    trailing_zeros = __internal_ctz((unsigned int)mask);
#endif
    assert(index != nullptr);
    *index = (unsigned long)trailing_zeros;
    return (unsigned char)(mask != 0);
}

#if __IS_X86_64

static inline
unsigned char
__builtin_BitScanForward64(unsigned long * index, unsigned long long mask)
{
    int trailing_zeros;
#if defined(__has_builtin_ctzll) || defined(__linux__)
    trailing_zeros = __builtin_ctzll((unsigned long long)mask);
#else
    trailing_zeros = __internal_ctzll((unsigned long long)mask);
#endif
    assert(index != nullptr);
    *index = (unsigned long)trailing_zeros;
    return (unsigned char)(mask != 0);
}

#endif // __IS_X86_64

#undef __IS_X86_64

#endif // SM_BITSCAN_FORWARD_H
