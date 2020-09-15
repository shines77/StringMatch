
#ifndef STRING_MATCH_BOYERMOORE_H
#define STRING_MATCH_BOYERMOORE_H

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

//
// See: http://www-igm.univ-mlv.fr/~lecroq/string/node14.html#SECTION00140
//

namespace StringMatch {

template <typename CharTy>
class BoyerMooreImpl {
public:
    typedef BoyerMooreImpl<CharTy>  this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                    uchar_type;

    static const size_t kMaxAscii = 256;

private:
    jstd::scoped_array<int> bmGs_;
    int bmBc_[kMaxAscii];

public:
    BoyerMooreImpl() : bmGs_() {}
    ~BoyerMooreImpl() {
        this->destroy();
    }

    static const char * name() { return "BoyerMoore"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const {
        return (this->bmGs_.get() != nullptr);
    }

    void destroy() {
        this->bmGs_.reset();
    }

    /* Preprocessing bad characters. */
    static void preBmBc(const char * pattern, size_type length, int * bmBc) {
        assert(pattern != nullptr);
        assert(bmBc != nullptr);

        for (size_type i = 0; i < kMaxAscii; ++i) {
            bmBc[i] = (int)length;
        }
        for (Long i = 0; i < ((Long)length - 1); ++i) {
            bmBc[(uchar_type)pattern[i]] = (int)((Long)length - 1 - i);
        }
    }

    /* Preprocessing suffixes. */
    static void suffixes(const char * pattern, size_type length, int * suffix) {
        assert(pattern != nullptr);
        assert(suffix != nullptr);

        int index, cursor, distance, offset;
        const int last = (int)length - 1;
        for (index = last - 1; index >= 0; --index) {
            suffix[index] = 0;
        }

        suffix[last] = (int)length;
        cursor = last;
        offset = 0;

        for (index = last - 1; index >= 0; --index) {
            distance = index - cursor;
            if (distance > 0 && suffix[index + offset] < distance) {
                suffix[index] = suffix[index + offset];
            }
            else {
                if (likely(distance < 0)) {
                    cursor = index;
                }
                offset = last - index;
                while ((cursor >= 0) && (pattern[cursor] == pattern[cursor + offset])) {
                    --cursor;
                }
                suffix[index] = index - cursor;
            }
        }
    }

    /* Preprocessing good suffixes. */
    static bool preBmGs(const char * pattern, size_type length, int * bmGs) {
        int i, j;
        int len = (int)length;

        jstd::scoped_array<int> suffix(new int[len]);
        if (likely(suffix.get() != nullptr)) {
            this_type::suffixes(pattern, length, suffix.get());

            assert(pattern != nullptr);
            assert(bmGs != nullptr);
            for (i = 0; i < len; ++i) {
                bmGs[i] = len;
            }

            j = 0;
            for (i = len - 1; i >= 0; --i) {
                if (suffix[i] == i + 1) {
                    for (; j < (len - 1) - i; ++j) {
                        if (unlikely(bmGs[j] == len)) {
                            bmGs[j] = (len - 1) - i;
                        }
                    }
                }
            }

            for (i = 0; i <= len - 2; ++i) {
                bmGs[(len - 1) - suffix[i]] = (len - 1) - i;
            }
            return true;
        }

        return false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) SM_NOEXCEPT {
        bool success = false;
        assert(pattern != nullptr);

        int * bmGs = new int[length + 1];
        if (likely(bmGs != nullptr)) {
            /* Preprocessing good suffixes. */
            success = this_type::preBmGs(pattern, length, bmGs);
        }
        this->bmGs_.reset(bmGs);

        /* Preprocessing bad characters. */
        int * bmBc = (int *)&this->bmBc_[0];
        this_type::preBmBc(pattern, length, bmBc);

        return (success && (bmGs != nullptr));
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        if (unlikely(pattern_len == 0))
            return 0;

        if (likely(pattern_len <= text_len)) {
            int * bmGs = this->bmGs_.get();
            int * bmBc = (int *)&this->bmBc_[0];

            assert(bmGs != nullptr);
            assert(bmBc != nullptr);

            const Long source_last = (Long)(text_len - pattern_len);
            const Long pattern_last = (Long)pattern_len - 1;
            Long source_offset = 0;
            do {
                register const char_type * source = text + source_offset + pattern_last;
                register const char_type * cursor = pattern + pattern_last;
                assert(source >= text && source < (text + text_len));

                while (likely(cursor >= pattern)) {
                    if (likely(*source != *cursor)) {
                        break;
                    }
                    source--;
                    cursor--;
                }

                if (likely(cursor >= pattern)) {
                    Long pattern_idx = cursor - pattern;
                    source_offset += sm_max(bmGs[pattern_idx],
                                            bmBc[(uchar_type)*source] - (pattern_last - pattern_idx));
                }
                else {
                    // Has found
                    assert(source_offset >= 0 && source_offset < (Long)text_len);
                    return source_offset;
                }
            } while (likely(source_offset <= source_last));
        }

        return Status::NotFound;
    }

private:
    // Reserved codes
    static void suffixes_old(const char * pattern, size_t length, int * suffix) {
        int i, f, g;
        int len = (int)length;

        suffix[len - 1] = len;
        g = len - 1;

        for (i = len - 2; i >= 0; --i) {
            if (i > g && suffix[i + len - 1 - f] < i - g) {
                suffix[i] = suffix[i + len - 1 - f];
            }
            else {
                if (i < g) {
                    g = i;
                }
                f = i;
                while ((g >= 0) && (pattern[g] == pattern[g + len - 1 - f])) {
                    --g;
                }
                suffix[i] = f - g;
            }
        }
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< BoyerMooreImpl<char> >    BoyerMoore;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< BoyerMooreImpl<wchar_t> > BoyerMoore;
}

} // namespace StringMatch

#endif // STRING_MATCH_BOYERMOORE_H
