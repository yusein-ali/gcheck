#pragma once

#include <tuple>
#include <type_traits>

namespace gcheck {

/**
 * @brief Base tag type used by SFINAE boolean wrappers.
 *
 * @tparam T A payload type carried through the trait. The payload keeps
 *         type identity during overload resolution and trait composition.
 *
 * Typical usage:
 * - Used as a common base for @ref sfinae_true and @ref sfinae_false.
 * - Enables the generic `operator*` overload that acts as a fallback
 *   logical AND for trait composition.
 */
template<class>
struct sfinae_bool {};

/**
 * @brief Boolean wrapper that evaluates to compile-time true.
 *
 * @tparam T Payload type used to preserve type identity in composed traits.
 *
 * This type inherits:
 * - @ref sfinae_bool to participate in `operator*` composition.
 * - `std::true_type` to expose `value == true`.
 *
 * Typical usage:
 * - Return type of successful detection probes (for example, expression is valid).
 * - Input to the specialized `operator*` overload implementing `true && true`.
 */
template<class T>
struct sfinae_true : sfinae_bool<T>, std::true_type{};

/**
 * @brief Boolean wrapper that evaluates to compile-time false.
 *
 * @tparam T Payload type used to preserve type identity in composed traits.
 *
 * This type inherits:
 * - @ref sfinae_bool to participate in `operator*` fallback.
 * - `std::false_type` to expose `value == false`.
 *
 * Typical usage:
 * - Return type of failed detection probes (expression substitution fails).
 * - Fallback operand/result for composed traits such as @ref has_begin_end.
 */
template<class T>
struct sfinae_false : sfinae_bool<T>, std::false_type{};

/**
 * @brief Fallback logical AND for SFINAE boolean wrappers.
 *
 * @tparam T Left payload type.
 * @tparam S Right payload type.
 * @param  Left operand accepted as @ref sfinae_bool.
 * @param  Right operand accepted as @ref sfinae_bool.
 * @return @ref sfinae_false for all combinations not matched by the
 *         `sfinae_true * sfinae_true` overload.
 *
 * Typical usage:
 * - Composing probe results without enumerating all false combinations.
 * - Example: `decltype(has_begin<T>(0) * has_end<T>(0))`.
 */
template<class T, class S>
sfinae_false<T> operator*(sfinae_bool<T>, sfinae_bool<S>);

/**
 * @brief Specialized logical AND for `true && true`.
 *
 * @tparam T Left payload type.
 * @tparam S Right payload type.
 * @param  Left operand as @ref sfinae_true.
 * @param  Right operand as @ref sfinae_true.
 * @return @ref sfinae_true when both operands are true wrappers.
 *
 * Typical usage:
 * - Preferred overload when both detection probes succeed.
 */
template<class T, class S>
sfinae_true<T> operator*(sfinae_true<T>, sfinae_true<S>);

/**
 * @brief Trait indicating whether a parameter pack is empty.
 *
 * @tparam T Parameter pack under test.
 *
 * Value:
 * - `false` for non-empty packs.
 * - `true` for the explicit specialization with no types.
 *
 * Typical usage:
 * - Generic code paths that need a compile-time check for zero-arity packs.
 */
template<typename... T>
struct is_empty : std::false_type{};

/**
 * @brief Specialization of @ref is_empty for an empty pack.
 */
template<>
struct is_empty<> : std::true_type{};

namespace {
    namespace detail {

    /**
     * @brief Detection probe for unqualified `to_string(T)` via ADL.
     *
     * @tparam T Type to test.
     * @param Unused rank tag favoring this overload.
     * @return @ref sfinae_true if `to_string(std::declval<T>())` is valid.
     *
     * ADL (Argument-Dependent Lookup, also called Koenig lookup) means
     * the compiler may find functions in namespaces associated with argument
     * types, even when the function name is unqualified.
     *
     * In practice, using unqualified `to_string(...)` here allows detection
     * of user-defined `to_string` overloads that live in the same namespace
     * as `T`, not only `std::to_string`.
     *
     * Typical usage:
     * - First-choice probe in trait construction for custom string conversion.
     */
    template<class T>
    static auto has_tostring(int) -> sfinae_true<decltype(to_string(std::declval<T>()))>;

    /**
     * @brief Fallback probe when unqualified `to_string(T)` is not valid.
     *
     * @tparam T Type to test.
     * @param Unused rank tag fallback.
     * @return @ref sfinae_false.
     */
    template<class T>
    static auto has_tostring(long) -> sfinae_false<T>;

    /**
     * @brief Detection probe for `std::to_string(T)`.
     *
     * @tparam T Type to test.
     * @param Unused rank tag favoring this overload.
     * @return @ref sfinae_true if `std::to_string(std::declval<T>())` is valid.
     *
     * Typical usage:
     * - Secondary conversion probe when custom `to_string` is unavailable.
     */
    template<class T>
    static auto has_std_tostring(int) -> sfinae_true<decltype(std::to_string(std::declval<T>()))>;

    /**
     * @brief Fallback probe when `std::to_string(T)` is not valid.
     *
     * @tparam T Type to test.
     * @param Unused rank tag fallback.
     * @return @ref sfinae_false.
     */
    template<class T>
    static auto has_std_tostring(long) -> sfinae_false<T>;

    /**
     * @brief Detection probe for member `begin()`.
     *
     * @tparam T Type to test.
     * @param Unused rank tag favoring this overload.
     * @return @ref sfinae_true if `std::declval<T>().begin()` is valid.
     *
     * Typical usage:
     * - Part of iterable/container detection combined with @ref has_end.
     */
    template<class T>
    static auto has_begin(int) -> sfinae_true<decltype(std::declval<T>().begin())>;

    /**
     * @brief Fallback probe when member `begin()` is not valid.
     *
     * @tparam T Type to test.
     * @param Unused rank tag fallback.
     * @return @ref sfinae_false.
     */
    template<class T>
    static auto has_begin(long) -> sfinae_false<T>;

    /**
     * @brief Detection probe for member `end()`.
     *
     * @tparam T Type to test.
     * @param Unused rank tag favoring this overload.
     * @return @ref sfinae_true if `std::declval<T>().end()` is valid.
     *
     * Typical usage:
     * - Part of iterable/container detection combined with @ref has_begin.
     */
    template<class T>
    static auto has_end(int) -> sfinae_true<decltype(std::declval<T>().end())>;

    /**
     * @brief Fallback probe when member `end()` is not valid.
     *
     * @tparam T Type to test.
     * @param Unused rank tag fallback.
     * @return @ref sfinae_false.
     */
    template<class T>
    static auto has_end(long) -> sfinae_false<T>;
    } // detail
} // anonymous

/**
 * @brief Public trait indicating ADL-visible `to_string(T)` support.
 *
 * @tparam T Type to test.
 *
 * ADL-visible means an unqualified call `to_string(value)` can be resolved
 * through argument-associated namespaces of `T`.
 *
 * Typical usage:
 * - `std::enable_if_t<has_tostring<T>::value>` in stringify/json conversion paths.
 */
template<class T>
struct has_tostring : decltype(detail::has_tostring<T>(0)){};

/**
 * @brief Public trait indicating `std::to_string(T)` support.
 *
 * @tparam T Type to test.
 *
 * Typical usage:
 * - Fallback conversion path when custom `to_string` is unavailable.
 */
template<class T>
struct has_std_tostring : decltype(detail::has_std_tostring<T>(0)){};

/**
 * @brief Public trait indicating presence of both `begin()` and `end()`.
 *
 * @tparam T Type to test.
 *
 * Internally composes two probe results using `operator*`:
 * - true only when both probes are true.
 *
 * Typical usage:
 * - Enabling container serialization in JSON/string formatting code.
 */
template<class T>
struct has_begin_end : decltype(detail::has_begin<T>(0) * detail::has_end<T>(0)){};

/**
 * @brief Build a tuple from an index sequence.
 *
 * @tparam Args Compile-time indices.
 * @param  Index sequence object.
 * @return `std::tuple<size_t...>` containing the indices in order.
 *
 * Typical usage:
 * - Internal helper for tuple zipping and indexed iteration.
 */
template<size_t... Args>
auto index_tuple(std::index_sequence<Args...>) {
    return std::make_tuple(Args...);
}

/**
 * @brief Build a tuple `(0, 1, ..., N-1)`.
 *
 * @tparam N Number of indices.
 * @return Tuple of indices from 0 to N-1.
 *
 * Typical usage:
 * - Preparing index tuples for @ref zip and @ref for_each.
 */
template<size_t N>
auto index_tuple() {
    return index_tuple(std::make_index_sequence<N>());
}

/**
 * @brief Extract one element position from multiple tuples.
 *
 * @tparam index Zero-based element position to extract.
 * @tparam Tuples Tuple types.
 * @param tuples Input tuples.
 * @return A tuple with `std::get<index>` from each input tuple.
 *
 * Typical usage:
 * - Core building block of tuple zipping by index.
 */
template<int index = 0, typename... Tuples>
auto tuple_from(const Tuples&... tuples) {
    return std::make_tuple(std::get<index>(tuples)...);
}

/**
 * @brief Zip tuples using an explicit integer sequence of indices.
 *
 * @tparam T Integer type used by the index sequence.
 * @tparam Indices Indices to zip.
 * @tparam Tuples Tuple types.
 * @param  Index sequence.
 * @param tuples Input tuples with compatible arity.
 * @return Tuple of tuples grouped by index.
 *
 * Typical usage:
 * - Convert `(a0,a1),(b0,b1)` into `((a0,b0),(a1,b1))`.
 */
template<typename T, T... Indices, typename... Tuples>
auto zip(std::integer_sequence<T, Indices...>, const Tuples&... tuples) {
    return std::make_tuple(tuple_from<Indices>(tuples...)...);
}

/**
 * @brief Zip tuples by deriving indices from a reference tuple type.
 *
 * @tparam Args Element types of the reference tuple.
 * @tparam Tuples Tuple types.
 * @param tuple Reference tuple used only for arity.
 * @param tuples Tuples to zip.
 * @return Tuple of tuples grouped by index.
 *
 * Typical usage:
 * - Convenience overload for call sites that already hold a tuple instance.
 */
template<typename... Args, typename... Tuples>
auto zip(const std::tuple<Args...>& tuple, const Tuples&... tuples) {
    return zip(std::index_sequence_for<Args...>(), tuple, tuples...);
}

/**
 * @brief Iterate over tuple elements with index-value callbacks.
 *
 * @tparam Func Callable type. Must accept `(index, element)` arguments.
 * @tparam Args Tuple element types.
 * @param tuple Tuple to iterate.
 * @param func Callback invoked once per element.
 *
 * Typical usage:
 * - Generic formatting, comparison, or serialization over tuple fields.
 */
template<typename Func, typename... Args>
inline void for_each(const std::tuple<Args...>& tuple, Func&& func) {
    std::apply([&func](auto... tuples){
        (..., std::apply(func, tuples));
    }, zip(index_tuple<sizeof...(Args)>(), tuple));
}

/**
 * @brief Iterate over a pair as `(index, value)` entries.
 *
 * @tparam Func Callable type. Must accept `(index, element)` arguments.
 * @tparam Arg1 Pair first type.
 * @tparam Args2 Pair second type.
 * @param pair Pair to iterate.
 * @param func Callback invoked for index 0 and index 1.
 *
 * Typical usage:
 * - Unified handling for tuple/pair iteration in generic code paths.
 */
template<typename Func, typename Arg1, typename Args2>
inline void for_each(const std::pair<Arg1, Args2>& pair, Func&& func) {
    func(0, pair.first);
    func(1, pair.second);
}

} // gcheck
