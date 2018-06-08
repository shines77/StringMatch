
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
#include "SSEHelper.h"
#include "support/bitscan_reverse.h"

//
// Implementing strcmp, strlen, and strstr using SSE 4.2 instructions
//
// See: https://www.strchr.com/strcmp_and_strlen_using_sse_4.2
//

namespace StringMatch {

/************************************************************

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42(const char_type * text, const char_type * pattern,
             typename std::enable_if<detail::is_wchar<char_type>::value, char_type>::type * = nullptr) {
    return nullptr;
}

************************************************************/

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;
    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualAny = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ANY
                               | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    assert(text != nullptr);
    assert(pattern != nullptr);

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, null;
#if defined(NDEBUG)
    (void)(__zero);
#else
    __zero = { 0 };
#endif
    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    __zero = _mm_xor_si128(__zero, __zero);
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);
    
    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (mask_128i[0] != 0 || mask_128i[1] != 0) {
        // The length of pattern is less than kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                null = _mm_cmpestri(__zero, kMaxSize, __text, kMaxSize, kEqualEach);
            } while (offset >= kMaxSize && null >= kMaxSize);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
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
        // The length of pattern is more than or equal kMaxSize (16 or 8).

        // Get the length of pattern
        int pattern_len;
        {
            __m128i __patt;
            const char_type * patt = pattern;
            pattern_len = 0;

            do {
                patt += kMaxSize;
                __patt = _mm_loadu_si128((const __m128i *)patt);
                null = _mm_cmpestri(__zero, kMaxSize, __patt, kMaxSize, kEqualEach);
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
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                null = _mm_cmpestri(__zero, 16, __text, 16, kEqualEach);
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
                        offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
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
                    offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
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
strstr_sse42_v1_old(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;
    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualAny = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ANY
                               | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, null;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

#if defined(NDEBUG)
    (void)(__zero);
#else
    __zero = { 0 };
#endif
    // Check the length of pattern is less than 16?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    __zero = _mm_xor_si128(__zero, __zero);
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
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                if (likely(offset >= kMaxSize)) {
                    if (likely(t_has_null == 0)) {
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
                null = _mm_cmpestri(__zero, kMaxSize, __patt, kMaxSize, kEqualEach);
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
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(offset >= kMaxSize)) {
                break;
            }
            else {
                assert(offset >= 0 && offset < 16);
                text += offset;
                if (likely(t_has_null == 0)) {
                    int rest_len = pattern_len;
                    const char_type * patt = pattern;
                    do {
                        __text = _mm_loadu_si128((const __m128i *)text);
                        offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
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
                    offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
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
strstr_sse42_v1(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach2 = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                 | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, null;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

#if defined(NDEBUG)
    (void)(__zero);
#else
    __zero = { 0 };
#endif
    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    __zero = _mm_xor_si128(__zero, __zero);
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);
    
    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (mask_128i[0] != 0 || mask_128i[1] != 0) {
        // The length of pattern is less than kMaxSize (16 or 8).
#if 0
        offset = 0;
        do {
            text += offset;
            __text = _mm_loadu_si128((const __m128i *)text);
            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
        } while (offset != 0 && t_has_null == 0);

        if (likely(offset >= kMaxSize)) {
            assert(t_has_null != 0);
            return nullptr;
        }
        else {
            return (text + offset);
        }
#elif 1
        do {
            __text = _mm_loadu_si128((__m128i *)text);
            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);

            if (likely(offset != 0)) {
                text += offset;
            }
            else {
                return text;
            }
        } while ((t_has_null == 0) || (t_has_null != 0 && offset < kMaxSize));

        return nullptr;
#else
        do {
            __text = _mm_loadu_si128((const __m128i *)text);
            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);

            if (likely(offset != 0)) {
                text += offset;     // Continue search
                if (likely(t_has_null != 0 && offset >= kMaxSize)) {
                    break;
                }
            }
            else {
                return text;        // Has found
            }
        } while (1);

        return nullptr;
#endif
    }
    else {
        // The length of pattern is more than or equal kMaxSize (16 or 8).

        // Get the length of pattern
        int pattern_len;
        {
            __m128i __patt;
            const char_type * p = pattern;
            pattern_len = 0;

            do {
                p += kMaxSize;
                __patt = _mm_loadu_si128((const __m128i *)p);
                null = _mm_cmpestri(__zero, kMaxSize, __patt, kMaxSize, kEqualEach);
                pattern_len += kMaxSize;
            } while (null >= kMaxSize);

            assert(null >= 0 && null < kMaxSize);
            pattern_len += null;
        }

        assert(pattern_len >= kMaxSize);

        offset = 0;
        do {
STRSTR_MAIN_LOOP:
            do {
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset != 0 && t_has_null == 0);

            if (likely(t_has_null != 0)) {
                assert(offset != 0);
                break;
            }
            else {
                assert(t_has_null == 0);
                int rest_len = pattern_len - kMaxSize;
                if (likely(rest_len > 0)) {
                    __m128i __patt;
                    const char_type * t = text + kMaxSize;
                    const char_type * p = pattern + kMaxSize;

                    assert(offset == 0);
                    __text = _mm_loadu_si128((const __m128i *)t);
                    __patt = _mm_loadu_si128((const __m128i *)p);
                    do {
                        offset = _mm_cmpistri(__patt, __text, kEqualOrdered);
                        if (likely(offset != 0)) {
                            // Restart the search
                            offset = 1;
                            goto STRSTR_MAIN_LOOP;
                        }
                        else {
                            // Scan the next part pattern
                            rest_len -= kMaxSize;
                            if (likely(rest_len > 0)) {
                                t += kMaxSize;
                                __text = _mm_loadu_si128((const __m128i *)t);
                                p += kMaxSize;
                                __patt = _mm_loadu_si128((const __m128i *)p);
                            }
                            else {
#ifndef NDEBUG
                                t += kMaxSize;
#endif
                                break;
                            }
                        }
                    } while (1);

                    assert(rest_len <= 0);
                    assert(text == (t - pattern_len + rest_len));
                    return text;
                }
                else {
                    return text;
                }
            }
        } while (1);
    }

    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v1b(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEachM = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                 | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT
                                 | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

#if defined(NDEBUG)
    (void)(__zero);
#else
    __zero = { 0 };
#endif
    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    __zero = _mm_xor_si128(__zero, __zero);
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);
    
    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (mask_128i[0] != 0 || mask_128i[1] != 0) {
        // The length of pattern is less than kMaxSize (16 or 8).
#if 1
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((__m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(t_has_null != 0 && offset >= kMaxSize)) {
                break;
            }
            else if (likely(offset != 0)) {
                text += offset;
                __text = _mm_loadu_si128((__m128i *)text);

                __m128i __patt_mask;
                __patt_mask = _mm_cmpistrm(__zero, __pattern, kEqualEachM);
                __text = _mm_and_si128(__text, __patt_mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    return text;
                }
            }
            else {
                return text;
            }
        } while (1);

        return nullptr;
#elif 0
        offset = 0;
        do {
            text += offset;
            __text = _mm_loadu_si128((const __m128i *)text);
            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
        } while (offset != 0 && t_has_null == 0);

        if (likely(offset >= kMaxSize)) {
            assert(t_has_null != 0);
            return nullptr;
        }
        else {
            return (text + offset);
        }
#elif 1
        do {
            __text = _mm_loadu_si128((__m128i *)text);
            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);

            if (likely(offset != 0)) {
                text += offset;
            }
            else {
                return text;
            }
        } while ((t_has_null == 0) || (t_has_null != 0 && offset < kMaxSize));

        return nullptr;
#else
        do {
            __text = _mm_loadu_si128((const __m128i *)text);
            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);

            if (likely(offset != 0)) {
                text += offset;     // Continue search
                if (likely(t_has_null != 0 && offset >= kMaxSize)) {
                    break;
                }
            }
            else {
                return text;        // Has found
            }
        } while (1);

        return nullptr;
#endif
    }
    else {
        // The length of pattern is more than or equal kMaxSize (16 or 8).

        text -= kMaxSize;
        do {
STRSTR_MAIN_LOOP:
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(t_has_null != 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else if (likely(offset != 0)) {
                assert(t_has_null == 0);
                assert(offset > 0);

                text += offset;

                __m128i __patt, __patt_mask;
                const char_type * t = text;
                const char_type * p = pattern;

                __text = _mm_loadu_si128((const __m128i *)t);
                __patt = _mm_loadu_si128((const __m128i *)p);
                do {
                    __patt_mask = _mm_cmpistrm(__zero, __patt, kEqualEachM);
                    __text = _mm_and_si128(__text, __patt_mask);
                    int full_matched = _mm_cmpistrc(__patt, __text, kEqualEach);
                    int p_has_null = _mm_cmpistrs(__patt, __text, kEqualEach);
                    if (likely(full_matched == 0)) {
                        if (likely(p_has_null == 0)) {
                            // Scan the next part pattern
                            t += kMaxSize;
                            __text = _mm_loadu_si128((const __m128i *)t);
                            p += kMaxSize;
                            __patt = _mm_loadu_si128((const __m128i *)p);
                        }
                        else {
                            // Has found, The pattern has null terminator and partial matched.
                            assert(p_has_null != 0);
                            break;
                        }
                    }
                    else {
                        // Restart to search the next char.
                        offset = 1;
                        goto STRSTR_MAIN_LOOP;
                    }
                } while (1);
            }

            // Has found
            return text;
        } while (1);
    }

    return nullptr;
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

    static const char * name() { return "strstr_sse42()"; }
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
        const char_type * substr = strstr_sse42_v1b(text, pattern);
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
