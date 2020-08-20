#pragma once

#include <random>
#include <type_traits>
#include <memory>
#include <tuple>
#include <variant>
#include <algorithm>

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

template <typename T, typename... Args>
struct are_same : std::conjunction<std::is_same<T, Args>...> {};

template<typename T, typename... Args>
struct are_base_of : std::conjunction<are_base_of<T, Args>...> {};
template <typename T, typename S>
struct are_base_of<T, S> : std::is_base_of<T, S> {};

template<template<typename...> class Base>
struct _get_templates {
    template<typename... Args>
    static std::tuple<Args...> of(Base<Args>&&...);
};

template<template<typename...> class Base, typename... Args>
struct get_templates {
    typedef decltype(_get_templates<Base>::of(std::declval<Args>()...)) types_tuple;
};

template<template<typename...> class Base, typename T>
struct get_templates<Base, T> {
    typedef decltype(_get_templates<Base>::of(std::declval<T>())) types_tuple;
    typedef std::tuple_element_t<0, types_tuple> type;
};

// Base class used for determining the existence of Next() function in class.
template<class T>
class NextType {
public:
    virtual T& Next() = 0;
    virtual NextType<T>* Clone() const = 0;
};

template<typename T>
struct Continuous : public NextType<T> {
    virtual double ChoiceLength() = 0;
};
template<typename T>
struct Discrete : public NextType<T> {
    virtual size_t ChoiceLength() = 0;
};

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

// Base class for different argument types
template <typename A>
class Argument : public NextType<A> {
public:
    Argument() : value_() {};
    Argument(A value) : value_(value) {};

    operator A() const { return value_; };
    A operator ()() const { return value_; };

    virtual A& Next() {
        return value_;
    }
    virtual NextType<A>* Clone() const {
        return new Argument(*this);
    }
protected:
    A value_;
};

// An argument that takes a random value from a distribution
template <typename A>
class Random : public Argument<A> {
public:
    Random(const A& start, const A& end, uint32_t seed = UINT32_MAX)
            : distribution_(new RangeDistribution<A>(start, end, seed)) {}
    Random(const std::vector<A>& choices, uint32_t seed = UINT32_MAX)
            : distribution_(new ChoiceDistribution<A>(choices, seed)) {}
    Random(std::shared_ptr<Distribution<A>> distribution) : distribution_(distribution) {}
    Random(const Random<A>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}

    A& Next() { return this->value_ = (*distribution_)(); };
    NextType<A>* Clone() const {
        return new Random(*this);
    }
private:
    std::shared_ptr<Distribution<A>> distribution_;
};
template<typename A>
Random(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) -> Random<A>;

template<typename T>
struct argumentize {
    typedef typename std::conditional<is_base_of_template<T, Argument>::value, T, Argument<T>>::type type;
};
template<typename T>
struct strip_next {
    typedef std::conditional<is_base_of_template<T, NextType>::value, typename strip_next<decltype(std::declval<T>().Next())>::type, T> type;
};
/*
    An argument that contains a container
*/
template <template <typename...> class ContainerT, typename T>
class Container : public Argument<ContainerT<strip_next<T>>> {
    typedef ContainerT<strip_next<T>> ReturnType;
    typedef std::vector<NextType<T>*> SourceType;
public:
    Container(const NextType<size_t>& size, T def = T())
            : Argument<ContainerT<T>>(ContainerT<T>(size, def)) {
        Resize(size, def);
    }
    Container(size_t size, T def = T())
            : Argument<ContainerT<T>>(ContainerT<T>(size, def)) {
        Resize(size, def);
    }
    Container(const Container<ContainerT, T>& container)
            : Argument<ContainerT<T>>(container), size_(container.size_->Clone()) {
        SetSource(container.source_);
    }
    ~Container() {
        delete size_;
        for(auto& item : source_) {
            delete item;
        }
    }

    void Resize(const NextType<size_t>& size, T val = T()) {
        default_ = val;
        size_ = size.Clone();
    }
    void Resize(size_t size, T val = T()) {
        default_ = val;
        size_ = new Argument(size);
    }
    size_t GetSize() {
        return this->value_.size();
    }
    void SetSource() {
        source_.clear();
    }
    void SetSource(const std::vector<NextType<T>>& source) {
        source_.resize(source.size(), nullptr);
        std::transform(source.begin(), source.end(), source_.begin(), [](const NextType<T>& in){ return in.Clone(); });
    }
    void SetSource(const std::vector<NextType<T>*>& source) {
        source_.resize(source.size(), nullptr);
        std::transform(source.begin(), source.end(), source_.begin(), [](const NextType<T>* in){ return in->Clone(); });
    }
    template<typename S>
    Container& operator<<(const S* item) {
        source_.push_back(((typename argumentize<S>::type*)&item).Clone());
        return *this;
    }
    ReturnType& Next() {
        this->value_.resize(size_->Next(), default_);
        auto it2 = source_.begin();
        if(it2 != source_.end()) {
            for(auto it = this->value_.begin(); it != this->value_.end(); ++it, ++it2) {
                if(it2 == source_.end())
                    it2 = source_.begin();
                *it = it2->Next();
            }
        }
        return this->value_;
    }

    NextType<ContainerT<strip_next<T>>>* Clone() {
        return new Container(*this);
    }
private:
    SourceType source_;
    NextType<size_t>* size_;
    T default_;
};
/*
class Container<std::basic_string, char> : public Argument<std::string>  {
    typedef std::string ReturnType;
    typedef std::vector<NextType<char>*> SourceType;
public:
    Container(const Args&... args)
            : Argument<ReturnType>(ReturnType(args...)), source_(args...) {}
    Container(const Container<std::tuple, Args...>& container)
            : Argument<ReturnType>(container), source_(container.source_) {}

    void Resize(size_t size, T val = T()) {
        this->value_.resize(size, val);
    }
    size_t GetSize() {
        return this->value_.size();
    }
    void SetSource() {
        source_.clear();
    }
    void SetSource(const std::vector<NextType<char>*>& source) {
        source_.resize(source.size(), nullptr);
        std::transform(source.begin(), source.end(), source_.begin(), [](const NextType<char>* in){ return in.Clone(); });
    }

    ReturnType Next() {
        auto it2 = source_.begin();
        if(it2 != source_.end()) {
            for(auto it = this->value_.begin(); it != this->value_.end(); ++it, ++it2) {
                if(it2 == source_.end())
                    it2 = source_.begin();
                *it = it2->Next();
            }
        }
        return this->value_;
    }
private:
    SourceType source_;
};*/



// An argument that sequentially takes items from a vector
template <class T>
class SequenceArgument : public Argument<T> {
public:
    SequenceArgument(const std::vector<T>& seq) : Argument<T>(seq[0]), sequence_(seq), it_(sequence_.begin()) {}
    SequenceArgument(const SequenceArgument<T>& seq) : Argument<T>(seq) {
        sequence_ = seq.sequence_;
        Reset();
    }

    T& Next() override {
        this->value_ = *it_;
        it_++;
        if(it_ == sequence_.end())
            it_ = sequence_.begin();

        return this->value_;
    }
    NextType<T>* Clone() const override {
        return new SequenceArgument(*this);
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
(RandomSizeContainer << Random).Next -> Random size and contents
(X * Y).Next -> std::tuple<X.Next, Y.Next>
(X + Y).Next -> (X.choices append Y.choices).Next
*/
enum CombineType {
    DiscreteCombine,
    ContinuousCombine,
    WeightedCombine
};

template<typename... Args>
class CombineBase {
    //static_assert(sizeof...(Args) != 0, "Combine must have at least one item in it");s
    typedef typename get_templates<NextType, Args...>::types_tuple TypesTuple;
    typedef std::conditional<are_same<Args>::value..., std::tuple_element<0, std::tuple<Args...>>::type, std::variant<Args...>> ReturnType;
protected:
    CombineBase(const Args&... args) : parts_(args...) {}

    RangeDistribution<double> rnd_ = RangeDistribution<double>(0.0, 1.0);
    std::tuple<Args...> parts_;
};

template<CombineType type, typename... Args>
class Combine;

template<typename... Args>
class Combine<DiscreteCombine, Args...> : public Discrete<typename CombineBase<Args...>::ReturnType>, public CombineBase<Args...> {
    //static_assert(are_base_of<Discrete, Args...>::value, "Items given to Combine<DiscreteCombine> must be derived from Discrete");
    typedef typename CombineBase<Args...>::ReturnType ReturnType;
    using CombineBase<Args...>::rnd_;
    using CombineBase<Args...>::parts_;
public:
    Combine(const Args&... args) : CombineBase<Args...>(args...) {}

    template<typename T>
    Combine<DiscreteCombine, Args..., T> Added(const T& n) {
        return std::make_from_tuple<Combine<DiscreteCombine>>(std::tuple_cat(parts_, std::tuple(n)));
    }
    template<typename T>
    Combine<WeightedCombine, Args..., T> Added(const T& n, double weight) {
        return std::make_from_tuple<Combine<WeightedCombine>>(std::tuple_cat(parts_, std::tuple(n, weight)));
    }

    size_t ChoiceLength() {
        return std::apply([](auto&... t){ return (t.ChoiceLength()+...); }, parts_);
    }

    ReturnType& Next() {
        auto vals = std::apply([](auto&... t){ return std::vector<ReturnType>({t.Next()...}); }, parts_);
        auto widths = std::apply([](auto&... t){ return std::vector({t.ChoiceLength()...}); }, parts_);
        auto rnd = rnd_()*ChoiceLength();
        size_t pos;
        for(pos = 0; pos != widths.size() && widths[pos] < rnd; pos++)
            rnd -= widths[pos];
        return vals[pos-1];
    }

    NextType<ReturnType>* Clone() const {
        return new Combine(*this);
    }
};

template<typename... Args>
class Combine<ContinuousCombine, Args...> : public Continuous<typename CombineBase<Args...>::ReturnType>, public CombineBase<Args...> {
    //static_assert(are_base_of<Continuous, Args...>::value, "Items given to Combine<ContinuousCombine> must be derived from Continuous");
    typedef typename CombineBase<Args...>::ReturnType ReturnType;
    using CombineBase<Args...>::rnd_;
    using CombineBase<Args...>::parts_;
public:
    Combine(const Continuous<Args>&... args) : CombineBase<Args...>(args...) {}

    template<typename T>
    Combine<ContinuousCombine, Args..., T> Added(const T& n) {
        return std::make_from_tuple<Combine<ContinuousCombine>>(std::tuple_cat(parts_, std::tuple(n)));
    }
    template<typename T>
    Combine<WeightedCombine, Args..., T> Added(const T& n, double weight) {
        return std::make_from_tuple<Combine<WeightedCombine>>(std::tuple_cat(parts_, std::tuple(n, weight)));
    }

    double ChoiceLength() {
        return std::apply([](auto&... t){ return (t.ChoiceLength()+...); }, parts_);
    }

    ReturnType& Next() {
        auto vals = std::apply([](auto&... t){ return std::vector<ReturnType>({t.Next()...}); }, parts_);
        auto widths = std::apply([](auto&... t){ return std::vector({t.ChoiceLength()...}); }, parts_);
        auto rnd = rnd_()*ChoiceLength();
        size_t pos;
        for(pos = 0; pos != widths.size() && widths[pos] < rnd; pos++)
            rnd -= widths[pos];
        return vals[pos-1];
    }

    NextType<ReturnType>* Clone() const {
        return new Combine(*this);
    }
};

template<typename T, typename S>
class Combine<WeightedCombine, T, S> : public NextType<typename CombineBase<T, S>::ReturnType>, CombineBase<T, S> {
    typedef typename CombineBase<T, S>::ReturnType ReturnType;
    using CombineBase<T, S>::rnd_;
    using CombineBase<T, S>::parts_;
public:
    Combine(const NextType<T>& first, const NextType<T>& second, double weight) : CombineBase<T, S>(first, second), weight_(weight) {}

    template<typename K>
    Combine<WeightedCombine, Combine<WeightedCombine, T, S>, K> Added(const NextType<K>& n, double weight) {
        return Combine<WeightedCombine>(*this, n, weight);
    }

    ReturnType& Next() {
        auto rnd = rnd_();
        if(rnd <= weight_)
            return std::get<0>(parts_).Next();
        return std::get<1>(parts_).Next();
    }

    NextType<ReturnType>* Clone() const {
        return new Combine(*this);
    }
private:
    double weight_;
};

template<typename T, typename... Args>
Combine<DiscreteCombine, Args..., T> operator+(const Combine<DiscreteCombine, Args...>& l, const T& r) {
    return l.Added(r);
}
template<typename T, typename... Args>
Combine<DiscreteCombine, Args..., T> operator+(const T& l, const Combine<DiscreteCombine, Args...>& r) {
    return r + l;
}

template<typename T, typename... Args>
Combine<ContinuousCombine, Args..., T> operator+(const Combine<ContinuousCombine, Args...>& l, const T& r) {
    return l.Added(r);
}
template<typename T, typename... Args>
Combine<ContinuousCombine, Args..., T> operator+(const T& l, const Combine<ContinuousCombine, Args...>& r) {
    return r + l;
}


template<typename... Args>
class Join : public Argument<typename get_templates<NextType, Args...>::types_tuple> {
    typedef typename get_templates<NextType, Args...>::types_tuple TypesTuple;
    using Argument<TypesTuple>::value_;
public:
    Join(const Args&... args) : parts_(args...) {}

    template<typename T>
    Join<Args..., T> Appended(const NextType<T>& n) {
        return std::make_from_tuple<Join>(std::tuple_cat<Args..., NextType<T>>(parts_, std::tuple(n)));
    }
    template<typename T>
    Join<T, Args...> Prepended(const NextType<T>& n) {
        return std::make_from_tuple<Join>(std::tuple_cat<NextType<T>, Args...>(std::tuple(n), parts_));
    }

    TypesTuple& Next() {
        value_ = std::apply([](auto&... t){ return std::tuple(t.Next()...); }, parts_);
        return value_;
    }

    NextType<TypesTuple>* Clone() const {
        return std::apply([](auto... args){return new Join(args...);}, parts_);
    }
private:
    std::tuple<Args...> parts_;
};

template<typename T, typename... Args>
Join<Args..., T> operator+(const Join<Args...>& l, const T& r) {
    return l.Appended(r);
}
template<typename T, typename... Args>
Join<T, Args...> operator+(const T& l, const Join<Args...>& r) {
    return r.Prepended(l);
}

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
extern template class SequenceArgument<int>;
extern template class SequenceArgument<unsigned int>;
extern template class SequenceArgument<double>;
extern template class SequenceArgument<float>;
extern template class SequenceArgument<std::string>;

}