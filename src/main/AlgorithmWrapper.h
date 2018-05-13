
#ifndef STRING_MATCH_ALGORITHM_WRAPPER_H
#define STRING_MATCH_ALGORITHM_WRAPPER_H

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

#include <tuple>
#include <type_traits>
#include <utility>

#include "StringRef.h"
#include "Apply.h"

namespace StringMatch {

template <typename T>
struct AlgorithmWrapper {

    typedef T                                   algorithm_type;
    typedef typename algorithm_type::tuple_type tuple_type;
    typedef typename algorithm_type::char_type  char_type;
    typedef std::basic_string<char_type>        string_type;
    typedef BasicStringRef<char_type>           stringref_type;

    class Matcher;

    struct Console {
        static void print_result(const char * text, size_t text_len,
                                 const char * pattern, size_t pattern_len, int index_of) {
            if (text != nullptr)
                printf("text     = \"%s\", text_len = %" PRIuPTR "\n", text, text_len);
            else
                printf("text     = \"NULL\", text_len = 0\n");

            if (pattern != nullptr)
                printf("pattern  = \"%s\", pattern_len = %" PRIuPTR "\n", pattern, pattern_len);
            else
                printf("pattern  = \"NULL\", pattern_len = 0\n");

            printf("index_of = %d\n", index_of);
            printf("\n");
        }

        static void print_result(const char * text, size_t text_len,
                                 const char * pattern, size_t pattern_len, int index_of,
                                 int sum, double time_spent) {
            print_result(text, text_len, pattern, pattern_len, index_of);
            printf("sum: %-11d, time spent: %0.3f ms\n", sum, time_spent);
            printf("\n");
        }
    };

    class Pattern {
    private:
        stringref_type pattern_;
        bool compiled_;
        algorithm_type algorithm_;

    public:
        Pattern() : pattern_(), compiled_(false) {
            // Do nothing!
        }
        Pattern(const char_type * pattern)
            : pattern_(pattern), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern);
        }
        Pattern(const char_type * pattern, size_t length)
            : pattern_(pattern, length), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern, length);
        }
        template <size_t N>
        Pattern(const char_type (&pattern)[N])
            : pattern_(pattern, N), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern, N);
        }
        Pattern(const string_type & pattern)
            : pattern_(pattern), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern);
        }
        Pattern(const stringref_type & pattern)
            : pattern_(pattern), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern);
        }
        ~Pattern() {
            this->destroy();
        }

        const char_type * c_str() const { return this->pattern_.c_str(); }
        char_type * data() { return this->pattern_.data(); }
        size_t size() const { return this->pattern_.size(); }
        size_t length() const { return this->pattern_.length(); }

        bool is_valid() const { return (this->pattern_.c_str() != nullptr); }
        bool is_alive() const { return (this->is_valid() && this->algorithm_.is_alive()); }
        bool is_compiled() const { return this->compiled_; }

        // Pattern::args
        tuple_type & get_args() const {
            return const_cast<tuple_type &>(this->algorithm_.get_args());
        }

        const tuple_type & get_cargs() const {
            return this->algorithm_.get_args();
        }

        void set_args(const tuple_type & args) {
            this->algorithm_.set_args(args);
        }

        template <size_t N>
        inline auto get(const tuple_type & args) const
            ->decltype(std::tuple_element<N, tuple_type>::type) {
            return std::get<N>(args);
        }

        template <size_t N>
        inline void set(const tuple_type & args,
                        typename std::tuple_element<N, tuple_type>::type && value) const {
            //std::set<N>(args, value);
        }

        // Pattern::prepare()
        bool preprocessing(const char_type * pattern, size_t length) {
            bool success = this->internal_preprocessing(pattern, length);
            this->compiled_ = success;
            return success;
        }

        bool preprocessing(const char_type * pattern) {
            return this->preprocessing(pattern, detail::strlen(pattern));
        }

        template <size_t N>
        bool preprocessing(const char_type (&pattern)[N]) {
            return this->preprocessing(pattern, N);
        }

        bool preprocessing(const string_type & pattern) {
            return this->preprocessing(pattern.c_str(), pattern.size());
        }

        bool preprocessing(const stringref_type & pattern) {
            return this->preprocessing(pattern.c_str(), pattern.size());
        }

        // Pattern::match(text, length);
        int match(const char_type * text, size_t length) {
            return algorithm_type::search(text, length,
                                          this->c_str(), this->size(),
                                          this->get_args());
        }

        int match(const char_type * text) {
            return this->match(text, detail::strlen(text));
        }

        template <size_t N>
        int match(const char_type(&text)[N]) {
            return this->match(text, N);
        }

        int match(const string_type & text) {
            return this->match(text.c_str(), text.size());
        }

        int match(const stringref_type & text) {
            return this->match(text.c_str(), text.size());
        }

        // Pattern::match(matcher);
        int match(const Matcher & matcher);

        // Pattern::print_result()
        void print_result(const Matcher & matcher, int index_of) {
            if (this->is_alive()) {
                Console::print_result(this->matcher_.c_str(), this->matcher_.size(),
                               this->c_str(), this->size(), index_of);
            }
            else {
                Console::print_result(this->matcher_.c_str(), this->matcher_.size(),
                               nullptr, 0, index_of);
            }
        }

        void print_result(const Matcher & matcher, int index_of, int sum, double time_spent) {
            if (this->is_alive()) {
                Console::print_result(matcher.c_str(), matcher.size(),
                               this->c_str(), this->size(),
                               index_of, sum, time_spent);
            }
            else {
                Console::print_result(matcher.c_str(), matcher.size(), nullptr, 0,
                               index_of, sum, time_spent);
            }
        }

    private:
        void destroy() {
            this->pattern_.reset();
            this->algorithm_.destroy();
        }

        bool internal_preprocessing(const char_type * pattern, size_t length) {
            this->pattern_.set_data(pattern, length);
            return this->algorithm_.preprocessing(pattern, length);
        }
    }; // class Pattern

    class Matcher {
    private:
        stringref_type text_;

    public:
        Matcher() : text_() {
        }
        Matcher(const char_type * text)
            : text_(text) {
        }
        Matcher(const char_type * text, size_t length)
            : text_(text, length) {
        }
        template <size_t N>
        Matcher(const char_type (&text)[N])
            : text_(text, N) {
        }
        Matcher(const string_type & text)
            : text_(text) {
        }
        Matcher(const stringref_type & text)
            : text_(text) {
        }
        ~Matcher() {
        }

        const char_type * c_str() const { return this->text_.c_str(); }
        char_type * data() const { return this->text_.data(); }

        size_t size() const { return this->text_.size(); }
        size_t length() const { return this->size(); }

        stringref_type & text() const { return this->text_; }

        // Matcher::set_text()
        void set_text(const char_type * text, size_t length) {
            this->text_.set_data(text, length);
        }

        void set_text(const char_type * text) {
            this->text_.set_data(text);
        }

        template <size_t N>
        void set_text(const char_type (&text)[N]) {
            this->text_.set_data(text, N);
        }

        void set_text(const string_type & text) {
            this->text_.set_data(text);
        }

        void set_text(const stringref_type & text) {
            this->text_.set_data(text);
        }

        // Matcher::find(pattern);
        int find(const Pattern & pattern) {
            return algorithm_type::search(this->text_.c_str(), this->text_.size(),
                                          pattern.c_str(), pattern.size(),
                                          pattern.get_args());
        }

        // Matcher::find(text, length, pattern);
        int find(const char_type * text, size_t length, const Pattern & pattern) {
            return this->search(text, length,
                                pattern.c_str(), pattern.size(),
                                pattern.get_args());
        }

        int find(const char_type * text, const Pattern & pattern) {
            return this->find(text, detail::strlen(text), pattern);
        }

        template <size_t N>
        int find(const char_type (&text)[N], const Pattern & pattern) {
            return this->find(text, N, pattern);
        }

        int find(const string_type & text, const Pattern & pattern) {
            return this->find(text.c_str(), text.size(), pattern);
        }

        int find(const stringref_type & text, const Pattern & pattern) {
            return this->find(text.c_str(), text.size(), pattern);
        }

        // Matcher::print_result()
        void print_result(const Pattern & pattern, int index_of) {
            Console::print_result(this->text_.c_str(), this->text_.size(),
                           pattern.c_str(), pattern.size(), index_of);
        }

        void print_result(const Pattern & pattern, int index_of, int sum, double time_spent) {
            Console::print_result(this->text_.c_str(), this->text_.size(),
                           pattern.c_str(), pattern.size(),
                           index_of, sum, time_spent);
        }

    private:
        int search(const char_type * text, size_t text_len,
                   const char_type * pattern, size_t pattern_len,
                   const tuple_type & tuple) {
            this->text_.set_data(text, text_len);
            return algorithm_type::search(text, text_len, pattern, pattern_len, tuple);
        }
    }; // class Matcher

    // AlgorithmWrapper::find(matcher, pattern)
    static int find(const Matcher & matcher, const Pattern & pattern) {
        return algorithm_type::search(matcher.c_str(), matcher.size(),
                                      pattern.c_str(), pattern.size(),
                                      pattern.get_cargs());
    }

}; // struct AlgorithmWrapper<T>

template <typename T>
inline
int AlgorithmWrapper<T>::Pattern::match(const typename AlgorithmWrapper<T>::Matcher & matcher) {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace StringMatch

#endif // STRING_MATCH_ALGORITHM_WRAPPER_H
