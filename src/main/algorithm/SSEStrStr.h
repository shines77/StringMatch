
#ifndef STRING_MATCH_SSE_STRSTR_H
#define STRING_MATCH_SSE_STRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/**************************************************

#include <mmintrin.h>   // MMX
#include <xmmintrin.h>  // SSE      (include mmintrin.h)
#include <emmintrin.h>  // SSE 2    (include xmmintrin.h)
#include <pmmintrin.h>  // SSE 3    (include emmintrin.h)
#include <tmmintrin.h>  // SSSE 3   (include pmmintrin.h)
#include <smmintrin.h>  // SSE 4.1  (include tmmintrin.h)
#include <nmmintrin.h>  // SSE 4.2  (include smmintrin.h)
#include <wmmintrin.h>  // AES      (include nmmintrin.h)
#include <immintrin.h>  // AVX      (include wmmintrin.h)
#include <intrin.h>     //          (include immintrin.h)

**************************************************/

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <limits.h>
#include <assert.h>
#include <nmmintrin.h>  // For SSE 4.2
#include <type_traits>

#ifndef __cplusplus
#include <stdalign.h>   // C11 defines _Alignas().  This header defines alignas()
#endif

#include "StringMatch.h"
#include "AlgorithmWrapper.h"

#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)
#include <intrin.h>     // For _BitScanReverse() & _BitScanReverse64()
#endif

namespace StringMatch {

// _BitScanReverse()

#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)

// Do nothing!! in MSVC or Intel C++ Compiler,
// _BitScanReverse() defined in <intrin.h>
#pragma intrinsic(_BitScanReverse)

#elif defined(__MINGW32__) || defined(__has_builtin_clz) || (__GNUC__ >= 4)
SM_INLINE_DECLARE(unsigned char)
__SM_CDECL
_BitScanReverse(unsigned long *firstBit1Index, unsigned long scanNum)
{
    unsigned char isNonzero;
    assert(firstBit1Index != NULL);
#if !defined(__has_builtin_clz) && 0
    __asm__ __volatile__ (
        "bsrl  %2, %%edx    \n\t"
        "movl  %%edx, (%1)  \n\t"
        "setnz %0           \n\t"
        : "=a"(isNonzero)
        : "r"(firstBit1Index), "r"(scanNum)
        : "edx", "memory"
    );
#else
    isNonzero = (unsigned char)scanNum;
    if (scanNum != 0) {
        // countLeadingZeros()
        size_t index = __builtin_clz(scanNum);
        *firstBit1Index = index ^ 31;
    }
    else {
        *firstBit1Index = 0;
    }
#endif
    return isNonzero;
}
#else
SM_INLINE_DECLARE(unsigned char)
__SM_CDECL
_BitScanReverse(unsigned long *firstBit1Index, unsigned long scanNum)
{
    assert(firstBit1Index == NULL);
    *firstBit1Index = 1;
    return 1;
}
#endif // in MSVC or Intel C++ Compiler

// _BitScanReverse64()

#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)

// Do nothing!! in MSVC or Intel C++ Compiler,
// _BitScanReverse64() defined in <intrin.h>
#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) \
    || defined(_M_ARM) || defined(_M_ARM64) \
    || defined(__amd64__) || defined(__x86_64__)
#pragma intrinsic(_BitScanReverse64)
#endif

#elif (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) \
    || defined(_M_ARM) || defined(_M_ARM64) \
    || defined(__amd64__) || defined(__x86_64__)) \
   && (defined(__MINGW32__) || defined(__has_builtin_clzll) || (__GNUC__ >= 4))
SM_INLINE_DECLARE(unsigned char)
__SM_CDECL
_BitScanReverse64(unsigned long * firstBit1Index, uint64_t scanNum)
{
    unsigned char isNonzero;
    size_t index;
    assert(firstBit1Index == NULL);
    isNonzero = (unsigned char)(scanNum != 0);
    if (scanNum != 0) {
        index = __builtin_clzll(scanNum);
        *firstBit1Index = index ^ 63;
    }
    else {
        *firstBit1Index = 0;
    }
    return isNonzero;
}
#else
SM_INLINE_DECLARE(unsigned char)
__SM_CDECL
_BitScanReverse64(unsigned long *firstBit1Index, unsigned long scanNum)
{
    assert(firstBit1Index == NULL);
    *firstBit1Index = 1;
    return 1;
}
#endif // in MSVC or Intel C++ Compiler

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
sse42_strstr(const char_type * text, const char_type * pattern,
             typename std::enable_if<detail::is_char8<char_type>::value, char_type>::type * = nullptr) {
    assert(text != nullptr);
    assert(pattern != nullptr);
    static const int kMaxSize = 16;
    static const int mode_ordered = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ORDERED
                                  | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int mode_each = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH
                               | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int mode_any = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY
                              | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, null;
#if defined(NDEBUG)
    (void)(__zero);
#else
    __zero = { 0 };
#endif
    __zero = _mm_xor_si128(__zero, __zero);

    // Check the length of pattern is less than 16?
    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    __pattern = _mm_loadu_si128((const __m128i *)pattern);
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);
    
    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (mask_128i[0] != 0 || mask_128i[1] != 0) {
        // The length of pattern is less than 16.
        text -= kMaxSize;
        do {
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                null = _mm_cmpestri(__zero, kMaxSize, __text, kMaxSize, mode_each);
            } while (offset >= kMaxSize && null >= kMaxSize);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                if (likely(offset >= kMaxSize)) {
                    if (likely(null >= kMaxSize)) {
                        //text += kMaxSize;
                        continue;
                    }
                    else {
                        break;
                    }
                }
                else {
                    // Has found
                    return (text + offset);
                }
            }
        } while (1);
    }
    else {
        // The length of pattern is more than or equal 16.

        // Get the length of pattern
        int pattern_len;
        {
            __m128i __patt;
            const char_type * patt = pattern;
            pattern_len = 0;

            do {
                patt += kMaxSize;
                __patt = _mm_loadu_si128((const __m128i *)patt);
                null = _mm_cmpestri(__zero, kMaxSize, __patt, kMaxSize, mode_each);
                pattern_len += kMaxSize;
            } while (null >= kMaxSize);

            assert(null >= 0 && null < kMaxSize);
            pattern_len += null;
        }

        text -= kMaxSize;
        do {
STRSTR_MAIN_LOOP:
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                null = _mm_cmpestri(__zero, 16, __text, 16, mode_each);
            } while (offset >= kMaxSize && null >= kMaxSize);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < 16);
                text += offset;
                if (likely(null >= kMaxSize)) {
                    int rest_len = pattern_len;
                    const char_type * patt = pattern;
                    do {
                        __text = _mm_loadu_si128((const __m128i *)text);
                        offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                        if (likely(offset >= kMaxSize)) {
                            //text += kMaxSize;
                            goto STRSTR_MAIN_LOOP;
                        }
                        else {
                            if (likely(offset == 0)) {
                                // Scan the next part pattern
                                text += kMaxSize;
                                rest_len -= kMaxSize;
                                if (likely(rest_len > 0)) {
                                    patt += kMaxSize;
                                    __pattern = _mm_loadu_si128((const __m128i *)patt);
                                }
                                else {
                                    break;
                                }
                            }
                            else {
                                // Reset the pattern
                                patt = pattern;
                                __pattern = _mm_loadu_si128((const __m128i *)patt);
                                text += offset;
                                rest_len = pattern_len;
                            }
                        }
                    } while (1);

                    assert(rest_len <= 0);
                    return (text - pattern_len + rest_len);
                }
                else {
                    __text = _mm_loadu_si128((const __m128i *)text);
                    offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                    if (likely(offset >= kMaxSize)) {
                        break;
                    }
                    else {
                        // Has found
                        return (text + offset);
                    }
                }
            }
        } while (1);
    }

    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
sse42_strstr(const char_type * text, const char_type * pattern,
             typename std::enable_if<detail::is_wchar<char_type>::value, char_type>::type * = nullptr) {
    assert(text != nullptr);
    assert(pattern != nullptr);
    static const int kMaxSize = 8;
    static const int mode_ordered = _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ORDERED
                                  | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int mode_each = _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_EACH
                               | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int mode_any = _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
                              | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, null;

#if defined(NDEBUG)
    (void)(__zero);
#else
    __zero = { 0 };
#endif
    __zero = _mm_xor_si128(__zero, __zero);

    // Check the length of pattern is less than 16?
    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    __pattern = _mm_loadu_si128((const __m128i *)pattern);
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);

    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (mask_128i[0] != 0 || mask_128i[1] != 0) {
        // The length of pattern is less than 16.
        text -= kMaxSize;
        do {
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                null = _mm_cmpestri(__zero, kMaxSize, __text, kMaxSize, mode_each);
            } while (offset >= kMaxSize && null >= kMaxSize);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                if (likely(offset >= kMaxSize)) {
                    if (likely(null >= kMaxSize)) {
                        //text += kMaxSize;
                        continue;
                    }
                    else {
                        break;
                    }
                }
                else {
                    // Has found
                    return (text + offset);
                }
            }
        } while (1);
    }
    else {
        // The length of pattern is more than or equal 16.

        // Get the length of pattern
        int pattern_len;
        {
            __m128i __patt;
            const char_type * patt = pattern;
            pattern_len = 0;

            do {
                patt += kMaxSize;
                __patt = _mm_loadu_si128((const __m128i *)patt);
                null = _mm_cmpestri(__zero, kMaxSize, __patt, kMaxSize, mode_each);
                pattern_len += kMaxSize;
            } while (null >= kMaxSize);

            assert(null >= 0 && null < kMaxSize);
            pattern_len += null;
        }

        text -= kMaxSize;
        do {
STRSTR_MAIN_LOOP:
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                null = _mm_cmpestri(__zero, kMaxSize, __text, kMaxSize, mode_each);
            } while (offset >= kMaxSize && null >= kMaxSize);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < kMaxSize);
                text += offset;
                if (likely(null >= kMaxSize)) {
                    int rest_len = pattern_len;
                    const char_type * patt = pattern;
                    do {
                        __text = _mm_loadu_si128((const __m128i *)text);
                        offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                        if (likely(offset >= kMaxSize)) {
                            //text += kMaxSize;
                            goto STRSTR_MAIN_LOOP;
                        }
                        else {
                            if (likely(offset == 0)) {
                                // Scan the next part pattern
                                text += kMaxSize;
                                rest_len -= kMaxSize;
                                if (likely(rest_len > 0)) {
                                    patt += kMaxSize;
                                    __pattern = _mm_loadu_si128((const __m128i *)patt);
                                }
                                else {
                                    break;
                                }
                            }
                            else {
                                // Reset the pattern
                                patt = pattern;
                                __pattern = _mm_loadu_si128((const __m128i *)patt);
                                text += offset;
                                rest_len = pattern_len;
                            }
                        }
                    } while (1);

                    assert(rest_len <= 0);
                    return (text - pattern_len + rest_len);
                }
                else {
                    __text = _mm_loadu_si128((const __m128i *)text);
                    offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                    if (likely(offset >= kMaxSize)) {
                        break;
                    }
                    else {
                        // Has found
                        return (text + offset);
                    }
                }
            }
        } while (1);
    }

    return nullptr;
}

/***********************************************************

  Author: lqeh
  Link: https://www.jianshu.com/p/d718c1ea5f22
  From: www.jianshu.com

***********************************************************/

/* unit test:
 * src = "00000000001234561234123456789abcdefghi", dest = "1234567"; ret = 20
 * src = "00000000001234561234123456789abcdefghi", dest = "123456789abcdefg";  ret = 20
 * src = "00000000001234561234123456789abcdefghi", dest = "1234"; ret = 10
 * src = "00000000001234561234123456789abcdefghi", dest = "00000000"; ret = 0
 * src = "00000000001234561234123456789abcdefghi", dest = "0000000000123456"; ret = 0
 * src = "00000000001234561234123456789abcdefghi", dest = "000000000012345612"; ret = 0
 * src = "00000000001234561234123456789abcdefghi", dest = "1000000000012345612"; ret = -1
 * src = "00000000001234561234123456789abcdefghi", dest = "fghi"; ret = 34
 * src = "00000000001234561234123456789abcdefghi", dest = "fghia"; ret = -1
 * src = "00000000001234561234123456789abcdefghi", dest = "3456789abcdefghi"; ret = 22
 * src = "00000000001234561234123456789abcdefghi", dest = "23456789abcdefghi"; ret = 21
 * src = "00000000001234561234123456789abcdefghi", dest = "3456789abcdefghiq"; ret = -1
 * src = "aaaabbbbaaaabbbbaaaabbbbacc", dest = "aaaabbbbaaaabbbbacc"; ret = 8
 * src = "aaaabbbbaaaabbbbaaaabbbbacc", dest = "aaaabbbbaaaabbbbccc"; ret = -1
 * src = "012345678", dest = "234"; ret = 2
 * src = "012345678", dest = "2346"; ret = -1  
 */  

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
sse42_strstr_v2(const char_type * text, const char_type * pattern) {
    assert(text != nullptr);
    assert(pattern != nullptr);

    static const int mode_ordered = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ORDERED
                                  | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    const char_type * t = text;
    const char_type * p = pattern;

    const char_type * t_16;
    const char_type * p_16;

    int offset;
    int p_has_null;
    int t_has_null;

    __m128i __text;
    __m128i __pattern;

    __text = _mm_loadu_si128((__m128i *)t);
    __pattern = _mm_loadu_si128((__m128i *)p);
    p_has_null = _mm_cmpistrs(__pattern, __text, mode_ordered);

    if (likely(p_has_null != 0)) {
        /* strlen(pattern) < 16 */
#if 1
        do {
            __text = _mm_loadu_si128((__m128i *)t);

            offset = _mm_cmpistri(__pattern, __text, mode_ordered);
            t_has_null = _mm_cmpistrz(__pattern, __text, mode_ordered);

            if (likely(offset != 0)) {
                t += offset;
            }
            else {
                return t;
            }
        } while ((t_has_null == 0) || (t_has_null == 1 && offset != 16));

        return nullptr;
#else
        int matched;
        do {
            __text = _mm_loadu_si128((__m128i *)t);

            offset = _mm_cmpistri(__pattern, __text, mode_ordered);
            matched = _mm_cmpistrc(__pattern, __text, mode_ordered);
            t_has_null = _mm_cmpistrz(__pattern, __text, mode_ordered);

            if (likely((matched == 0) || (offset != 0))) {
                t += offset;
            }
            else {
                break;
            }            
        } while (t_has_null == 0);

        if (likely(matched != 0)) {
            return t;
        }

        return nullptr;
#endif
    }
    else {
        /* strlen(pattern) >= 16 */
        do {
            __text = _mm_loadu_si128((__m128i *)t);
            __pattern = _mm_loadu_si128((__m128i *)p);

            offset = _mm_cmpistri(__pattern, __text, mode_ordered);
            t_has_null = _mm_cmpistrz(__pattern, __text, mode_ordered);
#if !defined(NDEBUG)
            p_has_null = _mm_cmpistrs(__pattern, __text, mode_ordered);
            assert(p_has_null == 0);
#endif
            if (likely(t_has_null == 0)) {
                if (likely(offset != 0)) {
                    /* It's suffix (offset > 0 and offset < 16)
                       or not match (offset = 16) */
                    t += offset;
                }
                else {
                    /* Part of pattern or full pattern matched (offset = 0) */
                    t_16 = t;
                    p_16 = p;
                    do {
                        t_16 += 16;
                        p_16 += 16;

                        __text = _mm_loadu_si128((__m128i *)t_16);
                        __pattern = _mm_loadu_si128((__m128i *)p_16);

                        offset = _mm_cmpistri(__pattern, __text, mode_ordered);
                        t_has_null = _mm_cmpistrz(__pattern, __text, mode_ordered);
                        if (likely(offset != 0))
                            break;

                        p_has_null = _mm_cmpistrs(__pattern, __text, mode_ordered);
                    } while (t_has_null == 0 && p_has_null == 0);

                    if (likely(offset != 0)) {
                        t += offset;
                        if (likely(t_has_null != 0))
                            break;
                    }
                    else {
                        return t;
                    }
                }
            }
            else {
#if 0
                if (likely(offset == 16)) {
                    break;
                }
                else if (likely(offset != 0)) {
                    /* It's suffix (offset > 0 and offset < 16)
                       or not match (offset = 16) */
                    t += offset;

                    if (t_has_null != 0)
                        break;
                }
#endif
                break;
            }
        } while (1);

        return nullptr;
    }
}

template <typename CharTy>
class SSEStrStrImpl {
public:
    typedef SSEStrStrImpl<CharTy>   this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

private:
    bool alive_;

public:
    SSEStrStrImpl() : alive_(true) {}
    ~SSEStrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "sse42_strstr()"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return this->alive_; }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        /* Don't need to do preprocessing. */
        return true;
    }

    /* Searching */
    Long search(const char_type * text, size_type text_len,
                const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
        const char_type * substr = sse42_strstr_v2(text, pattern);
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< SSEStrStrImpl<char> >     SSEStrStr;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< SSEStrStrImpl<wchar_t> >  SSEStrStr;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_SSE_STRSTR_H
