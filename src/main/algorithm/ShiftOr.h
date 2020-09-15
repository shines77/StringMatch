
#ifndef STRING_MATCH_SHIFTOR_H
#define STRING_MATCH_SHIFTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

namespace StringMatch {

template <typename CharTy, typename MaskTy = uint64_t>
class ShiftOrImpl {
public:
    typedef ShiftOrImpl<CharTy, MaskTy>     this_type;
    typedef CharTy                          char_type;
    typedef MaskTy                          mask_type;
    typedef std::size_t                     size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                            uchar_type;

    static const size_type kMaxAscii = 256;

private:
    mask_type limit_;
    mask_type bitmap_[kMaxAscii];

public:
    ShiftOrImpl() : limit_(0) {}
    ~ShiftOrImpl() {
        this->destroy();
    }

    static const char * name() { return "ShiftOr"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);

#if 1
        ::memset((void *)&this->bitmap_[0], 0xFF, kMaxAscii * sizeof(mask_type));
#else
        for (size_type i = 0; i < kMaxAscii; ++i) {
            this->bitmap_[i] = ~0;
        }
#endif

        mask_type mask = 1;
        mask_type limit = 0;
        for (size_type i = 0; i < length; mask <<= 1, ++i) {
            this->bitmap_[(uchar_type)pattern[i]] &= ~mask;
            limit |= mask;
        }
        limit = ~(limit >> 1);

        this->limit_ = limit;
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        mask_type limit = this->limit_;
        const mask_type * bitmap = &this->bitmap_[0];

        assert(bitmap != nullptr);

        if (likely(pattern_len <= text_len)) {
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(_M_ARM64) || defined(__amd64__) || defined(__x86_64__)
  #if 1
            register mask_type state = ~0;
            for (size_type i = 0; i < text_len; ++i) {
                state = (state << 1) | bitmap[(uchar_type)text[i]];
                if (unlikely(state < limit))
                    return (Long)(i + 1 - pattern_len);
            }
  #elif 1
            register mask_type state = ~0;
            size_type i;
            for (i = 0; i < pattern_len; ++i) {
                state = (state << 1) | bitmap[(uchar_type)text[i]];
            }
            if (unlikely(state < limit))
                return (int)(i + 1 - pattern_len);
            for (; (i + pattern_len - 1) < text_len; ++i) {
                state = (state << 1) | bitmap[(uchar_type)text[i]];
                if (unlikely(state < limit))
                    return (int)(i + 1 - pattern_len);
            }
            if (unlikely(state != ~0)) {
                for (; i < text_len; ++i) {
                    state = (state << 1) | bitmap[(uchar_type)text[i]];
                    if (unlikely(state < limit))
                        return (Long)(i + 1 - pattern_len);
                }
            }
  #else
            register mask_type state1 = ~0;
            register mask_type state2 = ~0;
            size_type half_len = (text_len / 2);
            size_type i;
            for (i = 0; i < half_len; ++i) {
                state1 = (state1 << 1) | bitmap[(uchar_type)text[i]];
                state2 = (state2 << 1) | bitmap[(uchar_type)text[half_len + i]];
                if (unlikely(state1 < limit))
                    return (Long)(i + 1 - pattern_len);
                if (unlikely(state2 < limit))
                    return (Long)(i + half_len + 1 - pattern_len);
            }
            size_type j;
            for (j = 0; j < pattern_len - 1; ++j) {
                state1 = (state1 << 1) | bitmap[(uchar_type)text[i + j]];
                if (unlikely(state1 < limit))
                    return (Long)(i + j + 1 - pattern_len);
            }
            if ((text_len & 2) != 0) {
                state2 = (state2 << 1) | bitmap[(uchar_type)text[half_len + i]];
                if (unlikely(state2 < limit))
                    return (Long)(i + half_len + 1 - pattern_len);
            }
  #endif
#else
            register mask_type state = ~0;
            for (size_type i = 0; i < text_len; ++i) {
                state = (state << 1) | bitmap[(uchar_type)text[i]];
                if (unlikely(state < limit))
                    return (Long)(i + 1 - pattern_len);
            }
#endif // _WIN64 || __amd64__
        }

        return Status::NotFound;
    }
};

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(_M_ARM64) || defined(__amd64__) || defined(__x86_64__)
namespace AnsiString {
    typedef AlgorithmWrapper< ShiftOrImpl<char, uint64_t> >    ShiftOr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftOrImpl<wchar_t, uint64_t> > ShiftOr;
}
#else
namespace AnsiString {
    typedef AlgorithmWrapper< ShiftOrImpl<char, uint32_t> >    ShiftOr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< ShiftOrImpl<wchar_t, uint32_t> > ShiftOr;
}
#endif // _WIN64 || __amd64__

} // namespace StringMatch

#endif // STRING_MATCH_SHIFTOR_H
