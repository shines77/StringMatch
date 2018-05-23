
#ifndef STRING_MATCH_MY_STRSTR_H
#define STRING_MATCH_MY_STRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"

namespace StringMatch {

template <typename char_type>
static inline
const char_type * my_strstr_glibc(const char_type * phaystack, const char_type * pneedle)
{
    typedef unsigned unsigned_type;
    typedef detail::uchar_traits<char_type>::type uchar_type;

    const uchar_type * needle;
    const uchar_type * rneedle;
    const uchar_type * haystack = (uchar_type *)phaystack;
    unsigned_type b;

    if ((b = *(needle = (const uchar_type *)pneedle))) {
        unsigned_type c;
        /* possible ANSI violation */
        haystack--;

        {
            unsigned_type a;
            do {
                if (!(a = *++haystack))
                    goto ret0;
            } while (a != b);
        }

        if (!(c = *++needle))
            goto found_needle;

        ++needle;
        goto jin;

        for (;;) {
            {
                unsigned_type a;
                if (0)
jin:
                {
                    if ((a = *++haystack) == c)
                        goto crest;
                }
                else {
                    a = *++haystack;
                }

                do {
                    for (; a != b; a = *++haystack) {
                        if (!a)
                            goto ret0;

                        if ((a = *++haystack) == b)
                            break;

                        if (!a)
                            goto ret0;
                    }
                } while ((a = *++haystack) != c);
            }

crest:
            {
                unsigned_type a;
                {
                    const uchar_type *rhaystack;
                    if (*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle))) {
                        do {
                            if (!a)
                                goto found_needle;

                            if (*++rhaystack != (a = *++needle))
                                break;

                            if (!a)
                                goto found_needle;
                        } while (*++rhaystack == (a = *++needle));
                    }

                    /* took the register-poor approach */
                    needle = rneedle;
                }

                if (!a)
                    break;
            }
        }
    }

found_needle:
    return (const char_type *)haystack;

ret0:
    return 0;
}

template <typename char_type>
static inline
const char_type * my_strstr(const char_type * text, const char_type * pattern) {
    assert(text != nullptr);
    assert(pattern != nullptr);
#if 0
    if (likely(*pattern != char_type('\0'))) {
        const char_type * scan_start = text;
        while (likely(*scan_start != char_type('\0'))) {
            const char_type * scan = scan_start;
            const char_type * cursor = pattern;
            while (unlikely((*scan != char_type('\0'))
                && (*cursor != char_type('\0'))
                && (*scan == *cursor))) {
                scan++;
                cursor++;
            };

            if (unlikely((*cursor == char_type('\0')))) {
                return scan_start;
            }

            scan_start++;
        }
        return nullptr;
    }
    else {
        return text;
    }
#elif 1
    if (likely(*pattern != char_type('\0'))) {
        const char_type * scan_start = text;
        while (likely(*scan_start != char_type('\0'))) {
            const char_type * cursor = pattern;
            while (likely(*scan_start != *cursor)) {
                scan_start++;
                if (unlikely(*scan_start == char_type('\0')))
                    return nullptr;
            }

            const char_type * scan = scan_start;
            do {
                cursor++;
                scan++;
                if (likely(*cursor != char_type('\0'))) {
                    if (likely(*scan != char_type('\0'))) {
                        if (likely(*scan != *cursor)) {
                            scan_start++;
                            break;
                        }
                    }
                    else {
                        return nullptr;
                    }
                }
                else {
                    return scan_start;
                }
            } while (1);
        }
        return nullptr;
    }
    else {
        return text;
    }
#else
    if (likely(*pattern != char_type('\0'))) {
        const char_type * scan_start = text;
        while (likely(*scan_start != char_type('\0'))) {
            const char_type * cursor = pattern;
            while (likely(*scan_start != *cursor)) {
                scan_start++;
                if (unlikely(*scan_start == char_type('\0')))
                    return nullptr;
            }

            const char_type * scan = scan_start;
            do {
                cursor++;
                if (likely(*cursor != char_type('\0'))) {
                    scan++;
                    if (likely(*scan != char_type('\0'))) {
                        if (likely(*scan != *cursor)) {
                            scan_start++;
                            break;
                        }
                    }
                    else {
                        return nullptr;
                    }
                }
                else {
                    return scan_start;
                }
            } while (1);
        }
        return nullptr;
    }
    else {
        return text;
    }
#endif
}

template <typename CharTy>
class MyStrStrImpl {
public:
    typedef MyStrStrImpl<CharTy>    this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

private:
    bool alive_;

public:
    MyStrStrImpl() : alive_(true) {}
    ~MyStrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "my_strstr()"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return this->alive_; }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        /* Don't need preprocessing. */
        return true;
    }

    /* Searching */
    int search(const char_type * text, size_type text_len,
               const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
        const char_type * substr = my_strstr_glibc(text, pattern);
        if (likely(substr != nullptr))
            return (int)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< MyStrStrImpl<char> >    MyStrStr;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< MyStrStrImpl<wchar_t> > MyStrStr;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_MY_STRSTR_H
