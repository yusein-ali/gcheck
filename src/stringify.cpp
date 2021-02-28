#include <string>
#include <stack>
#include <cstring>
#include <sstream>
#include <array>

#include "stringify.h"
#include "user_object.h"

namespace gcheck {

std::array<std::string, 256> replacee_array() {
    static const char* digits = "0123456789ABCDEF";

    std::array<std::string, 256> replacees;

    for (int i = 0; i < 256; ++i) {
        replacees[i] = "\\u0000";
        for (size_t j = 0; j < 2; ++j)
            replacees[i][j+4] = digits[(i >> 4*(1-j)) & 0xf];
    }

    return replacees;
}
std::string Replacee(unsigned char val) {
    static const std::array<std::string, 256> replacees = replacee_array();
    return replacees[val];
}

std::string JSONEscape(std::string str) {
    // find non-utf-8 characters
    for(size_t pos = 0; pos < str.length(); pos++) {
        if((unsigned char)str[pos] == 0xC7) {
            break;
        }
    }

    std::stack<size_t> positions;
    for(size_t pos = 0; pos < str.length(); pos++) {
        const unsigned char val = str[pos];
        if(val < 0x20 || val == '\\' || val == '\"') {
            positions.push(pos);
            continue;
        } else if(!(val & 0b10000000)) {
            continue;
        }
        int expected = 0;
        if(val >> 6 == 0b10) {
            positions.push(pos);
            continue;
        } else if(val >> 5 == 0b110) {
            expected = 1;
        } else if(val >> 4 == 0b1110) {
            expected = 2;
        } else if(val >> 3 == 0b11110) {
            expected = 3;
        } else {
            positions.push(pos);
            continue;
        }

        int count = 0;
        for(; count < expected; count++) {
            if((unsigned char)str[pos+count+1] >> 6 != 0b10)
                break;
        }

        if(count != expected) {
            for(int i = 0; i <= count; i++) {
                positions.push(pos+i);
            }
        }
        pos += count;
    }

    str.resize(str.length()+positions.size()*5); // add space for encoding

    // escape non-utf-8 characters
    size_t epos = str.length()-1;
    size_t offset = positions.size()*5;
    char* cstr = str.data();
    while(!positions.empty()) {
        size_t pos = positions.top();
        auto repl = Replacee(str[pos]);

        std::memmove(cstr+offset+pos+1, cstr+pos+1, epos-offset-pos);
        offset -= 5;
        std::copy(repl.data(), repl.data()+6, cstr+offset+pos);
        epos = offset+pos-1;

        positions.pop();
    }

    return str;
}


std::string toConstruct(const char* const& item) { return '"' + JSONEscape(item) + '"'; }
std::string toConstruct(const char*& item) { return (const char*)item; }
std::string toConstruct(const std::string& item) {
    return "std::string(" + toConstruct((const char*)item.c_str()) + ")";
}
std::string toConstruct(const unsigned char& item) { return "static_cast<unsigned char>(" + std::to_string((int)item) + ")"; }
std::string toConstruct(const char& item) { return "static_cast<char>(" + std::to_string((int)item) + ")"; }
std::string toConstruct(const bool& b) { return b ? "true" : "false"; }
#ifdef GCHECK_CONSTRUCT_DATA
std::string toConstruct(const UserObject& u) { return u.construct(); }
#else
std::string toConstruct(const UserObject&) { return ""; }
#endif
std::string toConstruct(decltype(nullptr)) { return "nullptr"; }
std::string toConstruct(const int& item) { return std::to_string(item); }
std::string toConstruct(const long& item) { return std::to_string(item) + 'L'; }
std::string toConstruct(const long long& item) { return std::to_string(item) + "LL"; }
std::string toConstruct(const unsigned& item) { return std::to_string(item) + 'U'; }
std::string toConstruct(const unsigned long& item) { return std::to_string(item) + "UL"; }
std::string toConstruct(const unsigned long long& item) { return std::to_string(item) + "ULL"; }

std::string toConstruct(const float& item) {
    std::stringstream ss;
    ss << std::hexfloat << item << 'f';
    return ss.str();
}

std::string toConstruct(const double& item) {
    std::stringstream ss;
    ss << std::hexfloat << item;
    return ss.str();
}

std::string toConstruct(const long double& item) {
    std::stringstream ss;
    ss << std::hexfloat << item << 'l';
    return ss.str();
}


std::string toString(const std::string& item) { return '"' + item + '"'; }
std::string toString(const char* const&item) { return toString(std::string(item)); }
std::string toString(const char*& item) { return toString((const char*)item); }
std::string toString(const char& item) { return std::string("'") + item + "'"; }
std::string toString(const unsigned char& item) { return std::to_string(item); }
std::string toString(const bool& b) { return b ? "true" : "false"; }
std::string toString(const UserObject& u) { return u.string(); }
std::string toString(decltype(nullptr)) { return "nullptr"; }
std::string toString(const int& item) { return std::to_string(item); }
std::string toString(const long& item) { return std::to_string(item); }
std::string toString(const long long& item) { return std::to_string(item); }
std::string toString(const unsigned& item) { return std::to_string(item); }
std::string toString(const unsigned long& item) { return std::to_string(item); }
std::string toString(const unsigned long long& item) { return std::to_string(item); }
std::string toString(const float& item) { return std::to_string(item); }
std::string toString(const double& item) { return std::to_string(item); }
std::string toString(const long double& item) { return std::to_string(item); }

} // gcheck