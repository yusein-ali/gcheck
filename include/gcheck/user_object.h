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
class UserObject;
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

template <size_t index, class... Args>
std::enable_if_t<index == 0> TupleToUserObjectVector(std::vector<UserObject>& cont, const std::tuple<Args...>& t) {
    cont[index] = UserObject(std::get<index>(t));
}
template <size_t index, class... Args>
std::enable_if_t<index != 0>  TupleToUserObjectVector(std::vector<UserObject>& cont, const std::tuple<Args...>& t) {
    cont[index] = UserObject(std::get<index>(t));
    TupleToUserObjectVector<index-1>(cont, t);
}

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
    void SetString(char* item) { as_string_ = item; }
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
    UserObject(const std::vector<UserObject>& cont);
    UserObject(const UserObject& v) {
        *this = v;
    }
    
    template <class T, class S>
    UserObject(const std::pair<T, S>& v) {
        std::vector<UserObject> cont = { UserObject(v.first), UserObject(v.second) };
        *this = UserObject(cont);
    }
    
    template <class T>
    UserObject(const std::vector<T>& v) {
        std::vector<UserObject> cont;
        cont.reserve(v.size());
        for(auto& i : v) {
            cont.emplace_back(i);
        }
        *this = UserObject(cont);
    }
    
    template <class... Args>
    UserObject(const std::tuple<Args...>& t) {
        constexpr size_t n = sizeof...(Args);
        std::vector<UserObject> cont(n);
        TupleToUserObjectVector<n-1>(cont, t);
        *this = UserObject(cont);
    }

    template<typename T>
    UserObject(T item) {
        as_json_ = JSON(item);
        SetString(item);
    }
    
    std::string json() const { return as_json_; }
    std::string string() const { return as_string_; }
};


// If the item to be added is not an Argument class type, just add it to the list
template<class T>
typename std::enable_if<!is_Argument<T>::value && !has_begin_end<T>::value>::type
AddToUserObjectVector(std::vector<UserObject>& container, T first) {
    container.emplace_back(first);
}

void AddToUserObjectVector(std::vector<UserObject>& container, const std::string& first);

// If the item to be added is an Argument class type (but not container, that comes later)
// Get the value and put it in the container
template<class T>
typename std::enable_if<is_Argument<T>::value>::type
AddToUserObjectVector(std::vector<UserObject>& container, T first) {
    AddToUserObjectVector(container, first());
}

// If the item to be added is a container with begin() and end()
// Transform it to a vector and put it in the container
template<class C>
typename std::enable_if<!is_Argument<C>::value && has_begin_end<C>::value>::type
AddToUserObjectVector(std::vector<UserObject>& container, C first){
    std::vector<UserObject> cont(first.begin(), first.end());
    container.emplace_back(cont);
}

// If the item to be added is a RandomContainer
// Get the value container and call the container version of AddToUserObjectVector
template <class... Args>
void AddToUserObjectVector(std::vector<UserObject>& container, RandomContainer<Args...> first) {
    AddToUserObjectVector(container, first());
}

// Called when there are multiple items to be added
template<class T, class... Args>
void AddToUserObjectVector(std::vector<UserObject>& container, T first, Args... rest) {
    AddToUserObjectVector(container, first);
    AddToUserObjectVector(container, rest...);
}

/* 
    MakeUserObjectList: function for making a vector containing UserObject types from any number of arguments of any types 
    If the type is an argument type, insert its value instead of the object itself
 */
template<class... Args>
std::vector<UserObject> MakeUserObjectList(Args... args) {
    std::vector<UserObject> out;
    AddToUserObjectVector(out, args...);
    return out;
}

} // gcheck