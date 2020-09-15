
#ifndef STRING_MATCH_RABIN_KARP_H
#define STRING_MATCH_RABIN_KARP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"

#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

namespace StringMatch {

//
// Karp-Rabin algorithm
//
// See: http://www-igm.univ-mlv.fr/~lecroq/string/node5.html
//
template <typename CharTy, std::size_t Power = 31>
class RabinKarpImpl {
public:
    typedef RabinKarpImpl<CharTy, Power>    this_type;
    typedef CharTy                          char_type;
    typedef std::size_t                     size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                            uchar_type;

    static const size_type kPower = Power;
    static const size_type kMaxAscii = 256;

private:
    size_type power_;
    size_type pattern_code_;

public:
    RabinKarpImpl() : power_(1), pattern_code_(0) {}
    ~RabinKarpImpl() {
        this->destroy();
    }

    static const char * name() {
        switch (kPower) {
            case 2:
                return "Rabin-Karp 2";
            case 31:
                return "Rabin-Karp 31";
            default:
                return "Rabin-Karp N";
        }
    }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);

        size_type power_len;
        if ((kPower & (kPower - 1)) == 0)
            power_len = length % (sizeof(size_type) * 8);
        else
            power_len = length;

        this->power_ = 1;
        for (size_type i = 1; i < power_len; ++i) {
            this->power_ *= kPower;
        }

        this->pattern_code_ = 0;
        for (size_type i = 0; i < length; ++i) {
            this->pattern_code_ = this->pattern_code_ * kPower + pattern[i];
        }

        return true;
    }

    SM_INLINE_DECLARE(size_type)
    rehash(size_type hash_code, char_type prefix, char_type suffix) const {
        return ((hash_code - prefix * this->power_) * kPower + suffix);
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        if (likely(pattern_len <= text_len)) {
            size_type hash_code = 0;
            for (size_type i = 0; i < pattern_len; ++i) {
                hash_code = hash_code * kPower + text[i];
            }

            for (size_type i = 0; i < (text_len - pattern_len); ++i) {
                // Double check: a + bcd + e, abcd
                if (unlikely(hash_code == this->pattern_code_)) {
                    if (::memcmp((const void *)&text[i], (const void *)&pattern[0],
                                 pattern_len * sizeof(char_type)) == 0) {
                        return Long(i);
                    }
                }

                // Move the hash value to next char.
                hash_code = rehash(hash_code, text[i], text[i + pattern_len]);
            }
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< RabinKarpImpl<char, 31> >     RabinKarp31;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< RabinKarpImpl<wchar_t, 31> >  RabinKarp31;
}

namespace AnsiString {
    typedef AlgorithmWrapper< RabinKarpImpl<char, 2> >      RabinKarp2;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< RabinKarpImpl<wchar_t, 2> >   RabinKarp2;
}

} // namespace StringMatch

#endif // STRING_MATCH_RABIN_KARP_H
