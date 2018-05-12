
#ifndef STRING_MATCH_KMP_H
#define STRING_MATCH_KMP_H

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
class KmpImpl {
public:
    typedef KmpImpl<CharT>      this_type;
    typedef CharT               char_type;
    typedef std::tuple<int *>   tuple_type;

private:
    std::unique_ptr<int[]> kmp_next_;
    tuple_type args_;

public:
    KmpImpl() : kmp_next_(), args_(nullptr) {}
    ~KmpImpl() {}

    const tuple_type & get_args() const { return args_; }
    void set_args(const tuple_type & args) {
        if ((void *)&args_ != (void *)&args) {
            args_ = args;
        }
        // Update args
        kmp_next_.reset(std::get<0>(args_));
    }

    int * kmp_next() const { return this->kmp_next_.get(); }
    void set_kmp_next(int * kmp_next) {
        this->kmp_next_.reset(kmp_next);
    }

    bool is_alive() const {
        return (this->kmp_next() != nullptr);
    }

    void destroy() {
        kmp_next_.reset();
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_t length) {
        int * kmp_next = nullptr;
        bool success = this_type::preprocessing(pattern, length, kmp_next);
        // Update args
        args_ = std::make_tuple(kmp_next);
        kmp_next_.reset(kmp_next);
        return success;
    }

    /* Preprocessing */
    static bool preprocessing(const char * pattern, size_t length, int * &out_kmp_next) {
        assert(pattern != nullptr);

        int * kmp_next = new int[length + 1];
        if (kmp_next != nullptr) {
            kmp_next[0] = -1;
            kmp_next[1] = 0;
            for (size_t index = 1; index < length; ++index) {
                if (pattern[index] == pattern[kmp_next[index - 1]]) {
                    kmp_next[index + 1] = kmp_next[index] + 1;
                }
                else {
                    kmp_next[index + 1] = 0;
                }
            }
        }
        out_kmp_next = kmp_next;
        return (kmp_next != nullptr);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern, size_t pattern_len,
                      const tuple_type & args) {
        int * kmp_next = std::get<0>(args);
        return this_type::search(text, text_len, pattern, pattern_len, kmp_next);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern_, size_t pattern_len,
                      const int * kmp_next) {
        assert(text != nullptr);
        assert(pattern_ != nullptr);
        assert(kmp_next != nullptr);

        if (text_len < pattern_len) {
            // Not found
            return Status::NotFound;
        }

        register const char * target = text;
        register const char * pattern = pattern_;

        if ((size_t)target | (size_t)pattern | (size_t)kmp_next) {
            const char * target_end = text + (text_len - pattern_len);
            const char * pattern_end = pattern + pattern_len;
            do {
                if (*target != *pattern) {
                    int search_index = (int)(pattern - pattern_);
                    if (search_index == 0) {
                        target++;
                        if (target > target_end) {
                            // Not found
                            return Status::NotFound;
                        }
                    }
                    else {
                        assert(search_index >= 1);
                        int search_offset = kmp_next[search_index];
                        int target_offset = search_index - search_offset;
                        assert(target_offset >= 1);
                        pattern = pattern_ + search_offset;
                        target = target + target_offset;
                        if (target > target_end) {
                            // Not found
                            return Status::NotFound;
                        }
                    }
                }
                else {
                    target++;
                    pattern++;
                    if (pattern >= pattern_end) {
                        // Found
                        assert((target - text) >= (intptr_t)pattern_len);
                        int pos = (int)((target - text) - (intptr_t)pattern_len);
                        assert(pos >= 0);
                        return pos;
                    }
                    assert(target < (text + text_len));
                }
            } while (1);
        }
        // Invalid parameters
        return Status::InvalidParameter;
    }
};

namespace AnsiString {
    typedef BasicAlgorithm< KmpImpl<char> >    Kmp;
} // namespace AnsiString

namespace UnicodeString {
    typedef BasicAlgorithm< KmpImpl<wchar_t> > Kmp;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_KMP_H
