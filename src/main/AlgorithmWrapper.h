
#ifndef STRING_MATCH_ALGORITHM_WRAPPER_H
#define STRING_MATCH_ALGORITHM_WRAPPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
#include <inttypes.h>
#else
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
	|| defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
#define PRIuPTR		"llu"
#else
#define PRIuPTR		"lu"
#endif
#endif
#include <assert.h>
#include <string>
#include <memory>

#include <tuple>
#include <type_traits>
#include <utility>

#include "StringMatch.h"
#include "StringRef.h"
#include "Apply.h"

namespace StringMatch {

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

template <typename AlgorithmTy>
struct AlgorithmWrapper {

    typedef AlgorithmTy                         algorithm_type;
//  typedef typename algorithm_type::tuple_type tuple_type;
    typedef typename algorithm_type::char_type  char_type;
    typedef std::basic_string<char_type>        string_type;
    typedef BasicStringRef<char_type>           stringref_type;

    class Matcher;

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
        Pattern(const char_type * first, const char_type * last)
            : pattern_(first, last), compiled_(false) {
            this->compiled_ = this->preprocessing(first, last);
        }
        template <size_t N>
        Pattern(const char_type (&pattern)[N])
            : pattern_(pattern, N - 1), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern, N - 1);
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
        bool has_compiled() const { return (this->need_preprocessing() ? this->compiled_ : true); }
        bool need_preprocessing() const { return this->algorithm_.need_preprocessing(); }

        /*
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
            -> decltype(typename std::tuple_element<N, tuple_type>::type) {
            return std::get<N>(args);
        }

        template <size_t N>
        inline void set(const tuple_type & args,
                        typename std::tuple_element<N, tuple_type>::type && value) const {
            std::set<N>(args, value);
        }
        */

        // Pattern::preprocessing()
        bool preprocessing(const char_type * pattern, size_t length) {
            assert(pattern != nullptr);
            bool success = this->internal_preprocessing(pattern, length);
            this->compiled_ = success;
            return success;
        }

        bool preprocessing(const char_type * pattern) {
            return this->preprocessing(pattern, detail::strlen(pattern));
        }

        bool preprocessing(const char_type * first, const char_type * last) {
            assert(first <= last);
            return this->preprocessing(first, (size_t)(last - first));
        }

        template <size_t N>
        bool preprocessing(const char_type (&pattern)[N]) {
            return this->preprocessing(pattern, N - 1);
        }

        bool preprocessing(const string_type & pattern) {
            return this->preprocessing(pattern.c_str(), pattern.size());
        }

        bool preprocessing(const stringref_type & pattern) {
            return this->preprocessing(pattern.c_str(), pattern.size());
        }

        // Pattern::match(text, length);
        int match(const char_type * text, size_t length) const {
            assert(text != nullptr);
            return this->algorithm_.search(text, length, this->c_str(), this->size());
        }

        int match(const char_type * text) const  {
            return this->match(text, detail::strlen(text));
        }

        int match(const char_type * first, const char_type * last) const {
            assert(first <= last);
            return this->match(first, (size_t)(last - first));
        }

        template <size_t N>
        int match(const char_type(&text)[N]) const {
            return this->match(text, N - 1);
        }

        int match(const string_type & text) const {
            return this->match(text.c_str(), text.size());
        }

        int match(const stringref_type & text) const {
            return this->match(text.c_str(), text.size());
        }

        // Pattern::match(matcher);
        int match(const Matcher & matcher) const;

        // Pattern::print_result()
        void print_result(const Matcher & matcher, int index_of) {
            Console::print_result(matcher.c_str(), matcher.size(),
                                  this->c_str(), this->size(), index_of);
        }

        void print_result(const Matcher & matcher, int index_of, int sum, double time_spent) {
            Console::print_result(matcher.c_str(), matcher.size(),
                                  this->c_str(), this->size(),
                                  index_of, sum, time_spent);
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
        Matcher(const char_type * first, const char_type * last)
            : text_(first, last) {
        }
        template <size_t N>
        Matcher(const char_type (&text)[N])
            : text_(text, N - 1) {
        }
        Matcher(const string_type & text)
            : text_(text) {
        }
        Matcher(const stringref_type & text)
            : text_(text) {
        }
        ~Matcher() {
            this->text_.reset();
        }

        const char_type * c_str() const { return this->text_.c_str(); }
        char_type * data() const { return this->text_.data(); }

        size_t size() const { return this->text_.size(); }
        size_t length() const { return this->size(); }

        bool is_valid() const { return (this->text_.c_str() != nullptr); }
        stringref_type & text() const { return this->text_; }

        // Matcher::set_text()
        void set_text(const char_type * text, size_t length) {
            this->text_.set_data(text, length);
        }

        void set_text(const char_type * text) {
            this->text_.set_data(text);
        }

        void set_text(const char_type * first, const char_type * last) {
            this->text_.set_data(first, last);
        }

        template <size_t N>
        void set_text(const char_type (&text)[N]) {
            this->text_.set_data(text, N - 1);
        }

        void set_text(const string_type & text) {
            this->text_.set_data(text);
        }

        void set_text(const stringref_type & text) {
            this->text_.set_data(text);
        }

        // Matcher::find(text, length, pattern, pattern_len);
        static int find(const char_type * text, size_t length,
                        const char_type * pattern, size_t pattern_len) {
            Pattern _pattern(pattern, pattern_len);
            return _pattern.match(text, length);
        }

        // Matcher::find(pattern);
        int find(const Pattern & pattern) const {
            return pattern.match(this->text_.c_str(), this->text_.size());
        }

        // Matcher::find(text, length, pattern);
        int find(const char_type * text, size_t length, const Pattern & pattern) {
            assert(text != nullptr);
            return this->internal_find(text, length, pattern);
        }

        int find(const char_type * text, const Pattern & pattern) {
            return this->find(text, detail::strlen(text), pattern);
        }

        int find(const char_type * first, const char_type * last, const Pattern & pattern) {
            assert(first <= last);
            return this->find(text, (size_t)(last - first), pattern);
        }

        template <size_t N>
        int find(const char_type (&text)[N], const Pattern & pattern) {
            return this->find(text, N - 1, pattern);
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
        int internal_find(const char_type * text, size_t text_len, const Pattern & pattern) {
            this->text_.set_data(text, text_len);
            return pattern.match(text, text_len);
        }
    }; // class Matcher

    static const char * name() { return algorithm_type::name(); }
    static bool need_preprocessing() { return algorithm_type::need_preprocessing(); }

    // AlgorithmWrapper::match(matcher, pattern)
    static int match(const Matcher & matcher, const Pattern & pattern) {
        return pattern.match(matcher.c_str(), matcher.size());
    }

    static int match(const char_type * text, size_t length,
                     const char_type * pattern, size_t pattern_len) {
        return Matcher::find(text, length, pattern, pattern_len);
    }

}; // struct AlgorithmWrapper<AlgorithmTy>

template <typename AlgorithmTy>
inline
int AlgorithmWrapper<AlgorithmTy>::Pattern::match(
    const typename AlgorithmWrapper<AlgorithmTy>::Matcher & matcher) const {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace StringMatch

#endif // STRING_MATCH_ALGORITHM_WRAPPER_H
