/*
    Collection of functions for converting objects to JSON string format. 
*/

#pragma once

#include <string>
#include <vector>

namespace gcheck {

struct JSON : public std::string {
    JSON() {}
    JSON(const std::string& str) : std::string(str) {}
    JSON(const char* str) : std::string(str) {}
};

// Escapes special JSON characters from 'str'
JSON JSONEscape(std::string str);
template<typename T>
std::string toJSON(T value);
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
JSON toJSON(T value) {
    return JSONEscape(std::to_string(value));
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

}