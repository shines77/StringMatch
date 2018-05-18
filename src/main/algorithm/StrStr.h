
#ifndef STRING_MATCH_STRSTR_H
#define STRING_MATCH_STRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include "basic/stdint.h"
#include <string>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"
#include "support/StringRef.h"

namespace StringMatch {

template <typename CharTy>
class StrStrImpl {
public:
    typedef StrStrImpl<CharTy>  this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

private:
    bool alive_;

public:
    StrStrImpl() : alive_(true) {}
    ~StrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "strstr()"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return this->alive_; }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        return true;
    }

    /* Searching */
    int search(const char_type * text, size_type text_len,
               const char_type * pattern_in, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern_in != nullptr);
#if 1
        const char * substr = strstr(text, pattern_in);
        if (substr != nullptr)
            return (int)(substr - text);
        else
            return Status::NotFound;
#else
        if (pattern_len <= text_len) {
            register const char * target = text;
            register const char * pattern = pattern_in;

            const char * substr = strstr(target, pattern);
            if (substr != nullptr)
                return (int)(substr - target);
            else
                return Status::NotFound;
        }

        return Status::NotFound;
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
