
#ifndef STRING_MATCH_WORDHASH_H
#define STRING_MATCH_WORDHASH_H

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

namespace StringMatch {

//
// See: https://blog.csdn.net/liangzhao_jay/article/details/8792486
//

template <typename CharTy>
class WordHashImpl {
public:
    typedef WordHashImpl<CharTy>            this_type;
    typedef CharTy                          char_type;
    typedef uint16_t                        word_t;
    typedef std::size_t                     size_type;
    typedef std::ptrdiff_t                  ssize_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                            uchar_type;

    static const size_type kHashMax = 65536;
    static const size_type kWordSize = sizeof(word_t);

private:
    BitMap<kHashMax> hashmap_;

public:
    WordHashImpl() {}
    ~WordHashImpl() {
        this->destroy();
    }

    static const char * name() { return "WordHash"; }
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
            ssize_type max_limit = ssize_type(text_len - pattern_len + 1);
            for (ssize_type i = 0; i < max_limit; i++) {
                ssize_type matched = 0;
                ssize_type tpos = i + pattern_len - kWordSize;
                ssize_type ppos = pattern_len - kWordSize;
                for (; ppos >= 0; --tpos, --ppos) {
                    assert(tpos >= 0);
                    assert(tpos < ssize_type(text_len - kWordSize + 1));
                    assert(ppos < ssize_type(pattern_len - kWordSize + 1));
                    size_type word = static_cast<size_type>(*(word_t *)&text[tpos]);
                    if (this->hashmap_.getv(word) == 0) {
                        if (matched == 0) {
                            i += pattern_len - kWordSize;
                        }
                        matched = -1;
                        break;
                    }
                    matched++;
                    if (matched < ssize_type(pattern_len - kWordSize + 1)) {
                        //
                    }
                    else {
                        assert(tpos < max_limit);
                        if (::memcmp((const void *)&text[tpos],
                                     (const void *)&pattern[0], pattern_len) == 0) {
                            return Long(tpos);
                        }
                        matched = -1;
                        break;
                    }
                }
                if (matched >= 0) {
                    return Long(i);
                }
            }

            return Status::NotFound;
        }
        else { 
            // fallback to std::search
            StringRef sText(text, text_len);
            StringRef sPattern(pattern, pattern_len);
            StringRef::iterator iter = std::search(sText.begin(), sText.end(),
                                                   sPattern.begin(), sPattern.end());
            if (likely(iter != sText.end()))
                return (Long)(iter - sText.begin());
            else
                return Status::NotFound;
        }
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< WordHashImpl<char> >    WordHash;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< WordHashImpl<wchar_t> > WordHash;
}

} // namespace StringMatch

#endif // STRING_MATCH_WORDHASH_H
