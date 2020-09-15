
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
#include "algorithm/AlgorithmWrapper.h"
#include "algorithm/SSEHelper.h"
#include "support/bitscan_reverse.h"

#include "algorithm/SSEStrStr_inl.h"

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
             typename std::enable_if<jstd::is_wchar<char_type>::value, char_type>::type * = nullptr) {
    return nullptr;
}

************************************************************/

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v1a(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEachMark = kEqualEach | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    //__zero = _mm_xor_si128(__zero, __zero);
    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);
    
    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        // The length of pattern is less than kMaxSize (16 or 8).
#if 1
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(t_has_null != 0 && offset >= kMaxSize)) {
                break;
            }
            else if (likely(offset != 0)) {
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);

                __m128i __patt_mask;
                __patt_mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMark);
                __text = _mm_and_si128(__text, __patt_mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    // Has found
                    assert(full_matched == 0);
                    return text;
                }
            }
            else {
                // Has found
                assert(offset == 0);
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
            __text = _mm_loadu_si128((const __m128i *)text);
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
        // The length of pattern is greater than or equal to kMaxSize (16 or 8).
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
                __patt = __pattern;
                do {
                    __patt_mask = _mm_cmpistrm(__zero, __patt, kEqualEachMark);
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
                        text -= (kMaxSize - 1);
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

    static const int kEqualEachMark = kEqualEach | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    //__zero = _mm_xor_si128(__zero, __zero);
    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    //offset = _mm_movemask_epi8(__mask);
    
    //_mm_store_si128((__m128i *)mask_128i, __mask);
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        // The length of pattern is less than kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(t_has_null != 0 && offset >= kMaxSize)) {
                break;
            }
            else if (likely(offset != 0)) {
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);

                __m128i __patt_mask;
                __patt_mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMark);
                __text = _mm_and_si128(__text, __patt_mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    // Has found
                    assert(full_matched == 0);
                    return text;
                }
            }
            else {
                // Has found
                assert(offset == 0);
                return text;
            }
        } while (1);

        return nullptr;
    }
    else {
        // The length of pattern is greater than or equal to kMaxSize (16 or 8).
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
            else {
                assert(t_has_null == 0);
                assert(offset >= 0 && offset < kMaxSize);

                const char_type * t;
                const char_type * p;
                if (likely(offset != 0)) {
                    text += offset;
                    t = text;
                    p = pattern;
                }
                else {
                    t = text + kMaxSize;
                    p = pattern + kMaxSize;
                }
                do {
                    __m128i __patt, __patt_mask;
                    __text = _mm_loadu_si128((const __m128i *)t);
                    __patt = _mm_loadu_si128((const __m128i *)p);
                    __patt_mask = _mm_cmpistrm(__zero, __patt, kEqualEachMark);
                    __text = _mm_and_si128(__text, __patt_mask);
                    t += kMaxSize;
                    p += kMaxSize;
                    int full_matched = _mm_cmpistrc(__patt, __text, kEqualEach);
                    int p_has_null = _mm_cmpistrs(__patt, __text, kEqualEach);
                    if (likely(full_matched == 0)) {
                        if (likely(p_has_null == 0)) {
                            // Scan the next part pattern
                            continue;
                        }
                        else {
                            // Has found, The pattern has null terminator and partial matched.
                            assert(p_has_null != 0);
                            break;
                        }
                    }
                    else {
                        // Restart to search the next char.
                        text -= (kMaxSize - 1);
                        goto STRSTR_MAIN_LOOP;
                    }
                } while (1);

                // Has found
                return text;
            }
        } while (1);
    }

    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v1c(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEachMark = kEqualEach | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset;
    //int matched;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);

    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        // The length of pattern is less than kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                //matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(t_has_null != 0 && offset >= kMaxSize)) {
                break;
            }
            else if (likely(offset != 0)) {
                assert(offset > 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);

                __m128i __patt_mask;
                __patt_mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMark);
                __text = _mm_and_si128(__text, __patt_mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    // Has found
                    assert(full_matched == 0);
                    return text;
                }
            }
            else {
                // Has found
                assert(offset == 0);
                return text;
            }
        } while (1);

        return nullptr;
    }
    else {
        // The length of pattern is greater than or equal to kMaxSize (16 or 8).
        do {
STRSTR_MAIN_LOOP:
            do {
                __text = _mm_loadu_si128((const __m128i *)text);
                //matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                text += kMaxSize;
            } while (offset >= kMaxSize && t_has_null == 0);

            if (likely(t_has_null != 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else {
                assert(t_has_null == 0);
                assert(offset >= 0 && offset < kMaxSize);

                const char_type * match_start = text - (kMaxSize - offset);
                const char_type * t = text;
                const char_type * p = pattern + (kMaxSize - offset);
                do {
                    __m128i __patt, __patt_mask;
                    __text = _mm_loadu_si128((const __m128i *)t);
                    __patt = _mm_loadu_si128((const __m128i *)p);
                    __patt_mask = _mm_cmpistrm(__zero, __patt, kEqualEachMark);
                    __text = _mm_and_si128(__text, __patt_mask);
                    t += kMaxSize;
                    p += kMaxSize;
                    int full_matched = _mm_cmpistrc(__patt, __text, kEqualEach);
                    int p_has_null = _mm_cmpistrs(__patt, __text, kEqualEach);
                    if (likely(full_matched == 0)) {
                        if (likely(p_has_null == 0)) {
                            // Scan the next part pattern
                            continue;
                        }
                        else {
                            // Has found, The pattern has null terminator and partial matched.
                            assert(p_has_null != 0);
                            break;
                        }
                    }
                    else {
                        // Restart to search the next char.
                        text = match_start + 1;
                        goto STRSTR_MAIN_LOOP;
                    }
                } while (1);

                // Has found
                return match_start;
            }
        } while (1);
    }

    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v1c2(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEachMark = kEqualEach | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset;
    int matched;
    int t_has_null, p_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);

    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        // The length of pattern is less than kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            } while (matched == 0 && t_has_null == 0);

            if (likely(matched == 0)) {
                break;
            }
            else if (likely(offset != 0)) {
                assert(offset > 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                __mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMark);
                __text = _mm_and_si128(__text, __mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    // Has found
                    assert(full_matched == 0);
                    return text;
                }
            }
            else {
                // Has found
                assert(offset == 0);
                return text;
            }
        } while (1);

        return nullptr;
    }
    else {
        // The length of pattern is greater than or equal to kMaxSize (16 or 8).
        do {
STRSTR_MAIN_LOOP:
            do {
                __text = _mm_loadu_si128((const __m128i *)text);
                matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                text += kMaxSize;
            } while (matched == 0 && t_has_null == 0);

            if (likely(matched == 0 || t_has_null != 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else {
                assert(t_has_null == 0);
                assert(offset >= 0 && offset < kMaxSize);

                const char_type * match_start = text - (kMaxSize - offset);
                const char_type * t = text;
                const char_type * p = pattern + (kMaxSize - offset);
                do {
                    __m128i __patt;
                    __text = _mm_loadu_si128((const __m128i *)t);
                    __patt = _mm_loadu_si128((const __m128i *)p);
                    __mask = _mm_cmpistrm(__zero, __patt, kEqualEachMark);
                    __text = _mm_and_si128(__text, __mask);
                    t += kMaxSize;
                    p += kMaxSize;
                    int full_matched = _mm_cmpistrc(__patt, __text, kEqualEach);
                    t_has_null       = _mm_cmpistrz(__patt, __text, kEqualEach);
                    p_has_null       = _mm_cmpistrs(__patt, __text, kEqualEach);
                    if (likely(full_matched == 0)) {
                        if (likely(p_has_null == 0 && t_has_null == 0)) {
                            // Scan the next part pattern
                            continue;
                        }
                        else {
                            // Has found, The pattern has null terminator and partial matched.
                            assert(p_has_null != 0);
                            break;
                        }
                    }
                    else {
                        // Restart to search the next char.
                        text = match_start + 1;
                        goto STRSTR_MAIN_LOOP;
                    }
                } while (1);

                // Has found
                return match_start;
            }
        } while (1);
    }

    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v1d(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEachMark = kEqualEach | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, matched;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        // The length of pattern is less than kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            } while (matched == 0 && t_has_null == 0);

            if (likely(matched == 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else if (likely(offset != 0)) {
                assert(offset > 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);
                __mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMark);
                __text = _mm_and_si128(__text, __mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    // Has found
                    assert(full_matched == 0);
                    return text;
                }
            }
            else {
                // Has found
                assert(offset == 0);
                return text;
            }
        } while (1);

        return nullptr;
    }
    else {
        // The length of pattern is greater than or equal to kMaxSize (16 or 8).
        do {
STRSTR_MAIN_LOOP:
            do {
                __text = _mm_loadu_si128((const __m128i *)text);
                text += kMaxSize;
                matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            } while (matched == 0 && t_has_null == 0);

            if (likely(matched == 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else {
                assert(t_has_null == 0);
                assert(offset >= 0 && offset < kMaxSize);

                const char_type * match_start = text - (kMaxSize - offset);
                const char_type * t = text;
                const char_type * p = pattern + (kMaxSize - offset);
                __m128i __patt;

                do {    
                    __patt = _mm_loadu_si128((const __m128i *)p);
                    __mask = _mm_cmpistrm(__zero, __patt, kEqualEachMark);
                    p += kMaxSize;                    
                    __text = _mm_loadu_si128((const __m128i *)t);
                    t += kMaxSize;
                    __text = _mm_and_si128(__text, __mask);
                    
                    int full_matched = _mm_cmpistrc(__patt, __text, kEqualEach);
                    int p_has_null   = _mm_cmpistrs(__patt, __text, kEqualEach);
                    if (likely(full_matched == 0)) {
                        if (likely(p_has_null == 0)) {
                            // Scan the next part pattern
                            continue;
                        }
                        else {
                            // Has found, The pattern has null terminator and partial matched.
                            assert(p_has_null != 0);
                            break;
                        }
                    }
                    else {
                        // Restart to search the next char.
                        text = match_start + 1;
                        goto STRSTR_MAIN_LOOP;
                    }
                } while (1);

                // Has found
                return match_start;
            }
        } while (1);
    }

    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v1e(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEachMark = kEqualEach | _SIDD_UNIT_MASK;

    //alignas(16) uint64_t mask_128i[2];
    __m128i __text, __pattern, __zero, __mask;
    int offset, matched;
    int t_has_null;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than kMaxSize (16 or 8)?
    __pattern = _mm_loadu_si128((const __m128i *)pattern);

    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);
    
    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        // The length of pattern is less than kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
            do {
STRSTR_MAIN_LOOP_16:
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            } while (matched == 0 && t_has_null == 0);

            if (likely(matched == 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else if (likely(offset != 0)) {
                assert(offset > 0 && offset < kMaxSize);
                text += offset;
                __text = _mm_loadu_si128((const __m128i *)text);

                __m128i __patt_mask;
                __patt_mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMark);
                __text = _mm_and_si128(__text, __patt_mask);
                int full_matched = _mm_cmpistrc(__pattern, __text, kEqualEach);
                if (likely(full_matched != 0)) {
                    text -= (kMaxSize - 1);
                    goto STRSTR_MAIN_LOOP_16;
                }
                else {
                    // Has found
                    assert(full_matched == 0);
                    return text;
                }
            }
            else {
                // Has found
                assert(offset == 0);
                return text;
            }
        } while (1);

        return nullptr;
    }
    else {
        // The length of pattern is greater than or equal to kMaxSize (16 or 8).
        text -= kMaxSize;
        do {
STRSTR_MAIN_LOOP:
            do {
                text += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)text);
                
                matched    = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                offset     = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            } while (matched == 0 && t_has_null == 0);

            if (likely(matched == 0)) {
                assert(offset >= kMaxSize);
                break;
            }
            else {
                assert(t_has_null == 0);
                assert(offset >= 0 && offset < kMaxSize);

                text += offset;
                const char_type * t = text + (kMaxSize - offset);
                const char_type * p = pattern + (kMaxSize - offset);
                __m128i __patt;

                do {
                    __patt = _mm_loadu_si128((const __m128i *)p);
                    __mask = _mm_cmpistrm(__zero, __patt, kEqualEachMark);
                    p += kMaxSize;
                    __text = _mm_loadu_si128((const __m128i *)t);
                    t += kMaxSize;
                    __text = _mm_and_si128(__text, __mask);
                    
                    int full_matched = _mm_cmpistrc(__patt, __text, kEqualEach);
                    t_has_null       = _mm_cmpistrz(__patt, __text, kEqualEach);
                    int p_has_null   = _mm_cmpistrs(__patt, __text, kEqualEach);
                    offset           = _mm_cmpistri(__patt, __text, kEqualEach);
                    if (likely(full_matched == 0)) {
                        if (likely(p_has_null == 0)) {
                            // Scan the next part pattern
                            continue;
                        }
                        else {
                            // Has found, The pattern has null terminator and partial matched.
                            assert(p_has_null != 0);
                            break;
                        }
                    }
                    else {
                        // Restart to search the next char.
                        text -= (kMaxSize - 1);
                        goto STRSTR_MAIN_LOOP;
                    }
                } while (1);

                // Has found
                return text;
            }
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

    SSEStrStrImpl() {}
    ~SSEStrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "strstr_sse42()"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        /* Don't need to do preprocessing. */
        SM_UNUSED_VAR(pattern);
        SM_UNUSED_VAR(length);
        return true;
    }

    /* Searching */
    Long search(const char_type * text, size_type text_len,
                const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
#if defined(_MSC_VER)
        const char_type * substr = strstr_sse42_v1c(text, pattern);
#else
        const char_type * substr = strstr_sse42_v1c(text, pattern);
#endif
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< SSEStrStrImpl<char> >     SSEStrStr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< SSEStrStrImpl<wchar_t> >  SSEStrStr;
}

} // namespace StringMatch

#endif // STRING_MATCH_SSE_STRSTR_H
