
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
    size_type pattern_hash_;

public:
    RabinKarpImpl() : power_(1), pattern_hash_(0) {}
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

        size_type power = 1;
        power = 1;
        for (size_type i = 1; i < power_len; ++i) {
            power *= kPower;
        }
        this->power_ = power;

        size_type pattern_hash = 0;
        for (size_type i = 0; i < length; ++i) {
            pattern_hash = pattern_hash * kPower + pattern[i];
        }
        this->pattern_hash_ = pattern_hash;

        return true;
    }

    SM_INLINE_DECLARE(size_type)
    next_hash(size_type hash_code, char_type prefix, char_type suffix) const {
        return ((hash_code - prefix * this->power_) * kPower + suffix);
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        if (likely(pattern_len <= text_len)) {
            const char_type * text_first = text;
#if 1
            // Runs strchr() on the first section of the text as it has a lower
            // algorithmic complexity for discarding the first non-matching characters.

            // First character of text is in the pattern.
            text = ::strchr(text_first, (unsigned char)*pattern);
            if (text == nullptr) {
                return Status::NotFound;
            }
#else
            const char_type * text_end = text + text_len;

            // First character of text is in the pattern.
            while (text < text_end) {
                if (*text == *pattern) {
                    goto search_start;
                }
                text++;
            }
            return Status::NotFound;

search_start:
#endif
            size_type offset = text - text_first;
            ssize_t scan_len = ssize_t(text_len - offset - pattern_len);
            if (scan_len < 0) {
                return Status::NotFound;
            }

            size_type hash_code = 0;
            for (size_type i = 0; i < pattern_len; ++i) {
                hash_code = hash_code * kPower + text[i];
            }

            const char_type * text_limit = text + scan_len;
            while (text <= text_limit) {
                // Double check: a + bcd + e, abcd
                if (unlikely(hash_code == this->pattern_hash_)) {
                    if (::memcmp((const void *)text, (const void *)&pattern[0],
                                 pattern_len * sizeof(char_type)) == 0) {
                        return Long(text - text_first);
                    }
                }

                // Move the hash value to next char.
                hash_code = next_hash(hash_code, *text, *(text + pattern_len));
                text++;
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
