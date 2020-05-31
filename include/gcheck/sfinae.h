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

template<int index = 0, typename Func, typename... Args>
std::enable_if_t<index == sizeof...(Args)> for_each(const std::tuple<Args...>&, Func) {}

template<int index = 0, typename Func, typename... Args>
std::enable_if_t<index != sizeof...(Args)> for_each(const std::tuple<Args...>& t, Func func) {
    func(index, std::get<index>(t));
    for_each<index+1>(t, func);
}

}