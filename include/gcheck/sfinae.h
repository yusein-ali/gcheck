#pragma once

#include <tuple>
#include <type_traits>

namespace gcheck {
    
template<class>
struct sfinae_bool {};
template<class T>
struct sfinae_true : sfinae_bool<T>, std::true_type{};
template<class T>
struct sfinae_false : sfinae_bool<T>, std::false_type{};

// AND operator
template<class T, class S>
sfinae_false<T> operator*(sfinae_bool<T>, sfinae_bool<S>);
template<class T, class S>
sfinae_true<T> operator*(sfinae_true<T>, sfinae_true<S>);

template<typename... T>
struct is_empty : std::false_type{};
template<>
struct is_empty<> : std::true_type{};

namespace {
    namespace detail {
    
    template<class T>
    static auto has_tostring(int) -> sfinae_true<decltype(to_string(std::declval<T>()))>;
    template<class T>
    static auto has_tostring(long) -> sfinae_false<T>;
    template<class T>
    static auto has_std_tostring(int) -> sfinae_true<decltype(std::to_string(std::declval<T>()))>;
    template<class T>
    static auto has_std_tostring(long) -> sfinae_false<T>;
    
    } // detail
} // anonymous

template<class T>
struct has_tostring : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_std_tostring : decltype(detail::has_std_tostring<T>(0)){};

template<int index = 0, typename Func, typename... Args>
std::enable_if_t<index == sizeof...(Args)> for_each(const std::tuple<Args...>&, Func) {}

template<int index = 0, typename Func, typename... Args>
std::enable_if_t<index != sizeof...(Args)> for_each(const std::tuple<Args...>& t, Func func) {
    func(index, std::get<index>(t));
    for_each<index+1>(t, func);
}

} // anonymous