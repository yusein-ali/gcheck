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
#include "stringify.h"

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
public:
    UserObject() {}
    UserObject(const UserObject& v) = default;
    
    template<typename T>
    UserObject(const T& item) {
        as_json_ = item;
        as_string_ = toString(item);
        construct_ = toConstruct(item);
    }
    template<typename... Args>
    UserObject(const Args&... items) : UserObject(std::tuple<Args...>(items...)) {}
    
    JSON json() const { return as_json_; }
    std::string string() const { return as_string_; }
    std::string construct() const { return construct_; }
    
    template<typename T>
    UserObject& operator=(const T& v) {
        return *this = UserObject(v); 
    }
private:
    std::string as_string_;
    JSON as_json_;
    std::string construct_; // a string representation on how to construct the object e.g. std::vector<int>({0, 1, 2})
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

// If the item to be added is a Container
// Get the value container and call the container version of AddToUserObjectVector
template <template<typename...> class C, class... Args>
void AddToUserObjectVector(std::vector<UserObject>& container, Container<C, Args...> first) {
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