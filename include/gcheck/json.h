/*
    Collection of functions for converting objects to JSON string format. 
*/

#pragma once

#include <string>
#include <vector>

#include "sfinae.h"

namespace gcheck {

namespace {
    
namespace detail{
    
    template<class T>
    static auto has_string_operator(int) -> sfinae_true<decltype(T::operator std::string)>;
    template<class T>
    static auto has_string_operator(long) -> sfinae_false<T>;
    template<class T>
    static auto has_tostring(int) -> sfinae_true<decltype(to_string(std::declval<T>()))>;
    template<class T>
    static auto has_tostring(long) -> sfinae_false<T>;
    template<class T>
    static auto has_std_tostring(int) -> sfinae_true<decltype(std::to_string(std::declval<T>()))>;
    template<class T>
    static auto has_std_tostring(long) -> sfinae_false<T>;
    template<class T>
    static auto has_tojson(int) -> sfinae_true<decltype(to_json(std::declval<T>()))>;
    template<class T>
    static auto has_tojson(long) -> sfinae_false<T>;
} // detail

template<class T>
struct has_string_operator : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_tostring : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_std_tostring : decltype(detail::has_std_tostring<T>(0)){};
template<class T>
struct has_tojson : decltype(detail::has_tojson<T>(0)){};

} // anonymous

struct TestReport;
struct CaseEntry;
enum TestStatus : int;
struct TestData;
class UserObject;

class JSON : public std::string {
public:
    JSON() : std::string("null") {}
    JSON(const std::string& str) : std::string("\"" + Escape(str) + "\"") {}
    JSON(const char* str) : JSON(std::string(str)) {}
    JSON(const std::string& key, const JSON& value) : std::string(JSON(key) + ":" + value) {}
    JSON(const std::string& key, const char* value) : JSON(key, JSON(value)) {}
    JSON(const JSON& str) = default;
    JSON(bool b) : std::string(b ? "true" : "false") {}
    
    JSON(const TestReport& r);
    JSON(const CaseEntry& e);
    JSON(const TestStatus& status);
    JSON(const TestData& data);
    JSON(const UserObject& o);
    
    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && !has_tostring<T>::value && !has_std_tostring<T>::value>, typename A = SFINAE, typename A2 = SFINAE, typename A3 = SFINAE>
    JSON(const T&) : JSON() {}
    
    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && !has_tostring<T>::value && has_std_tostring<T>::value>, typename A = SFINAE, typename A2 = SFINAE>
    JSON(const T& value) : JSON(std::to_string(value)) {}
    
    template<typename T, typename SFINAE = typename std::enable_if_t<!has_tojson<T>::value && has_tostring<T>::value>, typename A = SFINAE>
    JSON(const T& value) : JSON(to_string(value)) {}
    
    template<typename T, typename = typename std::enable_if_t<has_tojson<T>::value>>
    JSON(const T& value) : std::string(to_json(value)) {}
    
    template<typename T>
    JSON(const std::string& key, const T& value) : std::string("\"" + key + "\":" + JSON(value)) {}

    template<typename T>
    JSON(const std::vector<T>& v) {
        std::string ret = "[";
        for(auto it = v.begin(); it != v.end();) {
            ret += JSON(*it);
            if(++it != v.end())
                ret += ",";
        }
        ret += "]";
        
        Set(ret);
    }

    template<typename T>
    JSON(const std::vector<std::pair<std::string, T>>& v) {
        std::string ret = "{";

        for(auto it = v.begin(); it != v.end();) {
            ret += JSON(it->first, it->second);
            if(++it != v.end())
                ret += ",";
        }
        ret += "}";
        
        Set(ret);
    }
    
    template<typename T>
    JSON(const std::vector<std::pair<const char*, T>>& v) {
        std::string ret = "{";

        for(auto it = v.begin(); it != v.end();) {
            ret += JSON(it->first, it->second);
            if(++it != v.end())
                ret += ",";
        }
        ret += "}";
        
        Set(ret);
    }

    
    template<typename... Args>
    JSON(const std::tuple<Args...>& v) {
        std::string ret = "[";

        for_each(v, [&ret](int index, const auto& item) {
            ret += JSON(item);
            if(index != sizeof...(Args)-1)
                ret += ",";
        });
        ret += "]";
        
        Set(ret);
    }
    
    JSON(const std::tuple<>&) {
        Set("[]");
    }
    
    template<typename... Args>
    JSON(const std::tuple<std::pair<std::string, Args>...>& v) {
        std::string ret = "{";

        for_each(v, [&ret](int index, const auto& item) {
            ret += JSON(item.first, item.second);
            if(index != sizeof...(Args)-1)
                ret += ",";
        });
        ret += "}";
        
        Set(ret);
    }
    
    template<typename... Args>
    JSON(const std::tuple<std::pair<const char*, Args>...>& v) {
        std::string ret = "{";

        for_each(v, [&ret](int index, const auto& item) {
            ret += JSON(item.first, item.second);
            if(index != sizeof...(Args))
                ret += ",";
        });
        ret += "}";
        
        Set(ret);
    }

    JSON& Set(const std::string& str) {
        std::string::operator=(str);
        return *this;
    }

    JSON& operator=(const JSON& val) = default;
    template<typename T>
    JSON& operator=(const T& val) {
        return *this = JSON(val); 
    }
    
    // Escapes special JSON characters from 'str'
    static JSON Escape(std::string str);
    static std::string Unescape(const JSON& json) { return json.Unescape(); }
    
    std::string Unescape() const {
        std::string out = *this;
        size_t pos = 0;
        while((pos = out.find('\\', pos)) != npos) {
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

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template JSON::JSON(const std::string& key, const int& value);
extern template JSON::JSON(const int& v);
extern template JSON::JSON(const std::string& key, const unsigned int& value);
extern template JSON::JSON(const unsigned int& v);
extern template JSON::JSON(const std::string& key, const long& value);
extern template JSON::JSON(const long& v);
extern template JSON::JSON(const std::string& key, const unsigned long& value);
extern template JSON::JSON(const unsigned long& v);
extern template JSON::JSON(const std::string& key, const double& value);
extern template JSON::JSON(const double& v);
extern template JSON::JSON(const std::string& key, const float& value);
extern template JSON::JSON(const float& v);
extern template JSON::JSON(const std::string& key, const std::string& value);
extern template JSON::JSON(const std::string& v);

} // gcheck