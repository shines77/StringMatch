
#ifndef STRING_MATCH_MEMMEM_H
#define STRING_MATCH_MEMMEM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE     /* See feature_test_macros(7) */
#endif

#include "basic/stdint.h"
#include <string.h>     // for linux: memmem()
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

#if defined(_MSC_VER)
#define memmem  memmem_msvc
#endif // _MSC_VER

//
// memmem()
//
// See: http://man7.org/linux/man-pages/man3/memmem.3.html
//

namespace StringMatch {

#if 1
static
SM_NOINLINE_DECLARE(void *)
memmem_msvc(const void * haystack_start, size_t haystack_len,
            const void * needle_start, size_t needle_len) {
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string. */
    if (likely(needle_len != 0)) {
        /* Sanity check, otherwise the loop might search through the whole memory. */
        if (likely(haystack_len >= needle_len)) {
            const unsigned char * haystack = (const unsigned char *)haystack_start;
            const unsigned char * needle = (const unsigned char *)needle_start;
            const unsigned char * haystack_end = haystack + haystack_len - needle_len;
            const unsigned char * needle_end = needle + needle_len;
            for (;;) {
                const unsigned char * h = haystack;
                const unsigned char * n = needle;
                while (likely(*h != *n)) {
                    h++;
                    if (unlikely(h > haystack_end))
                        return nullptr;
                }

                haystack = h;

                do {
                    h++;
                    n++;
                    if (unlikely(n >= needle_end))
                        return (void *)haystack;
                } while (*h == *n);

                haystack++;
                if (unlikely(haystack > haystack_end))
                    return nullptr;
            }
        }

        return nullptr;
    }
    else {
        return (void *)haystack_start;
    }
}
#else
//
// memmem() on Windows
//
// See: https://codereview.stackexchange.com/questions/182156/memmem-on-windows
//
static
SM_NOINLINE_DECLARE(void *)
memmem_msvc(const void * haystack_start, size_t haystack_len,
            const void * needle_start, size_t needle_len) {
    const unsigned char * haystack = (const unsigned char *)haystack_start;
    const unsigned char * needle = (const unsigned char *)needle_start;

    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string. */
    if (likely(needle_len != 0)) {
        /* Sanity check, otherwise the loop might search through the whole memory. */
        if (likely(haystack_len >= needle_len)) {
            for (; haystack_len >= needle_len; ++haystack, --haystack_len) {
                const unsigned char * h = haystack;
                const unsigned char * n = needle;

                if (likely(*h != *n))
                    continue;

                h++; n++;

                size_t x = needle_len - 1;
                for (; x; h++, n++) {
                    if (likely(*h != *n))
                        break;
                    x--;
                }
                if (unlikely(x == 0))
                    return (void *)haystack;
            }
        }

        return nullptr;
    }
    else {
        return (void *)haystack_start;
    }
}
#endif

template <typename CharTy>
class MemMemImpl {
public:
    typedef MemMemImpl<CharTy>  this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

    MemMemImpl() {}
    ~MemMemImpl() {
        this->destroy();
    }

    static const char * name() { return "memmem()"; }
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
        const char_type * haystack = (const char_type *)memmem(
                                     (const void *)text, text_len * sizeof(char_type),
                                     (const void *)pattern, pattern_len * sizeof(char_type));
        if (likely(haystack != nullptr))
            return (Long)(haystack - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< MemMemImpl<char> >    MemMem;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< MemMemImpl<wchar_t> > MemMem;
}

} // namespace StringMatch

#endif // STRING_MATCH_MEMMEM_H
