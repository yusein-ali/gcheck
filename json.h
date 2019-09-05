/*
    Collection of functions for converting objects to JSON string format. 
*/

#pragma once

#include <string>
#include <vector>

namespace gcheck {
    
namespace detail{
    
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
}

template<class T>
struct has_string_operator : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_tostring : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_std_tostring : decltype(detail::has_std_tostring<T>(0)){};
template<class T>
struct has_tojson : decltype(detail::has_tojson<T>(0)){};

struct JSON : public std::string {
    JSON() {}
    JSON(const std::string& str) : std::string(str) {}
    JSON(const char* str) : std::string(str) {}
};

// Escapes special JSON characters from 'str'
JSON JSONEscape(std::string str);
template<typename T>
JSON toJSON(const std::string& key, const T& value);
JSON toJSON(const std::string& key, const JSON& value);
JSON toJSON(const std::string& key, const char* value);
JSON toJSON(const std::string& str);
JSON toJSON(const JSON& str);
JSON toJSON(bool b);

struct TestReport;
struct CaseEntry;
struct TestData;
class UserObject;
JSON toJSON(const TestReport& r);
JSON toJSON(const CaseEntry& e);
JSON toJSON(const TestData& data);
JSON toJSON(const UserObject& o);

template<typename T>
typename std::enable_if<!has_tojson<T>::value && !has_tostring<T>::value && !has_std_tostring<T>::value, JSON>::type
toJSON(const T&) {
    return "null";
}
template<typename T>
typename std::enable_if<!has_tojson<T>::value && !has_tostring<T>::value && has_std_tostring<T>::value, JSON>::type
toJSON(const T& value) {
    return JSONEscape(std::to_string(value));
}
template<typename T>
typename std::enable_if<!has_tojson<T>::value && has_tostring<T>::value, JSON>::type
toJSON(const T& value) {
    return toJSON(to_string(value));
}
template<typename T>
typename std::enable_if<has_tojson<T>::value, JSON>::type
toJSON(const T& value) {
    return to_json(value);
}

template<typename T>
JSON toJSON(const std::string& key, const T& value) {
    return "\"" + key + "\":" + toJSON(value);
}

template<typename T>
JSON toJSON(const std::vector<T>& v) {
    std::string out = "[";
    
    for(auto it = v.begin(); it != v.end();) {
        out += toJSON(*it);
        if(++it != v.end())
            out += ", ";
    }
    
    out += "]";
    return out;
}

template<typename T>
JSON toJSON(const std::vector<std::pair<std::string, T>>& v) {
    
    std::string out = "{";

    for(auto it = v.begin(); it != v.end();) {

        out += "\"" + JSONEscape(it->first) + "\":";
        out += toJSON(it->second);

        if(++it != v.end())
            out += ", ";
    }
    out += "}";
    
    return out;
}

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template JSON toJSON(const std::string& key, const int& value);
extern template JSON toJSON(const int& v);
extern template JSON toJSON(const std::string& key, const unsigned int& value);
extern template JSON toJSON(const unsigned int& v);
extern template JSON toJSON(const std::string& key, const long& value);
extern template JSON toJSON(const long& v);
extern template JSON toJSON(const std::string& key, const unsigned long& value);
extern template JSON toJSON(const unsigned long& v);
extern template JSON toJSON(const std::string& key, const double& value);
extern template JSON toJSON(const double& v);
extern template JSON toJSON(const std::string& key, const float& value);
extern template JSON toJSON(const float& v);
extern template JSON toJSON(const std::string& key, const std::string& value);
extern template JSON toJSON(const std::string& v);

}