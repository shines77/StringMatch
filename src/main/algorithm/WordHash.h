
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
    storge_type Bits_[kStorgeSize];

public:
    explicit BitMask() {
    }

    size_type size() const        { return kSize; }
    size_type capacity() const    { return kCapacity; }
    size_type storge_size() const { return kStorgeSize; }

    void clear() {
        ::memset((void *)this->Bits_, 0, kStorgeSize * sizeof(storge_type));
    }

    bool get(size_type pos) const {
        size_type storge_pos = pos / kStorgeBits;
        size_type index = pos & kStorgeMask;
        assert(storge_pos < kStorgeSize);
        return ((this->Bits_[storge_pos] & (size_type(1) << index)) != 0);
    }

    storge_type getv(size_type pos) const  {
        size_type storge_pos = pos / kStorgeBits;
        size_type index = pos & kStorgeMask;
        assert(storge_pos < kStorgeSize);
        return (this->Bits_[storge_pos] & (size_type(1) << index));
    }

    void set(size_type pos) {
        size_type storge_pos = pos / kStorgeBits;
        size_type index = pos & kStorgeMask;
        assert(storge_pos < kStorgeSize);
        this->Bits_[storge_pos] |= (size_type(1) << index);
    }
};

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
    BitMask<kHashMax, size_type> bitmask_;

public:
    WordHashImpl() {}
    ~WordHashImpl() {
        this->destroy();
    }

    static const char * name() { return "WordHash"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);

        //uint8_t * new_mask = new uint8_t[kHashMax];
        //::memset((void *)new_mask, 0, kHashMax * sizeof(uint8_t));

        //this->bitmask_.reset(new_mask);
        this->bitmask_.clear();

        ssize_type max_limit = ssize_type(length - kWordSize + 1);
        for (ssize_type i = 0; i < max_limit; i++) {
            word_t word = *(word_t *)&pattern[i];
            this->bitmask_.set(word);
        }
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
#if 0
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
        for (ssize_type i = 0; i <= max_limit; i++) {
            int matched = 0;
            ssize_type tpos = i + pattern_len - kWordSize;
            ssize_type ppos = pattern_len - kWordSize;
            for (; ppos >= 0; --tpos, --ppos) {
                word_t word = *(word_t *)&text[tpos];
                if (this->bitmask_.getv(word) == 0) {
                    i = tpos + 1;
                    matched = -1;
                    break;
                }
                matched++;
                if (matched < (pattern_len - kWordSize + 1)) {
                    if (text[tpos] != pattern[ppos]) {
                        matched = -1;
                        //if (ppos > half_p_len)
                        //    break;
                    }
                }
                else {
                    if (tpos <= max_limit) {
                        if (::memcmp((const void *)&text[tpos], (const void *)&pattern[0], pattern_len) == 0) {
                            return Long(tpos);
                        }
                    }
                    else {
                        return Status::NotFound;
                    }
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
