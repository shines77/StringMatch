
#ifndef STRING_MATCH_STRSTR_H
#define STRING_MATCH_STRSTR_H

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
class StrStrImpl {
public:
    typedef StrStrImpl<CharT>   this_type;
    typedef CharT               char_type;
    typedef std::tuple<int *>    tuple_type;

private:
    bool alive_;
    tuple_type args_;

public:
    StrStrImpl() : alive_(false), args_(nullptr) {}
    ~StrStrImpl() {
        this->destroy();
    }

    bool need_preprocessing() const { return false; }
    bool is_alive() const { return this->alive_; }

    const tuple_type & get_args() const { return this->args_; }
    void set_args(const tuple_type & args) {
        if ((void *)&args_ != (void *)&args) {
            args_ = args;
        }
    }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_t length) {
        this->alive_ = true;
        return true;
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern, size_t pattern_len,
                      const tuple_type & args) {
        return this_type::search(text, text_len, pattern, pattern_len);
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern_in, size_t pattern_len) {
        assert(text != nullptr);
        assert(pattern_in != nullptr);
#if 0
        const char * substr = strstr(text, pattern_in);
        if (substr != nullptr)
            return (int)(substr - text);
        else
            return Status::NotFound;
#else
        if (pattern_len <= text_len) {
            register const char * target = text;
            register const char * pattern = pattern_in;

            if ((size_t)target | (size_t)pattern) {
                const char * substr = strstr(target, pattern);
                if (substr != nullptr)
                    return (int)(substr - target);
                else
                    return Status::NotFound;
            }
            // Invalid parameters
            return Status::InvalidParameter;
        }
        else {
            // Not found
            return Status::NotFound;
        }
#endif
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< StrStrImpl<char> >    StrStr;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< StrStrImpl<wchar_t> > StrStr;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_STRSTR_H
