
#ifndef STRING_MATCH_GLIBC_STRSTR_OLD_H
#define STRING_MATCH_GLIBC_STRSTR_OLD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <limits.h>
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

namespace StringMatch {

//
// See: https://blog.csdn.net/dog250/article/details/5302948
//

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_glibc_old(const char_type * phaystack, const char_type * pneedle)
{
    typedef unsigned unsigned_type;
    typedef typename jstd::uchar_traits<char_type>::type uchar_type;

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
    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_glibc_old2(const char_type * phaystack, const char_type * pneedle)
{
    typedef unsigned unsigned_type;
    typedef typename jstd::uchar_traits<char_type>::type uchar_type;

    const uchar_type * haystack = (const uchar_type * )phaystack;
    const uchar_type * needle = (const uchar_type *)pneedle;
    unsigned_type needle_01 = *needle;
    
    if (needle_01 != char_type('\0')) {
        /* possible ANSI violation */
        --haystack;

        // Find first char in haystack is equal to the first char of needle.
        unsigned_type scan;
        do {
            ++haystack;
            scan = *haystack;
            if (scan == char_type('\0')) {
                goto ret_nullptr;
            }
        } while (scan != needle_01);

        ++needle;
        unsigned_type needle_02 = *needle;
        if (needle_02 == char_type('\0')) {
            goto found_needle;
        }

        ++needle;
        goto jin;

        for (;;) {
            {
                unsigned_type cur;
                if (0) {
jin:
                    {
                        ++haystack;
                        cur = *haystack;
                        if (cur == needle_02) {
                            goto crest;
                        }
                    } // jin: end
                }
                else {
                    ++haystack;
                    cur = *haystack;
                }

                do {
                    for (; cur != needle_01; cur = *++haystack) {
                        if (cur == char_type('\0')) {
                            goto ret_nullptr;
                        }

                        if ((cur = *++haystack) == needle_01) {
                            break;
                        }

                        if (cur == char_type('\0')) {
                            goto ret_nullptr;
                        }
                    }
                    cur = *++haystack;
                } while (cur != needle_02);
            }
crest:
            {
                const uchar_type * rneedle;
                unsigned_type rcursor;
                {
                    const uchar_type * rhaystack = (haystack--) + 1;
                    rneedle = needle;
                    rcursor = *rneedle;
                    if (rcursor == *rhaystack) {
                        do {
                            if (rcursor == char_type('\0')) {
                                goto found_needle;
                            }

                            rcursor = *++needle;
                            if (rcursor != *++rhaystack) {
                                break;
                            }

                            if (rcursor == char_type('\0')) {
                                goto found_needle;
                            }

                            rcursor = *++needle;
                        } while (rcursor == *++rhaystack);
                    }

                    /* took the register-poor approach */
                    needle = rneedle;
                }

                if (rcursor == char_type('\0')) {
                    break;
                }
            }
        } // for (;;) 
    } // if

found_needle:
    return (const char_type *)haystack;

ret_nullptr:
    return nullptr;
}

template <typename CharTy>
class GlibcStrStrOldImpl {
public:
    typedef GlibcStrStrOldImpl<CharTy>  this_type;
    typedef CharTy                      char_type;
    typedef std::size_t                 size_type;

    GlibcStrStrOldImpl() {}
    ~GlibcStrStrOldImpl() {
        this->destroy();
    }

    static const char * name() { return "strstr_glibc_old()"; }
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
        const char_type * haystack = strstr_glibc_old(text, pattern);
        if (likely(haystack != nullptr))
            return (Long)(haystack - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< GlibcStrStrOldImpl<char> >    GlibcStrStrOld;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< GlibcStrStrOldImpl<wchar_t> > GlibcStrStrOld;
}

} // namespace StringMatch

#endif // STRING_MATCH_GLIBC_STRSTR_OLD_H
