
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

template <typename CharT>
struct unsigned_traist {
    typedef CharT type;
};

template <>
struct unsigned_traist<char> {
    typedef unsigned char type;
};

} // namespace detail

template <typename CharT>
class ShiftOrImpl {
public:
    typedef ShiftOrImpl<CharT>                  this_type;
    typedef CharT                               char_type;
    typedef typename detail::unsigned_traist<CharT>::type
                                                uchar_type;
    typedef std::tuple<uint64_t *, uint64_t>    tuple_type;

private:
    std::unique_ptr<uint64_t[]> bitmap_;
    uint64_t limit_;
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

    uint64_t * bitmap() const { return this->bitmap_.get(); }
    void set_bitmap(uint64_t * bitmap) {
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
        uint64_t * bitmap = nullptr;
        uint64_t limit = 0;
        bool success = this_type::preprocessing(pattern, length, bitmap, limit);
        // Update args
        this->args_ = std::make_tuple(bitmap, limit);
        this->bitmap_.reset(bitmap);
        this->limit_ = limit;
        return success;
    }

    /* Preprocessing */
    static bool preprocessing(const char * pattern, size_t length,
                              uint64_t * &out_bitmap, uint64_t & limit) {
        assert(pattern != nullptr);
        static const size_t kMaxAscii = 256;

        limit = 0ULL;
        uint64_t * bitmap = new uint64_t[kMaxAscii];
        if (bitmap != nullptr) {
            for (size_t i = 0; i < kMaxAscii; ++i)
                bitmap[i] = ~(0ULL);

            uint64_t mask = 1ULL;
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
        uint64_t * bitmap = std::get<0>(args);
        uint64_t limit = std::get<1>(args);
        return this_type::search(text, text_len, pattern, pattern_len, bitmap, limit);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern, size_t pattern_len,
                      const uint64_t * bitmap, uint64_t limit) {
        assert(text != nullptr);
        assert(pattern != nullptr);
        assert(bitmap != nullptr);

        if (pattern_len <= text_len) {
            if ((size_t)text | (size_t)pattern | (size_t)bitmap) {
                uint64_t state = ~0;
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

namespace AnsiString {
    typedef AlgorithmWrapper< ShiftOrImpl<char> >    ShiftOr;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftOrImpl<wchar_t> > ShiftOr;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_SHIFTOR_H
