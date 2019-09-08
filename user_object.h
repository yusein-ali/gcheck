#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <sstream>
#include <utility>

#include "argument.h"
#include "json.h"
#include "sfinae.h"

namespace gcheck {
namespace {
namespace detail {
    template<class T>
    static auto has_begin(int) -> sfinae_true<decltype(std::declval<T>().begin())>;
    template<class T>
    static auto has_begin(long) -> sfinae_false<T>;
    template<class T>
    static auto has_end(int) -> sfinae_true<decltype(std::declval<T>().end())>;
    template<class T>
    static auto has_end(long) -> sfinae_false<T>;
} // detail

template<class T>
struct has_begin_end : decltype(detail::has_begin<T>(0) * detail::has_end<T>(0)){};

} // anonymous


/*
    Wrapper class for anything passed by users from tests.
    Includes a descriptor string constructed using operator std::string, to_string, std::to_string or "", 
        in that order by first available method.
 */
class UserObject {
    std::string as_string_;
    JSON as_json_;
    
    void SetString(std::string item) { as_string_ = item; }
    void SetString(const char* item) { as_string_ = item; }
    void SetString(char item) { as_string_ = item; }
    void SetString(bool b) { as_string_ = b ? "true" : "false"; }
    
    template<typename T>
    typename std::enable_if<!has_tostring<T>::value && !has_std_tostring<T>::value>::type
    SetString(T) {
        as_string_ = "";
    }
    template<typename T>
    typename std::enable_if<!has_tostring<T*>::value && !has_std_tostring<T*>::value>::type
    SetString(T* item) {
        if(item == nullptr) {
            as_string_ = "nullptr";
            as_json_ = "null";
        } else {
            std::stringstream ss;
            ss << static_cast<const void*>(item);
            as_string_ = ss.str(); 
            as_json_ = "\"" + as_string_ + "\"";
        }
    }
    /*template<typename T>
    typename std::enable_if<!has_tostring<T>::value && has_std_tostring<T>::value>::type
    SetString(T item) {
        as_string_ = std::to_string(item);
    }
    
    For some reason the above doesn't work on all systems, so make it explicit*/
    template<typename T = int>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(int item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = long>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(long item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = long long>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(long long item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = unsigned>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(unsigned item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = unsigned long>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(unsigned long item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = unsigned long long>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(unsigned long long item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = float>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(float item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = double>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(double item) {
        as_string_ = std::to_string(item);
    }
    template<typename T = long double>
    typename std::enable_if<!has_tostring<T>::value>::type
    SetString(long double item) {
        as_string_ = std::to_string(item);
    }
    template<typename T>
    typename std::enable_if<has_tostring<T>::value>::type
    SetString(T item) {
        as_string_ = to_string(item);
    }
    void SetString(decltype(nullptr)) {
        as_string_ = "nullptr";
        as_json_ = "null";
    }
public:
    UserObject() {}
    UserObject(std::vector<UserObject> cont);

    template<typename T>
    UserObject(T item) {
        as_json_ = toJSON(item);
        SetString(item);
    }
    
    std::string json() const { return as_json_; }
    std::string string() const { return as_string_; }
};

UserObject MakeUserObject(const UserObject& v);

UserObject MakeUserObject(const std::vector<UserObject>& v);

UserObject MakeUserObject(const char* v);

template <class T>
UserObject MakeUserObject(const T& v) {
    return UserObject(v);
}
template <class T, class S>
UserObject MakeUserObject(const std::pair<T, S>& v) {
    std::vector<UserObject> cont{ MakeUserObject(v.first), MakeUserObject(v.second) };
    return MakeUserObject(cont);
}
template <class T>
UserObject MakeUserObject(const std::vector<T>& v) {
    std::vector<UserObject> cont(v.size());
    for(unsigned int i = 0; i < v.size(); i++) {
        cont[i] = MakeUserObject(v[i]);
    }
    return MakeUserObject(cont);
}
template <class... Args>
UserObject MakeUserObject(const std::tuple<Args...>& v) {
    const std::size_t n = sizeof...(Args);
    std::vector<UserObject> cont(n);
    for(unsigned int i = 0; i < n; i++) {
        cont[i] = MakeUserObject(std::get<i>(v));
    }
    return MakeUserObject(cont);
}

// MakeUserObjectList: function for making a vector containing UserObject types from any number of any types of arguments
// If the type is an argument type, insert its value instead of the object itself

// If the item to be added is not an Argument class type, just add it to the list
template<class T>
typename std::enable_if<!is_Argument<T>::value && !has_begin_end<T>::value>::type
AddToUserObjectList(std::vector<UserObject>& container, T first) {
    container.push_back(MakeUserObject(first));
}

void AddToUserObjectList(std::vector<UserObject>& container, const std::string& first);

// If the item to be added is an Argument class type (but not container, that comes later)
// Get the value and put it in the container
template<class T>
typename std::enable_if<is_Argument<T>::value>::type
AddToUserObjectList(std::vector<UserObject>& container, T first) {
    AddToUserObjectList(container, first());
}

// If the item to be added is a container with begin() and end()
// Transform it to a vector and put it in the container
template<class C>
typename std::enable_if<!is_Argument<C>::value && has_begin_end<C>::value>::type
AddToUserObjectList(std::vector<UserObject>& container, C first){
    std::vector<UserObject> cont(first.begin(), first.end());
    container.push_back(MakeUserObject(cont));
}

// If the item to be added is a RandomContainer
// Get the value container and call the container version of AddToUserObjectList
template <class... Args>
void AddToUserObjectList(std::vector<UserObject>& container, RandomContainer<Args...> first) {
    AddToUserObjectList(container, first());
}

// Called when there are multiple items to be added
template<class T, class... Args>
void AddToUserObjectList(std::vector<UserObject>& container, T first, Args... rest) {
    AddToUserObjectList(container, first);
    AddToUserObjectList(container, rest...);
}

template<class... Args>
std::vector<UserObject> MakeUserObjectList(Args... args) {
    std::vector<UserObject> out;
    AddToUserObjectList(out, args...);
    return out;
}

} // gcheck