#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <vector>
#include <type_traits>

#include "argument.h"

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
    static auto has_begin(int) -> sfinae_true<decltype(std::declval<T>().begin())>;
    template<class T>
    static auto has_begin(long) -> sfinae_false<T>;
    template<class T>
    static auto has_end(int) -> sfinae_true<decltype(std::declval<T>().end())>;
    template<class T>
    static auto has_end(long) -> sfinae_false<T>;
}

template<class T>
struct has_string_operator : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_tostring : decltype(detail::has_tostring<T>(0)){};
template<class T>
struct has_std_tostring : decltype(detail::has_std_tostring<T>(0)){};

/*
    Wrapper class for anything passed by users from tests.
    Includes a descriptor string constructed using operator std::string, to_string, std::to_string or "", 
        in that order by first available method.
 */
class UserObject {
    std::any any_;
    std::string as_string_;
    
    void SetString(std::string item) { as_string_ = item; }
    void SetString(const char* item) { as_string_ = item; }
    void SetString(char item) { as_string_ = item; }
    void SetString(bool b) { as_string_ = b ? "true" : "false"; }
    
    template<typename T>
    typename std::enable_if<!has_tostring<T>::value && !has_std_tostring<T>::value>::type
    SetString(T item) {
        (void)item;
        as_string_ = "";
    }
    template<typename T>
    typename std::enable_if<!has_tostring<T>::value && has_std_tostring<T>::value>::type
    SetString(T item) {
        as_string_ = std::to_string(item);
    }
    template<typename T>
    typename std::enable_if<has_tostring<T>::value>::type
    SetString(T item) {
        as_string_ = to_string(item);
    }
public:
    UserObject() {}
    UserObject(std::any item);
    UserObject(std::vector<UserObject> cont);

    template<typename T>
    UserObject(T item) {
        SetString(item);
        any_ = std::make_any<T>(item);
    }
    
    std::string string() const { return as_string_; }
    std::any& any() { return any_; }
    const std::any& any() const { return any_; }
};

UserObject MakeUserObject(const UserObject& v);

UserObject MakeUserObject(const std::vector<UserObject>& v);

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

template<class T>
struct has_begin_end : decltype(detail::has_begin<T>(0) * detail::has_end<T>(0)){};

// MakeUserObjectList: function for making a vector containing std::any types from any number of any types of arguments
// If the type is an argument type, insert its value instead of the object itself

// If the item to be added is not an Argument class type, just add it to the list
template<class T>
typename std::enable_if<!is_Argument<T>::value && !has_begin_end<T>::value>::type
AddToUserObjectList(std::vector<UserObject>& container, T first) {
    container.push_back(MakeUserObject(first));
}
// If the item to be added is an Argument class type (but not container, that comes later)
// Get the value and put it in the container
template<class T>
typename std::enable_if<is_Argument<T>::value>::type
AddToUserObjectList(std::vector<UserObject>& container, T first) {
    AddToUserObjectList(container, first());
}

// If the item to be added is a container with begin() and end()
// Transform it to a vector and put it in the container
template<class T, template <class> class C>
typename std::enable_if<!is_Argument<C<T>>::value && has_begin_end<C<T>>::value>::type
AddToUserObjectList(std::vector<UserObject>& container, C<T> first){
    std::vector<UserObject> cont(first.begin(), first.end());
    container.push_back(MakeUserObject(cont));
}

void AddToUserObjectList(std::vector<UserObject>& container, std::string str);

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

extern template UserObject MakeUserObject(const int& v);
extern template UserObject MakeUserObject(const float& v);
extern template UserObject MakeUserObject(const double& v);
extern template UserObject MakeUserObject(const std::string& v);
extern template UserObject MakeUserObject(const std::vector<std::string>& v);
}