
#ifndef MAIN_STRING_MATCH_H
#define MAIN_STRING_MATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string>
#include <memory>

#include "StringRef.h"
#include "BasicAlgorithm.h"

//
// marco sm_countof(array)
//
#if defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
    #define sm_unaligned __unaligned
#else
    #define sm_unaligned
#endif

#ifndef sm_countof
#ifdef __cplusplus
    extern "C++" {
        template <typename CountOfType, size_t SizeOfArray>
        char(*sm_countof_helper(sm_unaligned CountOfType(&_Array)[SizeOfArray]))[SizeOfArray];

        #define sm_countof(_Array) (sizeof(*sm_countof_helper(_Array)) + 0)
    }
#else
    #define sm_countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif // __cplusplus
#endif // sm_countof

#ifndef sm_min
#define sm_min(a, b)    (((a) <= (b)) ? (a) : (b))
#endif // sm_min

#ifndef sm_max
#define sm_max(a, b)    (((a) >= (b)) ? (a) : (b))
#endif // sm_max

namespace StringMatch {

template <typename CharT>
class ShiftAndImpl {
public:
    typedef CharT char_type;
    typedef std::tuple<int *> tuple_type;

private:
    std::unique_ptr<int> kmp_next_;
    tuple_type args_;

public:
    ShiftAndImpl() : kmp_next_(), args_(nullptr) {}
    ~ShiftAndImpl() {}

    const tuple_type & get_args() const { return args_; }
    void set_args(const tuple_type & args) { args_ = args; }

    int * kmp_next() const { return this->kmp_next_.get(); }
    void set_kmp_next(int * kmp_next) {
        this->kmp_next_.reset(kmp_next);
    }

    bool is_alive() const {
        return (this->kmp_next() != nullptr);
    }

    void free() {
        kmp_next_.reset();
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_t length) {
        args_ = std::make_tuple(kmp_next_.get());
        return true;
    }

    /* Preprocessing */
    static bool preprocessing(const char_type * pattern, size_t length, tuple_type & args) {
        args = std::make_tuple(nullptr);
        return true;
    }

    /* Search */
    static int search(const char_type * text, size_t text_len,
                      const char_type * pattern_, size_t pattern_len,
                      const tuple_type & args) {
        //
        return 0;
    }
};

template <typename CharT>
struct BasicShiftAnd : public BasicAlgorithm< ShiftAndImpl<CharT> > {
    //
};

namespace AnsiString {
    typedef BasicAlgorithm< ShiftAndImpl<char> >    ShiftAnd;
} // namespace AnsiString

namespace UnicodeString {
    typedef BasicAlgorithm< ShiftAndImpl<wchar_t> > ShiftAnd;
} // namespace UnicodeString

struct AlgorithmBase {
    static void display(const char * text, size_t text_len,
                        const char * pattern, size_t pattern_len, int index_of) {
        if (text != nullptr) {
            printf("text     = \"%s\", text_len = %" PRIuPTR "\n", text, text_len);
        }
        if (pattern != nullptr) {
            printf("pattern  = \"%s\", pattern_len = %" PRIuPTR "\n", pattern, pattern_len);
        }
        printf("index_of = %d\n", index_of);
        printf("\n");
    }

    static void display(const char * text, size_t text_len,
                        const char * pattern, size_t pattern_len, int index_of,
                        int sum, double time_spent) {
        display(text, text_len, pattern, pattern_len, index_of);
        printf("sum: %11d, time spent: %0.3f ms\n", sum, time_spent);
        printf("\n");
    }
};

} // namespace StringMatch

#endif // MAIN_STRING_MATCH_H
