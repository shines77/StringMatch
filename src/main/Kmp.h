
#ifndef STRING_MATCH_KMP_H
#define STRING_MATCH_KMP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "StringMatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include "StringRef.h"

namespace StringMatch {
namespace AnsiString {

struct Kmp {

class Matcher;

struct Algorithm : public StringMatch::AlgorithmBase {
    /* Preprocessing */
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

    /* Search */
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

        if ((size_t)target | (size_t)pattern | (size_t)kmp_next) {
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
        // Invalid parameters
        return -2;
    }
}; // class Algorithm

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
    Pattern(const StringRef & pattern)
        : pattern_(pattern), kmp_next_(nullptr), matcher_() {
        prepare(pattern);
    }
    ~Pattern() {
        this->free();
    }

    const char * c_str() const { return pattern_.c_str(); }
    char * data() { return pattern_.data(); }
    size_t size() const { return pattern_.size(); }
    size_t length() const { return pattern_.length(); }
    int * kmp_next() const { return kmp_next_; }

    bool is_valid() const { return (pattern_.c_str() != nullptr); }
    bool is_alive() const { return ((pattern_.c_str() != nullptr) && (kmp_next_ != nullptr)); }

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

    void prepare(const StringRef & pattern) {
        return prepare(pattern.c_str(), pattern.size());
    }

    int match(const char * text, size_t length) {
        matcher_.set_ref(text, length);
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

    int match(const StringRef & text) {
        return this->match(text.c_str(), text.size());
    }

    int match(const Matcher & matcher);

    void display(int index_of) {
        if (this->is_alive()) {
            Algorithm::display(matcher_.c_str(), matcher_.size(), this->c_str(), this->size(), index_of);
        }
        else {
            Algorithm::display(matcher_.c_str(), matcher_.size(), nullptr, 0, index_of);
        }
    }

    void display(int index_of, int sum, double time_spent) {
        if (this->is_alive()) {
            Algorithm::display(matcher_.c_str(), matcher_.size(), this->c_str(), this->size(),
                index_of, sum, time_spent);
        }
        else {
            Algorithm::display(matcher_.c_str(), matcher_.size(), nullptr, 0, index_of, sum, time_spent);
        }
    }

private:
    void free() {
        if (kmp_next_ != nullptr) {
            delete[] kmp_next_;
            kmp_next_ = nullptr;
        }
    }

    void preprocessing(const char * pattern, size_t length) {
        pattern_.set_ref(pattern, length);

        int * kmp_next = Algorithm::preprocessing(pattern, length);
        if (kmp_next_ != nullptr) {
            delete[] kmp_next_;
        }
        kmp_next_ = kmp_next;
    }
}; // class Pattern

class Matcher {
private:
    StringRef text_;
    StringRef pattern_;

public:
    Matcher() : text_(), pattern_() {
    }
    Matcher(const char * text)
        : text_(text), pattern_() {
    }
    Matcher(const char * text, size_t length)
        : text_(text, length), pattern_() {
    }
    template <size_t N>
    Matcher(const char (&text)[N])
        : text_(text, N), pattern_() {
    }
    Matcher(const std::string & text)
        : text_(text), pattern_() {
    }
    Matcher(const StringRef & text)
        : text_(text), pattern_() {
    }
    ~Matcher() {
    }

    const char * c_str() const { return text_.c_str(); }
    char * data() const { return text_.data(); }

    size_t size() const { return text_.size(); }
    size_t length() const { return this->size(); }

    const char * text() const { return text_.c_str(); }
    size_t text_length() const { return text_.size(); }

    const char * pattern() const { return pattern_.c_str(); }
    size_t pattern_length() const { return pattern_.size(); }

    void set_text(const char * text, size_t length) {
        text_.set_ref(text, length);
    }

    void set_text(const char * text) {
        text_.set_ref(text);
    }

    template <size_t N>
    void set_text(const char (&text)[N]) {
        text_.set_ref(text, N);
    }

    void set_text(const std::string & text) {
        text_.set_ref(text);
    }

    void set_text(const StringRef & text) {
        text_.set_ref(text);
    }

    int find(const Pattern & pattern) {
        return Algorithm::search(text_.c_str(), text_.size(),
                                 pattern.c_str(), pattern.size(),
                                 pattern.kmp_next());
    }

    int find(const char * text, size_t length, const Pattern & pattern) {
        pattern_.set_ref(pattern.c_str(), pattern.size());
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

    int find(const StringRef & text, const Pattern & pattern) {
        return this->find(text.c_str(), text.size(), pattern);
    }

    void display(int index_of) {
        Algorithm::display(text_.c_str(), text_.size(), pattern_.c_str(), pattern_.size(), index_of);
    }

    void display(int index_of, int sum, double time_spent) {
        Algorithm::display(text_.c_str(), text_.size(), pattern_.c_str(), pattern_.size(),
                           index_of, sum, time_spent);
    }

private:
    int search(const char * text, size_t text_len,
               const char * pattern, size_t pattern_len,
               int * kmp_next) {
        text_.set_ref(text, text_len);
        return Algorithm::search(text, text_len, pattern, pattern_len, kmp_next);
    }
}; // class Matcher

}; // struct Kmp

inline int Kmp::Pattern::match(const Kmp::Matcher & matcher) {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace AnsiString
} // namespace StringMatch

#endif // STRING_MATCH_KMP_H
