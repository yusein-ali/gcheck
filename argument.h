#pragma once

#include <random>
#include <type_traits>

namespace gcheck {

// is_instance<A, B>; a struct for checking if A is a template specialization of B
template <class, template <typename...> class>
struct is_instance : public std::false_type {};

template <class... Args, template <typename...> class U>
struct is_instance<U<Args...>, U> : public std::true_type {};

// Base class for the different distributions
template<typename A>
class Distribution {
protected:
    std::default_random_engine generator_; 
public:
    Distribution() {}
    Distribution(unsigned int seed) : generator_(seed) {}

    //virtual Distribution Clone() = 0;
    virtual A operator()() = 0;
};

// A class that randomly selects an item from a std::vector
template<typename A>
class ChoiceDistribution : public Distribution<A> {
    std::vector<A> choices_;
    std::uniform_int_distribution<int> distribution_;
public:
    ChoiceDistribution(const std::vector<A>& choices) : choices_(choices), distribution_(0, choices.size()-1) {}
    A operator()() { return choices_[distribution_(Distribution<A>::generator_)]; }
};

// A class that randomly selects a value from a range
template<typename A, typename Enable = void>
class RangeDistribution;

// Specialization of RangeDistribution for floating point and integral values
template<typename A>
class RangeDistribution<A, typename std::enable_if<std::is_floating_point<A>::value || std::is_integral<A>::value>::type> : public Distribution<A> {

    A start_;
    A end_;
    // Use int distribution for integral types and real distribution for floating point types
    typename std::conditional<std::is_integral<A>::value, 
        std::uniform_int_distribution<A>, 
        std::uniform_real_distribution<A>
        >::type distribution_;
    
public:

    RangeDistribution(A start = std::numeric_limits<A>::min(), A end = std::numeric_limits<A>::max()) : start_(start), end_(end), distribution_(start, end) {}
    RangeDistribution(unsigned int seed, A start = std::numeric_limits<A>::min(), A end = std::numeric_limits<A>::max()) : Distribution<A>(seed), start_(start), end_(end), distribution_(start, end) {}
    A operator()() { return distribution_(Distribution<A>::generator_); }
};

// Specialization of RangeDistribution for values not floating point nor integral
template<typename A>
class RangeDistribution<A, typename std::enable_if<!std::is_floating_point<A>::value && !std::is_integral<A>::value>::type> : public Distribution<A> {

    typedef long long dist_t;

    A start_;
    A end_;
    std::uniform_int_distribution<dist_t> distribution_;
public:

    RangeDistribution(A start, A end) : start_(start), end_(end), distribution_(0, (dist_t)(end-start)) {}
    RangeDistribution(unsigned int seed, A start, A end) : Distribution<A>(seed), start_(start), end_(end), distribution_(0, (dist_t)(end-start)) {}
    A operator()() { return start_+distribution_(Distribution<A>::generator_); }
};

// Base class used for determining the existence of Next() function in class.
template<class T>
class NextType {
public:
    virtual T Next() = 0;
};

// Base class for different argument types
template <typename A>
class Argument {
protected:
    A value_;

    Argument(A value) : value_(value) {};
public:
    operator A() const { return value_; };
    A operator ()() const { return value_; };
};

// An argument with a constant value
template <typename A>
class ConstantArgument : public Argument<A> {
public:
    ConstantArgument(A value) : Argument<A>(value) {}
};

// An argument that takes a random value from a distribution
template <typename A, template<typename> class B = RangeDistribution>
class RandomArgument : public Argument<A>, public NextType<A> {
    B<A> distribution_;
public:
    template<typename... Args>
    RandomArgument(Args&&... args) : Argument<A>(A()), distribution_(args...) {}
    RandomArgument(RandomArgument<A, B>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}
    RandomArgument(const RandomArgument<A, B>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}
    
    A Next() { return this->value_ = distribution_(); };
};

// An argument that fills a container with random values from a distribution
template <template <typename> class C, typename T, template<typename> class B = RangeDistribution>
class RandomContainerArgument : public Argument<C<T>>, public NextType<C<T>> {
    B<T> distribution_;
public:
    template<typename... Args>
    RandomContainerArgument(C<T> container, Args... args) : Argument<C<T>>(container), distribution_(args...) {}
    RandomContainerArgument(RandomContainerArgument<C, T, B>& rnd) : Argument<C<T>>(rnd()), distribution_(rnd.distribution_) {}
    RandomContainerArgument(const RandomContainerArgument<C, T, B>& rnd) : Argument<C<T>>(rnd()), distribution_(rnd.distribution_) {}
    
    C<T> Next() { 
        for(auto it = this->value_.begin(); it != this->value_.end(); ++it) {
            *it = distribution_();
        }
        return this->value_;
    }
};

// An argument that sequentially takes items from a vector
template <class T>
class SequenceArgument : public Argument<T>, public NextType<T> {
    std::vector<T> sequence_;
    typename std::vector<T>::iterator it_;
public:
    SequenceArgument(const std::vector<T>& seq) : Argument<T>(seq[0]), sequence_(seq), it_(sequence_.begin()) {}
    T Next() {
        this->value_ = *it_;
        it_++;
        
        return this->value_;
    }
};

// Shorthand for RandomArgument
template<typename T, template<typename> class B = RangeDistribution>
using Rnd = RandomArgument<T, B>;

// Shorthand for RandomContainerArgument
template<template <typename> class C, typename T, template<typename> class B = RangeDistribution>
using RndContainer = RandomContainerArgument<C, T, B>;

// Shorthand for SequenceArgument
template<typename T>
using Seq = SequenceArgument<T>;
}