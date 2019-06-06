#pragma once

#include <any>
#include <type_traits>

namespace gcheck {

template<class>
struct ConstantArgument;
template<class>
struct RandomArgument;
template<template<class> class,class>
struct RandomContainerArgument;

// is_Random<A>::value; true if A is RandomArgument or RandomContainerArgument, false otherwise
template<typename A>
struct is_Random : public std::false_type {};
template <typename T>
struct is_Random<RandomArgument<T>> : public std::true_type {};
template <template <typename> class C, typename T>
struct is_Random<RandomContainerArgument<C, T>> : public std::true_type {};

// is_Argument<A>::value; true if A is any of the argument types, false otherwise
template<typename A>
struct is_Argument : public std::false_type {};
template <typename T>
struct is_Argument<ConstantArgument<T>> : public std::true_type {};
template <typename T>
struct is_Argument<RandomArgument<T>> : public std::true_type {};
template <template <typename> class C, typename T>
struct is_Argument<RandomContainerArgument<C, T>> : public std::true_type {};

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

// advance(item): Generates a new random value for item if it is a random argument type
template<class T>
typename std::enable_if<is_Random<T>::value>::type 
advance(T& item) { item.Next(); }

// if item isn't a random argument type, do nothing
template<class T>
typename std::enable_if<!is_Random<T>::value>::type 
advance(T& item) { (void)item; }

// Calls advance(arg) on all arguments
template<class T, class... Args>
void advance(T& first, Args&... rest) {
    advance(first);
    advance(rest...);
}

}