#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <type_traits>
#include "argument.h"

#include <iostream>
#include <string>

namespace gcheck {

/*
    Class for holding multiple types of data in a json-like structure.
*/
class JSON {
    std::unordered_map<std::string, std::any> items_;

public:
    static std::string AsString(const std::any& item);

    template <typename A>
    JSON& Set(const std::string& name, const A& value) {
        items_.insert_or_assign(name, value);
        return *this;
    }
    JSON& Set(const std::string& name, std::vector<std::any> value) {
        items_.insert_or_assign(name, value);
        //for(auto it = std::any_cast<std::vector<std::any>>(&items_[name])->begin(); it != std::any_cast<std::vector<std::any>>(&items_[name])->end(); ++it)
            //std::cout << JSON::AsString(*it) << " ";
        return *this;
    }
    JSON& Set(const std::string& name, const char value[]) {
        items_.insert_or_assign(name, std::string(value));
        return *this;
    }

    JSON& Set(JSON json);
    JSON& Remove(std::string key);

    bool Contains(std::string key) { return items_.find(key) != items_.end(); }
    std::any& Get(std::string key);
    std::any Get(std::string key) const;
    
    template<typename A>
    A& Get(std::string key) {
        auto out = std::any_cast<A>(&Get(key));

        if(out == NULL) {
            throw; //TODO: an actual exception
        }

        return *out;
    }

    template<typename A>
    A Get(std::string key) const {
        auto item = Get(key);
        auto out = std::any_cast<A>(&item);

        if(out == NULL) {
            throw; //TODO: an actual exception
        }

        return *out;
    }

    // Returns the object as a json string with or without the starting/ending {} characters
    std::string AsString(bool strip_ends = false) const;
    // Returns the item corresponding to key as a json string
    std::string AsString(const char* key) const;
    std::string AsString(std::string key) const;
    // Escapes all special json characters
    static std::string Escape(std::string str);

    auto begin() { return items_.begin(); }
    auto end() { return items_.end(); }
};

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

namespace detail{
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
struct has_begin_end : decltype(detail::has_begin<T>(0) * detail::has_end<T>(0)){};

// MakeAnyList: function for making a vector containing std::any types from any number of any types of arguments
// If the type is an argument type, insert its value instead of the object itself

// If the item to be added is not an Argument class type, just add it to the list
template<class T>
typename std::enable_if<!is_Argument<T>::value && !has_begin_end<T>::value>::type
AddToAnyList(std::vector<std::any>& container, T first) {
    container.push_back(std::any(first));
}
// If the item to be added is an Argument class type (but not container, that comes later)
// Get the value and put it in the container
template<class T>
typename std::enable_if<is_Argument<T>::value>::type
AddToAnyList(std::vector<std::any>& container, T first) {
    AddToAnyList(container, first());
}

// If the item to be added is a container with begin() and end()
// Transform it to a vector and put it in the container
template<class T, template <class> class C>
typename std::enable_if<!is_Argument<C<T>>::value && has_begin_end<C<T>>::value>::type
AddToAnyList(std::vector<std::any>& container, C<T> first){
    std::vector<std::any> cont(first.begin(), first.end());
    container.push_back(std::any(cont));
}
// If the item to be added is a RandomContainerArgument
// Get the value container and call the container version of AddToAnyList
template<template<class> class T, class S>
typename std::enable_if<has_begin_end<T<S>>::value>::type
AddToAnyList(std::vector<std::any>& container, RandomContainerArgument<T, S> first) {
    AddToAnyList(container, first());
}

// Called when there are multiple items to be added
template<class T, class... Args>
void AddToAnyList(std::vector<std::any>& container, T first, Args... rest) {
    AddToAnyList(container, first);
    AddToAnyList(container, rest...);
}

template<class... Args>
std::vector<std::any> MakeAnyList(Args... args) {
    std::vector<std::any> out;
    AddToAnyList(out, args...);
    return out;
}

}