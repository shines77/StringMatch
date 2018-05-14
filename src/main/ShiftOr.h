
#ifndef STRING_MATCH_SHIFTOR_H
#define STRING_MATCH_SHIFTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "StringMatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <cstdint>
#include <string>
#include <assert.h>

#include "StringRef.h"

namespace StringMatch {

namespace detail {

template <typename CharTy>
struct unsigned_traist {
    typedef CharTy type;
};

template <>
struct unsigned_traist<char> {
    typedef unsigned char type;
};

} // namespace detail

template <typename CharTy, typename MaskTy = uint64_t>
class ShiftOrImpl {
public:
    typedef ShiftOrImpl<CharTy, MaskTy>         this_type;
    typedef CharTy                              char_type;
    typedef MaskTy                              mask_type;
    typedef typename detail::unsigned_traist<CharTy>::type
                                                uchar_type;
    typedef std::tuple<mask_type *, mask_type>  tuple_type;

private:
    std::unique_ptr<mask_type[]> bitmap_;
    mask_type limit_;
    tuple_type args_;

public:
    ShiftOrImpl() : bitmap_(), limit_(0), args_() {}
    ~ShiftOrImpl() {
        this->destroy();
    }

    bool need_preprocessing() const { return true; }

    const tuple_type & get_args() const { return this->args_; }
    void set_args(const tuple_type & args) {
        if ((void *)&args_ != (void *)&args) {
            this->args_ = args;
        }
        // Update args
        this->bitmap_.reset(std::get<0>(args_));
        this->limit_ = std::get<1>(args_);
    }

    mask_type * bitmap() const { return this->bitmap_.get(); }
    void set_bitmap(mask_type * bitmap) {
        this->bitmap_.reset(bitmap);
    }

    bool is_alive() const {
        return (this->bitmap() != nullptr);
    }

    void destroy() {
        this->bitmap_.reset();
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_t length) {
        mask_type * bitmap = nullptr;
        mask_type limit = 0;
        bool success = this_type::preprocessing(pattern, length, bitmap, limit);
        // Update args
        this->args_ = std::make_tuple(bitmap, limit);
        this->bitmap_.reset(bitmap);
        this->limit_ = limit;
        return success;
    }

    /* Preprocessing */
    static bool preprocessing(const char * pattern, size_t length,
                              mask_type * & out_bitmap, mask_type & limit) {
        assert(pattern != nullptr);
        static const size_t kMaxAscii = 256;

        limit = 0;
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
        out_bitmap = bitmap;
        return (bitmap != nullptr);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern, size_t pattern_len,
                      const tuple_type & args) {
        mask_type * bitmap = std::get<0>(args);
        mask_type limit = std::get<1>(args);
        return this_type::search(text, text_len, pattern, pattern_len, bitmap, limit);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern, size_t pattern_len,
                      const mask_type * bitmap, mask_type limit) {
        assert(text != nullptr);
        assert(pattern != nullptr);
        assert(bitmap != nullptr);

        if (pattern_len <= text_len) {
            if ((size_t)text | (size_t)pattern | (size_t)bitmap) {
                mask_type state = ~0;
                for (size_t i = 0; i < text_len; ++i) {
                    state = (state << 1) | bitmap[(uchar_type)text[i]];
                    if (state < limit)
                        return (int)(i + 1 - pattern_len);
                }
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
