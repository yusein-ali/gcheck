#pragma once

#include <random>
#include <type_traits>

namespace gcheck {

// is_instance<A, B>; a struct for checking if A is a template specialization of B
template <class, template <typename...> class>
struct is_instance : public std::false_type {};

template <class... Args, template <typename...> class U>
struct is_instance<U<Args...>, U> : public std::true_type {};

template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);
template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);
template <typename T, template <typename...> class C>
using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

template<typename T, template<typename> class Distribution>
struct DistributionInfo;

// Base class for the different distributions
template<typename A>
class Distribution {
protected:
    std::default_random_engine generator_; 
public:
    Distribution() : generator_(std::random_device()()) {}
    Distribution(uint32_t seed) : generator_(seed == UINT32_MAX ? std::random_device()() : seed) {}

    //virtual Distribution Clone() = 0;
    virtual A operator()() = 0;
};

// A class that randomly selects an item from a std::vector
template<typename A>
class ChoiceDistribution : public Distribution<A> {
    std::vector<A> choices_;
    std::uniform_int_distribution<int> distribution_;
public:
    ChoiceDistribution(const DistributionInfo<A, ChoiceDistribution>& info) : Distribution<A>(info.seed), choices_(info.choices), distribution_(0, info.choices.size()-1) {}
    ChoiceDistribution(const std::vector<A>& choices, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), choices_(choices), distribution_(0, choices.size()-1) {}
    ChoiceDistribution(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), choices_(choices), distribution_(0, choices_.size()-1) {}
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

    RangeDistribution(const DistributionInfo<A, RangeDistribution>& info) : Distribution<A>(info.seed), start_(info.start), end_(info.end), distribution_(info.start, info.end) {}
    RangeDistribution(A start = std::numeric_limits<A>::min(), A end = std::numeric_limits<A>::max(), uint32_t seed = UINT32_MAX) 
        : Distribution<A>(seed), start_(start), end_(end), distribution_(start, end) {}
    RangeDistribution(const RangeDistribution<A>& dist) : Distribution<A>(dist), start_(dist.start_), end_(dist.end_), distribution_(dist.distribution_) {}
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

    RangeDistribution(const DistributionInfo<A, RangeDistribution>& info) : start_(info.start), end_(info.end), distribution_(0, (dist_t)(info.end-info.start)) {}
    RangeDistribution(A start, A end, uint32_t seed) : Distribution<A>(seed), start_(start), end_(end), distribution_(0, (dist_t)(end-start)) {}
    A operator()() { return start_+distribution_(Distribution<A>::generator_); }
};

template<typename T>
struct DistributionInfo<T, ChoiceDistribution> {
    std::vector<T> choices;
    uint32_t seed;
    DistributionInfo(const std::vector<T>& list, uint32_t seed = UINT32_MAX) : choices(list), seed(seed) {}
};

template<typename T>
struct DistributionInfo<T, RangeDistribution> {
    T start;
    T end;
    uint32_t seed;
    DistributionInfo(T start, T end, uint32_t seed = UINT32_MAX) : start(start), end(end), seed(seed) {}
};

template<typename T>
DistributionInfo(const std::vector<T>& list, uint32_t seed = UINT32_MAX) -> DistributionInfo<T, ChoiceDistribution>;
template<typename T>
DistributionInfo(const std::initializer_list<T>& list, uint32_t seed = UINT32_MAX) -> DistributionInfo<T, ChoiceDistribution>;
template<typename T>
DistributionInfo(T start, T end, uint32_t seed = UINT32_MAX) -> DistributionInfo<T, RangeDistribution>;

// Base class used for determining the existence of Next() function in class.
template<class T>
class NextType {
public:
    virtual T Next() = 0;
};

template<class A>
struct is_NextType : public is_base_of_template<A, NextType> {};

// Base class for different argument types
template <typename A>
class Argument {
protected:
    A value_;

    Argument() : value_() {};
    Argument(A value) : value_(value) {};
public:
    operator A() const { return value_; };
    A operator ()() const { return value_; };
};

// An argument with a constant value
template <typename A>
class Constant : public Argument<A> {
public:
    Constant(A value) : Argument<A>(value) {}
};

// An argument that takes a random value from a distribution
template <typename A, template<typename> class B = RangeDistribution>
class Random : public Argument<A>, public NextType<A> {
    B<A> distribution_;
public:
    Random(const A& start, const A& end, uint32_t seed = UINT32_MAX) : distribution_(start, end, seed) {}
    Random(const std::vector<A>& choices, uint32_t seed = UINT32_MAX) : distribution_(choices, seed) {}
    Random(const DistributionInfo<A, B>& info) : distribution_(info) {}
    Random(Random<A, B>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}
    Random(const Random<A, B>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}
    
    A Next() { return this->value_ = distribution_(); };
};
template<typename A>
Random(const A& start, const A& end, uint32_t seed = UINT32_MAX) -> Random<A, RangeDistribution>;
template<typename A>
Random(const std::vector<A>& choices, uint32_t seed = UINT32_MAX) -> Random<A, ChoiceDistribution>;
template<typename A>
Random(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) -> Random<A, ChoiceDistribution>;

/*
    An argument that fills a container with random values from a distribution.
*/
template <typename T, template <typename> class Container = std::vector, template<typename> class Distribution = RangeDistribution>
class RandomContainer : public Argument<Container<T>>, public NextType<Container<T>> {
public:
    RandomContainer(size_t size, const std::vector<T>& choices) : Argument<Container<T>>(Container<T>(size)), distribution_(choices) {}
    RandomContainer(size_t size, const T& start, const T& end) : Argument<Container<T>>(Container<T>(size)), distribution_(start, end) {}
    RandomContainer(const Container<T>& container, const std::vector<T>& choices) : Argument<Container<T>>(container), distribution_(choices) {}
    RandomContainer(const Container<T>& container, const T& start, const T& end) : Argument<Container<T>>(container), distribution_(start, end) {}
    RandomContainer(size_t size, const DistributionInfo<T, Distribution>& info) : Argument<Container<T>>(Container<T>(size)), distribution_(info) {}
    RandomContainer(const Container<T>& container, const DistributionInfo<T, Distribution>& info) : Argument<Container<T>>(container), distribution_(info) {}
    RandomContainer(RandomContainer<T, Container, Distribution>& rnd) : Argument<Container<T>>(rnd()), distribution_(rnd.distribution_) {}
    RandomContainer(const RandomContainer<T, Container, Distribution>& rnd) : Argument<Container<T>>(rnd()), distribution_(rnd.distribution_) {}
    
    void Resize(size_t size, T val = T()) {
        this->value_.resize(size, val);
    }
    size_t GetSize() {
        return this->value_.size();
    }
    Container<T> Next() {
        
        for(auto it = this->value_.begin(); it != this->value_.end(); ++it) {
            *it = distribution_();
        }
        return this->value_;
    }
    
private:
    Distribution<T> distribution_;
};

template<typename T>
RandomContainer(size_t size, const std::vector<T>& choices) -> RandomContainer<T, std::vector, ChoiceDistribution>;
template<typename T>
RandomContainer(size_t size, const std::initializer_list<T>& choices) -> RandomContainer<T, std::vector, ChoiceDistribution>;
template<typename T>
RandomContainer(size_t size, const T& start, const T& end) -> RandomContainer<T, std::vector, RangeDistribution>;
template<typename T, template <typename> class Container>
RandomContainer(const Container<T>& container, const std::vector<T>& choices) -> RandomContainer<T, Container, ChoiceDistribution>;
template<typename T, template <typename> class Container>
RandomContainer(const Container<T>& container, const std::initializer_list<T>& choices) -> RandomContainer<T, Container, ChoiceDistribution>;
template<typename T, template <typename> class Container>
RandomContainer(const Container<T>& container, const T& start, const T& end) -> RandomContainer<T, Container, RangeDistribution>;

// An argument that has contains a container of random size and content
template <typename T, template <typename> class Container = std::vector, template <typename> class SizeDistribution = RangeDistribution, template <typename> class ContentDistribution = RangeDistribution>
class RandomSizeContainer : public RandomContainer<T, Container, ContentDistribution>  {
public:

    RandomSizeContainer(size_t min_size, size_t max_size, const std::vector<T>& choices2) : RandomContainer<T, std::vector, ContentDistribution>(0, choices2), rnd_(min_size, max_size) {}
    RandomSizeContainer(size_t min_size, size_t max_size, const T& start, const T& end) : RandomContainer<T, std::vector, ContentDistribution>(0, start, end), rnd_(min_size, max_size) {}
    RandomSizeContainer(std::vector<size_t> choices, const std::vector<T>& choices2) : RandomContainer<T, std::vector, ContentDistribution>(0, choices2), rnd_(choices) {}
    RandomSizeContainer(std::vector<size_t> choices, const T& start, const T& end) : RandomContainer<T, std::vector, ContentDistribution>(0, start, end), rnd_(choices) {}
    RandomSizeContainer(const DistributionInfo<size_t, SizeDistribution>& size_info, const DistributionInfo<T, ContentDistribution>& content_info) : RandomContainer<T, Container, ContentDistribution>(0, content_info), rnd_(size_info) {}
    RandomSizeContainer(RandomSizeContainer<T, Container, SizeDistribution, ContentDistribution>&& rnd) : RandomContainer<T, Container, ContentDistribution>(rnd), rnd_(rnd.rnd_) {}
    RandomSizeContainer(const RandomSizeContainer<T, Container, SizeDistribution, ContentDistribution>& rnd) : RandomContainer<T, Container, ContentDistribution>(rnd), rnd_(rnd.rnd_) {}
    
    Container<T> Next() {
        this->value_.resize(rnd_.Next());
        return RandomContainer<T, Container, ContentDistribution>::Next();
    }
    
private:
    Random<size_t, SizeDistribution> rnd_;
};

template<typename T>
RandomSizeContainer(size_t min_size, size_t max_size, const std::vector<T>& choices) -> RandomSizeContainer<T, std::vector, RangeDistribution, ChoiceDistribution>;
template<typename T>
RandomSizeContainer(size_t min_size, size_t max_size, const std::initializer_list<T>& choices) -> RandomSizeContainer<T, std::vector, RangeDistribution, ChoiceDistribution>;
template<typename T>
RandomSizeContainer(size_t min_size, size_t max_size, const T& start, const T& end) -> RandomSizeContainer<T, std::vector, RangeDistribution, RangeDistribution>;
template<typename T>
RandomSizeContainer(std::vector<size_t> choices, const std::vector<T>& choices2) -> RandomSizeContainer<T, std::vector, ChoiceDistribution, ChoiceDistribution>;
template<typename T>
RandomSizeContainer(std::vector<size_t> choices, const std::initializer_list<T>& choices2) -> RandomSizeContainer<T, std::vector, ChoiceDistribution, ChoiceDistribution>;
template<typename T>
RandomSizeContainer(std::initializer_list<size_t> choices, const std::vector<T>& choices2) -> RandomSizeContainer<T, std::vector, ChoiceDistribution, ChoiceDistribution>;
template<typename T>
RandomSizeContainer(std::initializer_list<size_t> choices, const std::initializer_list<T>& choices2) -> RandomSizeContainer<T, std::vector, ChoiceDistribution, ChoiceDistribution>;
template<typename T>
RandomSizeContainer(std::vector<size_t> choices, const T& start, const T& end) -> RandomSizeContainer<T, std::vector, ChoiceDistribution, RangeDistribution>;
template<typename T>
RandomSizeContainer(std::initializer_list<size_t> choices, const T& start, const T& end) -> RandomSizeContainer<T, std::vector, ChoiceDistribution, RangeDistribution>;

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
    size_t GetSize() { return sequence_.size(); }
    void Reset() { it_ = sequence_.begin(); };
};

template <class T>
SequenceArgument(const std::vector<T>& v) -> SequenceArgument<T>;
template <class T>
SequenceArgument(const std::initializer_list<T>& v) -> SequenceArgument<T>;

// is_Random<A>::value; true if A is Random or RandomContainer, false otherwise
template<typename A>
struct is_Random : public std::false_type {};
template <typename... Ts>
struct is_Random<Random<Ts...>> : public std::true_type {};
template <typename... Ts>
struct is_Random<RandomContainer<Ts...>> : public std::true_type {};

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
    gcheck::advance(first);
    gcheck::advance(rest...);
}

// Shorthand for Random, Note: no template argument deduction
template<typename T, template<typename> class D = RangeDistribution>
using Rnd = Random<T, D>;

// Shorthand for RandomContainer, Note: no template argument deduction
template <class... T>
using RndContainer = RandomContainer<T...>;

// Shorthand for RandomSizeContainer, Note: no template argument deduction
template <class... T>
using RndSizeContainer = RandomSizeContainer<T...>;

// Shorthand for SequenceArgument, Note: no template argument deduction
template<typename T>
using Seq = SequenceArgument<T>;
}