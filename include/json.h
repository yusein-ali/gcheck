/**
 * @file json.h
 * @brief JSON conversion utilities and generic JSON string wrapper for gcheck.
 *
 * This header provides:
 * - SFINAE-based detection for custom `to_json(T)` support.
 * - The `_JSON` template, a string-backed JSON representation.
 * - Rich constructors that convert primitive types, tuples, containers,
 *   maps, and gcheck-specific result types into JSON text.
 *
 * Typical usage:
 * 1. Construct `gcheck::JSON` from primitive values, tuples, vectors, maps.
 * 2. Construct object-like JSON from key/value pairs using
 *    `JSON("key", value)`.
 * 3. Extend serialization for custom types by defining `to_json(const T&)`.
 */

#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <map>

#include "sfinae.h"

namespace gcheck {

namespace {

namespace detail{
    /**
     * @brief SFINAE probe for unqualified `to_json(T)` availability.
     *
     * @tparam T Type under test.
     * @param Unused overload-rank marker.
     * @return @ref sfinae_true when `to_json(std::declval<T>())` is valid.
     */
    template<class T>
    static auto has_tojson(int) -> sfinae_true<decltype(to_json(std::declval<T>()))>;

    /**
     * @brief Fallback probe when `to_json(T)` is not available.
     *
     * @tparam T Type under test.
     * @param Unused overload-rank marker.
     * @return @ref sfinae_false fallback.
     */
    template<class T>
    static auto has_tojson(long) -> sfinae_false<T>;
} // detail

/**
 * @brief Trait that checks support for unqualified `to_json(T)`.
 *
 * @tparam T Type under test.
 *
 * Value semantics:
 * - `has_tojson<T>::value == true` if `to_json(T)` is callable.
 * - `false` otherwise.
 *
 * Typical usage:
 * - Select custom JSON conversion constructor in @ref _JSON.
 */
template<class T>
struct has_tojson : decltype(detail::has_tojson<T>(0)){};

} // anonymous

/** @brief Forward declaration of test report type used by JSON constructors. */
struct TestReport;

/** @brief Forward declaration of case entry type used by JSON constructors. */
struct CaseEntry;

/** @brief Forward declaration of function entry type used by JSON constructors. */
struct FunctionEntry;

/** @brief Forward declaration of test status enum serialized by JSON constructors. */
enum TestStatus : int;

/** @brief Forward declaration of test data type used by JSON constructors. */
struct TestData;

/** @brief Forward declaration of user object wrapper serialized by JSON constructors. */
class UserObject;

/** @brief Forward declaration of process/fork status enum serialized by JSON constructors. */
enum ForkStatus : unsigned int;

/** @brief Forward declaration of prerequisite metadata serialized by JSON constructors. */
class Prerequisite;

/**
 * @brief Generic allocator-aware JSON wrapper base template.
 *
 * @tparam allocator Character allocator template used by base string type.
 *
 * This generic variant only provides null/default and copy construction.
 * The specialization for `std::allocator` provides full conversion behavior.
 */
template<template<typename> class allocator = std::allocator>
class _JSON : public std::basic_string<char, std::char_traits<char>, allocator<char>> {
    /** @brief Underlying allocator-aware string alias. */
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> string;
public:
    /** @brief Construct JSON `null`. */
    _JSON() : string("null") {}

    /** @brief Copy constructor. */
    _JSON(const _JSON& json) = default;

    /**
     * @brief Cross-allocator copy constructor.
     *
     * @tparam T Source allocator template.
     * @param json Source JSON value.
     */
    template<template<typename> class T>
    _JSON(const _JSON<T>& json) : string(json) {}
};

/**
 * @brief Full JSON wrapper specialization using `std::string` storage.
 *
 * This class is the primary JSON API (`using JSON = _JSON<>`).
 * It stores serialized JSON text directly as a string and provides many
 * constructors for automatic conversion.
 *
 * Conversion priority for generic `T` constructors:
 * 1. If `to_json(T)` exists, use it.
 * 2. Else if ADL `to_string(T)` exists, quote that string.
 * 3. Else if `std::to_string(T)` exists, use numeric-like string.
 * 4. Else fallback to `null`.
 *
 * Typical usage:
 * - `JSON j = 42;`
 * - `JSON j = JSON("points", 10);`
 * - `JSON j = std::vector<int>{1,2,3};`
 * - `JSON j = std::tuple{"name", 12};`
 */
template<>
class _JSON<std::allocator> : public std::string {
    /** @brief Underlying string alias. */
    typedef std::string string;
public:
    /** @brief Construct JSON `null`. */
    _JSON() : string("null") {}

    /** @brief Copy constructor. */
    _JSON(const _JSON& json) = default;

    /**
     * @brief Cross-allocator copy constructor.
     *
     * @tparam T Source allocator template.
     * @param json Source JSON value.
     */
    template<template<typename> class T>
    _JSON(const _JSON<T>& json) : string(json) {}

    /**
     * @brief Construct JSON string value from `std::string`.
     *
     * @param str Raw string content.
     *
     * Escapes content and wraps it in quotes.
     */
    _JSON(const std::string& str) : string("\"" + Escape(str) + "\"") {}

    /**
     * @brief Construct JSON string value from C string.
     *
     * @param str Null-terminated C string.
     */
    _JSON(const char* str);

    /**
     * @brief Construct JSON object field from key and JSON value.
     *
     * @param key Field name.
     * @param value Field JSON value.
     */
    _JSON(const std::string& key, const _JSON& value) : string(_JSON(key) + ":" + value) {}

    /**
     * @brief Construct JSON object field from key and C-string value.
     *
     * @param key Field name.
     * @param value String field value.
     */
    _JSON(const std::string& key, const char* value) : _JSON(key, _JSON(value)) {}

    /**
     * @brief Construct JSON boolean value.
     *
     * @param b Boolean input.
     */
    _JSON(bool b) : string(b ? "true" : "false") {}

    /** @brief Construct JSON from gcheck test report record. */
    _JSON(const TestReport& r);
    /** @brief Construct JSON from gcheck case entry record. */
    _JSON(const CaseEntry& e);
    /** @brief Construct JSON from gcheck function entry record. */
    _JSON(const FunctionEntry& e);
    /** @brief Construct JSON from gcheck test data aggregate. */
    _JSON(const TestData& data);
    /** @brief Construct JSON from gcheck user object. */
    _JSON(const UserObject& o);
    /** @brief Construct JSON from test status enum. */
    _JSON(const TestStatus& status);
    /** @brief Construct JSON from prerequisite metadata. */
    _JSON(const Prerequisite& o);
    /** @brief Construct JSON from fork status enum. */
    _JSON(const ForkStatus& s);

    /**
     * @brief Fallback constructor for types without JSON or string conversion hooks.
     *
     * @tparam T Input type.
     *
     * Produces JSON `null` by delegating to default constructor.
     */
    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && !has_tostring<T>::value && !has_std_tostring<T>::value>, typename A = SFINAE, typename A2 = SFINAE, typename A3 = SFINAE>
    _JSON(const T&) : _JSON() {}

    /**
     * @brief Constructor for types supporting `std::to_string` only.
     *
     * @tparam T Input type.
     * @param value Input value.
     *
     * Intended for numeric-like values represented directly in JSON text.
     */
    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && !has_tostring<T>::value && has_std_tostring<T>::value>, typename A = SFINAE, typename A2 = SFINAE>
    _JSON(const T& value) : string(std::to_string(value)) {}

    /**
     * @brief Constructor for types supporting ADL-visible `to_string(T)`.
     *
     * @tparam T Input type.
     * @param value Input value.
     *
     * Wraps resulting text as a quoted JSON string value.
     */
    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && has_tostring<T>::value>, typename A = SFINAE>
    _JSON(const T& value) : _JSON(to_string(value)) {}

    /**
     * @brief Constructor for types supporting custom `to_json(T)`.
     *
     * @tparam T Input type.
     * @param value Input value.
     */
    template<typename T, typename = typename std::enable_if_t<has_tojson<T>::value>>
    _JSON(const T& value) : string(to_json(value)) {}

    /**
     * @brief Construct object field from key and arbitrary value.
     *
     * @tparam T Value type.
     * @param key Field name.
     * @param value Field value converted with `_JSON(value)`.
     */
    template<typename T>
    _JSON(const std::string& key, const T& value) : string("\"" + key + "\":" + _JSON(value)) {}

    /**
     * @brief Construct JSON array from generic iterable container.
     *
     * @tparam C Container template.
     * @tparam Args Container template arguments.
     * @param c Container value.
     *
     * Enabled only when container has `begin()` and `end()`.
     */
    template<template<typename...> class C, typename... Args, class = std::enable_if_t<has_begin_end<C<Args...>>::value>>
    _JSON(const C<Args...>& c) {
        std::string ret = "[";
        for(auto it = c.begin(); it != c.end();) {
            ret += _JSON(*it);
            if(++it != c.end())
                ret += ",";
        }
        ret += "]";

        Set(ret);
    }

    /**
     * @brief Construct JSON object from iterable container of `pair<string, value>`.
     *
     * @tparam C Container template.
     * @tparam Args Pair value template arguments.
     * @param c Container value.
     */
    template<template<typename...> class C, typename... Args, class = std::enable_if_t<has_begin_end<C<std::pair<std::string, Args...>>>::value>>
    _JSON(const C<std::pair<std::string, Args...>>& c) {
        std::string ret = "{";

        for(auto it = c.begin(); it != c.end();) {
            ret += _JSON(it->first, it->second);
            if(++it != c.end())
                ret += ",";
        }
        ret += "}";

        Set(ret);
    }

    /**
     * @brief Construct JSON array from vector.
     *
     * @tparam T Element type.
     * @param v Vector value.
     */
    template<typename T>
    _JSON(const std::vector<T>& v) {
        std::string ret = "[";
        for(auto it = v.begin(); it != v.end();) {
            ret += _JSON(*it);
            if(++it != v.end())
                ret += ",";
        }
        ret += "]";

        Set(ret);
    }

    /**
     * @brief Construct JSON object from vector of `(string, value)` pairs.
     *
     * @tparam T Value type.
     * @param v Vector of object fields.
     */
    template<typename T>
    _JSON(const std::vector<std::pair<std::string, T>>& v) {
        std::string ret = "{";

        for(auto it = v.begin(); it != v.end();) {
            ret += _JSON(it->first, it->second);
            if(++it != v.end())
                ret += ",";
        }
        ret += "}";

        Set(ret);
    }

    /**
     * @brief Construct JSON object from vector of `(const char*, value)` pairs.
     *
     * @tparam T Value type.
     * @param v Vector of object fields.
     */
    template<typename T>
    _JSON(const std::vector<std::pair<const char*, T>>& v) {
        std::string ret = "{";

        for(auto it = v.begin(); it != v.end();) {
            ret += _JSON(it->first, it->second);
            if(++it != v.end())
                ret += ",";
        }
        ret += "}";

        Set(ret);
    }


    /**
     * @brief Construct JSON object from `std::map<std::string, T>`.
     *
     * @tparam T Value type.
     * @param m Map of fields.
     */
    template<typename T>
    _JSON(const std::map<std::string, T>& m) {
        std::string ret = "{";

        for(auto it = m.begin(); it != m.end();) {
            ret += _JSON(it->first, it->second);
            if(++it != m.end())
                ret += ",";
        }
        ret += "}";

        Set(ret);
    }


    /**
     * @brief Construct JSON array from tuple elements.
     *
     * @tparam Args Tuple element types.
     * @param v Tuple value.
     */
    template<typename... Args>
    _JSON(const std::tuple<Args...>& v) {
        std::string ret = "[";

        for_each(v, [&ret](int index, const auto& item) {
            ret += _JSON(item);
            if(index != sizeof...(Args)-1)
                ret += ",";
        });
        ret += "]";

        Set(ret);
    }

    /**
     * @brief Construct JSON empty array from empty tuple.
     *
     * @param Unused empty tuple value.
     */
    _JSON(const std::tuple<>&) {
        Set("[]");
    }

    /**
     * @brief Construct JSON object from tuple of `(string, value)` pairs.
     *
     * @tparam Args Value types in pair tuple.
     * @param v Tuple of object fields.
     */
    template<typename... Args>
    _JSON(const std::tuple<std::pair<std::string, Args>...>& v) {
        std::string ret = "{";

        for_each(v, [&ret](int index, const auto& item) {
            ret += _JSON(item.first, item.second);
            if(index != sizeof...(Args)-1)
                ret += ",";
        });
        ret += "}";

        Set(ret);
    }

    /**
     * @brief Construct JSON object from tuple of `(const char*, value)` pairs.
     *
     * @tparam Args Value types in pair tuple.
     * @param v Tuple of object fields.
     */
    template<typename... Args>
    _JSON(const std::tuple<std::pair<const char*, Args>...>& v) {
        std::string ret = "{";

        for_each(v, [&ret](int index, const auto& item) {
            ret += _JSON(item.first, item.second);
            if(index != sizeof...(Args))
                ret += ",";
        });
        ret += "}";

        Set(ret);
    }

    /**
     * @brief Assign raw JSON text.
     *
     * @param str Serialized JSON text.
     * @return `*this`.
     */
    _JSON& Set(const std::string& str) {
        string::operator=(str);
        return *this;
    }

    /** @brief Default copy assignment. */
    _JSON& operator=(const _JSON& val) = default;

    /**
     * @brief Assign from arbitrary value by re-constructing JSON.
     *
     * @tparam T Value type.
     * @param val Source value.
     * @return `*this`.
     */
    template<typename T>
    _JSON& operator=(const T& val) {
        return *this = _JSON(val);
    }

    /**
     * @brief Escape special JSON characters in plain text.
     *
     * @param str Raw text.
     * @return Escaped JSON-string-safe text.
     */
    static _JSON Escape(std::string str);

    /**
     * @brief Static helper to unescape JSON string text.
     *
     * @param json JSON string wrapper.
     * @return Unescaped string.
     */
    static std::string Unescape(const _JSON& json) { return json.Unescape(); }

    /**
     * @brief Unescape current JSON string content.
     *
     * @return Decoded string where escaped code points and escapes are resolved.
     */
    std::string Unescape() const {
        std::string out = *this;
        size_t pos = 0;
        while((pos = out.find('\\', pos)) != string::npos) {
            if(out[pos+1] == 'u') {
                char c;
                std::sscanf(out.c_str()+pos+2, "%4hhX", (unsigned char*)&c);
                out.replace(pos, 6, &c, 1);
            } else {
                out.erase(pos);
            }
        }
        return out;
    }
};

/**
 * @brief Default JSON alias used throughout the project.
 */
using JSON = _JSON<>;

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template JSON::_JSON(const std::string& key, const int& value);
extern template JSON::_JSON(const int& v);
extern template JSON::_JSON(const std::string& key, const unsigned int& value);
extern template JSON::_JSON(const unsigned int& v);
extern template JSON::_JSON(const std::string& key, const long& value);
extern template JSON::_JSON(const long& v);
extern template JSON::_JSON(const std::string& key, const unsigned long& value);
extern template JSON::_JSON(const unsigned long& v);
extern template JSON::_JSON(const std::string& key, const double& value);
extern template JSON::_JSON(const double& v);
extern template JSON::_JSON(const std::string& key, const float& value);
extern template JSON::_JSON(const float& v);
extern template JSON::_JSON(const std::string& key, const std::string& value);

} // gcheck
