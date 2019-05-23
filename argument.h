#pragma once

#include <random>
#include <type_traits>

template <class, template <typename...> class>
struct is_instance : public std::false_type {};

template <class... Args, template <typename...> class U>
struct is_instance<U<Args...>, U> : public std::true_type {};

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

template<typename A>
class ChoiceDistribution : public Distribution<A> {
    std::vector<A> choices_;
    std::uniform_int_distribution<int> distribution_;
public:
    ChoiceDistribution(const std::vector<A>& choices) : choices_(choices), distribution_(0, choices.size()-1) {}
    A operator()() { return choices_[distribution_(Distribution<A>::generator_)]; }
};

template<typename A, typename Enable = void>
class RangeDistribution;

template<typename A>
class RangeDistribution<A, typename std::enable_if<std::is_floating_point<A>::value || std::is_integral<A>::value>::type> : public Distribution<A> {

    A start_;
    A end_;
    typename std::conditional<std::is_integral<A>::value, 
        std::uniform_int_distribution<A>, 
        std::uniform_real_distribution<A>>
        ::type distribution_;
    
public:

    RangeDistribution(A start, A end) : start_(start), end_(end), distribution_(start, end) {}
    RangeDistribution(unsigned int seed, A start, A end) : Distribution<A>(seed), start_(start), end_(end), distribution_(start, end) {}
    A operator()() { return distribution_(Distribution<A>::generator_); }
};

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

template <typename A>
class Argument {
protected:
    A value_;

    Argument(A value) : value_(value) {};
public:
    operator A() const { return value_; };
    A operator ()() const { return value_; };
};

template <typename A>
class ConstantArgument : public Argument<A> {
public:
    ConstantArgument(A value) : Argument<A>(value) {}
};

template <typename A>
class RandomArgument : public Argument<A> {
    Distribution<A>& generator_;
public:
    RandomArgument(Distribution<A>& gen) : Argument<A>(A()), generator_(gen) {}
    A Next() { return this->value_ = generator_(); };
};

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

template<typename A>
struct is_Random : public std::false_type {};
template <typename T>
struct is_Random<RandomArgument<T>> : public std::true_type {};
template <template <typename> class C, typename T>
struct is_Random<RandomContainerArgument<C, T>> : public std::true_type {};

template<typename A>
struct is_Argument : public std::false_type {};
template <typename T>
struct is_Argument<ConstantArgument<T>> : public std::true_type {};
template <typename T>
struct is_Argument<RandomArgument<T>> : public std::true_type {};
template <template <typename> class C, typename T>
struct is_Argument<RandomContainerArgument<C, T>> : public std::true_type {};

template<typename T>
typename std::enable_if<is_Random<T>::value>::type 
advance(T& item) { item.Next(); }

template<typename T>
typename std::enable_if<!is_Random<T>::value>::type 
advance(T& item) { }

template<typename T, typename... Args>
void advance(T& first, Args&... rest) {
    advance(first);
    advance(rest...);
}
