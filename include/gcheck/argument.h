#pragma once

#include <random>
#include <type_traits>
#include <memory>

namespace gcheck {
/*
// is_instance<A, B>; a struct for checking if A is a template specialization of B
template <class, template <typename...> class>
struct is_instance : public std::false_type {};

template <class... Args, template <typename...> class U>
struct is_instance<U<Args...>, U> : public std::true_type {};
*/
template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);
template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);
template <typename T, template <typename...> class C>
using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

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
public:
    ChoiceDistribution(const std::vector<A>& choices, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), choices_(choices), distribution_(0, choices.size()-1) {}
    ChoiceDistribution(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), choices_(choices), distribution_(0, choices_.size()-1) {}
    A operator()() { return choices_[distribution_(Distribution<A>::generator_)]; }
private:
    std::vector<A> choices_;
    std::uniform_int_distribution<int> distribution_;
};

// A class that randomly selects a value from a range
template<typename A, typename Enable = void>
class RangeDistribution;

// Specialization of RangeDistribution for floating point and integral values
template<typename A>
class RangeDistribution<A, typename std::enable_if<std::is_floating_point<A>::value || std::is_integral<A>::value>::type> : public Distribution<A> {
public:

    RangeDistribution(const A& start = std::numeric_limits<A>::min(), const A& end = std::numeric_limits<A>::max(), uint32_t seed = UINT32_MAX) 
        : Distribution<A>(seed), start_(start), end_(end), distribution_(start, end) {}
    RangeDistribution(const RangeDistribution<A>& dist) : Distribution<A>(dist), start_(dist.start_), end_(dist.end_), distribution_(dist.distribution_) {}
    A operator()() { return distribution_(Distribution<A>::generator_); }
private:
    A start_;
    A end_;
    // Use int distribution for integral types and real distribution for floating point types
    typename std::conditional<std::is_integral<A>::value, 
        std::uniform_int_distribution<A>, 
        std::uniform_real_distribution<A>
        >::type distribution_;
};

// Specialization of RangeDistribution for values not floating point nor integral
template<typename A>
class RangeDistribution<A, typename std::enable_if<!std::is_floating_point<A>::value && !std::is_integral<A>::value>::type> : public Distribution<A> {
public:

    RangeDistribution(const A& start, const A& end, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), start_(start), end_(end), distribution_(0, (dist_t)(end-start)) {}
    A operator()() { return start_ + distribution_(Distribution<A>::generator_); }
private:
    typedef long long dist_t;

    A start_;
    A end_;
    std::uniform_int_distribution<dist_t> distribution_;
};

template<typename T>
std::shared_ptr<Distribution<T>> MakeDistribution(const std::vector<T>& choices, uint32_t seed = UINT32_MAX) {
    return std::shared_ptr<Distribution<T>>(new ChoiceDistribution<T>(choices, seed));
}
template<typename T>
std::shared_ptr<Distribution<T>> MakeDistribution(const std::initializer_list<T>& choices, uint32_t seed = UINT32_MAX) {
    return std::shared_ptr<Distribution<T>>(new ChoiceDistribution<T>(choices, seed));
}
template<typename T>
std::shared_ptr<Distribution<T>> MakeDistribution(const T& start, const T& end, uint32_t seed = UINT32_MAX) {
    return std::shared_ptr<Distribution<T>>(new RangeDistribution<T>(start, end, seed));
}


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
template <typename A>
class Random : public Argument<A>, public NextType<A> {
public:
    Random(const A& start, const A& end, uint32_t seed = UINT32_MAX) 
            : distribution_(new RangeDistribution<A>(start, end, seed)) {}
    Random(const std::vector<A>& choices, uint32_t seed = UINT32_MAX) 
            : distribution_(new ChoiceDistribution<A>(choices, seed)) {}
    Random(std::shared_ptr<Distribution<A>> distribution) : distribution_(distribution) {}
    Random(Random<A>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}
    Random(const Random<A>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}
    
    A Next() { return this->value_ = (*distribution_)(); };
private:
    std::shared_ptr<Distribution<A>> distribution_;
};
template<typename A>
Random(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) -> Random<A>;

/*
    An argument that fills a container with random values from a distribution.
*/
template <typename T, template <typename...> class Container = std::vector>
class RandomContainer : public Argument<Container<T>>, public NextType<Container<T>> {
public:
    template <typename... Args>
    RandomContainer(size_t size, const Args&... args) 
            : Argument<Container<T>>(Container<T>(size)), distribution_(MakeDistribution(args...)) {}
    RandomContainer(size_t size, std::shared_ptr<Distribution<T>> distribution) 
            : Argument<Container<T>>(Container<T>(size)), distribution_(distribution) {}
    RandomContainer(const Container<T>& container, std::shared_ptr<Distribution<T>> distribution) 
            : Argument<Container<T>>(container), distribution_(distribution) {}
    RandomContainer(RandomContainer<T, Container>& rnd) 
            : Argument<Container<T>>(rnd()), distribution_(rnd.distribution_) {}
    RandomContainer(const RandomContainer<T, Container>& rnd) 
            : Argument<Container<T>>(rnd()), distribution_(rnd.distribution_) {}
    
    void Resize(size_t size, T val = T()) {
        this->value_.resize(size, val);
    }
    size_t GetSize() {
        return this->value_.size();
    }
    Container<T> Next() {
        
        for(auto it = this->value_.begin(); it != this->value_.end(); ++it) {
            *it = (*distribution_)();
        }
        return this->value_;
    }
    
private:
    std::shared_ptr<Distribution<T>> distribution_;
};

// An argument that contains a container of random size and content
template <typename T, template <typename...> class Container = std::vector>
class RandomSizeContainer : public RandomContainer<T, Container>  {
public:

    template<typename... Args>
    RandomSizeContainer(size_t min_size, size_t max_size, const Args&... args) 
            : RandomContainer<T, Container>(0, args...), rnd_(min_size, max_size) {}
    template<typename... Args>
    RandomSizeContainer(std::vector<size_t> choices, const Args&... args) 
            : RandomContainer<T, Container>(0, args...), rnd_(choices) {}
    RandomSizeContainer(std::shared_ptr<Distribution<size_t>> size_distribution, std::shared_ptr<Distribution<T>> content_distribution) 
            : RandomContainer<T, Container>(0, content_distribution), rnd_(size_distribution) {}
    RandomSizeContainer(RandomSizeContainer<T, Container>&& rnd) 
            : RandomContainer<T, Container>(rnd), rnd_(rnd.rnd_) {}
    RandomSizeContainer(const RandomSizeContainer<T, Container>& rnd) 
            : RandomContainer<T, Container>(rnd), rnd_(rnd.rnd_) {}
    
    Container<T> Next() {
        this->value_.resize(rnd_.Next());
        return RandomContainer<T, Container>::Next();
    }
    
private:
    Random<size_t> rnd_;
};

template<typename T>
RandomSizeContainer(size_t min_size, size_t max_size, const std::initializer_list<T>& choices) -> RandomSizeContainer<T, std::vector>;
template<typename T, template <typename...> class Container>
RandomSizeContainer(size_t min_size, size_t max_size, const Container<T>& choices) -> RandomSizeContainer<T, std::vector>;
template<typename T>
RandomSizeContainer(size_t min_size, size_t max_size, const T& start, const T& end) -> RandomSizeContainer<T, std::vector>;
template<typename T>
RandomSizeContainer(std::vector<size_t> choices, const std::initializer_list<T>& choices2) -> RandomSizeContainer<T, std::vector>;
template<typename T, template <typename...> class Container>
RandomSizeContainer(std::vector<size_t> choices, const Container<T>& choices2) -> RandomSizeContainer<T, Container>;
template<typename T>
RandomSizeContainer(std::vector<size_t> choices, const T& start, const T& end) -> RandomSizeContainer<T, std::vector>;
template<typename T>
RandomSizeContainer(std::initializer_list<size_t> choices, const std::initializer_list<T>& choices2) -> RandomSizeContainer<T, std::vector>;
template<typename T, template <typename...> class Container>
RandomSizeContainer(std::initializer_list<size_t> choices, const Container<T>& choices2) -> RandomSizeContainer<T, Container>;
template<typename T>
RandomSizeContainer(std::initializer_list<size_t> choices, const T& start, const T& end) -> RandomSizeContainer<T, std::vector>;

// An argument that sequentially takes items from a vector
template <class T>
class SequenceArgument : public Argument<T>, public NextType<T> {
public:
    SequenceArgument(const std::vector<T>& seq) : Argument<T>(seq[0]), sequence_(seq), it_(sequence_.begin()) {}
    
    T Next() {
        this->value_ = *it_;
        it_++;
        if(it_ == sequence_.end())
            it_ = sequence_.begin();
        
        return this->value_;
    }
    size_t GetSize() { return sequence_.size(); }
    void Reset() { it_ = sequence_.begin(); };
private:
    std::vector<T> sequence_;
    typename std::vector<T>::iterator it_;
};

template <class T>
SequenceArgument(const std::vector<T>& v) -> SequenceArgument<T>;
template <class T>
SequenceArgument(const std::initializer_list<T>& v) -> SequenceArgument<T>;

/*
// is_Random<A>::value; true if A is Random or RandomContainer, false otherwise
template<typename A>
struct is_Random : public std::false_type {};
template <typename... Ts>
struct is_Random<Random<Ts...>> : public std::true_type {};
template <typename... Ts>
struct is_Random<RandomContainer<Ts...>> : public std::true_type {};
*/

// is_Argument<A>::value; true if A is any of the argument types, false otherwise
template<typename A>
struct is_Argument : public is_base_of_template<A, Argument> {};

// advance(item): Generates a new random value for item if it is a random argument type
template<class T>
std::enable_if_t<is_base_of_template<T, NextType>::value>
advance(T& item) { item.Next(); }

// if item isn't a random argument type, do nothing
template<class T>
std::enable_if_t<!is_base_of_template<T, NextType>::value>
advance(T&) { }

// Calls .Next() on all arguments inheriting from NextType
template<class T, class... Args>
void advance(T& first, Args&... rest) {
    gcheck::advance(first);
    gcheck::advance(rest...);
}

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template class RandomSizeContainer<int>;
extern template class RandomSizeContainer<unsigned int>;
extern template class RandomSizeContainer<double>;
extern template class RandomSizeContainer<float>;
extern template class RandomSizeContainer<std::string>;
extern template class SequenceArgument<int>;
extern template class SequenceArgument<unsigned int>;
extern template class SequenceArgument<double>;
extern template class SequenceArgument<float>;
extern template class SequenceArgument<std::string>;

}