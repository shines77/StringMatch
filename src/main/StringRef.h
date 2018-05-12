
#ifndef MAIN_STRING_REF_H
#define MAIN_STRING_REF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <string.h>
#include <stdint.h>
#include <string>

namespace StringMatch {

namespace detail {
    template <typename CharT>
    inline size_t strlen(const CharT * str) {
        return ::strlen(str);
    }

    template <>
    inline size_t strlen(const char * str) {
        return ::strlen(str);
    }

    template <>
    inline size_t strlen(const wchar_t * str) {
        return ::wcslen(str);
    }
} // namespace detail

template <typename CharT>
class BasicStringRef {
public:
    typedef CharT char_type;

private:
    const char_type * data_;
    size_t length_;

public:
    BasicStringRef() : data_(nullptr), length_(0) {
        // Do nothing!
    }
    BasicStringRef(const char_type * data)
        : data_(data), length_(detail::strlen(data)) {
        // Do nothing!
    }
    BasicStringRef(const char_type * data, size_t length)
        : data_(data), length_(length) {
        // Do nothing!
    }
    template <size_t N>
    BasicStringRef(const char_type(&data)[N])
        : data_(data), length_(length) {
        // Do nothing!
    }
    BasicStringRef(const std::basic_string<char_type> & src)
        : data_(src.c_str()), length_(src.size()) {
        // Do nothing!
    }
    BasicStringRef(const BasicStringRef<char_type> & src)
        : data_(src.c_str()), length_(src.size()) {
        // Do nothing!
    }
    ~BasicStringRef() { /* Do nothing! */ }

    BasicStringRef & operator = (const BasicStringRef & rhs) {
        this->data_ = rhs.data_;
        this->length_ = rhs.length_;
        return *this;
    }

    BasicStringRef & operator = (const char_type * data) {
        this->data_ = data;
        this->length_ = detail::strlen(data);
        return *this;
    }

    const char_type * c_str() const { return this->data_; }
    char_type * data() const { return const_cast<char_type *>(this->data_); }

    size_t size() const { return this->length_; }
    size_t length() const { return this->size(); }

    void set_data(const char_type * data, size_t length) {
        this->data_ = data;
        this->length_ = length;
    }

    void set_data(const char_type * data) {
        this->set_data(data, detail::strlen(data));
    }

    template <size_t N>
    void set_data(const char_type(&data)[N]) {
        this->set_data(data, N);
    }

    void set_data(const std::basic_string<char_type> & data) {
        this->set_data(data.c_str(), data.size());
    }

    void set_data(const BasicStringRef<char_type> & data) {
        this->set_data(data.c_str(), data.size());
    }
}; // class BasicStringRef<CharT>

typedef BasicStringRef<char>    StringRef;
typedef BasicStringRef<char>    AStringRef;
typedef BasicStringRef<wchar_t> WStringRef;
typedef BasicStringRef<char>    Utf8StringRef;

} // namespace StringMatch

#endif // MAIN_STRING_REF_H
