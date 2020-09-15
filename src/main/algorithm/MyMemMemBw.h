
#ifndef STRING_MATCH_MY_MEMMEM_BW_H
#define STRING_MATCH_MY_MEMMEM_BW_H

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

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
my_memmem_bw(const char_type * haystack, size_t haystack_len,
             const char_type * needle, size_t needle_len) {
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string. */
    if (likely(needle_len != 0)) {
        /* Sanity check, otherwise the loop might search through the whole memory. */
        if (likely(haystack_len >= needle_len)) {
            const char_type * haystack_end = haystack + haystack_len;
            const char_type * haystack_last = haystack + needle_len - 1;
            const char_type * needle_last = needle + needle_len - 1;
            do {
search_start:
                const char_type * n = needle_last;
                while (likely(*haystack_last != *n)) {
                    haystack_last++;
                    if (likely(haystack_last < haystack_end))
						continue;
					else
                        return nullptr;
                }

				const char_type * h = haystack_last;
                do {
                    assert(h >= haystack);
                    h--;
                    n--;
                    if (likely(n >= needle)) {
						if (likely(*h != *n)) {
							haystack_last++;
							if (likely(haystack_last < haystack_end))
								goto search_start;
							else
								return nullptr;
						}
					}
					else {
                        return (haystack_last - (needle_len - 1));
					}
                } while (1);
            } while (1);
        }
        return nullptr;
    }
    else {
        return haystack;
    }
}

template <typename CharTy>
class MyMemMemBwImpl {
public:
    typedef MyMemMemBwImpl<CharTy>  this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

    MyMemMemBwImpl() {}
    ~MyMemMemBwImpl() {
        this->destroy();
    }

    static const char * name() { return "my_memmem_bw()"; }
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
        const char_type * haystack = my_memmem_bw(text, text_len, pattern, pattern_len);
        if (likely(haystack != nullptr))
            return (Long)(haystack - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< MyMemMemBwImpl<char> >    MyMemMemBw;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< MyMemMemBwImpl<wchar_t> > MyMemMemBw;
}

} // namespace StringMatch

#endif // STRING_MATCH_MY_MEMMEM_BW_H
