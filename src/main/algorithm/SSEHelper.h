
#ifndef STRING_MATCH_SSE_HELPER_H
#define STRING_MATCH_SSE_HELPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <nmmintrin.h>  // For SSE 4.2

namespace StringMatch {

template <typename CharTy>
struct SSEHelper {
    static const int _SIDD_CHAR_OPS = _SIDD_UBYTE_OPS;
    static const int kMaxSize = 16;
    static const int kWordSize = 1;
};

template <>
struct SSEHelper<char> {
    static const int _SIDD_CHAR_OPS = _SIDD_UBYTE_OPS;
    static const int kMaxSize = 16;
    static const int kWordSize = 1;
};

template <>
struct SSEHelper<signed char> {
    static const int _SIDD_CHAR_OPS = _SIDD_UBYTE_OPS;
    static const int kMaxSize = 16;
    static const int kWordSize = 1;
};

template <>
struct SSEHelper<unsigned char> {
    static const int _SIDD_CHAR_OPS = _SIDD_UBYTE_OPS;
    static const int kMaxSize = 16;
    static const int kWordSize = 1;
};

template <>
struct SSEHelper<short> {
    static const int _SIDD_CHAR_OPS = _SIDD_UWORD_OPS;
    static const int kMaxSize = 8;
    static const int kWordSize = 2;
};

template <>
struct SSEHelper<unsigned short> {
    static const int _SIDD_CHAR_OPS = _SIDD_UWORD_OPS;
    static const int kMaxSize = 8;
    static const int kWordSize = 2;
};

template <>
struct SSEHelper<wchar_t> {
    static const int _SIDD_CHAR_OPS = _SIDD_UWORD_OPS;
    static const int kMaxSize = 8;
    static const int kWordSize = 2;
};

} // namespace StringMatch

#endif // STRING_MATCH_SSE_HELPER_H
