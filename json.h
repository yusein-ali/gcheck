/*
    Collection of functions for converting objects to JSON string format. 
*/

#pragma once

#include <string>
#include <vector>

namespace gcheck {

// Escapes special JSON characters from 'str'
std::string JSONEscape(std::string str);
template<typename T>
std::string toJSON(T value);
template<typename T>
std::string toJSON(std::string key, T value);
std::string toJSON(std::string key, bool value);
std::string toJSON(std::string key, std::string value);
std::string toJSON(std::string key, const char* value);
std::string toJSON(const std::string& str);

struct TestReport;
struct CaseEntry;
struct TestData;
class UserObject;
std::string toJSON(const TestReport& r);
std::string toJSON(const CaseEntry& e);
std::string toJSON(const TestData& data);
std::string toJSON(const UserObject& o);

template<typename T>
std::string toJSON(T value) {
    return JSONEscape(std::to_string(value));
}

template<typename T>
std::string toJSON(std::string key, T value) {
    return "\"" + key + "\":" + JSONEscape(std::to_string(value));
}

template<typename T>
std::string toJSON(const std::vector<T>& v) {
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
std::string toJSON(std::vector<std::pair<std::string, T>> v) {
    
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