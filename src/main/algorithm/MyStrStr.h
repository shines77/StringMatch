
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
#include "algorithm/AlgorithmWrapper.h"

namespace StringMatch {

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
my_strstr(const char_type * text, const char_type * pattern) {
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

    MyStrStrImpl() {}
    ~MyStrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "my_strstr()"; }
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
        const char_type * substr = my_strstr(text, pattern);
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< MyStrStrImpl<char> >    MyStrStr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< MyStrStrImpl<wchar_t> > MyStrStr;
}

} // namespace StringMatch

#endif // STRING_MATCH_MY_STRSTR_H
