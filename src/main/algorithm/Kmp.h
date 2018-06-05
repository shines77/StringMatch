
#ifndef STRING_MATCH_KMP_H
#define STRING_MATCH_KMP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <memory>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"

namespace StringMatch {

template <typename CharTy>
class KmpImpl {
public:
    typedef KmpImpl<CharTy>     this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

private:
    std::unique_ptr<int[]> kmp_next_;

public:
    KmpImpl() : kmp_next_() {}
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
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);
        int * kmp_next = new int[length + 1];
        if (kmp_next != nullptr) {
            kmp_next[0] = -1;
            kmp_next[1] = 0;
            for (size_t index = 1; index < length; ++index) {
                if (unlikely(pattern[index] == pattern[kmp_next[index - 1]])) {
                    kmp_next[index + 1] = kmp_next[index] + 1;
                }
                else {
                    kmp_next[index + 1] = 0;
                }
            }
        }
        this->kmp_next_.reset(kmp_next);
        return (kmp_next != nullptr);
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text_start, size_type text_len,
           const char_type * pattern_start, size_type pattern_len) const {
        assert(text_start != nullptr);
        assert(pattern_start != nullptr);

        int * kmp_next = this->kmp_next_.get();
        assert(kmp_next != nullptr);

        if (likely(pattern_len <= text_len)) {
            register const char_type * text = text_start;
            register const char_type * pattern = pattern_start;

            const char_type * text_end = text_start + (text_len - pattern_len);
            const char_type * pattern_end = pattern + pattern_len;
            do {
                if (likely(*text != *pattern)) {
                    int pattern_idx = (int)(pattern - pattern_start);
                    if (likely(pattern_idx == 0)) {
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
                        assert(pattern_idx >= 1);
                        int pattern_offset = kmp_next[pattern_idx];
                        int text_offset = pattern_idx - pattern_offset;
                        assert(text_offset >= 1);
                        pattern = pattern_start + pattern_offset;
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
                        Long pos = (Long)((text - text_start) - (intptr_t)pattern_len);
                        assert(pos >= 0);
                        return pos;
                    }
                    assert(text < (text_start + text_len));
                }
            } while (1);
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< KmpImpl<char> >    Kmp;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< KmpImpl<wchar_t> > Kmp;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_KMP_H
