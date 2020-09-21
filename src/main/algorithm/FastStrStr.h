
#ifndef STRING_MATCH_FASTSTRSTR_H
#define STRING_MATCH_FASTSTRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE     /* See feature_test_macros(7) */
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>     // for linux: memmem()
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

//
// fast_strstr()
//
// See: https://github.com/RaphaelJ/fast_strstr/blob/master/fast_strstr.c
//

namespace StringMatch {

/**
 * Finds the first occurrence of the sub-string needle in the string haystack.
 * Returns NULL if needle was not found.
 */
static
SM_NOINLINE_DECLARE(void *)
fast_strstr(const char * haystack, const char * needle)
{
    // Empty needle.
    if (!*needle) {
        return (char *)haystack;
    }

    const char needle_first = *needle;

    // Runs strchr() on the first section of the haystack as it has a lower
    // algorithmic complexity for discarding the first non-matching characters.

    // First character of haystack is in the needle.
    haystack = ::strchr(haystack, (unsigned char)needle_first);   
    if (haystack == nullptr) {
        return nullptr;
    }

    // First characters of haystack and needle are the same now. Both are
    // guaranteed to be at least one character long.
    // Now computes the sum of the first needle_len characters of haystack
    // minus the sum of characters values of needle.

    const char * i_haystack = haystack + 1;
    const char * i_needle   = needle   + 1;
    bool         identical  = true;

    unsigned int sums_diff = 0;

    while (*i_haystack && *i_needle) {
        sums_diff += *i_haystack;
        sums_diff -= *i_needle;
        identical &= (*i_haystack++ == *i_needle++);
    }

    // i_haystack now references the (needle_len + 1) - th character.
    // haystack is smaller than needle.
    if (*i_needle) {
        return nullptr;
    }
    else if (identical) {
        return (char *)haystack;
    }

    size_t needle_len_1 = i_needle - (needle + 1);

    // Loops for the remaining of the haystack, updating the sum iteratively.
    const char * sub_start;
    for (sub_start = haystack; *i_haystack; i_haystack++) {
        sums_diff -= *sub_start++;
        sums_diff += *i_haystack;

        // Since the sum of the characters is already known to be equal at that
        // point, it is enough to check just needle_len-1 characters for
        // equality.
        if (sums_diff == 0 &&
            needle_first == *sub_start && // Avoids some calls to memcmp.
            ::memcmp(sub_start + 1, needle + 1, needle_len_1) == 0) {
            return (char *)sub_start;
        }
    }

    return nullptr;
}

template <typename CharTy>
class FastStrStrImpl {
public:
    typedef FastStrStrImpl<CharTy>  this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

    FastStrStrImpl() {}
    ~FastStrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "fast_strstr()"; }
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
        const char_type * haystack = (const char_type *)fast_strstr((const char *)text, (const char *)pattern);
        if (likely(haystack != nullptr))
            return (Long)(haystack - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< FastStrStrImpl<char> >    FastStrStr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< FastStrStrImpl<wchar_t> > FastStrStr;
}

} // namespace StringMatch

#endif // STRING_MATCH_FASTSTRSTR_H
