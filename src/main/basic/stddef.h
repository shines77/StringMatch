
#ifndef JIMI_BASIC_STDDEF_H
#define JIMI_BASIC_STDDEF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stddef.h>

////////////////////////////////////////////////////////////////////////////////

//
// Clang Language Extensions
//
// See: http://clang.llvm.org/docs/LanguageExtensions.html#checking_language_features
//

////////////////////////////////////////////////////////////////////////////////

#ifndef __has_builtin                   // Optional of course.
  #define __has_builtin(x)  0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_feature                   // Optional of course.
  #define __has_feature(x)  0           // Compatibility with non-clang compilers.
#endif
#ifndef __has_extension
  #define __has_extension   __has_feature   // Compatibility with pre-3.0 compilers.
#endif

#ifndef __has_cpp_attribute             // Optional of course.
  #define __has_cpp_attribute(x)    0   // Compatibility with non-clang compilers.
#endif

#ifndef __has_c_attribute               // Optional of course.
  #define __has_c_attribute(x)      0   // Compatibility with non-clang compilers.
#endif

#ifndef __has_attribute                 // Optional of course.
  #define __has_attribute(x)        0   // Compatibility with non-clang compilers.
#endif

#ifndef __has_declspec_attribute        // Optional of course.
  #define __has_declspec_attribute(x) 0 // Compatibility with non-clang compilers.
#endif

#ifndef __is_identifier                 // Optional of course.
  // It evaluates to 1 if the argument x is just a regular identifier and not a reserved keyword.
  #define __is_identifier(x)    1       // Compatibility with non-clang compilers.
#endif

#if defined(_MSC_VER)
#ifndef __attribute__
  #define __attribute__(x)
#endif
#endif

////////////////////////////////////////////////////////////////////////////////

#if __has_feature(cxx_rvalue_references)
    // This code will only be compiled with the -std=c++11 and -std=gnu++11
    // options, because rvalue references are only standardized in C++11.
#endif

#if __has_extension(cxx_rvalue_references)
    // This code will be compiled with the -std=c++11, -std=gnu++11, -std=c++98
    // and -std=gnu++98 options, because rvalue references are supported as a
    // language extension in C++98.
#endif

#if __has_attribute(always_inline)
    #define FORCE_INLINE    __attribute__((always_inline))
#else
    #define FORCE_INLINE
#endif

#if __has_declspec_attribute(dllexport)
    #define DLL_EXPORT  __declspec(dllexport)
#else
    #define DLL_EXPORT
#endif

#if __has_declspec_attribute(dllimport)
    #define DLL_IMPORT  __declspec(dllimport)
#else
    #define DLL_IMPORT
#endif

#if __is_identifier(__wchar_t)
    // __wchar_t is not a reserved keyword
  #if !defined(_MSC_VER)
    typedef wchar_t __wchar_t;
  #endif // !_MSC_VER
#endif

////////////////////////////////////////////////////////////////////////////////

//
// C++ compiler macro define
// See: http://www.cnblogs.com/zyl910/archive/2012/08/02/printmacro.html
//
// LLVM Branch Weight Metadata
// See: http://llvm.org/docs/BranchWeightMetadata.html
//

////////////////////////////////////////////////////////////////////////////////

#if (defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 96) || (__GNUC__ >= 3))) \
 || (defined(__clang__) && ((__clang_major__ == 2 && __clang_minor__ >= 1) || (__clang_major__ >= 3)))
// Since gcc v2.96 or clang v2.1
#ifndef likely
#define likely(expr)        __builtin_expect(!!(expr), 1)
#endif
#ifndef unlikely
#define unlikely(expr)      __builtin_expect(!!(expr), 0)
#endif
#else // !likely() & unlikely()
#ifndef likely
#define likely(expr)        (expr)
#endif
#ifndef unlikely
#define unlikely(expr)      (expr)
#endif
#endif // likely() & unlikely()

#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)
#ifndef ALIGNED_PREFIX
#define ALIGNED_PREFIX(n)       _declspec(align(n))
#endif
#ifndef ALIGNED_SUFFIX
#define ALIGNED_SUFFIX(n)
#endif
#else // !ALIGNED(n)
#ifndef ALIGNED_PREFIX
#define ALIGNED_PREFIX(n)
#endif
#ifndef ALIGNED_SUFFIX
#define ALIGNED_SUFFIX(n)       __attribute__((aligned(n)))
#endif
#endif // ALIGNED(n)

// Declare for inline, forceinline, noinline

#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)

#define SM_DECLARE(type)                type
#define SM_DECLARE_DATA(type)           type

#ifdef __cplusplus
#define SM_INLINE_DECLARE(type)         inline type
#else
#define SM_INLINE_DECLARE(type)         __inline type
#endif // __cplusplus

#define SM_FORCEINLINE_DECLARE(type)    __forceinline type
#define SM_NOINLINE_DECLARE(type)       __declspec(noinline) type

#define SM_RESTRICT(type)               __restrict type

#elif defined(__GNUC__) || defined(__clang__) || defined(__linux__)

#define SM_DECLARE(type)                type
#define SM_DECLARE_DATA(type)           type

#ifdef __cplusplus
#define SM_INLINE_DECLARE(type)         inline type
#else
#define SM_INLINE_DECLARE(type)         __inline__ type
#endif // __cplusplus

#define SM_FORCEINLINE_DECLARE(type)    type __attribute__((__always_inline__))
#define SM_NOINLINE_DECLARE(type)       type __attribute__ ((noinline))

#define SM_RESTRICT(type)               __restrict type

#else

#define SM_DECLARE(type)                type
#define SM_DECLARE_DATA(type)           type

#define SM_INLINE_DECLARE(type)         inline type

#define SM_FORCEINLINE_DECLARE(type)    inline type
#define SM_NOINLINE_DECLARE(type)       type

#define SM_RESTRICT(type)               __restrict type

#endif // SM_INLINE


#ifndef __SM_CDECL
#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)
#define __SM_CDECL      __cdecl
#else
#define __SM_CDECL      __attribute__((__cdecl__))
#endif
#endif // __SM_CDECL

////////////////////////////////////////////////////////////////////////////////

#endif // !JIMI_BASIC_STDDEF_H
