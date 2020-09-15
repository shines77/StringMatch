
#ifndef STRING_MATCH_STD_BOYERMOORE_H
#define STRING_MATCH_STD_BOYERMOORE_H

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
#include <algorithm>
#include <functional>

// If use StringRef in std::search() algorithm ?
#define STD_SEARCH_USE_STRING_REF   1

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"
#include "support/StringRef.h"

namespace StringMatch {

// C++14 >= 201402L, C++17 >= 201703L
#if ((defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)) || (defined(__cplusplus) && (__cplusplus >= 201703L)))

template <typename CharTy>
class StdBoyerMooreImpl {
public:
    typedef StdBoyerMooreImpl<CharTy>   this_type;
    typedef CharTy                      char_type;
    typedef std::size_t                 size_type;

    StdBoyerMooreImpl() {}
    ~StdBoyerMooreImpl() {
        this->destroy();
    }

    static const char * name() { return "std::search()"; }
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
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
#if STD_SEARCH_USE_STRING_REF
        StringRef sText(text, text_len);
        StringRef sPattern(pattern, pattern_len);
        StringRef::iterator iter = std::search(sText.begin(), sText.end(),
                                               std::boyer_moore_searcher(
                                                   sPattern.begin(), sPattern.end()
                                               ));
        if (likely(iter != sText.end()))
            return (Long)(iter - sText.begin());
        else
            return Status::NotFound;
#else
        std::string sText(text, text_len);
        std::string sPattern(pattern, pattern_len);
        StringRef::iterator iter = std::search(sText.begin(), sText.end(),
                                               std::boyer_moore_searcher(
                                                   sPattern.begin(), sPattern.end()
                                               ));
        if (likely(iter != sText.end()))
            return (Long)(iter - sText.begin());
        else
            return Status::NotFound;
#endif
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< StdBoyerMooreImpl<char> >    StdBoyerMoore;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< StdBoyerMooreImpl<wchar_t> > StdBoyerMoore;
}

#endif // C++17: (__cplusplus >= 201703L)

} // namespace StringMatch

#endif // STRING_MATCH_STD_BOYERMOORE_H
