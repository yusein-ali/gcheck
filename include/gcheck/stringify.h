#pragma once

#include <string>
#include <functional>
#include <vector>
#include <tuple>
#include <sstream>
#include <list>

#include "sfinae.h"

namespace gcheck {
namespace {
namespace detail {
    template<class T>
    static auto has_toconstruct(int) -> sfinae_true<decltype(to_construct(std::declval<T>()))>;
    template<class T>
    static auto has_toconstruct(long) -> sfinae_false<T>;
} // detail

template<class T>
struct has_toconstruct : decltype(detail::has_toconstruct<T>(0)) {};

} // anonymous

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

// Escape non-utf8 characters and special characters e.g. \n and \t
std::string JSONEscape(std::string);

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

template<template<typename> class allocator>
class _UserObject;
using UserObject = _UserObject<std::allocator>;
template<typename T>
class DeltaCompare;

template<typename T>
constexpr auto to_constructer() -> std::string(&)(const T&);

std::string toConstruct(const char* const& item);
std::string toConstruct(const char*& item);
std::string toConstruct(const std::string& item);
std::string toConstruct(const unsigned char& item);
std::string toConstruct(const char& item);
std::string toConstruct(const bool& b);
std::string toConstruct(const UserObject& u);
std::string toConstruct(decltype(nullptr));
std::string toConstruct(const int& item);
std::string toConstruct(const long& item);
std::string toConstruct(const long long& item);
std::string toConstruct(const unsigned& item);
std::string toConstruct(const unsigned long& item);
std::string toConstruct(const unsigned long long& item);
std::string toConstruct(const float& item);
std::string toConstruct(const double& item);
std::string toConstruct(const long double& item);

template<typename T>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(const T&) { return ""; }

template<typename T>
typename std::enable_if<!has_toconstruct<T*>::value, std::string>::type
toConstruct(const T*& item) {
    if(item == nullptr)
        return "nullptr";

    return "new " + toConstruct(*item);
}

template<typename T>
typename std::enable_if<has_toconstruct<T>::value, std::string>::type
toConstruct(const T& item) { return to_construct(item); }

template<typename T>
std::string toConstruct(const DeltaCompare<T>& v) {
    return "gcheck::DeltaCompare(" + toConstruct((T)v) + "," + toConstruct(v.Delta()) + ")";;
}

template<typename T, typename... Args>
std::string toConstruct(const std::vector<T, Args...>& cont) {
    return Stringify(cont, to_constructer<T>(), "std::vector<" + type_name<T>() + ">({", ",", "})");
}

template<typename T>
std::string toConstruct(const std::list<T>& cont) {
    return Stringify(cont, to_constructer<T>(), "std::list<" + type_name<T>() + ">({", ",", "})");
}

template <class... Args>
std::string toConstruct(const std::tuple<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toConstruct, "std::tuple(", ",", ")");
    return ret;
}

template <class... Args>
std::string toConstruct(const std::pair<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toConstruct, "std::pair(", ",", ")");
    return ret;
}

template<typename T>
constexpr auto to_constructer() -> std::string(&)(const T&) {
    return toConstruct;
}


template<typename T>
constexpr auto to_stringer() -> std::string(&)(const T&);

std::string toString(const std::string& item);
std::string toString(const char* const& item);
std::string toString(const char*& item);
std::string toString(const char& item);
std::string toString(const bool& b);
std::string toString(const UserObject& u);
std::string toString(decltype(nullptr));
std::string toString(const int& item);
std::string toString(const long& item);
std::string toString(const long long& item);
std::string toString(const unsigned& item);
std::string toString(const unsigned long& item);
std::string toString(const unsigned long long& item);
std::string toString(const float& item);
std::string toString(const double& item);
std::string toString(const long double& item);

template<typename T>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(const T&) { return "error-type"; }

template<typename T>
std::string toString(T* const& item) {
    if(item == nullptr)
        return "nullptr";

    std::stringstream ss;
    ss << static_cast<const void*>(item);
    return ss.str();
}

template<typename T>
typename std::enable_if<has_tostring<T>::value, std::string>::type
toString(const T& item) { return to_string(item); }

template<typename T>
std::string toString(const DeltaCompare<T>& v) {
    return toString((T)v) + " +- " + toString(v.Delta());
}

template<typename T>
std::string toString(const std::vector<T>& cont) {
    return Stringify(cont, to_stringer<T>(), "[", ", ", "]");
}

template<typename T>
std::string toString(const std::list<T>& cont) {
    return Stringify(cont, to_stringer<T>(), "[", ", ", "]");
}


template <class... Args>
std::string toString(const std::tuple<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toString, "[", ", ", "]");
    return ret;
}

template <class... Args>
std::string toString(const std::pair<Args...>& t) {
    std::string ret;
    StringifyTuple(ret, t, toString, "[", ", ", "]");
    return ret;
}

template<typename T>
constexpr auto to_stringer() -> std::string(&)(const T&) {
    return toString;
}

}