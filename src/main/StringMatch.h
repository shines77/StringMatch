#ifndef JIMI_STRING_MATCH_H
#define JIMI_STRING_MATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <inttypes.h>
#include <assert.h>

#if defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
    #define StringMatch_Unaligned __unaligned
#else
    #define StringMatch_Unaligned
#endif

#ifdef __cplusplus
    extern "C++" {
        template <typename CountOfType, size_t SizeOfArray>
        char(*StringMatch_CountOf_Helper(StringMatch_Unaligned CountOfType(&_Array)[SizeOfArray]))[SizeOfArray];

        #define StringMatch_CountOf(_Array) (sizeof(*StringMatch_CountOf_Helper(_Array)) + 0)
    }
#else
    #define StringMatch_CountOf(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace StringMatch {
namespace AnsiString {

class kmp {
private:
    const char * pattern_;
    size_t pattern_len_;
    const char * text_;
    size_t text_len_;

    int * kmpNext_;

public:
    kmp() : pattern_(nullptr), pattern_len_(0),
            text_(nullptr), text_len_(0), kmpNext_(nullptr) {
        init();
    }
    ~kmp() {
        release();
    }

    void preprocessing(const char * pattern, size_t length) {
        assert(pattern != nullptr);
        pattern_ = pattern;
        pattern_len_ = length;

        int * kmpNext = new int[length];
        if (kmpNext != nullptr) {
            kmpNext[0] = 0;
            for (size_t index = 1; index < length; ++index) {
                if (pattern[index] == pattern[kmpNext[index - 1]]) {
                    kmpNext[index] = kmpNext[index - 1] + 1;
                }
                else {
                    kmpNext[index] = 0;
                }
            }
        }
        if (kmpNext_ != nullptr)
            delete[] kmpNext_;
        kmpNext_ = kmpNext;
    }

    void preprocessing(const char * pattern) {
        return preprocessing(pattern, strlen(pattern));
    }

    void preprocessing(const std::string & pattern) {
        return preprocessing(pattern.c_str(), pattern.size());
    }

    int match(const char * text, size_t length) {
        int pos = -1;
        text_ = text;
        text_len_ = length;
        return pos;
    }

    int match(const char * text) {
        return match(text, strlen(text));
    }

    int match(const std::string & text) {
        return match(text.c_str(), text.size());
    }

    static int match(const std::string & text, const std::string & pattern) {
        int pos = -1;
        return pos;
    }

    void display() {
        printf("text    = \"%s\", text_len = %" PRIuPTR "\n", text_, text_len_);
        printf("pattern = \"%s\", pattern_len = %" PRIuPTR "\n", pattern_, pattern_len_);
        printf("\n");
    }

private:
    void init() {
    }

    void release() {
        if (kmpNext_ != nullptr) {
            delete[] kmpNext_;
            kmpNext_ = nullptr;
        }
    }
};

} // namespace AnsiString
} // namespace StringMatch

#endif // JIMI_STRING_MATCH_H
