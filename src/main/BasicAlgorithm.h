
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

#include "StringRef.h"

namespace StringMatch {

namespace detail {

    //
    // About std::tuple<T...>
    //
    // See: http://www.cnblogs.com/qicosmos/p/3318070.html
    //
    template <size_t N>
    struct Apply {
        template<typename Func, typename Tuple, typename... Args>
        static inline auto apply(Func && func, Tuple && tuple, Args &&... args)
            -> decltype(Apply<N - 1>::apply(
                ::std::forward<Func>(func), ::std::forward<Tuple>(tuple),
                ::std::get<N - 1>(::std::forward<Tuple>(tuple)),
                ::std::forward<Args>(args)...
                ))
        {
            return Apply<N - 1>::apply(::std::forward<Func>(func),
                                       ::std::forward<Tuple>(tuple),
                                       ::std::get<N - 1>(::std::forward<Tuple>(tuple)),
                                       ::std::forward<Args>(args)...);
        }
    };

    template <>
    struct Apply<0> {
        template<typename Func, typename Tuple, typename... Args>
        static inline auto apply(Func && func, Tuple && tuple, Args &&... args)
            -> decltype(::std::forward<Func>(func)(::std::forward<Args>(args)...))
        {
            return ::std::forward<Func>(func)(::std::forward<Args>(args)...);
        }
    };

    template <typename Func, typename Tuple>
    inline auto apply(Func && func, Tuple && tuple)
        -> decltype(Apply< ::std::tuple_size<typename ::std::decay<Tuple>::type>::value >
            ::apply(::std::forward<Func>(func), ::std::forward<Tuple>(tuple)))
    {
        return Apply< ::std::tuple_size<typename ::std::decay<Tuple>::type>::value >
            ::apply(::std::forward<Func>(func), ::std::forward<Tuple>(tuple));
    }

} // namespace detail

template <typename T>
struct BasicAlgorithm {

    typedef T                                   algorithm_type;
    typedef typename algorithm_type::tuple_type tuple_type;
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
        inline void set(const tuple_type & args, typename std::tuple_element<N, tuple_type>::type && value) const {
            std::set<N>(args, value);
        }

        bool is_valid() const { return (pattern_.c_str() != nullptr); }
        bool is_alive() const { return (this->is_valid() && this->algorithm_.is_alive()); }

        bool prepare(const char_type * pattern, size_t length) {
            return this->preprocessing(pattern, length);
        }

        bool prepare(const char_type * pattern) {
            return this->prepare(pattern, strlen(pattern));
        }

        template <size_t N>
        bool prepare(const char_type (&pattern)[N]) {
            return this->prepare(pattern, N);
        }

        bool prepare(const string_type & pattern) {
            return this->prepare(pattern.c_str(), pattern.size());
        }

        bool prepare(const string_ref & pattern) {
            return this->prepare(pattern.c_str(), pattern.size());
        }

        int match(const char_type * text, size_t length) {
            matcher_.set_ref(text, length);
            return algorithm_type::search(text, length,
                                          this->c_str(), this->size(),
                                          this->get_args());
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

        bool preprocessing(const char_type * pattern, size_t length) {
            pattern_.set_ref(pattern, length);
            return this->algorithm_.preprocessing(pattern, length);
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
                                          pattern.get_args());
        }

        int find(const char_type * text, size_t length, const Pattern & pattern) {
            pattern_.set_ref(pattern.c_str(), pattern.size());
            return this->search(text, length,
                                pattern.c_str(), pattern.size(),
                                pattern.get_args());
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
                   const tuple_type & tuple) {
            text_.set_ref(text, text_len);
            return algorithm_type::search(text, text_len, pattern, pattern_len, tuple);
        }
    }; // class Matcher

}; // struct BasicAlgorithm<T>

template <typename T>
inline
int BasicAlgorithm<T>::Pattern::match(
    const typename BasicAlgorithm<T>::Matcher & matcher) {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace StringMatch

#endif // STRING_MATCH_BASIC_ALGORITHM_H