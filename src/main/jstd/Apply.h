
#ifndef STRING_MATCH_APPLY_H
#define STRING_MATCH_APPLY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <tuple>
#include <type_traits>
#include <utility>

namespace StringMatch {
namespace detail {

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)

    //
    // About std::tuple<Args...>
    //
    // See: http://www.cnblogs.com/qicosmos/p/3318070.html
    //
    template <size_t N>
    struct Apply {
        template <typename Func, typename Tuple, typename... Args>
        static inline auto apply(Func && func, Tuple && tuple, Args &&... args)
            -> decltype(Apply<N - 1>::apply(
                ::std::forward<Func>(func), ::std::forward<Tuple>(tuple),
                ::std::get<N - 1>(::std::forward<Tuple>(tuple)),
                ::std::forward<Args>(args)...
                ))
        {
            return Apply<N - 1>::apply(::std::forward<Func>(func),
                                       ::std::forward<Tuple>(tuple),
                                       ::std::get<N - 1>(::std::forward<Tuple>(tuple)),
                                       ::std::forward<Args>(args)...);
        }
    };

    template <>
    struct Apply<0> {
        template <typename Func, typename Tuple, typename... Args>
        static inline auto apply(Func && func, Tuple && tuple, Args &&... args)
            -> decltype(::std::forward<Func>(func)(::std::forward<Args>(args)...))
        {
            return ::std::forward<Func>(func)(::std::forward<Args>(args)...);
        }
    };

    template <typename Func, typename Tuple>
    inline auto apply(Func && func, Tuple && tuple)
        -> decltype(Apply< ::std::tuple_size<typename ::std::decay<Tuple>::type>::value >
            ::apply(::std::forward<Func>(func), ::std::forward<Tuple>(tuple)))
    {
        return Apply< ::std::tuple_size<typename ::std::decay<Tuple>::type>::value >
            ::apply(::std::forward<Func>(func), ::std::forward<Tuple>(tuple));
    }

#endif // (_MSC_VER >= 1800)

} // namespace detail
} // namespace StringMatch

#endif // STRING_MATCH_APPLY_H
