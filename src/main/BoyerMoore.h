
#ifndef STRING_MATCH_BOYERMOORE_H
#define STRING_MATCH_BOYERMOORE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "StringMatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <cstdint>
#include <string>
#include <assert.h>

#include "StringRef.h"

namespace StringMatch {

template <typename CharT>
class BoyerMooreImpl {
public:
    typedef BoyerMooreImpl<CharT>       this_type;
    typedef CharT                       char_type;
    typedef std::tuple<int *, int *>    tuple_type;

private:
    std::unique_ptr<int[]> bmGs_;
    std::unique_ptr<int[]> bmBc_;
    tuple_type args_;

public:
    BoyerMooreImpl() : bmGs_(), bmBc_(), args_(nullptr, nullptr) {}
    ~BoyerMooreImpl() {}

    const tuple_type & get_args() const { return args_; }
    void set_args(const tuple_type & args) {
        if ((void *)&args_ != (void *)&args) {
            args_ = args;
        }
        // Update args
        bmGs_.reset(std::get<0>(args_));
        bmBc_.reset(std::get<1>(args_));
    }

    int * bmGs() const { return this->bmGs_.get(); }
    void set_bmGs(int * bmGs) {
        this->bmGs_.reset(bmGs);
    }

    int * bmBc() const { return this->bmBc_.get(); }
    void set_bmBc(int * bmBc) {
        this->bmBc_.reset(bmBc);
    }

    bool is_alive() const {
        return (this->bmGs() != nullptr && this->bmBc() != nullptr);
    }

    void free() {
        bmGs_.reset();
        bmBc_.reset();
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
        int * bmGs = nullptr, * bmBc = nullptr;
        bool success = this_type::preprocessing(pattern, length, bmGs, bmBc);
        // Update args
        args_ = std::make_tuple(bmGs, bmBc);
        bmGs_.reset(bmGs);
        bmBc_.reset(bmBc);
        return success;
    }

    /* Preprocessing */
    static bool preprocessing(const char * pattern, size_t length,
                              int * &out_bmGs, int * &out_bmBc) {
        bool success = false;
        assert(pattern != nullptr);

        const int gsLen = (int)length + 1;
        int * bmGs = new int[gsLen];
        if (bmGs != nullptr) {
            /* Preprocessing good suffixes. */
            success = this_type::preBmGs(pattern, length, bmGs, gsLen);
        }
        out_bmGs = bmGs;

        static const int bcLen = 256;
        int * bmBc = new int[bcLen];
        if (bmBc != nullptr) {
            /* Preprocessing bad characters. */
            this_type::preBmBc(pattern, length, bmBc, bcLen);
        }
        out_bmBc = bmBc;

        return (success && (bmBc != nullptr) && (bmGs != nullptr));
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern, size_t pattern_len,
                      const tuple_type & args) {
        int * bmGs = std::get<0>(args);
        int * bmBc = std::get<1>(args);
        return search(text, text_len, pattern, pattern_len, bmGs, bmBc);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern_, size_t pattern_len,
                      const int * bmGs, const int * bmBc) {
        assert(text != nullptr);
        assert(pattern_ != nullptr);
        assert(bmBc != nullptr);
        assert(bmGs != nullptr);

        if (text_len < pattern_len) {
            // Not found
            return Status::NotFound;
        }

        if ((size_t)text | (size_t)pattern_ | (size_t)bmGs | (size_t)bmBc) {
            const char * pattern_end = pattern_;
            const char * target_end = text + (text_len - pattern_len);
            const int pattern_last = (int)pattern_len - 1;
            int target_idx = 0;
            do {
                register const char * target = text + target_idx + pattern_last;
                register const char * pattern = pattern_ + pattern_last;
                assert(target < (text + text_len));

                while (pattern >= pattern_end) {
                    if (*target != *pattern) {
                        break;
                    }
                    target--;
                    pattern--;
                }

                if (pattern >= pattern_end) {
                    int pattern_idx = (int)(pattern - pattern_);
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

private:
    // Reserve codes
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
    typedef BasicAlgorithm< BoyerMooreImpl<char> >    BoyerMoore;
} // namespace AnsiString

namespace UnicodeString {
    typedef BasicAlgorithm< BoyerMooreImpl<wchar_t> > BoyerMoore;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_BOYERMOORE_H
