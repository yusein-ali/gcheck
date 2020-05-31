#include <string>

#include "stringify.h"
#include "user_object.h"

namespace gcheck {
    
std::string toConstruct(const char* item) { return std::string(1, '"') + item + '"'; }
std::string toConstruct(char* item) { return (const char*)item; }
std::string toConstruct(const std::string& item) { 
    return "std::string(" + toConstruct((const char*)item.c_str()) + ")"; 
}
std::string toConstruct(unsigned char item) { return "static_cast<unsigned char>(" + std::to_string((int)item) + ")"; }
std::string toConstruct(char item) { return "static_cast<unsigned char>(" + std::to_string((int)item) + ")"; }
std::string toConstruct(bool b) { return b ? "true" : "false"; }
std::string toConstruct(const UserObject& u) { return u.construct(); }
std::string toConstruct(decltype(nullptr)) { return "nullptr"; }

std::string toString(const std::string& item) { return '"' + item + '"'; }
std::string toString(const char* item) { return toString(std::string(item)); }
std::string toString(char* item) { return toString((const char*)item); }
std::string toString(char item) { return std::string("'") + item + "'"; }
std::string toString(bool b) { return b ? "true" : "false"; }
std::string toString(const UserObject& u) { return u.string(); }
std::string toString(decltype(nullptr)) { return "nullptr"; }
    
} // gcheck