
#ifndef STRING_MATCH_SSE_STRSTR2_H
#define STRING_MATCH_SSE_STRSTR2_H

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
#include "asm/asmlib.h"

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

namespace StringMatch {

/***********************************************************

  Author: lqeh
  Link: https://www.jianshu.com/p/d718c1ea5f22
  From: www.jianshu.com

***********************************************************/

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v2a(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    const char_type * t = text;
    const char_type * p = pattern;

    const char_type * t_16;
    const char_type * p_16;

    int offset;
    int t_has_null;
    int p_has_null;

    __m128i __text, __pattern;
    __m128i __zero, __mask;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // Check the length of pattern is less than 16?

    //__text = _mm_loadu_si128((const __m128i *)t);
    __pattern = _mm_loadu_si128((const __m128i *)p);
    //p_has_null = _mm_cmpistrs(__pattern, __text, kEqualOrdered);

    // pxor         xmm0, xmm0
    // pcmpeqb      xmm1, xmm0
    // pmovmskb     edx,  xmm1
    //__zero = _mm_xor_si128(__zero, __zero);
    __zero = _mm_setzero_si128();
    __mask = _mm_cmpeq_epi8(__pattern, __zero);

    uint64_t * mask_128i = (uint64_t *)&__mask;
    if (likely(mask_128i[0] != 0 || mask_128i[1] != 0)) {
        /* strlen(pattern) < kMaxSize (16 or 8) */
#if 0
        do {
            __text = _mm_loadu_si128((const __m128i *)t);

            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);

            if (likely(offset != 0)) {
                t += offset;
            }
            else {
                return t;
            }
        } while ((t_has_null == 0) || (t_has_null != 0 && offset < kMaxSize));

        return nullptr;
#else
        int matched;
        do {
            __text = _mm_loadu_si128((const __m128i *)t);

            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            matched = _mm_cmpistrc(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);

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
        /* strlen(pattern) >= kMaxSize (16 or 8) */
        do {
            __text = _mm_loadu_si128((const __m128i *)t);
            __pattern = _mm_loadu_si128((const __m128i *)p);

            offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
#ifndef NDEBUG
            p_has_null = _mm_cmpistrs(__pattern, __text, kEqualOrdered);
            assert(p_has_null == 0);
#endif
            if (likely(t_has_null == 0)) {
                if (likely(offset != 0)) {
                    /* It's suffix (offset > 0 and offset < kMaxSize)
                       or not match (offset = kMaxSize), kMaxSize = 16 or 8 */
                    t += offset;
                }
                else {
                    /* Part of pattern or full pattern matched (offset = 0) */
                    t_16 = t;
                    p_16 = p;
                    do {
                        t_16 += kMaxSize;
                        p_16 += kMaxSize;

                        __text = _mm_loadu_si128((const __m128i *)t_16);
                        __pattern = _mm_loadu_si128((const __m128i *)p_16);

                        offset = _mm_cmpistri(__pattern, __text, kEqualOrdered);
                        t_has_null = _mm_cmpistrz(__pattern, __text, kEqualOrdered);
                        p_has_null = _mm_cmpistrs(__pattern, __text, kEqualOrdered);
                        if (likely(offset != 0))
                            break;
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
                assert(t_has_null != 0);
                // Because the length of pattern is >= 16,
                // if text has null terminator, must be dismatch.
                break;
            }
        } while (1);

        return nullptr;
    }
}

//
// See: https://www.strchr.com/strcmp_and_strlen_using_sse_4.2
//
template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v2b(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;
    static const int kEqualEachMask = kEqualEach | _SIDD_UNIT_MASK;

    const char_type * t = text;
    const char_type * p = pattern;

    const char_type * t_16;
    const char_type * p_16;

    int offset, matched;
    int t_has_null;

    __m128i __text, __pattern, __pattern_0;
    __m128i __zero, __mask;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // load the first 16 bytes of pattern
    __pattern_0 = _mm_loadu_si128((const __m128i *)p);
    __zero = _mm_setzero_si128();

    t -= kMaxSize;

    do {
        // find the first possible match of 16-byte fragment in text
        do {
            t += kMaxSize;
            __text = _mm_loadu_si128((const __m128i *)t);
            matched    = _mm_cmpistrc(__pattern_0, __text, kEqualOrdered);
            t_has_null = _mm_cmpistrz(__pattern_0, __text, kEqualOrdered);
            offset     = _mm_cmpistri(__pattern_0, __text, kEqualOrdered);
        } while (matched == 0 && t_has_null == 0);     // ja: ~CF & ~ZF

        if (matched == 0) {
            return nullptr;
        }

        // save the possible match start
        t += offset;

        t_16 = t;
        p_16 = p;

        // compare the strings
        do {
            __pattern = _mm_loadu_si128((const __m128i *)p_16);
            // mask out invalid bytes in the text
            __mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMask);
            p_16 += kMaxSize;
            __text = _mm_loadu_si128((const __m128i *)t_16);
            t_16 += kMaxSize;
            __text = _mm_and_si128(__text, __mask);

            matched    = _mm_cmpistrc(__pattern, __text, kEqualEach);
            t_has_null = _mm_cmpistrz(__pattern, __text, kEqualEach);
            offset     = _mm_cmpistri(__pattern, __text, kEqualEach);
        } while (matched == 0 && t_has_null == 0);     // ja: ~CF & ~ZF

        if (matched == 0) {
            return t;
        }

        // continue searching from the next byte
        t -= (kMaxSize - 1);
    } while (1);

    return nullptr;
}

//
// See: https://www.strchr.com/strcmp_and_strlen_using_sse_4.2
//
template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_sse42_v2(const char_type * text, const char_type * pattern) {
    static const int kMaxSize = SSEHelper<char_type>::kMaxSize;
    static const int _SIDD_CHAR_OPS = SSEHelper<char_type>::_SIDD_CHAR_OPS;

    static const int kEqualOrdered = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_ORDERED
                                   | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;
    static const int kEqualEachMask = kEqualEach | _SIDD_UNIT_MASK;

    const char_type * t = text;
    const char_type * p = pattern;

    const char_type * t_16;
    const char_type * p_16;

    int offset, matched;
    int t_has_null, p_has_null;

    __m128i __text, __pattern, __pattern_0;
    __m128i __zero, __mask;

    assert(text != nullptr);
    assert(pattern != nullptr);

    // load the first 16 bytes of pattern
    __pattern_0 = _mm_loadu_si128((const __m128i *)p);
    __zero = _mm_setzero_si128();

    t -= kMaxSize;

    // find the first possible match of 16-byte fragment in text
    do {
        t += kMaxSize;
        __text = _mm_loadu_si128((const __m128i *)t);
        matched    = _mm_cmpistrc(__pattern_0, __text, kEqualOrdered);
        t_has_null = _mm_cmpistrz(__pattern_0, __text, kEqualOrdered);
        offset     = _mm_cmpistri(__pattern_0, __text, kEqualOrdered);
        if (likely(matched == 0)) {
            if (likely(t_has_null == 0)) {
                continue;
            }
            else {
                return nullptr;
            }
        }
        else {
            // save the possible match start
            t += offset;

            t_16 = t;
            p_16 = p;

            // compare the strings
            do {
                __pattern = _mm_loadu_si128((const __m128i *)p_16);
                // mask out invalid bytes in the text
                __mask = _mm_cmpistrm(__zero, __pattern, kEqualEachMask);
                p_16 += kMaxSize;
                __text = _mm_loadu_si128((const __m128i *)t_16);
                t_16 += kMaxSize;
                __text = _mm_and_si128(__text, __mask);

                matched    = _mm_cmpistrc(__pattern, __text, kEqualEach);
                t_has_null = _mm_cmpistrz(__pattern, __text, kEqualEach);
                p_has_null = _mm_cmpistrs(__pattern, __text, kEqualEach);
                offset     = _mm_cmpistri(__pattern, __text, kEqualEach);
                if (likely(matched == 0)) {
                    if (likely(t_has_null == 0 && p_has_null == 0)) {
                        continue;
                    }
                    else {
                        return t;
                    }
                }
                else {
                    // continue searching from the next byte
                    t -= (kMaxSize - 1);
                    break;
                }
            } while (1);
        }
    } while (1);

    return nullptr;
}

template <typename CharTy>
class SSEStrStr2Impl {
public:
    typedef SSEStrStr2Impl<CharTy>  this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

    SSEStrStr2Impl() {}
    ~SSEStrStr2Impl() {
        this->destroy();
    }

    static const char * name() { return "strstr_sse42_v2()"; }
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
#if 0
        const char_type * substr = strstr_sse42_v2(text, pattern);
#else
        const char_type * substr = A_strstr_v2(text, pattern);
#endif
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< SSEStrStr2Impl<char> >    SSEStrStr2;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< SSEStrStr2Impl<wchar_t> > SSEStrStr2;
}

} // namespace StringMatch

#endif // STRING_MATCH_SSE_STRSTR2_H
