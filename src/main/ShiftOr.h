
#ifndef STRING_MATCH_SHIFTOR_H
#define STRING_MATCH_SHIFTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <cstdint>
#include <string>
#include <assert.h>

#include "StringMatch.h"
#include "StringRef.h"

namespace StringMatch {

namespace detail {

template <typename CharTy>
struct uchar_traits {
    typedef CharTy type;
};

template <>
struct uchar_traits<char> {
    typedef unsigned char type;
};

} // namespace detail

template <typename CharTy, typename MaskTy = uint64_t>
class ShiftOrImpl {
public:
    typedef ShiftOrImpl<CharTy, MaskTy>         this_type;
    typedef CharTy                              char_type;
    typedef MaskTy                              mask_type;
    typedef typename detail::uchar_traits<CharTy>::type
                                                uchar_type;

private:
    std::unique_ptr<mask_type[]> bitmap_;
    mask_type limit_;

public:
    ShiftOrImpl() : bitmap_(), limit_(0) {}
    ~ShiftOrImpl() {
        this->destroy();
    }

    bool need_preprocessing() const { return true; }

    bool is_alive() const {
        return (this->bitmap() != nullptr);
    }

    void destroy() {
        this->bitmap_.reset();
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_t length) {
        assert(pattern != nullptr);
        static const size_t kMaxAscii = 256;
        mask_type limit = 0;
        mask_type * bitmap = new mask_type[kMaxAscii];
        if (bitmap != nullptr) {
            for (size_t i = 0; i < kMaxAscii; ++i)
                bitmap[i] = ~0;

            mask_type mask = 1;
            for (size_t i = 0; i < length; mask <<= 1, ++i) {
                bitmap[(uchar_type)pattern[i]] &= ~mask;
                limit |= mask;
            }
            limit = ~(limit >> 1);
        }
        this->bitmap_.reset(bitmap);
        this->limit_ = limit;
        return (bitmap != nullptr);
    }

    /* Searching */
    int search(const char_type * text, size_t text_len,
               const char_type * pattern, size_t pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        mask_type * bitmap = this->bitmap_.get();
        mask_type limit = this->limit_;

        assert(bitmap != nullptr);

        if (pattern_len <= text_len) {
            if ((size_t)text | (size_t)pattern | (size_t)bitmap) {
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
                register mask_type state1 = ~0;
                register mask_type state2 = ~0;
                size_t half_len = (text_len / 2);
                size_t i;
                for (i = 0; i < half_len; ++i) {
                    state1 = (state1 << 1) | bitmap[(uchar_type)text[i]];
                    state2 = (state2 << 1) | bitmap[(uchar_type)text[half_len + i]];
                    if (state1 < limit)
                        return (int)(i + 1 - pattern_len);
                    if (state2 < limit)
                        return (int)(i + half_len + 1 - pattern_len);
                }
                size_t j;
                for (j = 0; j < pattern_len - 1; ++j) {
                    state1 = (state1 << 1) | bitmap[(uchar_type)text[i + j]];
                    if (state1 < limit)
                        return (int)(i + j + 1 - pattern_len);
                }
                if ((text_len & 2) != 0) {
                    state2 = (state2 << 1) | bitmap[(uchar_type)text[half_len + i]];
                    if (state2 < limit)
                        return (int)(i + half_len + 1 - pattern_len);
                }
#else
                register mask_type state = ~0;
                for (size_t i = 0; i < text_len; ++i) {
                    state = (state << 1) | bitmap[(uchar_type)text[i]];
                    if (state < limit)
                        return (int)(i + 1 - pattern_len);
                }
#endif // _WIN64 || __amd64__
                return Status::NotFound;
            }
            else {
                // Invalid parameters
                return Status::InvalidParameter;
            }
        }

        return Status::NotFound;
    }
};

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
namespace AnsiString {
    typedef AlgorithmWrapper< ShiftOrImpl<char, uint64_t> >    ShiftOr;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftOrImpl<wchar_t, uint64_t> > ShiftOr;
} // namespace UnicodeString
#else
namespace AnsiString {
    typedef AlgorithmWrapper< ShiftOrImpl<char, uint32_t> >    ShiftOr;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftOrImpl<wchar_t, uint32_t> > ShiftOr;
} // namespace UnicodeString
#endif // _WIN64 || __amd64__

} // namespace StringMatch

#endif // STRING_MATCH_SHIFTOR_H
