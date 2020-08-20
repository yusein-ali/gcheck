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

template<size_t... Args>
auto index_tuple(std::index_sequence<Args...>) {
    return std::make_tuple(Args...);
}

template<size_t N>
auto index_tuple() {
    return index_tuple(std::make_index_sequence<N>());
}

template<int index = 0, typename... Tuples>
auto tuple_from(const Tuples&... tuples) {
    return std::make_tuple(std::get<index>(tuples)...);
}

template<typename T, T... Indices, typename... Tuples>
auto zip(std::integer_sequence<T, Indices...>, const Tuples&... tuples) {
    return std::make_tuple(tuple_from<Indices>(tuples...)...);
}

template<typename... Args, typename... Tuples>
auto zip(const std::tuple<Args...>& tuple, const Tuples&... tuples) {
    return zip(std::index_sequence_for<Args...>(), tuple, tuples...);
}

template<typename Func, typename... Args>
inline void for_each(const std::tuple<Args...>& tuple, Func&& func) {
    std::apply([&func](auto... tuples){
        (..., std::apply(func, tuples));
    }, zip(index_tuple<sizeof...(Args)>(), tuple));
}

} // anonymous