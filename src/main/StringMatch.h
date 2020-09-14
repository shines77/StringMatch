
#ifndef MAIN_STRING_MATCH_H
#define MAIN_STRING_MATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"

#if defined(_MSC_VER) && (_MSC_VER < 1600)
    #ifndef nullptr
    #define nullptr     ((void *)(NULL))
    #endif
#endif

// Define std::nullptr_t under msvc 2008
#if defined(_MSC_VER) && (_MSC_VER < 1600)
#ifdef __cplusplus
namespace std {
    struct __nullptr {
        typedef void * type;
    };
    typedef typename __nullptr::type nullptr_t;
}

using ::std::nullptr_t;
#endif // __cplusplus
#endif // _MSC_VER

//
// macro sm_countof(array)
//
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(_M_ARM64)
    #define sm_unaligned __unaligned
#else
    #define sm_unaligned
#endif

#ifndef sm_countof
#ifdef __cplusplus
    extern "C++" {
        template <typename CountOfType, size_t SizeOfArray>
        char(*sm_countof_helper(sm_unaligned CountOfType(&_Array)[SizeOfArray]))[SizeOfArray];

        #define sm_countof(_Array)      (sizeof(*sm_countof_helper(_Array)) + 0)
    }
#else
    #define sm_countof(_Array)      (sizeof(_Array) / sizeof(_Array[0]))
#endif // __cplusplus
#endif // sm_countof

#ifndef sm_countof_i
#define sm_countof_i(_Array)    (int)(sm_countof(_Array))
#define sm_countof_l(_Array)    (long)(sm_countof(_Array))
#define sm_countof_p(_Array)    (intptr_t)(sm_countof(_Array))
#endif // sm_countof_i

#ifndef sm_min
#define sm_min(a, b)    (((a) <= (b)) ? (a) : (b))
#endif // sm_min

#ifndef sm_max
#define sm_max(a, b)    (((a) >= (b)) ? (a) : (b))
#endif // sm_max

#if defined(_MSC_VER)
#  if _MSC_VER >= 1700
#    define SM_NOEXCEPT     _NOEXCEPT
#  else
#    define SM_NOEXCEPT
#  endif
#else
#  define SM_NOEXCEPT       noexcept
#endif

#ifndef SM_UNUSED_VAR
#define SM_UNUSED_VAR(var)  (void)(var)
#endif

#ifdef Long
#undef Long
#endif

namespace StringMatch {

// Define portable long type
typedef ptrdiff_t Long;

struct Status {
    enum MatchStatus {
        InvalidParameter = -2,
        NotFound = -1,
        Found = 0
    };
};

} // namespace StringMatch

#endif // MAIN_STRING_MATCH_H
