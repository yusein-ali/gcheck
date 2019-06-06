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
        std::uniform_real_distribution<A>>
        ::type distribution_;
    
public:

    RangeDistribution(A start, A end) : start_(start), end_(end), distribution_(start, end) {}
    RangeDistribution(unsigned int seed, A start, A end) : Distribution<A>(seed), start_(start), end_(end), distribution_(start, end) {}
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
template <typename A>
class RandomArgument : public Argument<A> {
    Distribution<A>& generator_;
public:
    RandomArgument(Distribution<A>& gen) : Argument<A>(A()), generator_(gen) {}
    A Next() { return this->value_ = generator_(); };
};

// An argument that fills a container with random values from a distribution
template <template <typename> class C, typename T>
class RandomContainerArgument : public Argument<C<T>> {
    Distribution<T>& generator_;
public:
    RandomContainerArgument(C<T> container, Distribution<T>& gen) : Argument<C<T>>(container), generator_(gen) {}
    C<T> Next() { 
        for(auto it = this->value_.begin(); it != this->value_.end(); ++it) {
            *it = generator_();
        }
        return this->value_;
    }
};

}