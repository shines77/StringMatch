
#ifndef STRING_MATCH_WORDHASH_H
#define STRING_MATCH_WORDHASH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

#include <memory>
#include <algorithm>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"
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
    explicit BitMask() {
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

template <std::size_t Capacity, typename T = std::uint8_t>
class BitMap {
public:
    typedef std::size_t     size_type;
    typedef T               storge_type;

    static const size_type kAlignment = sizeof(std::size_t);
    static const size_type kSize = Capacity;
    static const size_type kCapacity = (kSize + kAlignment - 1) / kAlignment * kAlignment;

private:
    jstd::scoped_array<storge_type> bytes_;

public:
    explicit BitMap() {
    }

    ~BitMap() {
        this->bytes_.reset();
    }

    storge_type * data() const    { return this->bytes_.get(); }
    size_type size() const        { return kSize; }
    size_type capacity() const    { return kCapacity; }
    size_type storge_size() const { return kCapacity * sizeof(storge_type); }

    void init() {
        storge_type * new_bytes = new storge_type[kCapacity];
        ::memset((void *)new_bytes, 0, kCapacity * sizeof(storge_type));

        this->bytes_.reset(new_bytes);
    }

    void clear() {
        ::memset((void *)this->bytes_.get(), 0, kCapacity * sizeof(storge_type));
    }

    bool get(size_type pos) const {
        return (this->bytes_[pos] != 0);
    }

    storge_type getv(size_type pos) const  {
        return this->bytes_[pos];
    }

    void set(size_type pos) {
        this->bytes_[pos] = 1;
    }

    void set(size_type pos, storge_type value) {
        this->bytes_[pos] = value;
    }
};

//
// See: https://blog.csdn.net/liangzhao_jay/article/details/8792486
//

template <typename CharTy>
class WordHashImpl {
public:
    typedef WordHashImpl<CharTy>            this_type;
    typedef CharTy                          char_type;
    typedef uint16_t                        word_t;
    typedef std::size_t                     size_type;
    typedef std::ptrdiff_t                  ssize_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                            uchar_type;

    static const size_type kHashMax = 65536;
    static const size_type kWordSize = sizeof(word_t);

private:
    BitMap<kHashMax> bitmap_;

public:
    WordHashImpl() {}
    ~WordHashImpl() {
        this->destroy();
    }

    static const char * name() { return "WordHash"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return (this->bitmap_.data() != nullptr); }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);

        this->bitmap_.init();

        ssize_type max_limit = ssize_type(length - kWordSize + 1);
        for (ssize_type i = 0; i < max_limit; i++) {
            word_t word = *(word_t *)&pattern[i];
            this->bitmap_.set(word, i + 1);
        }
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
#if 1
        // check arg sizes 
        if ((pattern_len < 2 * kWordSize - 1)
            || (pattern_len >= (std::numeric_limits<uint8_t>::max)())
            || (text_len < pattern_len * 2)) {
            // fallback to std::search
            StringRef sText(text, text_len);
            StringRef sPattern(pattern, pattern_len);
            StringRef::iterator iter = std::search(sText.begin(), sText.end(),
                                                   sPattern.begin(), sPattern.end());
            if (likely(iter != sText.end()))
                return (Long)(iter - sText.begin());
            else
                return Status::NotFound;
        }
#endif

        ssize_type half_p_len = ssize_type(pattern_len / 2);
        ssize_type max_limit = ssize_type(text_len - pattern_len + 1);
        for (ssize_type i = 0; i < max_limit; i++) {
            ssize_type matched = 0;
            ssize_type tpos = i + pattern_len - kWordSize;
            ssize_type ppos = pattern_len - kWordSize;
            for (; ppos >= 0; --tpos, --ppos) {
                assert(tpos >= 0);
                assert(tpos < ssize_type(text_len - kWordSize + 1));
                assert(ppos < ssize_type(pattern_len - kWordSize + 1));
                word_t word = *(word_t *)&text[tpos];
                if (this->bitmap_.getv(word) == 0) {
                    if (matched == 0) {
                        i += pattern_len - 1;
                    }
                    matched = -1;
                    break;
                }
                matched++;
                //ppos--;
                if (matched < ssize_type(pattern_len - kWordSize + 1)) {
                    /*
                    if (text[tpos] != pattern[ppos]) {
                        matched = -1;
                        //if (ppos > half_p_len)
                        //    break;
                    }
                    //*/
                }
                else {
                    assert(tpos < max_limit);
                    if (::memcmp((const void *)&text[tpos],
                                 (const void *)&pattern[0], pattern_len) == 0) {
                        return Long(tpos);
                    }
                    matched = -1;
                    break;
                }
            }
            if (matched >= 0) {
                return Long(i);
            }
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< WordHashImpl<char> >    WordHash;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< WordHashImpl<wchar_t> > WordHash;
}

} // namespace StringMatch

#endif // STRING_MATCH_WORDHASH_H
