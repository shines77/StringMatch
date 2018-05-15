
#ifndef STRING_MATCH_BOYERMOORE_H
#define STRING_MATCH_BOYERMOORE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdint>
#include <string>
#include <assert.h>

#include "StringMatch.h"
#include "StringRef.h"

namespace StringMatch {

template <typename CharTy>
class BoyerMooreImpl {
public:
    typedef BoyerMooreImpl<CharTy>  this_type;
    typedef CharTy                  char_type;

private:
    std::unique_ptr<int[]> bmGs_;
    std::unique_ptr<int[]> bmBc_;

public:
    BoyerMooreImpl() : bmGs_(), bmBc_() {}
    ~BoyerMooreImpl() {
        this->destroy();
    }

    bool need_preprocessing() const { return true; }

    bool is_alive() const {
        return (this->bmGs() != nullptr && this->bmBc() != nullptr);
    }

    void destroy() {
        this->bmGs_.reset();
        this->bmBc_.reset();
    }

private:
    /* Preprocessing bad characters. */
    static void preBmBc(const char * pattern, size_t length,
                        int * bmBc, int bcLen) {
        assert(pattern != nullptr);
        assert(bmBc != nullptr);
        assert(bcLen == 256L);

        for (int i = 0; i < bcLen; ++i) {
            bmBc[i] = (int)length;
        }
        for (int i = 0; i < (int)length - 1; ++i) {
            bmBc[(int)pattern[i]] = ((int)length - 1) - i;
        }
    }

    /* Preprocessing suffixes. */
    static void suffixes(const char * pattern, size_t length, int * suffix) {
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
                if (distance < 0) {
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
    static bool preBmGs(const char * pattern, size_t length,
                        int * bmGs, int gsLen) {
        int i, j;
        int len = (int)length;

        if (length <= 0) {
            return false;
        }

        std::unique_ptr<int[]> suffix(new int[len]);
        if (suffix.get() == nullptr) {
            return false;
        }
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
                    if (bmGs[j] == len) {
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

public:
    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_t length) {
        bool success = false;
        assert(pattern != nullptr);

        const int gsLen = (int)length + 1;
        int * bmGs = new int[gsLen];
        if (bmGs != nullptr) {
            /* Preprocessing good suffixes. */
            success = this_type::preBmGs(pattern, length, bmGs, gsLen);
        }
        this->bmGs_.reset(bmGs);

        static const int bcLen = 256;
        int * bmBc = new int[bcLen];
        if (bmBc != nullptr) {
            /* Preprocessing bad characters. */
            this_type::preBmBc(pattern, length, bmBc, bcLen);
        }
        this->bmBc_.reset(bmBc);
        return (success && (bmBc != nullptr) && (bmGs != nullptr));;
    }

    /* Searching */
    int search(const char_type * text, size_t text_len,
               const char_type * pattern_in, size_t pattern_len) const {
        assert(text != nullptr);
        assert(pattern_in != nullptr);

        int * bmGs = this->bmGs_.get();
        int * bmBc = this->bmBc_.get();

        assert(bmBc != nullptr);
        assert(bmGs != nullptr);

        if (pattern_len <= text_len) {
            if ((size_t)text | (size_t)pattern_in | (size_t)bmGs | (size_t)bmBc) {
                const char * pattern_end = pattern_in;
                const char * target_end = text + (text_len - pattern_len);
                const int pattern_last = (int)pattern_len - 1;
                int target_idx = 0;
                do {
                    register const char * target = text + target_idx + pattern_last;
                    register const char * pattern = pattern_in + pattern_last;
                    assert(target < (text + text_len));

                    while (pattern >= pattern_end) {
                        if (*target != *pattern) {
                            break;
                        }
                        target--;
                        pattern--;
                    }

                    if (pattern >= pattern_end) {
                        int pattern_idx = (int)(pattern - pattern_in);
                        target_idx += sm_max(bmGs[pattern_idx],
                                             bmBc[(int)*target] - (pattern_last - pattern_idx));
                    }
                    else {
                        assert(target_idx >= 0);
                        assert(target_idx < (int)text_len);
                        // Found
                        return target_idx;
                    }
                } while (target_idx <= (int)(text_len - pattern_len));

                // Not found
                return Status::NotFound;
            }
            // Invalid parameters
            return Status::InvalidParameter;
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
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< BoyerMooreImpl<wchar_t> > BoyerMoore;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_BOYERMOORE_H
