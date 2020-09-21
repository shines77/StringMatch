
#ifndef STRING_MATCH_SUNDAY_H
#define STRING_MATCH_SUNDAY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"

#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <memory>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

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
    typedef typename jstd::uchar_traits<CharTy>::type
                                    uchar_type;

    static const size_t kMaxAscii = 256;

private:
    int shift_[kMaxAscii];

public:
    SundayImpl() {}
    ~SundayImpl() {
        this->destroy();
    }

    static const char * name() { return "Sunday"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return true; }

    void destroy() {
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

        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        if (likely(pattern_len <= text_len)) {
            int * shift = (int *)&this->shift_[0];
            assert(shift != nullptr);

            const char_type * target_end = pattern + pattern_len;
            const Long scan_len = (Long)(text_len - pattern_len);
            Long index = 0;
            do {
                register const char_type * source = text + index;
                register const char_type * target = pattern;
                assert(source >= text && source < (text + text_len));

                while (likely(target < target_end)) {
                    if (likely(*source != *target)) {
                        index = source - text;
                        index += shift[(uchar_type)text[index + pattern_len]];
                        break;
                    }
                    source++;
                    target++;
                    if (likely(target >= target_end)) {
                        // Has found
                        assert(index >= 0 && index < (Long)text_len);
                        return index;
                    }
                }
            } while (likely(index <= scan_len));
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< SundayImpl<char> >    Sunday;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< SundayImpl<wchar_t> > Sunday;
}

} // namespace StringMatch

#endif // STRING_MATCH_SUNDAY_H
