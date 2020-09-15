
#ifndef STRING_MATCH_SSE_STRSTR2A_H
#define STRING_MATCH_SSE_STRSTR2A_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <limits.h>
#include <assert.h>
#include <nmmintrin.h>  // For SSE 4.2
#include <type_traits>

#ifndef __cplusplus
#include <stdalign.h>   // C11 defines _Alignas().  This header defines alignas()
#endif

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"
#include "algorithm/SSEHelper.h"
#include "asm/asmlib.h"

namespace StringMatch {

template <typename CharTy>
class SSEStrStrAImpl {
public:
    typedef SSEStrStrAImpl<CharTy>  this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

    SSEStrStrAImpl() {}
    ~SSEStrStrAImpl() {
        this->destroy();
    }

    static const char * name() { return "A_strstr_sse42()"; }
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
        const char_type * substr = A_strstr(text, pattern);
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< SSEStrStrAImpl<char> >    SSEStrStrA;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< SSEStrStrAImpl<wchar_t> > SSEStrStrA;
}

} // namespace StringMatch

#endif // STRING_MATCH_SSE_STRSTR2A_H
