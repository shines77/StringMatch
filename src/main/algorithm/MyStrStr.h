
#ifndef STRING_MATCH_MY_STRSTR_H
#define STRING_MATCH_MY_STRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <limits.h>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"

namespace StringMatch {

#if 1

//
// See: https://blog.csdn.net/dog250/article/details/5302948
//

template <typename char_type>
static
const char_type * strstr_glibc_old(const char_type * phaystack, const char_type * pneedle)
{
    typedef unsigned unsigned_type;
    typedef typename detail::uchar_traits<char_type>::type uchar_type;

    const uchar_type * needle;
    const uchar_type * haystack = (const uchar_type * )phaystack;
    unsigned_type cursor;

    if ((cursor = *(needle = (const uchar_type *)pneedle))) {
        unsigned_type cursor2;
        /* possible ANSI violation */
        --haystack;

        {
            unsigned_type scan;
            do {
                ++haystack;
                if ((scan = *haystack) == char_type('\0'))
                    goto ret_0;
            } while (scan != cursor);
        }

        ++needle;
        if ((cursor2 = *needle) == char_type('\0'))
            goto found_needle;

        ++needle;
        goto jin;

        for (;;) {
            {
                unsigned_type scan;
                if (0) {
jin:
                    {
                        ++haystack;
                        if ((scan = *haystack) == cursor2)
                            goto crest;
                    } // jin: end
                }
                else {
                    ++haystack;
                    scan = *haystack;
                }

                do {
                    for (; scan != cursor; scan = *++haystack) {
                        if (scan == char_type('\0'))
                            goto ret_0;

                        if ((scan = *++haystack) == cursor)
                            break;

                        if (scan == char_type('\0'))
                            goto ret_0;
                    }
                } while ((scan = *++haystack) != cursor2);
            }

crest:
            {
                const uchar_type * rneedle;
                unsigned_type rcursor;
                {
                    const uchar_type * rhaystack;
                    if (*(rhaystack = (haystack--) + 1)
                        == (rcursor = *(rneedle = needle))) {
                        do {
                            if (rcursor == char_type('\0'))
                                goto found_needle;

                            if (*++rhaystack != (rcursor = *++needle))
                                break;

                            if (rcursor == char_type('\0'))
                                goto found_needle;
                        } while (*++rhaystack == (rcursor = *++needle));
                    }

                    /* took the register-poor approach */
                    needle = rneedle;
                }

                if (rcursor == char_type('\0'))
                    break;
            } // crest: end
        } // for(;;)
    } // if

found_needle:
    return (const char_type *)haystack;

ret_0:
    return 0;
}

#endif

template <typename char_type>
static
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
    Long search(const char_type * text, size_type text_len,
                const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
        const char_type * substr = my_strstr(text, pattern);
        if (likely(substr != nullptr))
            return (Long)(substr - text);
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
