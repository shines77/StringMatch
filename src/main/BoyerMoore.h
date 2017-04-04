
#ifndef STRING_MATCH_BOYERMOORE_H
#define STRING_MATCH_BOYERMOORE_H

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

namespace StringMatch {
namespace AnsiString {
namespace BoyerMoore {

class Matcher;

class Algorithm : public StringMatch::AlgorithmBase {
private:
    /* Preprocessing bad characters. */
    static void preBmBc(const char * pattern, size_t length,
                        int * bmBc, int bcLen) {
        assert(pattern != nullptr);
        assert(bmBc != nullptr);
        assert(bcLen == 256L);

        for (int i = 0; i < bcLen; ++i) {
            bmBc[i] = (int)length;
        }
        for (int i = 0; i < (int)length - 1; ++i) {
            bmBc[pattern[i]] = ((int)length - 1) - i;
        }
    }

    static void suffixes_old(const char * pattern, size_t length, int * suffix) {
        int f, g, i;
        int len = (int)length;

        suffix[len - 1] = len;
        g = len - 1;

        for (i = len - 2; i >= 0; --i) {
            if (i > g && suffix[i + len - 1 - f] < i - g) {
                suffix[i] = suffix[i + len - 1 - f];
            }
            else {
                if (i < g) {
                    g = i;
                }
                f = i;
                while ((g >= 0) && (pattern[g] == pattern[g + len - 1 - f])) {
                    --g;
                }
                suffix[i] = f - g;
            }
        }
    }

    static void suffixes(const char * pattern, size_t length, int * suffix) {
        assert(pattern != nullptr);
        assert(suffix != nullptr);

        int i, g, diff;
        int offset;
        const int last = (int)length - 1;
        for (i = last - 1; i >= 0; --i) {
            suffix[i] = 0;
        }

        suffix[last] = (int)length;
        g = last;
        offset = 0;

        for (i = last - 1; i >= 0; --i) {
            diff = i - g;
            if (diff > 0 && suffix[i + offset] < diff) {
                suffix[i] = suffix[i + offset];
            }
            else {
                if (diff < 0) {
                    g = i;
                }
                offset = last - i;
                while ((g >= 0) && (pattern[g] == pattern[g + offset])) {
                    --g;
                }
                suffix[i] = i - g;
            }
        }
    }

    /* Preprocessing good suffixs. */
    static bool preBmGs(const char * pattern, size_t length,
                        int * bmGs, int gsLen) {
        int i, j;
        int len = (int)length;

        if (length <= 0) {
            return false;
        }

        int * suffix = new int[len];
        if (suffix == nullptr) {
            return false;
        }
        Algorithm::suffixes(pattern, length, suffix);

        assert(pattern != nullptr);
        assert(bmGs != nullptr);
        for (i = 0; i < len; ++i) {
            bmGs[i] = len;
        }

        j = 0;
        for (i = len - 1; i >= 0; --i) {
            if (suffix[i] == i + 1) {
                for (; j < (len - 1) - i; ++j) {
                    if (bmGs[j] == len) {
                        bmGs[j] = (len - 1) - i;
                    }
                }
            }
        }

        for (i = 0; i <= len - 2; ++i) {
            bmGs[(len - 1) - suffix[i]] = (len - 1) - i;
        }

        if (suffix != nullptr) {
            delete[] suffix;
        }
        return true;
    }

public:
    /* Preprocessing */
    static bool preprocessing(const char * pattern, size_t length,
                              int * bmBc, int bcLen,
                              int * bmGs, int gsLen) {
        assert(pattern != nullptr);

        /* Preprocessing bad characters. */
        Algorithm::preBmBc(pattern, length, bmBc, bcLen);

        /* Preprocessing good suffixs. */
        return Algorithm::preBmGs(pattern, length, bmGs, gsLen);
    }

    /* Search */
    static int search(const char * text, size_t text_len,
                      const char * pattern_, size_t pattern_len,
                      int * bmBc, int * bmGs) {
        assert(text != nullptr);
        assert(pattern_ != nullptr);
        assert(bmBc != nullptr);
        assert(bmGs != nullptr);

        if (text_len < pattern_len) {
            // Not found
            return -1;
        }

        if ((size_t)text | (size_t)pattern_ | (size_t)bmBc | (size_t)bmGs) {
            const char * pattern_end = pattern_;
            const char * target_end = text + (text_len - pattern_len);
            const int pattern_last = (int)pattern_len - 1;
            int target_idx = 0;
            do {
                register const char * target = text + target_idx + pattern_last;
                register const char * pattern = pattern_ + pattern_last;
                assert(target < (text + text_len));

                while (pattern >= pattern_end) {
                    if (*target != *pattern) {
                        break;
                    }
                    target--;
                    pattern--;
                }

                if (pattern >= pattern_end) {
                    int pattern_idx = (int)(pattern - pattern_);
                    target_idx += sm_max(bmGs[pattern_idx],
                                         bmBc[*target] - (pattern_last - pattern_idx));
                }
                else {
                    assert(target_idx >= 0);
                    assert(target_idx < (int)text_len);
                    // Found
                    return target_idx;
                }
            } while (target_idx <= (int)(text_len - pattern_len));

            // Not found
            return -1;
        }
        // Invalid parameters
        return -2;
    }
};

class Pattern {
private:
    StringRef pattern_;
    int * bmGs_;
    StringRef matcher_;
    int bmBc_[256];

public:
    Pattern() : pattern_(), bmGs_(nullptr), matcher_() {
        // Do nothing!
    }
    Pattern(const char * pattern)
        : pattern_(pattern), bmGs_(nullptr), matcher_() {
        prepare(pattern);
    }
    Pattern(const char * pattern, size_t length)
        : pattern_(pattern, length), bmGs_(nullptr), matcher_() {
        prepare(pattern, length);
    }
    template <size_t N>
    Pattern(const char (&pattern)[N])
        : pattern_(pattern, N), bmGs_(nullptr), matcher_() {
        return prepare(pattern, N);
    }
    Pattern(const std::string & pattern)
        : pattern_(pattern), bmGs_(nullptr), matcher_() {
        prepare(pattern);
    }
    Pattern(const StringRef & pattern)
        : pattern_(pattern), bmGs_(nullptr), matcher_() {
        prepare(pattern);
    }
    ~Pattern() {
        this->free();
    }

    const char * c_str() const { return pattern_.c_str(); }
    char * data() { return pattern_.data(); }
    size_t size() const { return pattern_.size(); }
    size_t length() const { return pattern_.length(); }

    int * bmBc() const { return (int *)&bmBc_[0]; }
    int * bmGs() const { return bmGs_; }

    bool is_valid() const { return (pattern_.c_str() != nullptr); }
    bool is_alive() const { return ((pattern_.c_str() != nullptr) && (bmGs_ != nullptr)); }

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
                                 this->bmBc(),
                                 this->bmGs());
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
    void free() {
        if (bmGs_ != nullptr) {
            delete[] bmGs_;
            bmGs_ = nullptr;
        }
    }

    void preprocessing(const char * pattern, size_t length) {
        pattern_.set_ref(pattern, length);

        int * bmGs = new int[(int)length + 1];
        if (bmGs != nullptr)  {
            bool success = Algorithm::preprocessing(pattern, length, bmBc_, 256L, bmGs, (int)length);
            assert(success);
        }
        bmGs_ = bmGs;
    }
};

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

    int find(const char * text, size_t length, const Pattern & pattern) {
        pattern_.set_ref(pattern.c_str(), pattern.size());
        return this->search(text, length,
                            pattern.c_str(), pattern.size(),
                            pattern.bmBc(), pattern.bmGs());
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

    int find(const Pattern & pattern) {
        return Algorithm::search(text_.c_str(), text_.size(),
                                 pattern.c_str(), pattern.size(),
                                 pattern.bmBc(), pattern.bmGs());
    }

    void display(int index_of) {
        Algorithm::display(text_.c_str(), text_.size(), pattern_.c_str(), pattern_.size(), index_of);
    }

    void display_test(int index_of, int sum, double time_spent) {
        Algorithm::display_test(text_.c_str(), text_.size(), pattern_.c_str(), pattern_.size(),
                                index_of, sum, time_spent);
    }

private:
    int search(const char * text, size_t text_len,
               const char * pattern, size_t pattern_len,
               int * bmBc, int * bmGs) {
        text_.set_ref(text, text_len);
        return Algorithm::search(text, text_len, pattern, pattern_len, bmBc, bmGs);
    }
};

inline int Pattern::match(const Matcher & matcher) {
    return this->match(matcher.c_str(), matcher.size());
}

} // namespace BoyerMoore
} // namespace AnsiString
} // namespace StringMatch

#endif // STRING_MATCH_BOYERMOORE_H
