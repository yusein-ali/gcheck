#include "utility.h"

namespace gcheck {

std::string to_string(bool b) {
    return std::string(b ? "true" : "false");
}

JSON& JSON::Remove(std::string key) {
    items_.erase(key);
    return *this;
}

std::any& JSON::Get(std::string key) {
    auto it = items_.find(key);
    if(it != items_.end()) {
        return it->second;
    } else
        throw; //TODO: proper exception
}

std::any JSON::Get(std::string key) const {
    auto it = items_.find(key);
    if(it != items_.end()) {
        return it->second;
    } else
        throw; //TODO: proper exception
}

std::string JSON::AsString(const std::any& item) {
    std::string out;
    if(auto s = std::any_cast<bool>(&item))
        out += to_string(*s);
    else if(auto s = std::any_cast<int>(&item))
        out += std::to_string(*s);
    else if(auto s = std::any_cast<char>(&item))
        out += '\"' + Escape(std::string()+*s) + '\"';
    else if(auto s = std::any_cast<double>(&item))
        out += std::to_string(*s);
    else if(auto s = std::any_cast<std::string>(&item))
        out += "\"" + Escape(*s) + "\"";
    else if(auto s = std::any_cast<JSON>(&item))
        out += s->AsString();
    else if(auto s = std::any_cast<std::vector<JSON>>(&item)) {
        out += "[";
        for(auto it2 = s->begin(); it2 != s->end();) {
            out += it2->AsString();
            if(++it2 != s->end())
                out += ", ";
        }
        out += "]";
    } else if(auto s = std::any_cast<std::vector<std::any>>(&item)) {
        out += "[";
        for(auto it2 = s->begin(); it2 != s->end();) {
            out += AsString(*it2);
            if(++it2 != s->end())
                out += ", ";
        }
        out += "]";
    } else
        out += "null";

    return out;//Escape(out);
}

std::string JSON::AsString(bool strip_ends) const {

    std::string out;
    if(!strip_ends) out += "{";

    for(auto it = items_.begin(); it != items_.end();) {

        out += "\"" + it->first + "\":";
        out += AsString(it->first);

        if(++it != items_.end())
            out += ", ";
    }
    if(!strip_ends) out += "}";
    return out;
}

std::string JSON::AsString(const char* key) const {
    return AsString(std::string(key));
}

std::string JSON::AsString(std::string key) const {
    return JSON::AsString(Get(key));
}

std::string JSON::Escape(std::string str) {
    
    std::string escapees = "\\\n\t\b\f\r\"";
    std::vector<std::string> replacees{"\\\\", "\\n", "\\t", "\\b", "\\f", "\\r", "\\\""};
    size_t pos = 0;
    int index = 0;
    size_t min = std::string::npos;
    for(unsigned int i = 0; i < escapees.length(); i++) {
        size_t p = str.find(escapees[i], pos);
        if(p < min) {
            min = p;
            index = i;
        }
    }
    pos = min;
    while(pos != std::string::npos) {
        str.replace(pos, 1, replacees[index]);
        pos += replacees[index].length();
        
        index = 0;
        min = std::string::npos;
        for(unsigned int i = 0; i < escapees.length(); i++) {
            size_t p = str.find(escapees[i], pos);
            if(p < min) {
                min = p;
                index = i;
            }
        }
        pos = min;
    }
    
    return str;
}

}