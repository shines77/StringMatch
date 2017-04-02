#ifndef JIMI_STRING_MATCH_H
#define JIMI_STRING_MATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
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

class kmp_pattern {
private:
    const char * pattern_;
    size_t pattern_len_;
    int * kmp_next_;

public:
    kmp_pattern() : pattern_(nullptr), pattern_len_(0), kmp_next_(nullptr) {
        // Do nothing!
    }
    kmp_pattern(const char * pattern)
        : pattern_(pattern), pattern_len_(strlen(pattern)), kmp_next_(nullptr) {
        prepare(pattern);
    }
    kmp_pattern(const char * pattern, size_t length)
        : pattern_(pattern), pattern_len_(length), kmp_next_(nullptr) {
        prepare(pattern, length);
    }
    template <size_t N>
    kmp_pattern(const char (&pattern)[N]) {
        return prepare(pattern, N);
    }
    kmp_pattern(const std::string & pattern)
        : pattern_(pattern.c_str()), pattern_len_(pattern.size()), kmp_next_(nullptr) {
        prepare(pattern);
    }
    ~kmp_pattern() {
        release();
    }

    const char * c_str() const { return pattern_; }
    char * data() { return const_cast<char *>(pattern_); }
    size_t size() const { return pattern_len_; }
    size_t length() const { return size(); }
    int * kmp_next() const { return kmp_next_; }

    void prepare(const char * pattern) {
        return preprocessing(pattern, strlen(pattern));
    }

    void prepare(const char * pattern, size_t length) {
        return preprocessing(pattern, length);
    }

    template <size_t N>
    void prepare(const char (&pattern)[N]) {
        return preprocessing(pattern, N);
    }

    void prepare(const std::string & pattern) {
        return preprocessing(pattern.c_str(), pattern.size());
    }

private:
    void release() {
        if (kmp_next_ != nullptr) {
            delete[] kmp_next_;
            kmp_next_ = nullptr;
        }
    }

    void preprocessing(const char * pattern, size_t length) {
        assert(pattern != nullptr);
        pattern_ = pattern;
        pattern_len_ = length;

        int * kmp_next = new int[length + 1];
        if (kmp_next != nullptr) {
            kmp_next[0] = -1;
            kmp_next[1] = 0;
            for (size_t index = 1; index < length; ++index) {
                if (pattern[index] == pattern[kmp_next[index - 1]]) {
                    kmp_next[index + 1] = kmp_next[index] + 1;
                }
                else {
                    kmp_next[index + 1] = 0;
                }
            }
        }
        if (kmp_next_ != nullptr)
            delete[] kmp_next_;
        kmp_next_ = kmp_next;
    }
};

class kmp {
private:
    const char * text_;
    size_t text_len_;

    const kmp_pattern * pattern_;

public:
    kmp() : text_(nullptr), text_len_(0), pattern_(nullptr) {
    }
    ~kmp() {
    }

    int find(const char * text, size_t length, const kmp_pattern & pattern) {
        pattern_ = &pattern;
        return search(text, length,
                      pattern.c_str(), pattern.size(),
                      pattern.kmp_next());
    }

    int find(const char * text, const kmp_pattern & pattern) {
        pattern_ = &pattern;
        return search(text, strlen(text),
                      pattern.c_str(), pattern.size(),
                      pattern.kmp_next());
    }

    template <size_t N>
    void find(const char (&text)[N], const kmp_pattern & pattern) {
        pattern_ = &pattern;
        return search(text, N,
                      pattern.c_str(), pattern.size(),
                      pattern.kmp_next());
    }

    int find(const std::string & text, const kmp_pattern & pattern) {
        pattern_ = &pattern;
        return search(text.c_str(), text.size(),
                      pattern.c_str(), pattern.size(),
                      pattern.kmp_next());
    }

    void display(int index_of) {
        printf("text     = \"%s\", text_len = %" PRIuPTR "\n", text_, text_len_);
        printf("pattern  = \"%s\", pattern_len = %" PRIuPTR "\n", pattern_->c_str(), pattern_->size());
        printf("index_of = %d\n", index_of);
        printf("\n");
    }

private:
    int search(const char * text, size_t text_len,
               const char * pattern_, size_t pattern_len,
               int * kmp_next) {
        text_ = text;
        text_len_ = text_len;

        assert(text != nullptr);
        assert(pattern_ != nullptr);
        assert(kmp_next != nullptr);

        if (text_len < pattern_len) {
            // Not found
            return -1;
        }

        register const char * target = text;
        register const char * pattern = pattern_;

        if ((size_t)target & (size_t)pattern & (size_t)kmp_next) {
            const char * target_end = text + (text_len - pattern_len);
            const char * pattern_end = pattern + pattern_len;
            do {
                if (*target != *pattern) {
                    int search_index = (int)(pattern - pattern_);
                    if (search_index == 0) {
                        target++;
                        if (target >= target_end) {
                            // Not found
                            return -1;
                        }
                    }
                    else {
                        assert(search_index >= 1);
                        int search_offset = kmp_next[search_index];
                        int target_offset = search_index - search_offset;
                        assert(target_offset >= 1);
                        pattern = pattern_ + search_offset;
                        target = target + target_offset;
                        if (target >= target_end) {
                            // Not found
                            return -1;
                        }
                    }
                }
                else {
                    target++;
                    pattern++;
                    if (pattern >= pattern_end) {
                        // Found
                        assert((target - text) >= (ptrdiff_t)pattern_len);
                        int pos = (int)((target - text) - (ptrdiff_t)pattern_len);
                        assert(pos >= 0);
                        return pos;
                    }
                    assert(target < (text + text_len));
                }
            } while (1);
        }
        return -2;  // Invalid parameters
    }
};

} // namespace AnsiString
} // namespace StringMatch

#endif // JIMI_STRING_MATCH_H
