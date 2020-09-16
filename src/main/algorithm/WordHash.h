
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
#include "algorithm/MemMem.h"

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
                this->hashmap_.set(word, 1);
            }
        }
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search1(const char_type * text, size_type text_len,
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
                        break;
                    }
                    matched++;
                    if (matched >= ssize_type(pattern_len - kWordSize + 1)) {
                        assert(tpos < max_limit);
                        if (::memcmp((const void *)&text[tpos], (const void *)&pattern[0],
                                     pattern_len * sizeof(char_type)) == 0) {
                            return Long(tpos);
                        }
                        break;
                    }
                }
            }

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
            const char_type * src_start = text + pattern_len - kWordSize;
            const char_type * text_end = text + text_len;
            const char_type * pattern_end = pattern + pattern_len;
            const char_type * src_limit = text_end - kWordSize + 1;
            while (src_start < src_limit) {
                assert(src_start >= text);
                assert(src_start < (text + (text_len - kWordSize + 1)));
                size_type word = static_cast<size_type>(*(word_t *)src_start);
                size_type exists = this->hashmap_.getv(word);
                if (likely(exists == 0)) {
                    src_start += pattern_len - kWordSize + 1;
                }
                else {
                    const char_type * source = src_start - (pattern_len - kWordSize);
                    const char_type * target = pattern;
                    if (likely(*source != *target)) {
                        src_start += 1;
                    }
                    else {
                        const char_type * src = source;
                        ssize_type index;
                        source++;
                        target++;
                        do {
                            if (likely(*source != *target)) {
                                src_start += 1;
                                goto SKIP_TO_NEXT_POS;
                            }

                            target++;
                            if (likely(target < pattern_end)) {
                                source++;
                                assert(source < text_end);
                            }
                            else {
                                index = src - text;
                                assert(index >= 0);
                                assert(index < ssize_type(text_len));
                                return Long(index);
                            }
                        } while (1);
SKIP_TO_NEXT_POS:
                        ;
                    }
                }
            }

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

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search3(const char_type * text, size_type text_len,
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
                size_type exists = this->hashmap_.getv(word);
                if (likely(exists == 0)) {
                    src += pattern_len - kWordSize + 1;
                }
                else {
                    ssize_type index;
                    const char_type * src_start = src - (pattern_len - kWordSize);
                    const char_type * source = src_start;
                    const char_type * target = pattern;
                    do {
                        if (likely(*source != *target)) {
                            src += 1;
                            goto SKIP_TO_NEXT_POS;
                        }
                        else {
                            target++;
                            if (likely(target < pattern_end)) {
                                source++;
                                assert(source < text_end);
                            }
                            else {
                                index = src_start - text;
                                assert(index >= 0);
                                assert(index < ssize_type(text_len));
                                return Long(index);
                            }
                        }
                    } while (1);
SKIP_TO_NEXT_POS:
                    ;
                }
            }

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
    typedef AlgorithmWrapper< WordHashImpl<char> >    WordHash;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< WordHashImpl<wchar_t> > WordHash;
}

} // namespace StringMatch

#endif // STRING_MATCH_WORDHASH_H
