
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

#include <tuple>
#include <type_traits>
#include <utility>

#include "StringRef.h"
#include "Apply.h"

namespace StringMatch {

template <typename T>
struct BasicAlgorithm {

    typedef T                                   algorithm_type;
    typedef typename algorithm_type::tuple_type tuple_type;
    typedef typename algorithm_type::char_type  char_type;
    typedef std::basic_string<char_type>        string_type;
    typedef BasicStringRef<char_type>           stringref_type;

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
            printf("sum: %-11d, time spent: %0.3f ms\n", sum, time_spent);
            printf("\n");
        }
    };

    class Pattern {
    private:
        stringref_type pattern_;
        stringref_type matcher_;
        bool compiled_;
        algorithm_type algorithm_;

    public:
        Pattern() : pattern_(), matcher_(), compiled_(false) {
            // Do nothing!
        }
        Pattern(const char_type * pattern)
            : pattern_(pattern), matcher_(), compiled_(false) {
            compiled_ = prepare(pattern);
        }
        Pattern(const char_type * pattern, size_t length)
            : pattern_(pattern, length), matcher_(), compiled_(false) {
            compiled_ = prepare(pattern, length);
        }
        template <size_t N>
        Pattern(const char_type (&pattern)[N])
            : pattern_(pattern, N), matcher_(), compiled_(false) {
            compiled_ = prepare(pattern, N);
        }
        Pattern(const string_type & pattern)
            : pattern_(pattern), matcher_(), compiled_(false) {
            compiled_ = prepare(pattern);
        }
        Pattern(const stringref_type & pattern)
            : pattern_(pattern), matcher_(), compiled_(false) {
            compiled_ = prepare(pattern);
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
        bool prepare(const char_type * pattern, size_t length) {
            bool success = this->preprocessing(pattern, length);
            compiled_ = success;
            return success;
        }

        bool prepare(const char_type * pattern) {
            return this->prepare(pattern, detail::strlen(pattern));
        }

        template <size_t N>
        bool prepare(const char_type (&pattern)[N]) {
            return this->prepare(pattern, N);
        }

        bool prepare(const string_type & pattern) {
            return this->prepare(pattern.c_str(), pattern.size());
        }

        bool prepare(const stringref_type & pattern) {
            return this->prepare(pattern.c_str(), pattern.size());
        }

        // Pattern::match(text, length);
        int match(const char_type * text, size_t length) {
            this->matcher_.set_data(text, length);
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

        // Pattern::display()
        void display(int index_of) {
            if (this->is_alive()) {
                Basic::display(this->matcher_.c_str(), this->matcher_.size(),
                               this->c_str(), this->size(), index_of);
            }
            else {
                Basic::display(this->matcher_.c_str(), this->matcher_.size(),
                               nullptr, 0, index_of);
            }
        }

        void display(int index_of, int sum, double time_spent) {
            if (this->is_alive()) {
                Basic::display(this->matcher_.c_str(), this->matcher_.size(),
                               this->c_str(), this->size(),
                               index_of, sum, time_spent);
            }
            else {
                Basic::display(this->matcher_.c_str(), this->matcher_.size(), nullptr, 0,
                               index_of, sum, time_spent);
            }
        }

    private:
        void destroy() {
            this->pattern_.reset();
            this->matcher_.reset();
            this->algorithm_.destroy();
        }

        bool preprocessing(const char_type * pattern, size_t length) {
            this->pattern_.set_data(pattern, length);
            return this->algorithm_.preprocessing(pattern, length);
        }
    }; // class Pattern

    class Matcher {
    private:
        stringref_type text_;
        stringref_type pattern_;

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
        size_t text_length() const { return this->text_.size(); }

        stringref_type & pattern() const { return this->pattern_; }
        size_t pattern_length() const { return this->pattern_.size(); }

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
            this->pattern_.set_data(pattern.c_str(), pattern.size());
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

        // Matcher::display()
        void display(int index_of) {
            Basic::display(this->text_.c_str(), this->text_.size(),
                           this->pattern_.c_str(), this->pattern_.size(), index_of);
        }

        void display(int index_of, int sum, double time_spent) {
            Basic::display(this->text_.c_str(), this->text_.size(),
                           this->pattern_.c_str(), this->pattern_.size(),
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

    // BasicAlgorithm::find(matcher, pattern)
    static int find(const Matcher & matcher, const Pattern & pattern) {
        return algorithm_type::search(matcher.c_str(), matcher.size(),
                                      pattern.c_str(), pattern.size(),
                                      pattern.get_cargs());
    }

}; // struct BasicAlgorithm<T>

template <typename T>
inline
int BasicAlgorithm<T>::Pattern::match(const typename BasicAlgorithm<T>::Matcher & matcher) {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace StringMatch

#endif // STRING_MATCH_BASIC_ALGORITHM_H
