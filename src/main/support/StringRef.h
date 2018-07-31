
#ifndef MAIN_STRING_REF_H
#define MAIN_STRING_REF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <string.h>
#include <cstdint>  // for std::intptr_t
#include <cstddef>  // for std::ptrdiff_t
#include <string>
#include <type_traits>

#include "jstd/string_iterator.h"

namespace StringMatch {

namespace detail {

//////////////////////////////////////////
// detail::strlen<T>()
//////////////////////////////////////////

template <typename CharTy>
inline std::size_t strlen(const CharTy * str) {
    return (std::size_t)::strlen((const char *)str);
}

template <>
inline std::size_t strlen(const char * str) {
    return (std::size_t)::strlen(str);
}

template <>
inline std::size_t strlen(const unsigned char * str) {
    return (std::size_t)::strlen((const char *)str);
}

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)

template <>
inline std::size_t strlen(const short * str) {
    return (std::size_t)::wcslen((const wchar_t *)str);
}

template <>
inline std::size_t strlen(const unsigned short * str) {
    return (std::size_t)::wcslen((const wchar_t *)str);
}

#endif // _WIN32

template <>
inline std::size_t strlen(const wchar_t * str) {
    return (std::size_t)::wcslen((const wchar_t *)str);
}

} // namespace detail

template <typename CharTy>
class BasicStringRef {
public:
    typedef CharTy          value_type;
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef CharTy *        pointer;
    typedef const CharTy *  const_pointer;
    typedef CharTy &        reference;
    typedef const CharTy &  const_reference;

    typedef jstd::string_iterator<BasicStringRef<CharTy>>       iterator;
    typedef jstd::const_string_iterator<BasicStringRef<CharTy>> const_iterator;

    typedef std::basic_string<value_type> string_type;
    typedef BasicStringRef<value_type> stringref_type;

private:
    const value_type * data_;
    size_type length_;

public:
    BasicStringRef() : data_(nullptr), length_(0) {}
    BasicStringRef(const value_type * data)
        : data_(data), length_(detail::strlen(data)) {}
    BasicStringRef(const value_type * data, size_type length)
        : data_(data), length_(length) {}
    BasicStringRef(const value_type * first, const value_type * last)
        : data_(first), length_(size_type(last - first)) {}
    template <size_type N>
    BasicStringRef(const value_type(&data)[N])
        : data_(data), length_(N - 1) {}
    BasicStringRef(const string_type & src)
        : data_(src.c_str()), length_(src.size()) {}
    BasicStringRef(const stringref_type & src)
        : data_(src.data()), length_(src.length()) {}
    BasicStringRef(string_type && src)
        : data_(src.c_str()), length_(src.size()) {
        // Only reference from src string
    }
    BasicStringRef(stringref_type && src)
        : data_(src.data()), length_(src.length()) {
        src.reset();
    }
    ~BasicStringRef() { /* Do nothing! */ }

    BasicStringRef & operator = (const value_type * data) {
        this->data_ = data;
        this->length_ = detail::strlen(data);
        return *this;
    }

    BasicStringRef & operator = (const string_type & rhs) {
        this->data_ = rhs.c_str();
        this->length_ = rhs.size();
        return *this;
    }

    BasicStringRef & operator = (const stringref_type & rhs) {
        this->data_ = rhs.data();
        this->length_ = rhs.length();
        return *this;
    }

    const value_type * data() const { return this->data_; }
    size_t length() const { return this->length_; }

    const value_type * c_str() const { return this->data(); }
    size_t size() const { return this->length(); }

    value_type * data() { return const_cast<value_type *>(this->data_); }
    value_type * c_str() { return const_cast<value_type *>(this->data()); }

    bool empty() const { return (this->length() == 0); }

    iterator begin() const { return iterator(this->data_); }
    iterator end() const { return iterator(this->data_ + this->length_); }

    const_iterator cbegin() const { return const_iterator(this->data_); }
    const_iterator cend() const { return const_iterator(this->data_ + this->length_); }

    void reset() {
        this->data_ = nullptr;
        this->length_ = 0;
    }

    void clear() {
        this->reset();
    }

    void assign(const value_type * data) {
        this->set_data(data, detail::strlen(data));
    }

    void assign(const value_type * data, size_type length) {
        this->set_data(data, length);
    }

    void assign(const value_type * first, const value_type * last) {
        this->set_data(first, size_type(last - first));
    }

    template <size_type N>
    void assign(const value_type (&data)[N]) {
        this->set_data(data, N - 1);
    }

    void assign(const string_type & src) {
        this->set_data(src.c_str(), src.size());
    }

    void assign(const stringref_type & src) {
        this->set_data(src.data(), src.length());
    }

    void set_data(const value_type * data, size_t length) {
        this->data_ = data;
        this->length_ = length;
    }

    void set_data(const value_type * data) {
        this->set_data(data, detail::strlen(data));
    }

    void set_data(const value_type * first, const value_type * last) {
        this->set_data(first, size_type(last - first));
    }

    template <size_t N>
    void set_data(const value_type(&data)[N]) {
        this->set_data(data, N - 1);
    }

    void set_data(const string_type & src) {
        this->set_data(src.c_str(), src.size());
    }

    void set_data(const stringref_type & src) {
        this->set_data(src.data(), src.length());
    }

    string_type toString() const {
        return string_type(this->data_, this->length_);
    }
}; // class BasicStringRef<CharTy>

template <typename CharType>
class BasicStringRefHelper {
public:
    typedef CharType value_type;
    typedef BasicStringRef<value_type> stringref_type;

private:
    stringref_type str_;
    bool truncated_;
    value_type save_char_;

public:
    BasicStringRefHelper()
        : truncated_(false),
          save_char_(static_cast<value_type>('\0')) {
        (void)save_char_;
    }
    ~BasicStringRefHelper() { detach(); }

    bool attach(const stringref_type & str) {
        // If the string reference don't recover the truncated char,
        // don't accept the new attach.
        if (likely(!truncated_)) {
            str_ = str;
        }
        return (!truncated_);
    }

    void detach() {
        // If have be truncated, recover the saved terminator char first,
        // and then clear the string reference.
        if (unlikely(truncated_)) {
            recover();
            str_.clear();
        }
    }

    void truncate() {
        if (likely(!truncated_)) {
            value_type * first = (value_type *)str_.data();
            value_type * last = first + str_.size();
            assert(last != nullptr);
            save_char_ = *last;
            *last = static_cast<value_type>('\0');
            truncated_ = true;
        }
    }

    void recover() {
        if (likely(truncated_)) {
            value_type * first = (value_type *)str_.data();
            value_type * last = first + str_.size();
            assert(last != nullptr);
            *last = save_char_;
            truncated_ = false;
        }
    }
};

typedef BasicStringRef<char>    StringRefA;
typedef BasicStringRef<wchar_t> StringRefW;
typedef BasicStringRef<char>    StringRef;

typedef BasicStringRefHelper<char>      StringRefHelperA;
typedef BasicStringRefHelper<wchar_t>   StringRefHelperW;
typedef BasicStringRefHelper<char>      StringRefHelper;

} // namespace StringMatch

#endif // MAIN_STRING_REF_H
