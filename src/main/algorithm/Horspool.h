
#ifndef STRING_MATCH_HORSPOOL_H
#define STRING_MATCH_HORSPOOL_H

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
// See: http://www-igm.univ-mlv.fr/~lecroq/string/node18.html#SECTION00180
//

namespace StringMatch {

template <typename CharTy>
class HorspoolImpl {
public:
    typedef HorspoolImpl<CharTy>    this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                    uchar_type;

    static const size_t kMaxAscii = 256;

private:
    int hpBc_[kMaxAscii];

public:
    HorspoolImpl() {}
    ~HorspoolImpl() {
        this->destroy();
    }

    static const char * name() { return "Horspool"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        bool success = false;
        assert(pattern != nullptr);

        for (size_t i = 0; i < kMaxAscii; ++i) {
            this->hpBc_[i] = (int)length;
        }
        for (Long i = 0; i < ((Long)length - 1); ++i) {
            this->hpBc_[(uchar_type)pattern[i]] = (int)((Long)length - 1 - i);
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
            int * shift = (int *)&this->hpBc_[0];
            assert(shift != nullptr);

            const char_type * pattern_end = pattern + pattern_len;
            const Long scan_len = (Long)(text_len - pattern_len);
            const Long pattern_last = (Long)pattern_len - 1;
            Long index = 0;
            do {
                register const char_type * source = text + pattern_last + index;
                register const char_type * target = pattern + pattern_last;
                assert(source >= text && source < (text + text_len));

                // Save the last compare char.
                uchar_type last_char = (uchar_type)*source;

                while (likely(target >= pattern)) {
                    if (likely(*source != *target)) {
                        index += shift[last_char];
                        break;
                    }
                    source--;
                    target--;
                    if (likely(target < pattern)) {
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
    typedef AlgorithmWrapper< HorspoolImpl<char> >      Horspool;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< HorspoolImpl<wchar_t> >   Horspool;
}

} // namespace StringMatch

#endif // STRING_MATCH_HORSPOOL_H
