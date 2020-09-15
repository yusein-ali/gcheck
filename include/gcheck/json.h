/*
    Collection of functions for converting objects to JSON string format.
*/

#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "sfinae.h"

namespace gcheck {

namespace {

namespace detail{
    template<class T>
    static auto has_tojson(int) -> sfinae_true<decltype(to_json(std::declval<T>()))>;
    template<class T>
    static auto has_tojson(long) -> sfinae_false<T>;
} // detail

template<class T>
struct has_tojson : decltype(detail::has_tojson<T>(0)){};

} // anonymous

template<template<typename> class allocator>
class _TestReport;

template<template<typename> class allocator>
struct _CaseEntry;

template<template<typename> class allocator>
struct _FunctionEntry;

enum TestStatus : int;
template<template<typename> class allocator>
struct _TestData;

template<template<typename> class allocator>
class _UserObject;

class Prerequisite;

template<template<typename> class allocator = std::allocator>
class _JSON : public std::basic_string<char, std::char_traits<char>, allocator<char>> {
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> string;
public:
    _JSON() : string("null") {}
    _JSON(const _JSON& json) = default;
    template<template<typename> class T>
    _JSON(const _JSON<T>& json) : string(json) {}
};

template<>
class _JSON<std::allocator> : public std::string {
    typedef std::string string;
public:
    _JSON() : string("null") {}
    _JSON(const _JSON& json) = default;
    template<template<typename> class T>
    _JSON(const _JSON<T>& json) : string(json) {}
    _JSON(const std::string& str) : string("\"" + Escape(str) + "\"") {}
    _JSON(const char* str) : _JSON(std::string(str)) {}
    _JSON(const std::string& key, const _JSON& value) : string(_JSON(key) + ":" + value) {}
    _JSON(const std::string& key, const char* value) : _JSON(key, _JSON(value)) {}
    _JSON(bool b) : string(b ? "true" : "false") {}

    _JSON(const _TestReport<std::allocator>& r);
    _JSON(const _CaseEntry<std::allocator>& e);
    _JSON(const _FunctionEntry<std::allocator>& e);
    _JSON(const _TestData<std::allocator>& data);
    _JSON(const _UserObject<std::allocator>& o);
    _JSON(const TestStatus& status);
    _JSON(const Prerequisite& o);

    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && !has_tostring<T>::value && !has_std_tostring<T>::value>, typename A = SFINAE, typename A2 = SFINAE, typename A3 = SFINAE>
    _JSON(const T&) : _JSON() {}

    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && !has_tostring<T>::value && has_std_tostring<T>::value>, typename A = SFINAE, typename A2 = SFINAE>
    _JSON(const T& value) : string(std::to_string(value)) {}

    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && has_tostring<T>::value>, typename A = SFINAE>
    _JSON(const T& value) : _JSON(to_string(value)) {}

    template<typename T, typename = typename std::enable_if_t<has_tojson<T>::value>>
    _JSON(const T& value) : string(to_json(value)) {}

    template<typename T>
    _JSON(const std::string& key, const T& value) : string("\"" + key + "\":" + _JSON(value)) {}

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

    _JSON(const std::tuple<>&) {
        Set("[]");
    }

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

    _JSON& Set(const std::string& str) {
        string::operator=(str);
        return *this;
    }

    _JSON& operator=(const _JSON& val) = default;
    template<typename T>
    _JSON& operator=(const T& val) {
        return *this = _JSON(val);
    }

    // Escapes special JSON characters from 'str'
    static _JSON Escape(std::string str);
    static std::string Unescape(const _JSON& json) { return json.Unescape(); }

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