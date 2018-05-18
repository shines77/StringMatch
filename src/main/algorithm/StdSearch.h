
#ifndef STRING_MATCH_STDSEARCH_H
#define STRING_MATCH_STDSEARCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include "basic/stdint.h"
#include <string>
#include <memory>
#include <algorithm>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"
#include "support/StringRef.h"

namespace StringMatch {

template <typename CharTy>
class StdSearchImpl {
public:
    typedef StdSearchImpl<CharTy>   this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

private:
    bool alive_;

public:
    StdSearchImpl() : alive_(true) {}
    ~StdSearchImpl() {
        this->destroy();
    }

    static const char * name() { return "std::search()"; }
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
               const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
#if 1
        StringRef sText(text, text_len);
        StringRef sPattern(pattern, pattern_len);
        StringRef::iterator iter = std::search(sText.begin(), sText.end(),
                                               sPattern.begin(), sPattern.end());
        if (iter != sText.end())
            return (int)(iter - sText.begin());
        else
            return Status::NotFound;
#else
        std::string sText(text, text_len);
        std::string sPattern(pattern, pattern_len);
        std::string::iterator iter = std::search(sText.begin(), sText.end(),
                                                 sPattern.begin(), sPattern.end());
        if (iter != sText.end())
            return (int)(iter - sText.begin());
        else
            return Status::NotFound;
#endif
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< StdSearchImpl<char> >    StdSearch;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< StdSearchImpl<wchar_t> > StdSearch;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_STDSEARCH_H
