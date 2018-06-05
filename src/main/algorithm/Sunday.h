
#ifndef STRING_MATCH_SUNDAY_H
#define STRING_MATCH_SUNDAY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <memory>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"

//
// See: https://blog.csdn.net/q547550831/article/details/51860017
// See: https://blog.csdn.net/v_JULY_v/article/details/7041827
//

namespace StringMatch {

template <typename CharTy>
class SundayImpl {
public:
    typedef SundayImpl<CharTy>      this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;
    typedef typename detail::uchar_traits<CharTy>::type
                                    uchar_type;

    static const size_t kMaxAscii = 256;

private:
    bool alive_;
    int shift_[kMaxAscii];

public:
    SundayImpl() : alive_(false) {}
    ~SundayImpl() {
        this->destroy();
    }

    static const char * name() { return "Sunday"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const {
        return this->alive_;
    }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        bool success = false;
        assert(pattern != nullptr);

        for (size_t i = 0; i < kMaxAscii; ++i) {
            this->shift_[i] = (int)(length + 1);
        }
        for (size_type i = 0; i < length; ++i) {
            this->shift_[(uchar_type)pattern[i]] = (int)(length - i);
        }

        this->alive_ = true;
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        int * shift = (int *)&this->shift_[0];
        assert(shift != nullptr);

        if (likely(pattern_len <= text_len)) {
            const char_type * pattern_end = pattern + pattern_len;
            const Long source_last = (Long)(text_len - pattern_len);
            Long source_idx = 0;
            do {
                register const char_type * source = text + source_idx;
                register const char_type * cursor = pattern;
                assert(source >= text && source < (text + text_len));

                while (likely(cursor < pattern_end)) {
                    if (likely(*source != *cursor)) {
                        source_idx = source - text;
                        source_idx += shift[(uchar_type)text[source_idx + pattern_len]];
                        break;
                    }
                    source++;
                    cursor++;
                    if (likely(cursor >= pattern_end)) {
                        // Has found
                        assert(source_idx >= 0 && source_idx < (Long)text_len);
                        return source_idx;
                    }
                }
            } while (likely(source_idx <= source_last));
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< SundayImpl<char> >    Sunday;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< SundayImpl<wchar_t> > Sunday;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_SUNDAY_H
