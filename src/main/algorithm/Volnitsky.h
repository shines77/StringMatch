
#ifndef STRING_MATCH_VOLNITSKY_H
#define STRING_MATCH_VOLNITSKY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <algorithm>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"
#include "algorithm/AlgorithmUtils.h"
#include "algorithm/MemMem.h"

namespace StringMatch {

//
// See: http://volnitsky.com/project/str_search/index.html
// See: https://github.com/ox/Volnitsky-ruby/blob/master/volnitsky.cc
//

template <typename CharTy>
class VolnitskyImpl {
public:
    typedef VolnitskyImpl<CharTy>           this_type;
    typedef CharTy                          char_type;
    typedef uint16_t                        word_t;
    typedef std::size_t                     size_type;
    typedef std::ptrdiff_t                  ssize_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                            uchar_type;

    static const size_type kHashMax = 65536;
    static const size_type kWordSize = sizeof(word_t);

private:
    BitMap<kHashMax, word_t> hashmap_;

public:
    VolnitskyImpl() {}
    ~VolnitskyImpl() {
        this->destroy();
    }

    static const char * name() { return "Volnitsky"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return (this->hashmap_.data() != nullptr); }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);

        if (length < (std::numeric_limits<uint8_t>::max)()) {
            this->hashmap_.init();

            ssize_type max_limit = ssize_type(length - kWordSize + 1);
            for (ssize_type i = 0; i < max_limit; i++) {
                size_type word = static_cast<size_type>(*(word_t *)&pattern[i]);
                while (this->hashmap_.getv(word) != 0) {
                    word = this->hashmap_.nextKey(word);
                }
                this->hashmap_.set(word, static_cast<uint8_t>(i + 1));
            }    
        }

        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        // check arg sizes 
        if (!((text_len < pattern_len * 2) ||
             (pattern_len < 2 * kWordSize - 1) ||
             (pattern_len >= (std::numeric_limits<uint8_t>::max)()))) {
            const char_type * src = text + pattern_len - kWordSize;
            const char_type * text_end = text + text_len;
            const char_type * pattern_end = pattern + pattern_len;
            const char_type * src_limit = text_end - kWordSize + 1;
            while (src < src_limit) {
                assert(src >= text);
                assert(src < (text + (text_len - kWordSize + 1)));
                size_type word = static_cast<size_type>(*(word_t *)src);
                size_type offset = this->hashmap_.getv(word);
                if (likely(offset == 0)) {
                    src += pattern_len - kWordSize + 1;
                }
                else {
                    ssize_type index;
                    do {
                        const char_type * src_start = src - (offset - 1);
                        if (likely((src_start + pattern_len) <= text_end)) {
                            const char_type * target = pattern;
                            while (target < pattern_end) {
                                assert(src_start >= text);
                                assert(src_start < text_end);
                                if (*src_start++ != *target++) {
                                    word = this->hashmap_.nextKey(word);
                                    goto SKIP_TO_NEXT_HASH;
                                }
                            }
                        }
                        else {
                            word = this->hashmap_.nextKey(word);
                            goto SKIP_TO_NEXT_HASH;
                        }

                        index = src - (offset - 1) - text;
                        assert(index >= 0);
                        assert(index < ssize_type(text_len));
                        return Long(index);
SKIP_TO_NEXT_HASH:
                        ;
                    } while ((offset = this->hashmap_.getv(word)) != 0);

                    src += pattern_len - kWordSize + 1;
                }
            }

            //if (::memcmp((const void *)&src,
            //             (const void *)&pattern[0], pattern_len) == 0) {
            //    return Long(src - text);
            //}

            return Status::NotFound;
        }
        else {
#if defined(_MSC_VER)
            const char_type * haystack = (const char_type *)memmem_msvc(
                                         (const void *)text, text_len * sizeof(char_type),
                                         (const void *)pattern, pattern_len * sizeof(char_type));
            if (likely(haystack != nullptr))
                return (Long)(haystack - text);
            else
                return Status::NotFound;
#else
            // fallback to std::search
            StringRef sText(text, text_len);
            StringRef sPattern(pattern, pattern_len);
            StringRef::iterator iter = std::search(sText.begin(), sText.end(),
                                                   sPattern.begin(), sPattern.end());
            if (likely(iter != sText.end()))
                return (Long)(iter - sText.begin());
            else
                return Status::NotFound;
#endif
        }
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< VolnitskyImpl<char> >    Volnitsky;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< VolnitskyImpl<wchar_t> > Volnitsky;
}

} // namespace StringMatch

#endif // STRING_MATCH_VOLNITSKY_H
