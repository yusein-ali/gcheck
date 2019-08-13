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