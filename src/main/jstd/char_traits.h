
#ifndef JSTD_CHAR_TRAITS_H
#define JSTD_CHAR_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

namespace jstd {

// jstd::uchar_traits<T>

template <typename CharTy>
struct uchar_traits {
    typedef CharTy type;
};

template <>
struct uchar_traits<char> {
    typedef unsigned char type;
};

template <>
struct uchar_traits<short> {
    typedef unsigned short type;
};

template <>
struct uchar_traits<int> {
    typedef unsigned int type;
};

template <>
struct uchar_traits<long> {
    typedef unsigned long type;
};

// jstd::schar_traits<T>

template <typename CharTy>
struct schar_traits {
    typedef CharTy type;
};

template <>
struct schar_traits<char> {
    typedef signed char type;
};

template <>
struct schar_traits<short> {
    typedef signed short type;
};

template <>
struct schar_traits<int> {
    typedef signed int type;
};

template <>
struct schar_traits<long> {
    typedef signed long type;
};

template <>
struct schar_traits<unsigned char> {
    typedef signed char type;
};

template <>
struct schar_traits<unsigned short> {
    typedef signed short type;
};

template <>
struct schar_traits<unsigned int> {
    typedef signed int type;
};

template <>
struct schar_traits<unsigned long> {
    typedef signed long type;
};

// jstd::is_char8<T>

template <typename CharTy>
struct is_char8 {
    static const bool value = false;
};

template <>
struct is_char8<bool> {
    static const bool value = true;
};

template <>
struct is_char8<char> {
    static const bool value = true;
};

template <>
struct is_char8<unsigned char> {
    static const bool value = true;
};

// jstd::is_wchar<T>

template <typename CharTy>
struct is_wchar {
    static const bool value = false;
};

template <>
struct is_wchar<short> {
    static const bool value = true;
};

template <>
struct is_wchar<unsigned short> {
    static const bool value = true;
};

template <>
struct is_wchar<wchar_t> {
    static const bool value = true;
};

} // namespace jstd

#endif // JSTD_CHAR_TRAITS_H
