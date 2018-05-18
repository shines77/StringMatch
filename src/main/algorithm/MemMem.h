
#ifndef STRING_MATCH_MEMMEM_H
#define STRING_MATCH_MEMMEM_H

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
class MemMemImpl {
public:
    typedef MemMemImpl<CharTy>  this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

private:
    bool alive_;

public:
    MemMemImpl() : alive_(true) {}
    ~MemMemImpl() {
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
        const char * substr = memmem(text, pattern_in);
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
    typedef AlgorithmWrapper< MemMemImpl<char> >    MemMem;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< MemMemImpl<wchar_t> > MemMem;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_MEMMEM_H
