
#ifndef JSTD_ITERATOR_H
#define JSTD_ITERATOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <cstddef>
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

// function template const_cast

// remove const-ness from a fancy pointer
template <typename PtrTy>
inline
auto __const_cast(PtrTy ptr) {
    using Element = typename std::pointer_traits<PtrTy>::element_type;
    using Modifiable = std::remove_const_t<Element>;
    using Dest = typename std::pointer_traits<PtrTy>::template std::rebind<Modifiable>;

    return (std::pointer_traits<Dest>::pointer_to(const_cast<Modifiable &>(*ptr)));
}

// remove const-ness from a plain pointer
template <typename Ty>
inline
auto __const_cast(Ty * ptr) {
    return (const_cast<std::remove_const_t<Ty> *>(ptr));
}

} // namespace detail

} // namespace jstd

#endif // JSTD_ITERATOR_H
