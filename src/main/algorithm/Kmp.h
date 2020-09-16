
#ifndef STRING_MATCH_KMP_H
#define STRING_MATCH_KMP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <memory>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"
#include "jstd/scoped_ptr.h"

namespace StringMatch {

template <typename CharTy>
class KmpImpl {
public:
    typedef KmpImpl<CharTy>     this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

private:
    jstd::scoped_array<int> kmp_next_;

public:
    KmpImpl() {}
    ~KmpImpl() {
        this->destroy();
    }

    static const char * name() { return "Kmp"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const {
        return (this->kmp_next_.get() != nullptr);
    }

    void destroy() {
        this->kmp_next_.reset();
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) SM_NOEXCEPT {
        assert(pattern != nullptr);
        int * kmp_next = new int[length + 1];
        if (kmp_next != nullptr) {
            kmp_next[0] = -1;
            kmp_next[1] = 0;
            for (size_type index = 1; index < length; ++index) {
                if (likely(pattern[index] != pattern[kmp_next[index - 1]])) {
                    kmp_next[index + 1] = 0;
                }
                else {
                    kmp_next[index + 1] = kmp_next[index] + 1;
                }
            }
        }
        this->kmp_next_.reset(kmp_next);
        return (kmp_next != nullptr);
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text_first, size_type text_len,
           const char_type * pattern_first, size_type pattern_len) const {
        assert(text_first != nullptr);
        assert(pattern_first != nullptr);

        int * kmp_next = this->kmp_next_.get();
        assert(kmp_next != nullptr);

        if (likely(pattern_len <= text_len)) {
            register const char_type * text = text_first;
            register const char_type * pattern = pattern_first;

            const char_type * text_end = text_first + (text_len - pattern_len);
            const char_type * pattern_end = pattern + pattern_len;
            do {
                if (likely(*text != *pattern)) {
                    int matched_chars = (int)(pattern - pattern_first);
                    if (likely(matched_chars == 0)) {
                        text++;
                        if (likely(text <= text_end)) {
                            // continue
                        }
                        else {
                            // Not found
                            return Status::NotFound;
                        }
                    }
                    else {
                        assert(matched_chars >= 1);
                        int partial_matched = kmp_next[matched_chars];
                        int text_offset = matched_chars - partial_matched;
                        assert(text_offset >= 1);
                        pattern = pattern_first + partial_matched;
                        text = text + text_offset;
                        if (unlikely(text > text_end)) {
                            // Not found
                            return Status::NotFound;
                        }
                    }
                }
                else {
                    text++;
                    pattern++;
                    if (likely(pattern >= pattern_end)) {
                        // Has found
                        Long index_of = (Long)((text - text_first) - (intptr_t)pattern_len);
                        assert(index_of >= 0);
                        return index_of;
                    }
                    assert(text < (text_first + text_len));
                }
            } while (1);
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< KmpImpl<char> >    Kmp;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< KmpImpl<wchar_t> > Kmp;
}

} // namespace StringMatch

#endif // STRING_MATCH_KMP_H
