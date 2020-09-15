
#ifndef STRING_MATCH_MY_MEMMEM_H
#define STRING_MATCH_MY_MEMMEM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

//
// memmem()
//
// See: http://man7.org/linux/man-pages/man3/memmem.3.html
//

namespace StringMatch {

#if 1
template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
my_memmem(const char_type * haystack_start, size_t haystack_len,
          const char_type * needle_start, size_t needle_len) {
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string. */
    if (likely(needle_len != 0)) {
        /* Sanity check, otherwise the loop might search through the whole memory. */
        if (likely(haystack_len >= needle_len)) {
            const char_type * haystack = haystack_start;
            const char_type * needle = needle_start;
            const char_type * haystack_end = haystack_start + haystack_len - needle_len;
            const char_type * needle_end = needle_start + needle_len;
            do {
search_start:
                const char_type * n = needle;
                while (likely(*haystack != *n)) {
                    haystack++;
                    if (likely(haystack <= haystack_end))
						continue;
					else
                        return nullptr;
                }

				const char_type * h = haystack;
                do {
                    assert(h <= (haystack_start + haystack_len));
                    h++;
                    n++;
                    if (likely(n < needle_end)) {
						if (likely(*h != *n)) {
							haystack++;
							if (likely(haystack <= haystack_end))
								goto search_start;
							else
								return nullptr;
						}
					}
					else {
                        return haystack;
					}
                } while (1);
            } while (1);
        }
        return nullptr;
    }
    else {
        return haystack_start;
    }
}
#elif 1
template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
my_memmem(const char_type * haystack_start, size_t haystack_len,
          const char_type * needle_start, size_t needle_len) {
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string. */
    if (likely(needle_len != 0)) {
        /* Sanity check, otherwise the loop might search through the whole memory. */
        if (likely(haystack_len >= needle_len)) {
            const char_type * haystack = haystack_start;
            const char_type * needle = needle_start;
            const char_type * haystack_end = haystack_start + haystack_len - needle_len;
            const char_type * needle_end = needle_start + needle_len;
            for (;;) {
                const char_type * h = haystack;
                const char_type * n = needle;
                while (likely(*h != *n)) {
                    h++;
                    if (unlikely(h > haystack_end))
                        return nullptr;
                }

                haystack = h;

                do {
                    assert(h <= (haystack_start + haystack_len));
                    h++;
                    n++;
                    if (unlikely(n >= needle_end))
                        return haystack;
                } while (unlikely(*h == *n));

                haystack++;
                if (unlikely(haystack > haystack_end))
                    return nullptr;
            }
        }

        return nullptr;
    }
    else {
        return haystack_start;
    }
}
#else
//
// memmem() on Windows
//
// See: https://codereview.stackexchange.com/questions/182156/memmem-on-windows
//
template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
my_memmem(const char_type * haystack_start, size_t haystack_len,
          const char_type * needle_start, size_t needle_len) {
    const char_type * haystack = haystack_start;
    const char_type * needle = needle_start;

    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string. */
    if (likely(needle_len != 0)) {
        /* Sanity check, otherwise the loop might search through the whole memory. */
        if (likely(haystack_len >= needle_len)) {
            for (; haystack_len >= needle_len; ++haystack, --haystack_len) {
                const char_type * h = haystack;
                const char_type * n = needle;

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
                    return haystack;
            }
        }

        return nullptr;
    }
    else {
        return haystack_start;
    }
}
#endif

template <typename CharTy>
class MyMemMemImpl {
public:
    typedef MyMemMemImpl<CharTy>    this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

    MyMemMemImpl() {}
    ~MyMemMemImpl() {
        this->destroy();
    }

    static const char * name() { return "my_memmem()"; }
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
        const char_type * haystack = my_memmem(text, text_len, pattern, pattern_len);
        if (likely(haystack != nullptr))
            return (Long)(haystack - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< MyMemMemImpl<char> >    MyMemMem;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< MyMemMemImpl<wchar_t> > MyMemMem;
}

} // namespace StringMatch

#endif // STRING_MATCH_MY_MEMMEM_H
