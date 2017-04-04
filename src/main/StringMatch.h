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
        char(*__CountOf_Helper(StringMatch_Unaligned CountOfType(&_Array)[SizeOfArray]))[SizeOfArray];

        #define __CountOf(_Array) (sizeof(*__CountOf_Helper(_Array)) + 0)
    }
#else
    #define __CountOf(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace StringMatch {

class StringRef {
private:
    const char * data_;
    size_t length_;

public:
    StringRef() : data_(nullptr), length_(0) {
        // Do nothing!
    }
    StringRef(const char * data)
        : data_(data), length_(strlen(data)) {
    }
    StringRef(const char * data, size_t length)
        : data_(data), length_(length) {
    }
    template <size_t N>
    StringRef(const char(&data)[N])
        : data_(data), length_(length) {
        return prepare(pattern, N);
    }
    StringRef(const std::string & src)
        : data_(src.c_str()), length_(src.size()) {
    }
    ~StringRef() {}

    const char * c_str() const { return data_; }
    char * data() const { return const_cast<char *>(data_); }

    size_t size() const { return length_; }
    size_t length() const { return this->size(); }

    void set_data(const char * data, size_t length) {
        data_ = data;
        length_ = length;
    }

    void set_data(const char * data) {
        set_data(data, strlen(data));
    }

    template <size_t N>
    void set_data(const char(&data)[N]) {
        set_data(data, N);
    }

    void set_data(const std::string & data) {
        set_data(data.c_str(), data.size());
    }
};

} // namespace StringMatch

namespace StringMatch {
namespace AnsiString {
namespace kmp {

class Macther;

struct Algorithm {
    static int * preprocessing(const char * pattern, size_t length) {
        assert(pattern != nullptr);

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
        return kmp_next;
    }

    static int search(const char * text, size_t text_len,
                      const char * pattern_, size_t pattern_len,
                      int * kmp_next) {
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
                        if (target > target_end) {
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
                        if (target > target_end) {
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

    static void display_test(const char * text, size_t text_len,
                             const char * pattern, size_t pattern_len, int index_of,
                             int sum, double time_spent) {
        display(text, text_len, pattern, pattern_len, index_of);
        printf("sum: %11d, time spent: %0.3f ms\n", sum, time_spent);
        printf("\n");
    }
};

class Pattern {
private:
    StringRef pattern_;
    int * kmp_next_;
    StringRef matcher_;

public:
    Pattern() : pattern_(), kmp_next_(nullptr), matcher_() {
        // Do nothing!
    }
    Pattern(const char * pattern)
        : pattern_(pattern), kmp_next_(nullptr), matcher_() {
        prepare(pattern);
    }
    Pattern(const char * pattern, size_t length)
        : pattern_(pattern, length), kmp_next_(nullptr), matcher_() {
        prepare(pattern, length);
    }
    template <size_t N>
    Pattern(const char (&pattern)[N])
        : pattern_(pattern, N), kmp_next_(nullptr), matcher_() {
        return prepare(pattern, N);
    }
    Pattern(const std::string & pattern)
        : pattern_(pattern), kmp_next_(nullptr), matcher_() {
        prepare(pattern);
    }
    ~Pattern() {
        release();
    }

    const char * c_str() const { return pattern_.c_str(); }
    char * data() { return pattern_.data(); }
    size_t size() const { return pattern_.size(); }
    size_t length() const { return pattern_.length(); }
    int * kmp_next() const { return kmp_next_; }

    bool is_valid() const { return (pattern_.c_str() != nullptr); }
    bool is_alive() const { return (pattern_.c_str() != nullptr && kmp_next_ != nullptr); }

    void prepare(const char * pattern, size_t length) {
        return this->preprocessing(pattern, length);
    }

    void prepare(const char * pattern) {
        return this->prepare(pattern, strlen(pattern));
    }

    template <size_t N>
    void prepare(const char (&pattern)[N]) {
        return prepare(pattern, N);
    }

    void prepare(const std::string & pattern) {
        return prepare(pattern.c_str(), pattern.size());
    }

    int match(const char * text, size_t length) {
        matcher_.set_data(text, length);
        return Algorithm::search(text, length,
                                 this->c_str(), this->size(),
                                 this->kmp_next());
    }

    int match(const char * text) {
        return this->match(text, strlen(text));
    }

    template <size_t N>
    int match(const char(&text)[N]) {
        return this->match(text, N);
    }

    int match(const std::string & text) {
        return this->match(text.c_str(), text.size());
    }

    void display(int index_of) {
        if (this->is_alive()) {
            Algorithm::display(matcher_.c_str(), matcher_.size(), this->c_str(), this->size(), index_of);
        }
        else {
            Algorithm::display(matcher_.c_str(), matcher_.size(), nullptr, 0, index_of);
        }
    }

    void display_test(int index_of, int sum, double time_spent) {
        if (this->is_alive()) {
            Algorithm::display_test(matcher_.c_str(), matcher_.size(), this->c_str(), this->size(),
                index_of, sum, time_spent);
        }
        else {
            Algorithm::display_test(matcher_.c_str(), matcher_.size(), nullptr, 0, index_of, sum, time_spent);
        }
    }

private:
    void release() {
        if (kmp_next_ != nullptr) {
            delete[] kmp_next_;
            kmp_next_ = nullptr;
        }
    }

    void preprocessing(const char * pattern, size_t length) {
        pattern_.set_data(pattern, length);

        int * kmp_next = Algorithm::preprocessing(pattern, length);
        if (kmp_next_ != nullptr) {
            delete[] kmp_next_;
        }
        kmp_next_ = kmp_next;
    }
};

class Matcher {
private:
    StringRef text_;
    const Pattern * pattern_;

public:
    Matcher() : text_(), pattern_(nullptr) {
    }
    ~Matcher() {
    }

    const char * text() const { return text_.c_str(); }
    size_t text_length() const { return text_.size(); }

    const char * pattern() const {
        if (pattern_ != nullptr) {
            return pattern_->c_str();
        }
        return nullptr;
    }

    size_t pattern_length() const {
        if (pattern_ != nullptr) {
            return pattern_->size();
        }
        return 0;
    }

    void set_text(const char * text, size_t length) {
        text_.set_data(text, length);
    }

    int find(const char * text, size_t length, const Pattern & pattern) {
        pattern_ = &pattern;
        return this->search(text, length,
                            pattern.c_str(), pattern.size(),
                            pattern.kmp_next());
    }

    int find(const char * text, const Pattern & pattern) {
        return this->find(text, strlen(text), pattern);
    }

    template <size_t N>
    int find(const char (&text)[N], const Pattern & pattern) {
        return this->find(text, N, pattern);
    }

    int find(const std::string & text, const Pattern & pattern) {
        return this->find(text.c_str(), text.size(), pattern);
    }

    void display(int index_of) {
        if (pattern_ != nullptr && pattern_->is_alive()) {
            Algorithm::display(text_.c_str(), text_.size(), pattern_->c_str(), pattern_->size(), index_of);
        }
        else {
            Algorithm::display(text_.c_str(), text_.size(), nullptr, 0, index_of);
        }
    }

    void display_test(int index_of, int sum, double time_spent) {
        if (pattern_ != nullptr && pattern_->is_alive()) {
            Algorithm::display_test(text_.c_str(), text_.size(), pattern_->c_str(), pattern_->size(),
                                    index_of, sum, time_spent);
        }
        else {
            Algorithm::display_test(text_.c_str(), text_.size(), nullptr, 0, index_of, sum, time_spent);
        }
    }

private:
    int search(const char * text, size_t text_len,
               const char * pattern, size_t pattern_len,
               int * kmp_next) {
        text_.set_data(text, text_len);
        return Algorithm::search(text, text_len, pattern, pattern_len, kmp_next);
    }
};

} // namespace kmp
} // namespace AnsiString
} // namespace StringMatch

#endif // JIMI_STRING_MATCH_H
