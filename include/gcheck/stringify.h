#pragma once

#include <string>
#include <functional>
#include <vector>
#include <tuple>
#include <sstream>

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

template<template<typename> class C, typename T, typename Func>
std::string Stringify(const C<T>& container, Func func, const std::string& start, const std::string& separator, const std::string& end) {
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

class UserObject;

std::string toConstruct(const char* item);
std::string toConstruct(char* item);
std::string toConstruct(const std::string& item);
std::string toConstruct(unsigned char item);
std::string toConstruct(char item);
std::string toConstruct(bool b);
std::string toConstruct(const UserObject& u);
std::string toConstruct(decltype(nullptr));

template<typename T>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(T) { return ""; }

template<typename T>
typename std::enable_if<!has_toconstruct<T*>::value, std::string>::type
toConstruct(const T* item) {
    if(item == nullptr)
        return "nullptr";

    return "new " + toConstruct(*item);
}

template<typename T = int>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(int item) { return std::to_string(item); }

template<typename T = long>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(long item) { return std::to_string(item) + 'L'; }

template<typename T = long long>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(long long item) { return std::to_string(item) + "LL"; }

template<typename T = unsigned>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(unsigned item) { return std::to_string(item) + 'U'; }

template<typename T = unsigned long>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(unsigned long item) { return std::to_string(item) + "UL"; }

template<typename T = unsigned long long>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(unsigned long long item) { return std::to_string(item) + "ULL"; }

template<typename T = float>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(float item) {
    std::stringstream ss;
    ss << "std::stof(" << std::hexfloat << item << ')';
    return ss.str();
}

template<typename T = double>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(double item) {
    std::stringstream ss;
    ss << "std::stod(" << std::hexfloat << item << ')';
    return ss.str();
}

template<typename T = long double>
typename std::enable_if<!has_toconstruct<T>::value, std::string>::type
toConstruct(long double item) {
    std::stringstream ss;
    ss << "std::stold(" << std::hexfloat << item << ')';
    return ss.str();
}

template<typename T>
typename std::enable_if<has_toconstruct<T>::value, std::string>::type
toConstruct(T item) { return to_construct(item); }

template<typename T>
std::string toConstruct(const std::vector<T>& cont) {
    return Stringify(cont, toConstruct, "std::vector({", ",", "})");
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



std::string toString(const std::string& item);
std::string toString(const char* item);
std::string toString(char* item);
std::string toString(char item);
std::string toString(bool b);
std::string toString(const UserObject& u);
std::string toString(decltype(nullptr));

template<typename T>
typename std::enable_if<!has_tostring<T>::value && !has_std_tostring<T>::value>::type
toString(T) { return ""; }

template<typename T>
typename std::enable_if<!has_tostring<T*>::value && !has_std_tostring<T*>::value>::type
toString(T* item) {
    if(item == nullptr)
        return "nullptr";

    std::stringstream ss;
    ss << static_cast<const void*>(item);
    return ss.str();
}
/*template<typename T>
typename std::enable_if<!has_tostring<T>::value && has_std_tostring<T>::value>::type
toString(T item) {
    as_string_ = std::to_string(item);
}

For some reason the above doesn't work on all systems, so make it explicit*/
template<typename T = int>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(int item) { return std::to_string(item); }

template<typename T = long>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(long item) { return std::to_string(item); }

template<typename T = long long>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(long long item) { return std::to_string(item); }

template<typename T = unsigned>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(unsigned item) { return std::to_string(item); }

template<typename T = unsigned long>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(unsigned long item) { return std::to_string(item); }

template<typename T = unsigned long long>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(unsigned long long item) { return std::to_string(item); }

template<typename T = float>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(float item) { return std::to_string(item); }

template<typename T = double>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(double item) { return std::to_string(item); }

template<typename T = long double>
typename std::enable_if<!has_tostring<T>::value, std::string>::type
toString(long double item) { return std::to_string(item); }

template<typename T>
typename std::enable_if<has_tostring<T>::value, std::string>::type
toString(T item) { return to_string(item); }

template<typename T>
std::string toString(const std::vector<T>& cont) {
    return Stringify(cont, toString, "[", ", ", "]");
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

}