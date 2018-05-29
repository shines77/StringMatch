
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
#include "AlgorithmWrapper.h"

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

template <typename CharTy>
class GlibcStrStrOldImpl {
public:
    typedef GlibcStrStrOldImpl<CharTy>  this_type;
    typedef CharTy                      char_type;
    typedef std::size_t                 size_type;

private:
    bool alive_;

public:
    GlibcStrStrOldImpl() : alive_(true) {}
    ~GlibcStrStrOldImpl() {
        this->destroy();
    }

    static const char * name() { return "strstr() glibc-old"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return this->alive_; }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        /* Don't need to do preprocessing. */
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
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< GlibcStrStrOldImpl<wchar_t> > GlibcStrStrOld;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_GLIBC_STRSTR_OLD_H
