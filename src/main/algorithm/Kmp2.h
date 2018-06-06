
#ifndef STRING_MATCH_KMP2_H
#define STRING_MATCH_KMP2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <memory>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"
#include "jstd/scoped_ptr.h"

namespace StringMatch {

template <typename CharTy>
class KmpImpl2 {
public:
    typedef KmpImpl2<CharTy>    this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

private:
    jstd::scoped_array<int> kmp_next_;

public:
    KmpImpl2() : kmp_next_() {}
    ~KmpImpl2() {
        this->destroy();
    }

    static const char * name() { return "Kmp2"; }
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
            int index = 0, next = -1;
            while (index < (int)length) {
                while (next > -1 && pattern[index] != pattern[next]) {
                    next = kmp_next[next];
                }
                index++;
                next++;
                if (pattern[index] == pattern[next])
                    kmp_next[index] = kmp_next[next];
                else
                    kmp_next[index] = next;
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

            const char_type * text_end = text_start + text_len;
            int pattern_idx = 0;
            while (text < text_end) {
                while (pattern_idx > -1 && *text != pattern[pattern_idx])
                    pattern_idx = kmp_next[pattern_idx];
                pattern_idx++;
                text++;
                if (pattern_idx >= (Long)pattern_len) {
                    // Has found
                    Long index_of = ((Long)(text - text_start) - pattern_len);
                    assert(index_of >= 0);
                    return index_of;
                }
            }
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< KmpImpl2<char> >    Kmp2;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< KmpImpl2<wchar_t> > Kmp2;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_KMP2_H
