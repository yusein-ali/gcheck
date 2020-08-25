#include <string>
#include <stack>
#include <cstring>

#include "stringify.h"
#include "user_object.h"

namespace gcheck {

std::string Escapees() {
    std::string escapees = "\\\"";
    for(char c = 0; c < 0x20; c++) {
        escapees.push_back(c);
    }
    return escapees;
}

std::string Replacee(char val) {
    static const char* digits = "0123456789ABCDEF";

    std::string ret = "\\u0000";
    for (size_t i = 0; i < 2; ++i)
        ret[i+4] = digits[(val >> 4*(1-i)) & 0xf];
    return ret;
}
std::vector<std::string> Replacees() {
    std::vector<std::string> replacees{"\\\\", "\\\""};
    for(char c = 0; c < 0x20; c++) {
        replacees.push_back(Replacee(c));
    }
    return replacees;
}

std::string UTF8Encode(std::string str) {

    static const std::string escapees = Escapees();
    static const std::vector<std::string> replacees = Replacees();

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

    // encode non-utf-8 characters
    std::stack<size_t> positions;
    for(size_t pos = 0; pos < str.length(); pos++) {
        int num = 0;
        while((str[pos] << num) & 0b10000000) num++;
        if(num == 0)
            continue;
        else if(num != 1) {
            int count = 0;
            while(count < num-1 && (str[pos+count+1] & 0b11000000) == 0b10000000) count++;
            if(count == num-1) {
                pos += count;
                continue;
            }
        }
        positions.push(pos);
    }

    str.resize(str.length()+positions.size()*5); // add space for encoding

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

std::string toConstruct(const char* item) { return '"' + UTF8Encode(item) + '"'; }
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