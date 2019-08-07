#pragma once

#include <any>
#include <type_traits>

namespace gcheck {

template<class>
struct NextType;
template<class>
struct Argument;
template<class>
struct ConstantArgument;
template<class, template<typename> class>
struct RandomArgument;
template<template<class> class,class,template<typename> class>
struct RandomContainerArgument;

// is_Random<A>::value; true if A is RandomArgument or RandomContainerArgument, false otherwise
template<typename A>
struct is_Random : public std::false_type {};
template <typename T, template<typename> class B>
struct is_Random<RandomArgument<T, B>> : public std::true_type {};
template <template <typename> class C, typename T, template<typename> class B>
struct is_Random<RandomContainerArgument<C, T, B>> : public std::true_type {};

template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);
template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);
template <typename T, template <typename...> class C>
using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

template<class A>
struct is_NextType : public is_base_of_template<A, NextType> {};

// is_Argument<A>::value; true if A is any of the argument types, false otherwise
template<typename A>
struct is_Argument : public is_base_of_template<A, Argument> {};

// advance(item): Generates a new random value for item if it is a random argument type
template<class T>
typename std::enable_if<is_NextType<T>::value>::type 
advance(T& item) { item.Next(); }

// if item isn't a random argument type, do nothing
template<class T>
typename std::enable_if<!is_NextType<T>::value>::type 
advance(T& item) { (void)item; }

// Calls .Next() on all arguments inheriting from NextType
template<class T, class... Args>
void advance(T& first, Args&... rest) {
    advance(first);
    advance(rest...);
}

}