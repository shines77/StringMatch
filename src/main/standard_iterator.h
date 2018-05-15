
#ifndef JSTD_STANDARD_ITERATOR_H
#define JSTD_STANDARD_ITERATOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <memory>
#include <type_traits>

#include "StringMatch.h"

namespace jstd {

template <typename T>
class standard_iterator_base {
public:
    typedef typename T::node_type node_type;
    typedef typename T::node_ptr node_ptr;
    typedef typename T::value_type value_type;
    typedef standard_iterator_base this_type;

    static value_type default_value_;

protected:
    node_ptr node_;

public:
    standard_iterator_base(node_ptr node = nullptr) : node_(node) {}
    explicit standard_iterator_base(void * p) : node_(static_cast<node_ptr>(p)) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    standard_iterator_base(std::nullptr_t) : node_(nullptr) {}
#endif
    explicit standard_iterator_base(standard_iterator_base const & src)
        : node_(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit standard_iterator_base(standard_iterator_base && src)
        : node_(std::forward<standard_iterator_base>(src).node_) {}
#endif
    ~standard_iterator_base() {}

    standard_iterator_base & operator = (standard_iterator_base const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    standard_iterator_base & operator = (standard_iterator_base && rhs) {
        this->node_ = std::forward<standard_iterator_base>(rhs).node_;
        return *this;
    }
#endif

    operator node_ptr () { return this->node_; }
    operator const node_ptr () const { return this->node_; }

    operator value_type () {
        if (this->node_ != nullptr)
            return this->node_->value;
        else
            return value_type();
    }

    operator const value_type () const {
        if (this->node_ != nullptr)
            return this->node_->value;
        else
            return value_type();
    }

    bool is_valid() const { return (this->node_ != nullptr); }

    node_ptr node() {
        return this->node_;
    }

    const node_ptr node() const {
        return this->node_;
    }

    value_type & value() {
        if (this->node_ != nullptr)
            return this->node_->value;
        else
            return this_type::default_value_;
    }

    const value_type & value() const {
        if (this->node_ != nullptr)
            return this->node_->value;
        else
            return this_type::default_value_;
    }
};

template <typename T>
class standard_iterator;

template <typename T>
class const_standard_iterator : public standard_iterator_base<T> {
public:
    typedef typename T::node_type node_type;
    typedef typename T::node_ptr node_ptr;
    typedef typename T::value_type value_type;
    typedef const_standard_iterator<T> this_type;

    friend class standard_iterator<T>;

public:
    const_standard_iterator(node_ptr node = nullptr)
        : standard_iterator_base<T>(node) {}
    explicit const_standard_iterator(void * p)
        : standard_iterator_base<T>(p) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    const_standard_iterator(std::nullptr_t node)
        : standard_iterator_base<T>(node) {}
#endif
    explicit const_standard_iterator(const_standard_iterator const & src)
        : standard_iterator_base<T>(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit const_standard_iterator(const_standard_iterator && src)
        : standard_iterator_base<T>(std::forward<const_standard_iterator>(src).node_) {}
#endif
    ~const_standard_iterator() {}

    const_standard_iterator & operator = (const_standard_iterator const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    const_standard_iterator & operator = (const_standard_iterator && rhs) {
        this->node_ = std::forward<const_standard_iterator>(rhs).node_;
        return *this;
    }
#endif

    bool is_equal(node_ptr rhs) const {
        return (this->node_ == rhs);
    }

    bool is_equal(void * rhs) const {
        return (this->node_ == static_cast<node_ptr>(rhs));
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    bool is_equal(std::nullptr_t rhs) const {
        return (this->node_ == nullptr);
    }
#endif

    bool is_equal(const_standard_iterator const & rhs) const {
        return (this->node_ == rhs.node_);
    }

    bool operator == (const_standard_iterator const & rhs) const {
        return this->is_equal(rhs);
    }

    bool operator != (const_standard_iterator const & rhs) const {
        return !(this->is_equal(rhs));
    }

    bool operator == (node_ptr rhs) const {
        return this->is_equal(rhs);
    }

    bool operator == (void * rhs) const {
        return this->is_equal(rhs);
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    bool operator == (std::nullptr_t rhs) const {
        return this->is_equal(rhs);
    }
#endif

    bool operator != (node_ptr rhs) const {
        return !(this->is_equal(rhs));
    }

    bool operator != (void * rhs) const {
        return !(this->is_equal(rhs));
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    bool operator != (std::nullptr_t rhs) const {
        return !(this->is_equal(rhs));
    }
#endif

    const value_type & operator *  () const {
        assert(this->node_ != nullptr);
        return *const_cast<const value_type *>(&this->node_->value);
    }
    const value_type * operator -> () const {
        assert(this->node_ != nullptr);
        return const_cast<const value_type *>(&this->node_->value);
    }

    operator bool () const {
        return this->is_valid();
    }

    operator standard_iterator<T> * () {
        return reinterpret_cast<standard_iterator<T> *>(const_cast<const_standard_iterator *>(this));
    }
    operator const standard_iterator<T> * () const {
        return const_cast<const standard_iterator<T> *>(
            reinterpret_cast<standard_iterator<T> *>(const_cast<const_standard_iterator *>(this)));
    }

    operator standard_iterator<T> () {
        return *reinterpret_cast<standard_iterator<T> *>(const_cast<const_standard_iterator *>(this));
    }
    operator const standard_iterator<T> () const {
        return *const_cast<const standard_iterator<T> *>(
            reinterpret_cast<standard_iterator<T> *>(const_cast<const_standard_iterator *>(this)));
    }

    operator standard_iterator<T> & () {
        return *reinterpret_cast<standard_iterator<T> *>(const_cast<const_standard_iterator *>(this));
    }
    operator const standard_iterator<T> & () const {
        return *const_cast<const standard_iterator<T> *>(
            reinterpret_cast<standard_iterator<T> *>(const_cast<const_standard_iterator *>(this)));
    }

    const_standard_iterator & operator ++ () {
        assert(this->node_ != nullptr);
        this->node_ = this->node_->next;
        return *this;
    }

    const_standard_iterator operator ++ (int) {
        const_standard_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    const_standard_iterator next() {
        assert(this->node_ != nullptr);
        return const_standard_iterator(this->node_->next);
    }

    const const_standard_iterator next() const {
        assert(this->node_ != nullptr);
        return const_standard_iterator(this->node_->next);
    }
};

template <typename T>
class standard_iterator : public const_standard_iterator<T> {
public:
    typedef typename T::node_type node_type;
    typedef typename T::node_ptr node_ptr;
    typedef typename T::value_type value_type;
    typedef standard_iterator<T> this_type;

public:
    standard_iterator(node_ptr node = nullptr) : const_standard_iterator<T>(node) {}
    explicit standard_iterator(void * p) : const_standard_iterator<T>(p) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    standard_iterator(std::nullptr_t node) : const_standard_iterator<T>(node) {}
#endif
    explicit standard_iterator(standard_iterator const & src)
        : const_standard_iterator<T>(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit standard_iterator(standard_iterator && src)
        : const_standard_iterator<T>(std::forward<standard_iterator>(src).node_) {}
#endif
    explicit standard_iterator(const_standard_iterator<T> const & src)
        : const_standard_iterator<T>(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit standard_iterator(const_standard_iterator<T> && src)
        : const_standard_iterator<T>(std::forward<const_standard_iterator<T>>(src).node_) {}
#endif
    ~standard_iterator() {}

    standard_iterator & operator = (standard_iterator const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    standard_iterator & operator = (standard_iterator && rhs) {
        this->node_ = std::forward<standard_iterator>(rhs).node_;
        return *this;
    }
#endif

    standard_iterator & operator = (const_standard_iterator<T> const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    standard_iterator & operator = (const_standard_iterator<T> && rhs) {
        this->node_ = std::forward<const_standard_iterator<T>>(rhs).node_;
        return *this;
    }
#endif

    bool is_equal(node_ptr rhs) const {
        return (this->node_ == rhs);
    }

    bool is_equal(void * rhs) const {
        return (this->node_ == static_cast<node_ptr>(rhs));
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    bool is_equal(std::nullptr_t rhs) const {
        return (this->node_ == nullptr);
    }
#endif

    bool is_equal(standard_iterator const & rhs) const {
        return (this->node_ == rhs.node_);
    }

    bool is_equal(const_standard_iterator<T> const & rhs) const {
        return (this->node_ == rhs.node_);
    }

    bool operator == (standard_iterator const & rhs) const {
        return this->is_equal(rhs);
    }

    bool operator != (standard_iterator const & rhs) const {
        return !(this->is_equal(rhs));
    }

    bool operator == (const_standard_iterator<T> const & rhs) const {
        return this->is_equal(rhs);
    }

    bool operator != (const_standard_iterator<T> const & rhs) const {
        return !(this->is_equal(rhs));
    }

    bool operator == (node_ptr rhs) const {
        return this->is_equal(rhs);
    }

    bool operator == (void * rhs) const {
        return this->is_equal(rhs);
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    bool operator == (std::nullptr_t rhs) const {
        return this->is_equal(rhs);
    }
#endif

    bool operator != (node_ptr rhs) const {
        return !(this->is_equal(rhs));
    }

    bool operator != (void * rhs) const {
        return !(this->is_equal(rhs));
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    bool operator != (std::nullptr_t rhs) const {
        return !(this->is_equal(rhs));
    }
#endif

    value_type & operator *  () const {
        return *const_cast<value_type *>(&this->node_->value);
    }
    value_type * operator -> () const {
        return const_cast<value_type *>(&this->node_->value);
    }

    operator bool() const {
        return this->is_valid();
    }

    operator const_standard_iterator<T> * () {
        return reinterpret_cast<const_standard_iterator<T> *>(const_cast<standard_iterator *>(this));
    }
    operator const const_standard_iterator<T> * () const {
        return const_cast<const const_standard_iterator<T> *>(
            reinterpret_cast<const_standard_iterator<T> *>(const_cast<standard_iterator *>(this)));
    }

    operator const_standard_iterator<T> () {
        return *reinterpret_cast<const_standard_iterator<T> *>(const_cast<standard_iterator *>(this));
    }
    operator const const_standard_iterator<T> () const {
        return *const_cast<const const_standard_iterator<T> *>(
            reinterpret_cast<const_standard_iterator<T> *>(const_cast<standard_iterator *>(this)));
    }

    operator const_standard_iterator<T> & () {
        return *reinterpret_cast<const_standard_iterator<T> *>(const_cast<standard_iterator *>(this));
    }
    operator const const_standard_iterator<T> & () const {
        return *const_cast<const const_standard_iterator<T> *>(
            reinterpret_cast<const_standard_iterator<T> *>(const_cast<standard_iterator *>(this)));
    }

    standard_iterator & operator ++ () {
        assert(this->node_ != nullptr);
        this->node_ = this->node_->next;
        return *this;
    }

    standard_iterator operator ++ (int) {
        standard_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    standard_iterator next() {
        assert(this->node_ != nullptr);
        return standard_iterator(this->node_->next);
    }

    const_standard_iterator<T> next() const {
        assert(this->node_ != nullptr);
        return const_standard_iterator<T>(this->node_->next);
    }
};

template <typename T>
typename standard_iterator_base<T>::value_type
standard_iterator_base<T>::default_value_ = typename standard_iterator_base<T>::value_type();

// (standard_iterator<T> lhs == standard_iterator<T> rhs)?

//template <typename T>
//inline bool operator == (const standard_iterator<T> & lhs, const standard_iterator<T> & rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_standard_iterator<T> & lhs, const const_standard_iterator<T> & rhs)
//{
//    return lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator == (const standard_iterator<T> & lhs, const const_standard_iterator<T> & rhs)
{
    return lhs.is_equal(rhs);
}

template <typename T>
inline bool operator == (const const_standard_iterator<T> & lhs, const standard_iterator<T> & rhs)
{
    return lhs.is_equal(rhs);
}

// (standard_iterator<T> lhs != standard_iterator<T> rhs)?

//template <typename T>
//inline bool operator != (const standard_iterator<T> & lhs, const standard_iterator<T> & rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_standard_iterator<T> & lhs, const const_standard_iterator<T> & rhs)
//{
//    return !lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator != (const standard_iterator<T> & lhs, const const_standard_iterator<T> & rhs)
{
    return !lhs.is_equal(rhs);
}

template <typename T>
inline bool operator != (const const_standard_iterator<T> & lhs, const standard_iterator<T> & rhs)
{
    return !lhs.is_equal(rhs);
}

#if 1

// (standard_iterator<T> lhs == node_ptr * rhs)?

//template <typename T>
//inline bool operator == (const standard_iterator<T> & lhs,
//                         typename standard_iterator<T>::node_ptr rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_standard_iterator<T> & lhs,
//                         typename const_standard_iterator<T>::node_ptr rhs)
//{
//    return lhs.is_equal(rhs);
//}

//template <typename T>
//inline bool operator == (typename standard_iterator<T>::node_ptr lhs,
//                         const standard_iterator<T> & rhs)
//{
//    return rhs.is_equal(lhs);
//}
//
//template <typename T>
//inline bool operator == (typename const_standard_iterator<T>::node_ptr lhs,
//                         const const_standard_iterator<T> & rhs)
//{
//    return rhs.is_equal(lhs);
//}

template <typename T>
inline bool operator == (typename standard_iterator_base<T>::node_ptr lhs,
                         const standard_iterator<T> & rhs)
{
    return rhs.is_equal(static_cast<typename standard_iterator<T>::node_ptr>(lhs));
}

template <typename T>
inline bool operator == (typename standard_iterator_base<T>::node_ptr lhs,
                         const const_standard_iterator<T> & rhs)
{
    return rhs.is_equal(static_cast<typename const_standard_iterator<T>::node_ptr>(lhs));
}

// (standard_iterator<T> lhs != node_ptr * rhs)?

//template <typename T>
//inline bool operator != (const standard_iterator<T> & lhs,
//                         typename standard_iterator<T>::node_ptr rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_standard_iterator<T> & lhs,
//                         typename const_standard_iterator<T>::node_ptr rhs)
//{
//    return !lhs.is_equal(rhs);
//}

//template <typename T>
//inline bool operator != (typename standard_iterator<T>::node_ptr lhs,
//                         const standard_iterator<T> & rhs)
//{
//    return !rhs.is_equal(lhs);
//}
//
//template <typename T>
//inline bool operator != (typename const_standard_iterator<T>::node_ptr lhs,
//                         const const_standard_iterator<T> & rhs)
//{
//    return !rhs.is_equal(lhs);
//}

template <typename T>
inline bool operator != (typename standard_iterator_base<T>::node_ptr lhs,
                         const standard_iterator<T> & rhs)
{
    return !rhs.is_equal(static_cast<typename standard_iterator<T>::node_ptr>(lhs));
}

template <typename T>
inline bool operator != (typename standard_iterator_base<T>::node_ptr lhs,
                         const const_standard_iterator<T> & rhs)
{
    return !rhs.is_equal(static_cast<typename const_standard_iterator<T>::node_ptr>(lhs));
}

// (standard_iterator<T> lhs == void * rhs)?

//template <typename T>
//inline bool operator == (const standard_iterator<T> & lhs, void * rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_standard_iterator<T> & lhs, void * rhs)
//{
//    return lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator == (void * lhs, const standard_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

template <typename T>
inline bool operator == (void * lhs, const const_standard_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

// (standard_iterator<T> lhs != void * rhs)?

//template <typename T>
//inline bool operator != (const standard_iterator<T> & lhs, void * rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_standard_iterator<T> & lhs, void * rhs)
//{
//    return !lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator != (void * lhs, const standard_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

template <typename T>
inline bool operator != (void * lhs, const const_standard_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

// (standard_iterator<T> lhs == std::nullptr_t rhs)?

//template <typename T>
//inline bool operator == (const standard_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_standard_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return lhs.is_equal(rhs);
//}

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)

template <typename T>
inline bool operator == (std::nullptr_t lhs, const standard_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

template <typename T>
inline bool operator == (std::nullptr_t lhs, const const_standard_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

#endif

// (standard_iterator<T> lhs != std::nullptr_t rhs)?

//template <typename T>
//inline bool operator != (const standard_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_standard_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return !lhs.is_equal(rhs);
//}

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)

template <typename T>
inline bool operator != (std::nullptr_t lhs, const standard_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

template <typename T>
inline bool operator != (std::nullptr_t lhs, const const_standard_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

#endif

#endif

} // namespace jstd

#endif // JSTD_STANDARD_ITERATOR_H
