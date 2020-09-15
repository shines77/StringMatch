
#ifndef STRING_MATCH_SHIFTAND_H
#define STRING_MATCH_SHIFTAND_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

namespace StringMatch {

template <typename CharTy, typename MaskTy = uint64_t>
class ShiftAndImpl {
public:
    typedef ShiftAndImpl<CharTy, MaskTy>        this_type;
    typedef CharTy                              char_type;
    typedef MaskTy                              mask_type;
    typedef std::size_t                         size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                                uchar_type;

    static const size_type kMaxAscii = 256;

private:
    mask_type mask_;
    mask_type bitmap_[kMaxAscii];

public:
    ShiftAndImpl() : mask_(0) {}
    ~ShiftAndImpl() {
        this->destroy();
    }

    static const char * name() { return "ShiftAnd"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);

#if 1
        ::memset((void *)&this->bitmap_[0], 0, kMaxAscii * sizeof(mask_type));
#else
        for (size_type i = 0; i < kMaxAscii; ++i)
            this->bitmap_[i] = 0;
#endif

        mask_type mask = 1;
        for (size_type i = 0; i < length; ++i) {
            this->bitmap_[(uchar_type)pattern[i]] |= mask;
            mask <<= 1;
        }
        this->mask_ = mask >> 1;
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        mask_type mask = this->mask_;
        const mask_type * bitmap = &this->bitmap_[0];

        assert(bitmap != nullptr);

        if (likely(pattern_len <= text_len)) {
            register mask_type state = 0;
            for (size_type i = 0; i < text_len; ++i) {
                state = ((state << 1) | 1) & bitmap[(uchar_type)text[i]];
                if (unlikely((state & mask) != 0))
                    return (Long)(i + 1 - pattern_len);
            }
        }

        return Status::NotFound;
    }
};

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(_M_ARM64) || defined(__amd64__) || defined(__x86_64__)
namespace AnsiString {
    typedef AlgorithmWrapper< ShiftAndImpl<char, uint64_t> >    ShiftAnd;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftAndImpl<wchar_t, uint64_t> > ShiftAnd;
}
#else
namespace AnsiString {
    typedef AlgorithmWrapper< ShiftAndImpl<char, uint32_t> >    ShiftAnd;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftAndImpl<wchar_t, uint32_t> > ShiftAnd;
}
#endif // _WIN64 || __amd64__

} // namespace StringMatch

#endif // STRING_MATCH_SHIFTAND_H
