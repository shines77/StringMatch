
#ifndef JSTD_FORWARD_ITERATOR_H
#define JSTD_FORWARD_ITERATOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <memory>
#include <type_traits>

#include "StringMatch.h"

namespace jstd {

template <typename T>
class forward_iterator_base {
public:
    typedef typename T::node_type node_type;
    typedef typename T::node_ptr node_ptr;
    typedef typename T::value_type value_type;
    typedef forward_iterator_base this_type;

    static value_type default_value_;

protected:
    node_ptr node_;

public:
    forward_iterator_base(node_ptr node = nullptr) : node_(node) {}
    explicit forward_iterator_base(void * p) : node_(static_cast<node_ptr>(p)) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    forward_iterator_base(std::nullptr_t) : node_(nullptr) {}
#endif
    explicit forward_iterator_base(forward_iterator_base const & src)
        : node_(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit forward_iterator_base(forward_iterator_base && src)
        : node_(std::forward<forward_iterator_base>(src).node_) {}
#endif
    ~forward_iterator_base() {}

    forward_iterator_base & operator = (forward_iterator_base const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    forward_iterator_base & operator = (forward_iterator_base && rhs) {
        this->node_ = std::forward<forward_iterator_base>(rhs).node_;
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
class forward_iterator;

template <typename T>
class const_forward_iterator : public forward_iterator_base<T> {
public:
    typedef typename T::node_type node_type;
    typedef typename T::node_ptr node_ptr;
    typedef typename T::value_type value_type;
    typedef const_forward_iterator<T> this_type;

    friend class forward_iterator<T>;

public:
    const_forward_iterator(node_ptr node = nullptr)
        : forward_iterator_base<T>(node) {}
    explicit const_forward_iterator(void * p)
        : forward_iterator_base<T>(p) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    const_forward_iterator(std::nullptr_t node)
        : forward_iterator_base<T>(node) {}
#endif
    explicit const_forward_iterator(const_forward_iterator const & src)
        : forward_iterator_base<T>(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit const_forward_iterator(const_forward_iterator && src)
        : forward_iterator_base<T>(std::forward<const_forward_iterator>(src).node_) {}
#endif
    ~const_forward_iterator() {}

    const_forward_iterator & operator = (const_forward_iterator const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    const_forward_iterator & operator = (const_forward_iterator && rhs) {
        this->node_ = std::forward<const_forward_iterator>(rhs).node_;
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

    bool is_equal(const_forward_iterator const & rhs) const {
        return (this->node_ == rhs.node_);
    }

    bool operator == (const_forward_iterator const & rhs) const {
        return this->is_equal(rhs);
    }

    bool operator != (const_forward_iterator const & rhs) const {
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

    operator forward_iterator<T> * () {
        return reinterpret_cast<forward_iterator<T> *>(const_cast<const_forward_iterator *>(this));
    }
    operator const forward_iterator<T> * () const {
        return const_cast<const forward_iterator<T> *>(
            reinterpret_cast<forward_iterator<T> *>(const_cast<const_forward_iterator *>(this)));
    }

    operator forward_iterator<T> () {
        return *reinterpret_cast<forward_iterator<T> *>(const_cast<const_forward_iterator *>(this));
    }
    operator const forward_iterator<T> () const {
        return *const_cast<const forward_iterator<T> *>(
            reinterpret_cast<forward_iterator<T> *>(const_cast<const_forward_iterator *>(this)));
    }

    operator forward_iterator<T> & () {
        return *reinterpret_cast<forward_iterator<T> *>(const_cast<const_forward_iterator *>(this));
    }
    operator const forward_iterator<T> & () const {
        return *const_cast<const forward_iterator<T> *>(
            reinterpret_cast<forward_iterator<T> *>(const_cast<const_forward_iterator *>(this)));
    }

    const_forward_iterator & operator ++ () {
        assert(this->node_ != nullptr);
        this->node_ = this->node_->next;
        return *this;
    }

    const_forward_iterator operator ++ (int) {
        const_forward_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    const_forward_iterator next() {
        assert(this->node_ != nullptr);
        return const_forward_iterator(this->node_->next);
    }

    const const_forward_iterator next() const {
        assert(this->node_ != nullptr);
        return const_forward_iterator(this->node_->next);
    }
};

template <typename T>
class forward_iterator : public const_forward_iterator<T> {
public:
    typedef typename T::node_type node_type;
    typedef typename T::node_ptr node_ptr;
    typedef typename T::value_type value_type;
    typedef forward_iterator<T> this_type;

public:
    forward_iterator(node_ptr node = nullptr) : const_forward_iterator<T>(node) {}
    explicit forward_iterator(void * p) : const_forward_iterator<T>(p) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    forward_iterator(std::nullptr_t node) : const_forward_iterator<T>(node) {}
#endif
    explicit forward_iterator(forward_iterator const & src)
        : const_forward_iterator<T>(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit forward_iterator(forward_iterator && src)
        : const_forward_iterator<T>(std::forward<forward_iterator>(src).node_) {}
#endif
    explicit forward_iterator(const_forward_iterator<T> const & src)
        : const_forward_iterator<T>(src.node_) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    explicit forward_iterator(const_forward_iterator<T> && src)
        : const_forward_iterator<T>(std::forward<const_forward_iterator<T>>(src).node_) {}
#endif
    ~forward_iterator() {}

    forward_iterator & operator = (forward_iterator const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    forward_iterator & operator = (forward_iterator && rhs) {
        this->node_ = std::forward<forward_iterator>(rhs).node_;
        return *this;
    }
#endif

    forward_iterator & operator = (const_forward_iterator<T> const & rhs) {
        this->node_ = rhs.node_;
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    forward_iterator & operator = (const_forward_iterator<T> && rhs) {
        this->node_ = std::forward<const_forward_iterator<T>>(rhs).node_;
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

    bool is_equal(forward_iterator const & rhs) const {
        return (this->node_ == rhs.node_);
    }

    bool is_equal(const_forward_iterator<T> const & rhs) const {
        return (this->node_ == rhs.node_);
    }

    bool operator == (forward_iterator const & rhs) const {
        return this->is_equal(rhs);
    }

    bool operator != (forward_iterator const & rhs) const {
        return !(this->is_equal(rhs));
    }

    bool operator == (const_forward_iterator<T> const & rhs) const {
        return this->is_equal(rhs);
    }

    bool operator != (const_forward_iterator<T> const & rhs) const {
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

    operator const_forward_iterator<T> * () {
        return reinterpret_cast<const_forward_iterator<T> *>(const_cast<forward_iterator *>(this));
    }
    operator const const_forward_iterator<T> * () const {
        return const_cast<const const_forward_iterator<T> *>(
            reinterpret_cast<const_forward_iterator<T> *>(const_cast<forward_iterator *>(this)));
    }

    operator const_forward_iterator<T> () {
        return *reinterpret_cast<const_forward_iterator<T> *>(const_cast<forward_iterator *>(this));
    }
    operator const const_forward_iterator<T> () const {
        return *const_cast<const const_forward_iterator<T> *>(
            reinterpret_cast<const_forward_iterator<T> *>(const_cast<forward_iterator *>(this)));
    }

    operator const_forward_iterator<T> & () {
        return *reinterpret_cast<const_forward_iterator<T> *>(const_cast<forward_iterator *>(this));
    }
    operator const const_forward_iterator<T> & () const {
        return *const_cast<const const_forward_iterator<T> *>(
            reinterpret_cast<const_forward_iterator<T> *>(const_cast<forward_iterator *>(this)));
    }

    forward_iterator & operator ++ () {
        assert(this->node_ != nullptr);
        this->node_ = this->node_->next;
        return *this;
    }

    forward_iterator operator ++ (int) {
        forward_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    forward_iterator next() {
        assert(this->node_ != nullptr);
        return forward_iterator(this->node_->next);
    }

    const_forward_iterator<T> next() const {
        assert(this->node_ != nullptr);
        return const_forward_iterator<T>(this->node_->next);
    }
};

template <typename T>
typename forward_iterator_base<T>::value_type
forward_iterator_base<T>::default_value_ = typename forward_iterator_base<T>::value_type();

// (forward_iterator<T> lhs == forward_iterator<T> rhs)?

//template <typename T>
//inline bool operator == (const forward_iterator<T> & lhs, const forward_iterator<T> & rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_forward_iterator<T> & lhs, const const_forward_iterator<T> & rhs)
//{
//    return lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator == (const forward_iterator<T> & lhs, const const_forward_iterator<T> & rhs)
{
    return lhs.is_equal(rhs);
}

template <typename T>
inline bool operator == (const const_forward_iterator<T> & lhs, const forward_iterator<T> & rhs)
{
    return lhs.is_equal(rhs);
}

// (forward_iterator<T> lhs != forward_iterator<T> rhs)?

//template <typename T>
//inline bool operator != (const forward_iterator<T> & lhs, const forward_iterator<T> & rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_forward_iterator<T> & lhs, const const_forward_iterator<T> & rhs)
//{
//    return !lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator != (const forward_iterator<T> & lhs, const const_forward_iterator<T> & rhs)
{
    return !lhs.is_equal(rhs);
}

template <typename T>
inline bool operator != (const const_forward_iterator<T> & lhs, const forward_iterator<T> & rhs)
{
    return !lhs.is_equal(rhs);
}

#if 1

// (forward_iterator<T> lhs == node_ptr * rhs)?

//template <typename T>
//inline bool operator == (const forward_iterator<T> & lhs,
//                         typename forward_iterator<T>::node_ptr rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_forward_iterator<T> & lhs,
//                         typename const_forward_iterator<T>::node_ptr rhs)
//{
//    return lhs.is_equal(rhs);
//}

//template <typename T>
//inline bool operator == (typename forward_iterator<T>::node_ptr lhs,
//                         const forward_iterator<T> & rhs)
//{
//    return rhs.is_equal(lhs);
//}
//
//template <typename T>
//inline bool operator == (typename const_forward_iterator<T>::node_ptr lhs,
//                         const const_forward_iterator<T> & rhs)
//{
//    return rhs.is_equal(lhs);
//}

template <typename T>
inline bool operator == (typename forward_iterator_base<T>::node_ptr lhs,
                         const forward_iterator<T> & rhs)
{
    return rhs.is_equal(static_cast<typename forward_iterator<T>::node_ptr>(lhs));
}

template <typename T>
inline bool operator == (typename forward_iterator_base<T>::node_ptr lhs,
                         const const_forward_iterator<T> & rhs)
{
    return rhs.is_equal(static_cast<typename const_forward_iterator<T>::node_ptr>(lhs));
}

// (forward_iterator<T> lhs != node_ptr * rhs)?

//template <typename T>
//inline bool operator != (const forward_iterator<T> & lhs,
//                         typename forward_iterator<T>::node_ptr rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_forward_iterator<T> & lhs,
//                         typename const_forward_iterator<T>::node_ptr rhs)
//{
//    return !lhs.is_equal(rhs);
//}

//template <typename T>
//inline bool operator != (typename forward_iterator<T>::node_ptr lhs,
//                         const forward_iterator<T> & rhs)
//{
//    return !rhs.is_equal(lhs);
//}
//
//template <typename T>
//inline bool operator != (typename const_forward_iterator<T>::node_ptr lhs,
//                         const const_forward_iterator<T> & rhs)
//{
//    return !rhs.is_equal(lhs);
//}

template <typename T>
inline bool operator != (typename forward_iterator_base<T>::node_ptr lhs,
                         const forward_iterator<T> & rhs)
{
    return !rhs.is_equal(static_cast<typename forward_iterator<T>::node_ptr>(lhs));
}

template <typename T>
inline bool operator != (typename forward_iterator_base<T>::node_ptr lhs,
                         const const_forward_iterator<T> & rhs)
{
    return !rhs.is_equal(static_cast<typename const_forward_iterator<T>::node_ptr>(lhs));
}

// (forward_iterator<T> lhs == void * rhs)?

//template <typename T>
//inline bool operator == (const forward_iterator<T> & lhs, void * rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_forward_iterator<T> & lhs, void * rhs)
//{
//    return lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator == (void * lhs, const forward_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

template <typename T>
inline bool operator == (void * lhs, const const_forward_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

// (forward_iterator<T> lhs != void * rhs)?

//template <typename T>
//inline bool operator != (const forward_iterator<T> & lhs, void * rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_forward_iterator<T> & lhs, void * rhs)
//{
//    return !lhs.is_equal(rhs);
//}

template <typename T>
inline bool operator != (void * lhs, const forward_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

template <typename T>
inline bool operator != (void * lhs, const const_forward_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

// (forward_iterator<T> lhs == std::nullptr_t rhs)?

//template <typename T>
//inline bool operator == (const forward_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator == (const const_forward_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return lhs.is_equal(rhs);
//}

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)

template <typename T>
inline bool operator == (std::nullptr_t lhs, const forward_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

template <typename T>
inline bool operator == (std::nullptr_t lhs, const const_forward_iterator<T> & rhs)
{
    return rhs.is_equal(lhs);
}

#endif

// (forward_iterator<T> lhs != std::nullptr_t rhs)?

//template <typename T>
//inline bool operator != (const forward_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return !lhs.is_equal(rhs);
//}
//
//template <typename T>
//inline bool operator != (const const_forward_iterator<T> & lhs, std::nullptr_t rhs)
//{
//    return !lhs.is_equal(rhs);
//}

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)

template <typename T>
inline bool operator != (std::nullptr_t lhs, const forward_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

template <typename T>
inline bool operator != (std::nullptr_t lhs, const const_forward_iterator<T> & rhs)
{
    return !rhs.is_equal(lhs);
}

#endif

#endif

template <typename T>
inline intptr_t operator - (const forward_iterator<T> & lhs,
                            const forward_iterator<T> & rhs)
{
    return lhs.next - rhs.next;
}

template <typename T>
inline intptr_t operator - (const const_forward_iterator<T> & lhs,
                            const const_forward_iterator<T> & rhs)
{
    return lhs.next - rhs.next;
}

} // namespace jstd

#endif // JSTD_FORWARD_ITERATOR_H
