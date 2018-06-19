
#ifndef JSTD_VECTOR_H
#define JSTD_VECTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include "basic/stdsize.h"
#include <memory.h>
#include <cstdint>
#include <cstddef>

#include "support/bitscan_reverse.h"
#include "support/bitscan_forward.h"

namespace jstd {

namespace detail {

std::size_t round_to_pow2(std::size_t n)
{
    unsigned long index;
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
 || defined(_M_ARM64)
    unsigned char nonZero = __BitScanReverse64(index, n);
    return (nonZero ? (1ULL << index) : 2ULL);
#else
    unsigned char nonZero = __BitScanReverse(index, n);
    return (nonZero ? (1UL << index) : 2UL);
#endif
}

std::size_t round_up_pow2(std::size_t n)
{
    unsigned long index;
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
 || defined(_M_ARM64)
    unsigned char nonZero = __BitScanReverse64(index, n);
    return (nonZero ? (1ULL << (index + 1)) : 2ULL);
#else
    unsigned char nonZero = __BitScanReverse(index, n);
    return (nonZero ? (1UL << (index + 1)) : 2UL);
#endif
}

} // namespace detail

template <typename T>
class vector {
public:
    typedef T           value_type;
    typedef vector<T>   this_type;
    typedef std::size_t size_type;

private:
    value_type * data_;
    size_type size_;
    size_type capacity_;

public:
    vector() : data_(nullptr), size_(0), capacity_(0) {}
    ~vector() {
        destroy();
    }

    size_type size() const { return this->size_; }
    size_type capacity() const { return this->capacity_; }
    value_type * data() const { return this->data_; }

    void destroy() {
        if (likely(this->data_ != nullptr)) {
            delete[] this->data_;
            this->data_ = nullptr;
        }
    }

    void reserve_fast(size_type new_capacity) {
        if (unlikely(new_capacity > this->capacity_)) {
            value_type * new_data = new value_type[new_capacity];
            if (likely(new_data != nullptr)) {
                if (likely(this->data_ != nullptr)) {
                    memcpy((void *)new_data, (const void *)this->data_, this->size_ * sizeof(value_type));
                    delete[] this->data_;
                }
                this->data_ = new_data;
                this->capacity_ = new_capacity;
            }
        }
    }

    void reserve(size_type new_size) {
        size_type new_capacity = detail::round_up_pow2(new_size);
        this->reserve_fast(new_capacity);
    }

    void emplace_back(const value_type & value) {
        if (unlikely(this->size_ >= this->capacity_)) {
            this->reserve_fast(this->capacity_ * 2);
        }
        assert(this->data_ != nullptr);
        this->data_[this->size_] = value;
        ++this->size_;
    }

    value_type & operator [] (size_type index) {
        assert(index < this->size_);
        return this->data_[index];
    }
};

} // namespace jstd

#endif // JSTD_VECTOR_H
