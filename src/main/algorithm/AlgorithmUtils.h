
#ifndef STRING_MATCH_ALGORITHM_UTILS_H
#define STRING_MATCH_ALGORITHM_UTILS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"

#include <memory.h>
#include <string.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <algorithm>

#include "StringMatch.h"
#include "jstd/scoped_ptr.h"

namespace StringMatch {

template <std::size_t Capacity, typename T = std::size_t>
class BitMask {
public:
    typedef std::size_t     size_type;
    typedef T               storge_type;

    static const size_type kSize = Capacity;
    static const size_type kStorgeBits = sizeof(storge_type) * 8;
    static const size_type kStorgeMask = kStorgeBits - 1;
    static const size_type kStorgeMaxShift = sizeof(storge_type) * 8;
    static const size_type kStorgeSize = (Capacity + sizeof(storge_type) - 1) / sizeof(storge_type);
    static const size_type kCapacity = kStorgeSize * sizeof(storge_type);

private:
    jstd::scoped_array<storge_type> bits_;

public:
    BitMask() {
    }

    ~BitMask() {
        this->bits_.reset();
    }

    storge_type * data() const    { return this->bits_.get(); }
    size_type size() const        { return kSize; }
    size_type capacity() const    { return kCapacity; }
    size_type storge_size() const { return kStorgeSize; }

    void init() {
        storge_type * new_bits = new storge_type[kStorgeSize];
        ::memset((void *)new_bits, 0, kStorgeSize * sizeof(storge_type));

        this->bits_.reset(new_bits);
    }

    void clear() {
        ::memset((void *)this->bits_.get(), 0, kStorgeSize * sizeof(storge_type));
    }

    bool get(size_type pos) const {
        size_type storge_pos = pos / kStorgeBits;
        size_type index = pos & kStorgeMask;
        assert(storge_pos < kStorgeSize);
        return ((this->bits_[storge_pos] & (size_type(1) << index)) != 0);
    }

    storge_type getv(size_type pos) const  {
        size_type storge_pos = pos / kStorgeBits;
        size_type index = pos & kStorgeMask;
        assert(storge_pos < kStorgeSize);
        return (this->bits_[storge_pos] & (size_type(1) << index));
    }

    void set(size_type pos) {
        size_type storge_pos = pos / kStorgeBits;
        size_type index = pos & kStorgeMask;
        assert(storge_pos < kStorgeSize);
        this->bits_[storge_pos] |= (size_type(1) << index);
    }
};

template <std::size_t Capacity, typename Key = std::uint16_t, typename Value = std::uint8_t>
class BitMap {
public:
    typedef std::size_t     size_type;
    typedef Key             key_type;
    typedef Value           value_type;

    static const size_type kAlignment = sizeof(std::size_t);
    static const size_type kSize = Capacity;
    static const size_type kCapacity = (kSize + kAlignment - 1) / kAlignment * kAlignment;

private:
    jstd::scoped_array<value_type> data_;

public:
    BitMap() {
    }

    ~BitMap() {
        this->data_.reset();
    }

    value_type * data() const     { return this->data_.get(); }
    size_type size() const        { return kSize; }
    size_type capacity() const    { return kCapacity; }
    size_type storge_size() const { return kCapacity * sizeof(value_type); }

    void init() {
        value_type * new_data = new value_type[kCapacity];
        ::memset((void *)new_data, 0, kCapacity * sizeof(value_type));

        this->data_.reset(new_data);
    }

    void clear() {
        ::memset((void *)this->data_.get(), 0, kCapacity * sizeof(value_type));
    }

    size_type nextKey(size_type key) const {
        return ((key + 1) % kCapacity);
    }

    bool get(size_type pos) const {
        return (this->data_[pos] != 0);
    }

    size_type getv(size_type pos) const  {
        return static_cast<size_type>(this->data_[pos]);
    }

    void set(size_type pos) {
        this->data_[pos] = 1;
    }

    void set(size_type pos, value_type value) {
        this->data_[pos] = value;
    }
};

} // namespace StringMatch

#endif // STRING_MATCH_ALGORITHM_UTILS_H
