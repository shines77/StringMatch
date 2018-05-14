
#ifndef MAIN_STRING_REF_H
#define MAIN_STRING_REF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <string.h>
#include <stdint.h>
#include <string>
#include <type_traits>

#include "string_iterator.h"
#include "forward_iterator.h"
#include "standard_iterator.h"

namespace StringMatch {

namespace detail {
    template <typename CharTy>
    inline size_t strlen(const CharTy * str) {
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

template <typename T>
struct BasicStringRef_Node {
    typedef T value_type;
    typedef BasicStringRef_Node node_type;
    typedef BasicStringRef_Node * node_ptr;

    node_ptr next;
    value_type value;

    BasicStringRef_Node(node_ptr _next = nullptr) : next(_next), value() {}
    BasicStringRef_Node(value_type const & _value)
        : next(nullptr), value(_value) {
    }
    BasicStringRef_Node(value_type && _value)
        : next(nullptr), value(std::forward<value_type>(_value)) {
    }
    BasicStringRef_Node(node_ptr _next, value_type const & _value)
        : next(_next), value(_value) {
    }
    BasicStringRef_Node(node_ptr _next, value_type && _value)
        : next(_next), value(std::forward<value_type>(_value)) {
    }
    BasicStringRef_Node(node_type const & src)
        : next(src.next), value(src.value) {
    }
    BasicStringRef_Node(node_type && src)
        : next(std::forward<value_type>(src).next),
          value(std::forward<value_type>(src).value) {
    }
    ~BasicStringRef_Node() {
#ifndef NDEBUG
        next = nullptr;
#endif
    }
};

template <typename CharTy>
class BasicStringRef {
public:
    typedef CharTy          value_type;
    typedef intptr_t        difference_type;
    typedef CharTy *        pointer;
    typedef const CharTy *  const_pointer;
    typedef CharTy &        reference;
    typedef const CharTy &  const_reference;

    typedef jstd::string_iterator<BasicStringRef<CharTy>>          iterator;
    typedef jstd::const_string_iterator<BasicStringRef<CharTy>>    const_iterator;

private:
    const value_type * data_;
    size_t length_;

public:
    BasicStringRef() : data_(nullptr), length_(0) {
        // Do nothing!
    }
    BasicStringRef(const value_type * data)
        : data_(data), length_(detail::strlen(data)) {
        // Do nothing!
    }
    BasicStringRef(const value_type * data, size_t length)
        : data_(data), length_(length) {
        // Do nothing!
    }
    BasicStringRef(const value_type * first, const value_type * last)
        : data_(first), length_((size_t)(last - first)) {
        // Do nothing!
    }
    template <size_t N>
    BasicStringRef(const value_type(&data)[N])
        : data_(data), length_(N - 1) {
        // Do nothing!
    }
    BasicStringRef(const std::basic_string<value_type> & src)
        : data_(src.c_str()), length_(src.size()) {
        // Do nothing!
    }
    BasicStringRef(const BasicStringRef<value_type> & src)
        : data_(src.c_str()), length_(src.size()) {
        // Do nothing!
    }
    ~BasicStringRef() { /* Do nothing! */ }

    BasicStringRef & operator = (const BasicStringRef & rhs) {
        this->data_ = rhs.data_;
        this->length_ = rhs.length_;
        return *this;
    }

    BasicStringRef & operator = (const value_type * data) {
        this->data_ = data;
        this->length_ = detail::strlen(data);
        return *this;
    }

    const value_type * c_str() const { return this->data_; }
    value_type * data() const { return const_cast<value_type *>(this->data_); }

    size_t size() const { return this->length_; }
    size_t length() const { return this->size(); }

    iterator begin() { return iterator(this->data_); }
    iterator end() { return iterator(this->data_ + this->length_); }

    const_iterator cbegin() const { return const_iterator(this->data_); }
    const_iterator cend() const { return const_iterator(this->data_ + this->length_); }

    void reset() {
        this->data_ = nullptr;
        this->length_ = 0;
    }

    void set_data(const value_type * data, size_t length) {
        this->data_ = data;
        this->length_ = length;
    }

    void set_data(const value_type * data) {
        this->set_data(data, detail::strlen(data));
    }

    template <size_t N>
    void set_data(const value_type(&data)[N]) {
        this->set_data(data, N - 1);
    }

    void set_data(const std::basic_string<value_type> & data) {
        this->set_data(data.c_str(), data.size());
    }

    void set_data(const BasicStringRef<value_type> & data) {
        this->set_data(data.c_str(), data.size());
    }
}; // class BasicStringRef<CharT>

typedef BasicStringRef<char>    StringRef;
typedef BasicStringRef<char>    AStringRef;
typedef BasicStringRef<wchar_t> WStringRef;
typedef BasicStringRef<char>    Utf8StringRef;

} // namespace StringMatch

#endif // MAIN_STRING_REF_H
