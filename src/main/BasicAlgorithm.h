
#ifndef STRING_MATCH_BASIC_ALGORITHM_H
#define STRING_MATCH_BASIC_ALGORITHM_H

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

namespace StringMatch {

template <typename T, typename Arg1 = int, typename Arg2 = int>
struct BasicAlgorithm {

    typedef T                                   algorithm_type;
    typedef Arg1                                arg1_type;
    typedef Arg2                                arg2_type;
    typedef typename algorithm_type::char_type  char_type;
    typedef std::basic_string<char_type>        string_type;
    typedef BasicStringRef<char_type>           string_ref;

    class Matcher;

    struct Basic {
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

    class Pattern {
    private:
        string_ref pattern_;
        string_ref matcher_;
        algorithm_type algorithm_;

    public:
        Pattern() : pattern_(), matcher_() {
            // Do nothing!
        }
        Pattern(const char_type * pattern)
            : pattern_(pattern), matcher_() {
            prepare(pattern);
        }
        Pattern(const char_type * pattern, size_t length)
            : pattern_(pattern, length), matcher_() {
            prepare(pattern, length);
        }
        template <size_t N>
        Pattern(const char_type (&pattern)[N])
            : pattern_(pattern, N), matcher_() {
            return prepare(pattern, N);
        }
        Pattern(const string_type & pattern)
            : pattern_(pattern), matcher_() {
            prepare(pattern);
        }
        Pattern(const string_ref & pattern)
            : pattern_(pattern), matcher_() {
            prepare(pattern);
        }
        ~Pattern() {
            this->free();
        }

        const char_type * c_str() const { return pattern_.c_str(); }
        char_type * data() { return pattern_.data(); }
        size_t size() const { return pattern_.size(); }
        size_t length() const { return pattern_.length(); }

        arg1_type * arg1() const { return this->algorithm_.arg1(); }
        arg2_type * arg2() const { return this->algorithm_.arg2(); }

        bool is_valid() const { return (pattern_.c_str() != nullptr); }
        bool is_alive() const { return (this->is_valid() && this->algorithm_.is_alive()); }

        void prepare(const char_type * pattern, size_t length) {
            return this->preprocessing(pattern, length);
        }

        void prepare(const char_type * pattern) {
            return this->prepare(pattern, strlen(pattern));
        }

        template <size_t N>
        void prepare(const char_type (&pattern)[N]) {
            return this->prepare(pattern, N);
        }

        void prepare(const string_type & pattern) {
            return this->prepare(pattern.c_str(), pattern.size());
        }

        void prepare(const string_ref & pattern) {
            return this->prepare(pattern.c_str(), pattern.size());
        }

        int match(const char_type * text, size_t length) {
            matcher_.set_ref(text, length);
            return algorithm_type::search(text, length,
                                          this->c_str(), this->size(),
                                          this->algorithm_.arg1(),
                                          this->algorithm_.arg2());
        }

        int match(const char_type * text) {
            return this->match(text, strlen(text));
        }

        template <size_t N>
        int match(const char_type(&text)[N]) {
            return this->match(text, N);
        }

        int match(const string_type & text) {
            return this->match(text.c_str(), text.size());
        }

        int match(const string_ref & text) {
            return this->match(text.c_str(), text.size());
        }

        int match(const Matcher & matcher);

    private:
        void free() {
            this->algorithm_.free();
        }

        void preprocessing(const char_type * pattern, size_t length) {
            this->algorithm_.preprocessing(pattern, length);
        }
    }; // class Pattern

    class Matcher {
    private:
        string_ref text_;
        string_ref pattern_;

    public:
        Matcher() : text_(), pattern_() {
        }
        Matcher(const char_type * text)
            : text_(text), pattern_() {
        }
        Matcher(const char_type * text, size_t length)
            : text_(text, length), pattern_() {
        }
        template <size_t N>
        Matcher(const char_type (&text)[N])
            : text_(text, N), pattern_() {
        }
        Matcher(const string_type & text)
            : text_(text), pattern_() {
        }
        Matcher(const string_ref & text)
            : text_(text), pattern_() {
        }
        ~Matcher() {
        }

        const char_type * c_str() const { return text_.c_str(); }
        char_type * data() const { return text_.data(); }

        size_t size() const { return text_.size(); }
        size_t length() const { return this->size(); }

        const char_type * text() const { return text_.c_str(); }
        size_t text_length() const { return text_.size(); }

        const char_type * pattern() const { return pattern_.c_str(); }
        size_t pattern_length() const { return pattern_.size(); }

        // set_text()
        void set_text(const char_type * text, size_t length) {
            text_.set_ref(text, length);
        }

        void set_text(const char_type * text) {
            text_.set_ref(text);
        }

        template <size_t N>
        void set_text(const char_type (&text)[N]) {
            text_.set_ref(text, N);
        }

        void set_text(const string_type & text) {
            text_.set_ref(text);
        }

        void set_text(const string_ref & text) {
            text_.set_ref(text);
        }

        // find()
        int find(const Pattern & pattern) {
            return algorithm_type::search(text_.c_str(), text_.size(),
                                          pattern.c_str(), pattern.size(),
                                          pattern.arg1(), pattern.arg2);
        }

        int find(const char_type * text, size_t length, const Pattern & pattern) {
            pattern_.set_ref(pattern.c_str(), pattern.size());
            return this->search(text, length,
                                pattern.c_str(), pattern.size(),
                                pattern.arg1(), pattern.arg2);
        }

        int find(const char_type * text, const Pattern & pattern) {
            return this->find(text, strlen(text), pattern);
        }

        template <size_t N>
        int find(const char_type (&text)[N], const Pattern & pattern) {
            return this->find(text, N, pattern);
        }

        int find(const string_type & text, const Pattern & pattern) {
            return this->find(text.c_str(), text.size(), pattern);
        }

        int find(const string_ref & text, const Pattern & pattern) {
            return this->find(text.c_str(), text.size(), pattern);
        }

    private:
        int search(const char_type * text, size_t text_len,
                   const char_type * pattern, size_t pattern_len,
                   arg1_type * arg1, arg2_type * arg2) {
            text_.set_ref(text, text_len);
            return algorithm_type::search(text, text_len, pattern, pattern_len, arg1, arg2);
        }
    }; // class Matcher

}; // struct BasicAlgorithm<T, Args>

template <typename T, typename Arg1, typename Arg2>
inline
int StringMatch::BasicAlgorithm<T, Arg1, Arg2>::Pattern::match(
    const typename StringMatch::BasicAlgorithm<T, Arg1, Arg2>::Matcher & matcher) {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace StringMatch

#endif // STRING_MATCH_BASIC_ALGORITHM_H
