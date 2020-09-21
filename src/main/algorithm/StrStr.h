
#ifndef STRING_MATCH_STRSTR_H
#define STRING_MATCH_STRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

namespace StringMatch {

template <typename CharTy>
class StrStrImpl {
public:
    typedef StrStrImpl<CharTy>  this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

    StrStrImpl() {}
    ~StrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "strstr()"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        /* Don't need to do preprocessing. */
        SM_UNUSED_VAR(pattern);
        SM_UNUSED_VAR(length);
        return true;
    }

    /* Searching */
    Long search(const char_type * text, size_type text_len,
                const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
#if !defined(_UNICODE)
        const char_type * substr = ::strstr(text, pattern);
#else
        const char_type * substr = ::wcsstr(text, pattern);
#endif
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< StrStrImpl<char> >    StrStr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< StrStrImpl<wchar_t> > StrStr;
}

} // namespace StringMatch

#endif // STRING_MATCH_STRSTR_H
