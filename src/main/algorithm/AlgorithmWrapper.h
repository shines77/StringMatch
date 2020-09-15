
#ifndef STRING_MATCH_ALGORITHM_WRAPPER_H
#define STRING_MATCH_ALGORITHM_WRAPPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include "basic/stddef.h"
#include "basic/stdint.h"
#include "basic/inttypes.h"
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <string>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "StringMatch.h"
#include "jstd/char_traits.h"
#include "support/StringRef.h"
#include "algorithm/AhoCorasick.h"

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

template <typename AlgorithmImpl>
struct AlgorithmCounter {
    static void reset_counter() {
    }

    static int get_counter() {
        return 0;
    }
};

template <>
struct AlgorithmCounter<AhoCorasickImpl<char>> {
    static void reset_counter() {
        AhoCorasickImpl<char>::reset_counter();
    }

    static int get_counter() {
        return AhoCorasickImpl<char>::get_counter();
    }
};

template <>
struct AlgorithmCounter<AhoCorasickImpl<wchar_t>> {
    static void reset_counter() {
        AhoCorasickImpl<wchar_t>::reset_counter();
    }

    static int get_counter() {
        return AhoCorasickImpl<wchar_t>::get_counter();
    }
};

template <typename AlgorithmTy>
struct AlgorithmWrapper {

    typedef AlgorithmTy                         algorithm_type;
//  typedef typename algorithm_type::tuple_type tuple_type;
    typedef typename algorithm_type::char_type  char_type;
    typedef typename algorithm_type::size_type  size_type;
    typedef std::basic_string<char_type>        string_type;
    typedef BasicStringRef<char_type>           stringref_type;

    class Matcher;

    class Pattern {
    private:
        stringref_type pattern_;
        algorithm_type algorithm_;
        bool compiled_;

    public:
        Pattern() : pattern_(), compiled_(false) {
            // Do nothing!
        }
        Pattern(const char_type * pattern)
            : pattern_(pattern), compiled_(false) {
            this->compiled_ = this->preprocessing(pattern);
        }
        Pattern(const char_type * pattern, size_type length)
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
        Pattern(const Pattern & src)
            : pattern_(src.pattern_), algorithm_(src.algorithm_),
              compiled_(src.compiled_) {
        }
        ~Pattern() {
            this->destroy();
        }

        const char_type * c_str() const { return this->pattern_.c_str(); }
        const char_type * data() const { return this->pattern_.data(); }
        char_type * c_str() { return this->pattern_.c_str(); }
        char_type * data() { return this->pattern_.data(); }
        size_type size() const { return this->pattern_.size(); }
        size_type length() const { return this->pattern_.length(); }

        bool is_valid() const { return (this->pattern_.c_str() != nullptr); }
        bool is_alive() const { return (this->is_valid() && this->algorithm_.is_alive()); }
        bool has_compiled() const { return (this->need_preprocessing() ? this->compiled_ : true); }
        bool need_preprocessing() const { return this->algorithm_.need_preprocessing(); }

        // Pattern::preprocessing()
        bool preprocessing(const char_type * pattern, size_type length) {
            assert(pattern != nullptr);
            bool success = this->preprocessing_impl(pattern, length);
            this->compiled_ = success;
            return success;
        }

        bool preprocessing(const char_type * pattern) {
            return this->preprocessing(pattern, detail::strlen(pattern));
        }

        bool preprocessing(const char_type * first, const char_type * last) {
            assert(first <= last);
            return this->preprocessing(first, (size_type)(last - first));
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
        Long match(const char_type * text, size_type length) const {
            assert(text != nullptr);
            return this->algorithm_.search(text, length, this->c_str(), this->size());
        }

        Long match(const char_type * text) const  {
            return this->match(text, detail::strlen(text));
        }

        Long match(const char_type * first, const char_type * last) const {
            assert(first <= last);
            return this->match(first, (size_type)(last - first));
        }

        template <size_t N>
        Long match(const char_type(&text)[N]) const {
            return this->match(text, N - 1);
        }

        Long match(const string_type & text) const {
            return this->match(text.c_str(), text.size());
        }

        Long match(const stringref_type & text) const {
            return this->match(text.c_str(), text.size());
        }

        // Pattern::match(matcher);
        Long match(const Matcher & matcher) const;

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

        bool preprocessing_impl(const char_type * pattern, size_type length) {
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
        Matcher(const char_type * text, size_type length)
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
        Matcher(const Matcher & src)
            : text_(src.text_) {
        }
        ~Matcher() {
            this->text_.reset();
        }

        const char_type * c_str() const { return this->text_.c_str(); }
        const char_type * data() const { return this->text_.data(); }
        char_type * c_str() { return this->text_.c_str(); }
        char_type * data() { return this->text_.data(); }

        size_t size() const { return this->text_.size(); }
        size_t length() const { return this->size(); }

        bool is_valid() const { return (this->text_.c_str() != nullptr); }
        stringref_type & text() const { return this->text_; }

        // Matcher::set_text()
        void set_text(const char_type * text, size_type length) {
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
        static Long find(const char_type * text, size_type length,
                        const char_type * pattern, size_type pattern_len) {
            Pattern _pattern(pattern, pattern_len);
            return _pattern.match(text, length);
        }

        // Matcher::find(pattern);
        Long find(const Pattern & pattern) const {
            return pattern.match(this->text_.c_str(), this->text_.size());
        }

        // Matcher::find(text, length, pattern);
        Long find(const char_type * text, size_type length, const Pattern & pattern) {
            assert(text != nullptr);
            return this->find_impl(text, length, pattern);
        }

        Long find(const char_type * text, const Pattern & pattern) {
            return this->find(text, detail::strlen(text), pattern);
        }

        Long find(const char_type * first, const char_type * last, const Pattern & pattern) {
            assert(first <= last);
            return this->find(first, (size_type)(last - first), pattern);
        }

        template <size_t N>
        Long find(const char_type (&text)[N], const Pattern & pattern) {
            return this->find(text, N - 1, pattern);
        }

        Long find(const string_type & text, const Pattern & pattern) {
            return this->find(text.c_str(), text.size(), pattern);
        }

        Long find(const stringref_type & text, const Pattern & pattern) {
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
        Long find_impl(const char_type * text, size_type text_len, const Pattern & pattern) {
            this->text_.set_data(text, text_len);
            return pattern.match(text, text_len);
        }
    }; // class Matcher

    static const char * name() { return algorithm_type::name(); }
    static bool need_preprocessing() { return algorithm_type::need_preprocessing(); }

    static void reset_counter() {
        AlgorithmCounter<algorithm_type>::reset_counter();
    }

    static int get_counter() {
        return AlgorithmCounter<algorithm_type>::get_counter();
    }

    // AlgorithmWrapper::match(matcher, pattern)
    static Long match(const Matcher & matcher, const Pattern & pattern) {
        return pattern.match(matcher.c_str(), matcher.size());
    }

    static Long match(const char_type * text, size_type length,
                     const char_type * pattern, size_type pattern_len) {
        return Matcher::find(text, length, pattern, pattern_len);
    }
}; // struct AlgorithmWrapper<AlgorithmTy>

template <typename AlgorithmTy>
inline
Long AlgorithmWrapper<AlgorithmTy>::Pattern::match(
    const typename AlgorithmWrapper<AlgorithmTy>::Matcher & matcher) const {
    return this->match(matcher.c_str(), matcher.size());
}

namespace AnsiString {
    typedef AlgorithmWrapper< AhoCorasickImpl<char> >       AhoCorasick;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< AhoCorasickImpl<wchar_t> >    AhoCorasick;
}

} // namespace StringMatch

#endif // STRING_MATCH_ALGORITHM_WRAPPER_H
