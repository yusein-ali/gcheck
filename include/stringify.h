#pragma once

#include <string>
#include <functional>
#include <vector>
#include <tuple>
#include <sstream>
#include <list>
#include <string_view>

#include "sfinae.h"

namespace gcheck {
namespace {
namespace detail {
    /**
     * @brief SFINAE probe for unqualified `to_construct(T)` availability.
     *
     * @tparam T Type under test.
     * @param Unused overload-rank marker.
     * @return @ref sfinae_true when `to_construct(std::declval<T>())` is valid.
     *
     * Typical usage:
     * - Internal detection path used by @ref has_toconstruct.
     */
    template<class T>
    static auto has_toconstruct(int) -> sfinae_true<decltype(to_construct(std::declval<T>()))>;

    /**
     * @brief Fallback SFINAE probe when `to_construct(T)` is not available.
     *
     * @tparam T Type under test.
     * @param Unused overload-rank marker.
     * @return @ref sfinae_false fallback.
     */
    template<class T>
    static auto has_toconstruct(long) -> sfinae_false<T>;
} // detail

/**
 * @brief Trait that checks support for unqualified `to_construct(T)`.
 *
 * @tparam T Type under test.
 *
 * Value semantics:
 * - `has_toconstruct<T>::value == true` if `to_construct(T)` is callable.
 * - `false` otherwise.
 *
 * Typical usage:
 * - Enable specialized construction-string generation in @ref toConstruct.
 */
template<class T>
struct has_toconstruct : decltype(detail::has_toconstruct<T>(0)) {};

} // anonymous

/**
 * @brief Return a best-effort demangled type name string for `T`.
 *
 * @tparam T Type whose compiler-reported name is requested.
 * @return Human-readable type name extracted from compiler function signature macros.
 *
 * Notes:
 * - The exact text is compiler-dependent (`__PRETTY_FUNCTION__` / `__FUNCSIG__`).
 * - This is mainly for diagnostics and generated constructor strings.
 *
 * Typical usage:
 * - Building readable container constructor text in @ref toConstruct overloads.
 */
template <typename T>
constexpr auto type_name() noexcept {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto gcheck::type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto gcheck::type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl gcheck::type_name<";
  suffix = ">(void) noexcept";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return std::string(name);
}

/**
 * @brief Escape non-UTF8 bytes and special characters for JSON-safe text.
 *
 * @param Input raw string.
 * @return Escaped string suitable for JSON payloads.
 *
 * Typical usage:
 * - Used by string conversion/serialization code before emitting JSON strings.
 */
std::string JSONEscape(std::string);

/**
 * @brief Generic container-to-string join helper.
 *
 * @tparam C Container type supporting `begin()` / `end()`.
 * @tparam Func Callable that maps each element to string.
 * @param container Input container to stringify.
 * @param func Element formatting function.
 * @param start Prefix emitted before first element.
 * @param separator Delimiter emitted between elements.
 * @param end Suffix emitted after last element.
 * @return Composed string.
 *
 * Typical usage:
 * - Implement vector/list formatting in @ref toString and @ref toConstruct.
 */
template<typename C, typename Func>
std::string Stringify(const C& container, Func func, const std::string& start, const std::string& separator, const std::string& end) {
    std::stringstream ss;
    ss << start;
    for(auto it = container.begin(); it != container.end();) {
        ss << func(*it);
        if(++it != container.end()) {
            ss << separator;
        } else {
            break;
        }
    }
    ss << end;
    return ss.str();
}

/**
 * @brief Helper macro for tuple/pair stringification using @ref for_each.
 *
 * Expands to a local block that writes formatted elements into `ret`.
 *
 * @param saveto (Unused in current implementation; kept for compatibility).
 * @param tuple Tuple/pair object to iterate.
 * @param func Element formatter function (`toString` or `toConstruct`).
 * @param start Prefix before elements.
 * @param separator Delimiter between elements.
 * @param end Suffix after elements.
 *
 * Typical usage:
 * - `toString(tuple)` and `toConstruct(tuple)` overload implementations.
 */
#define StringifyTuple(saveto, tuple, func, start, separator, end) \
    { \
        constexpr size_t n = sizeof...(Args); \
        std::stringstream ss; \
        ss << start; \
        for_each(tuple, [&ss](int i, const auto& a) { \
            ss << func(a); \
            if(i != n-1) \
                ss << separator; \
        }); \
        ss << end; \
        ret = ss.str(); \
    }

/** @brief Forward declaration of value wrapper used in reports/formatting. */
class UserObject;

/**
 * @brief Forward declaration for delta-based floating comparison wrapper.
 *
 * @tparam T Numeric payload type.
 */
template<typename T>
class DeltaCompare;

/**
 * @brief Return a function pointer to the best matching @ref toConstruct overload.
 *
 * @tparam T Element type.
 * @return Function pointer with signature `std::string(const T&)`.
 *
 * Typical usage:
 * - Passed into @ref Stringify for container construction-string generation.
 */
template<typename T>
constexpr auto to_constructer() -> std::string(&)(const T&);

/** @brief Convert C-string literal/pointer value into constructor-style text. */
std::string toConstruct(const char* const& item);
/** @brief Convert mutable C-string pointer into constructor-style text. */
std::string toConstruct(const char*& item);
/** @brief Convert `std::string` into constructor-style text. */
std::string toConstruct(const std::string& item);
/** @brief Convert unsigned char into constructor-style text. */
std::string toConstruct(const unsigned char& item);
/** @brief Convert char into constructor-style text. */
std::string toConstruct(const char& item);
/** @brief Convert bool into constructor-style text. */
std::string toConstruct(const bool& b);
/** @brief Convert @ref UserObject into constructor-style text. */
std::string toConstruct(const UserObject& u);
/** @brief Convert null pointer literal into constructor-style text. */
std::string toConstruct(decltype(nullptr));
/** @brief Convert int into constructor-style text. */
std::string toConstruct(const int& item);
/** @brief Convert long into constructor-style text. */
std::string toConstruct(const long& item);
/** @brief Convert long long into constructor-style text. */
std::string toConstruct(const long long& item);
/** @brief Convert unsigned int into constructor-style text. */
std::string toConstruct(const unsigned& item);
/** @brief Convert unsigned long into constructor-style text. */
std::string toConstruct(const unsigned long& item);
/** @brief Convert unsigned long long into constructor-style text. */
std::string toConstruct(const unsigned long long& item);
/** @brief Convert float into constructor-style text. */
std::string toConstruct(const float& item);
/** @brief Convert double into constructor-style text. */
std::string toConstruct(const double& item);
/** @brief Convert long double into constructor-style text. */
std::string toConstruct(const long double& item);

/**
 * @brief Fallback constructor-string conversion for unsupported types.
 *
 * @tparam T Unsupported type.
 * @param Unused input object.
 * @return Empty string.
 *
 * Typical usage:
 * - Selected when no user/custom `to_construct(T)` path exists.
 */
template<typename T>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(const T&) { return ""; }

/**
 * @brief Pointer constructor conversion for pointee types without custom construction support.
 *
 * @tparam T Pointee type.
 * @param item Pointer value.
 * @return `"nullptr"` for null pointers, otherwise `"new " + toConstruct(*item)`.
 *
 * Typical usage:
 * - Build object construction snippets for heap-allocated pointees.
 */
template<typename T>
typename std::enable_if<!has_toconstruct<T*>::value, std::string>::type
toConstruct(const T*& item) {
    if(item == nullptr)
        return "nullptr";

    return "new " + toConstruct(*item);
}

/**
 * @brief Delegate constructor-string conversion to user-provided `to_construct(T)`.
 *
 * @tparam T Supported type.
 * @param item Input object.
 * @return Result of `to_construct(item)`.
 *
 * Typical usage:
 * - Extension point for user-defined object construction rendering.
 */
template<typename T>
typename std::enable_if<has_toconstruct<T>::value, std::string>::type
toConstruct(const T& item) { return to_construct(item); }

/**
 * @brief Constructor-string conversion for @ref DeltaCompare values.
 *
 * @tparam T Numeric payload type.
 * @param v Delta comparison wrapper.
 * @return String in form `gcheck::DeltaCompare(value,delta)`.
 */
template<typename T>
std::string toConstruct(const DeltaCompare<T>& v) {
    return "gcheck::DeltaCompare(" + toConstruct((T)v) + "," + toConstruct(v.Delta()) + ")";;
}

/**
 * @brief Constructor-string conversion for `std::vector`.
 *
 * @tparam T Element type.
 * @tparam Args Allocator/extra template parameters.
 * @param cont Vector value.
 * @return String in form `std::vector<T>({e1,e2,...})`.
 */
template<typename T, typename... Args>
std::string toConstruct(const std::vector<T, Args...>& cont) {
    return Stringify(cont, to_constructer<T>(), "std::vector<" + type_name<T>() + ">({", ",", "})");
}

/**
 * @brief Constructor-string conversion for `std::list`.
 *
 * @tparam T Element type.
 * @param cont List value.
 * @return String in form `std::list<T>({e1,e2,...})`.
 */
template<typename T>
std::string toConstruct(const std::list<T>& cont) {
    return Stringify(cont, to_constructer<T>(), "std::list<" + type_name<T>() + ">({", ",", "})");
}

/**
 * @brief Constructor-string conversion for tuples.
 *
 * @tparam Args Tuple element types.
 * @param t Tuple value.
 * @return String in form `std::tuple(e1,e2,...)`.
 */
template <class... Args>
std::string toConstruct(const std::tuple<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toConstruct, "std::tuple(", ",", ")");
    return ret;
}

/**
 * @brief Constructor-string conversion for pairs.
 *
 * @tparam Args Pair element types.
 * @param t Pair value.
 * @return String in form `std::pair(e1,e2)`.
 */
template <class... Args>
std::string toConstruct(const std::pair<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toConstruct, "std::pair(", ",", ")");
    return ret;
}

/**
 * @brief Return function pointer to @ref toConstruct overload for `T`.
 *
 * @tparam T Element type.
 * @return Pointer to `toConstruct(const T&)`.
 */
template<typename T>
constexpr auto to_constructer() -> std::string(&)(const T&) {
    return toConstruct;
}

/**
 * @brief Return a function pointer to the best matching @ref toString overload.
 *
 * @tparam T Element type.
 * @return Function pointer with signature `std::string(const T&)`.
 *
 * Typical usage:
 * - Passed into @ref Stringify for container display-string generation.
 */
template<typename T>
constexpr auto to_stringer() -> std::string(&)(const T&);

/** @brief Convert `std::string` to user-facing display string. */
std::string toString(const std::string& item);
/** @brief Convert constant C-string pointer to user-facing display string. */
std::string toString(const char* const& item);
/** @brief Convert mutable C-string pointer to user-facing display string. */
std::string toString(const char*& item);
/** @brief Convert char to user-facing display string. */
std::string toString(const char& item);
/** @brief Convert unsigned char to user-facing display string. */
std::string toString(const unsigned char& item);
/** @brief Convert bool to user-facing display string. */
std::string toString(const bool& b);
/** @brief Convert @ref UserObject to user-facing display string. */
std::string toString(const UserObject& u);
/** @brief Convert null pointer literal to user-facing display string. */
std::string toString(decltype(nullptr));
/** @brief Convert int to user-facing display string. */
std::string toString(const int& item);
/** @brief Convert long to user-facing display string. */
std::string toString(const long& item);
/** @brief Convert long long to user-facing display string. */
std::string toString(const long long& item);
/** @brief Convert unsigned int to user-facing display string. */
std::string toString(const unsigned& item);
/** @brief Convert unsigned long to user-facing display string. */
std::string toString(const unsigned long& item);
/** @brief Convert unsigned long long to user-facing display string. */
std::string toString(const unsigned long long& item);
/** @brief Convert float to user-facing display string. */
std::string toString(const float& item);
/** @brief Convert double to user-facing display string. */
std::string toString(const double& item);
/** @brief Convert long double to user-facing display string. */
std::string toString(const long double& item);

/**
 * @brief Fallback display conversion for types without ADL-visible `to_string`.
 *
 * @tparam T Unsupported type.
 * @param Unused input object.
 * @return Literal `"error-type"`.
 *
 * Typical usage:
 * - Prevent hard compilation failures and surface unsupported type formatting.
 */
template<typename T>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(const T&) { return "error-type"; }

/**
 * @brief Display conversion for pointers.
 *
 * @tparam T Pointee type.
 * @param item Pointer value.
 * @return `"nullptr"` for null; otherwise the pointer address text.
 *
 * Typical usage:
 * - Diagnostics where identity/location of pointed object matters.
 */
template<typename T>
std::string toString(T* const& item) {
    if(item == nullptr)
        return "nullptr";

    std::stringstream ss;
    ss << static_cast<const void*>(item);
    return ss.str();
}

/**
 * @brief Delegate display conversion to user-provided `to_string(T)`.
 *
 * @tparam T Supported type.
 * @param item Input object.
 * @return Result of unqualified `to_string(item)` (ADL-enabled).
 *
 * Typical usage:
 * - Extension point for custom display text for domain objects.
 */
template<typename T>
typename std::enable_if<has_tostring<T>::value, std::string>::type
toString(const T& item) { return to_string(item); }

/**
 * @brief Display conversion for @ref DeltaCompare values.
 *
 * @tparam T Numeric payload type.
 * @param v Delta comparison wrapper.
 * @return String in form `value +- delta`.
 */
template<typename T>
std::string toString(const DeltaCompare<T>& v) {
    return toString((T)v) + " +- " + toString(v.Delta());
}

/**
 * @brief Display conversion for `std::vector`.
 *
 * @tparam T Element type.
 * @param cont Vector value.
 * @return String in form `[e1, e2, ...]`.
 */
template<typename T>
std::string toString(const std::vector<T>& cont) {
    return Stringify(cont, to_stringer<T>(), "[", ", ", "]");
}

/**
 * @brief Display conversion for `std::list`.
 *
 * @tparam T Element type.
 * @param cont List value.
 * @return String in form `[e1, e2, ...]`.
 */
template<typename T>
std::string toString(const std::list<T>& cont) {
    return Stringify(cont, to_stringer<T>(), "[", ", ", "]");
}

/**
 * @brief Display conversion for tuples.
 *
 * @tparam Args Tuple element types.
 * @param t Tuple value.
 * @return String in form `[e1, e2, ...]`.
 */
template <class... Args>
std::string toString(const std::tuple<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toString, "[", ", ", "]");
    return ret;
}

/**
 * @brief Display conversion for pairs.
 *
 * @tparam Args Pair element types.
 * @param t Pair value.
 * @return String in form `[first, second]`.
 */
template <class... Args>
std::string toString(const std::pair<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toString, "[", ", ", "]");
    return ret;
}

/**
 * @brief Return function pointer to @ref toString overload for `T`.
 *
 * @tparam T Element type.
 * @return Pointer to `toString(const T&)`.
 */
template<typename T>
constexpr auto to_stringer() -> std::string(&)(const T&) {
    return toString;
}

} // gcheck
