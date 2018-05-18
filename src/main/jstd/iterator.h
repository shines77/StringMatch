
#ifndef JSTD_ITERATOR_H
#define JSTD_ITERATOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace jstd {

// iterator stuff (from <iterator>)
// iterator tags (from <iterator>)

struct input_iterator_tag {
    // identifying tag for input iterators
};

// transition, remove for dev15
struct mutable_iterator_tag {
    // identifying tag for mutable iterators
};

struct output_iterator_tag : mutable_iterator_tag {
    // identifying tag for output iterators
};

struct forward_iterator_tag : input_iterator_tag, mutable_iterator_tag {
    // identifying tag for forward iterators
};

struct bidirectional_iterator_tag : forward_iterator_tag {
    // identifying tag for bidirectional iterators
};

struct random_access_iterator_tag : bidirectional_iterator_tag {
    // identifying tag for random-access iterators
};

// template class iterator
template <typename Category,
          typename T,
          typename Diff = std::ptrdiff_t,
          typename Pointer = T *,
          typename Reference = T &>
struct iterator {
    // base type for iterator classes
    typedef Category iterator_category;
    typedef T value_type;
    typedef Diff difference_type;

    typedef Pointer pointer;
    typedef Reference reference;
};

namespace detail {

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)

// function template const_cast

// remove const-ness from a fancy pointer
template <typename Ptr>
inline
auto __const_cast(Ptr ptr)
#if !defined(_MSC_VER) || (_MSC_VER < 1900)
                           -> typename std::pointer_traits<
                                typename std::pointer_traits<Ptr>::template rebind<
                                  typename std::remove_const<
                                    typename std::pointer_traits<Ptr>::element_type
                                  >::type
                                >
                              >::pointer
#endif // !_MSC_VER
{
    using Element = typename std::pointer_traits<Ptr>::element_type;
    using Modifiable = typename std::remove_const<Element>::type;
    using Dest = typename std::pointer_traits<Ptr>::template rebind<Modifiable>;

    return (std::pointer_traits<Dest>::pointer_to(const_cast<Modifiable &>(*ptr)));
}

// remove const-ness from a plain pointer
template <typename T>
inline
auto __const_cast(T * ptr) -> const typename std::remove_const<T>::type * {
    return (const_cast<typename std::remove_const<T>::type *>(ptr));
}

#endif // (_MSC_VER >= 1800)

} // namespace detail

} // namespace jstd

#endif // JSTD_ITERATOR_H
