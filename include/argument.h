#pragma once

/**
 * @file argument.h
 * @brief Argument generation primitives for deterministic and random test inputs.
 *
 * This header defines a composable framework for generating argument values:
 * - fixed arguments (`Argument<T>`)
 * - random arguments (`Random<T>`) backed by distributions
 * - sequence-based arguments (`SequenceArgument<T>`)
 * - callable-based generators (`Generator<T, F>`)
 * - container generators (`Container<T, ...>`, `RandomSizeContainer(...)`)
 * - composition operators (`+` for distribution combination, `*` for tuple joins)
 *
 * Supported distribution families:
 * 1. Choice/Discrete values: @ref ChoiceDistribution, e.g. from `std::vector<T>`.
 * 2. Uniform ranges for numeric types: @ref RangeDistribution.
 * 3. Difference-based range fallback for non-arithmetic types supporting
 *    subtraction/addition (`end - start`, `start + offset`).
 *
 * Effective discrete-generation patterns:
 * 1. Use explicit choice sets for edge-focused tests:
 *    `Random<int> x({0, 1, -1, 42});`
 * 2. Keep reproducibility with explicit seed:
 *    `Random<int> x({1,2,3}, 12345);`
 * 3. Build mixed discrete spaces with `+` composition:
 *    `auto mixed = Random<int>({0,1}) + Random<int>({100,101});`
 * 4. Build structured tuples with `*`:
 *    `auto args = Random<int>({0,1}) * Random<char>({'a','b'});`
 * 5. For containers, vary size discretely and value source independently:
 *    `auto c = RandomSizeContainer(std::vector<size_t>{0,1,5}, std::vector<int>{-1,0,1});`
 *
 * Typical workflow:
 * 1. Define reusable generators.
 * 2. Call `Next()` manually or use `advance(...)` helpers.
 * 3. Feed generated values into Function/IO/Method test setup.
 */
#include <random>
#include <type_traits>
#include <memory>
#include <tuple>
#include <variant>
#include <algorithm>

namespace gcheck {

/** @brief gcheck argument-generation namespace. */

/**
 * @brief Positive probe overload for detecting template-base relationship.
 *
 * @tparam C Candidate base template.
 * @tparam Ts Template arguments.
 * @param Pointer to `C<Ts...>` used only for overload resolution.
 * @return `std::true_type` when `Derived*` can bind to `C<...>*`.
 *
 * Typical usage:
 * - Internal implementation detail for @ref is_base_of_template.
 */
template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);

/**
 * @brief Fallback probe overload for template-base detection.
 *
 * @tparam C Candidate base template.
 * @param Ellipsis fallback parameter.
 * @return `std::false_type` when the positive probe is not viable.
 */
template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);
/**
 * @brief Trait-like alias to test whether `Derived` is an instantiation
 *        (or derives from an instantiation) of class template `Base`.
 *
 * @tparam Derived Type being tested.
 * @tparam Base Class template to test against.
 */
template <typename Derived, template <typename...> class Base>
using is_base_of_template = decltype(is_base_of_template_impl<Base>(std::declval<Derived*>()));

/** @brief Compile-time conjunction for `std::is_same<T, Args>` over all `Args`. */
template <typename T, typename... Args>
struct are_same : std::conjunction<std::is_same<T, Args>...> {};

/** @brief Compile-time conjunction for `std::is_base_of<T, Args>` over all `Args`. */
template<typename T, typename... Args>
struct are_base_of : std::conjunction<are_base_of<T, Args>...> {};
/** @brief Base case for @ref are_base_of recursion. */
template <typename T, typename S>
struct are_base_of<T, S> : std::is_base_of<T, S> {};

/** @brief Internal extractor for template argument packs of `Base<...>`. */
template<template<typename...> class Base>
struct _get_templates {
    /** @brief Forms `std::tuple<Args...>` from `Base<Args>...` argument list. */
    template<typename... Args>
    static std::tuple<Args...> of(Base<Args>&&...);
};

/** @brief Extracts template argument tuple types from `Base`-instantiated arguments. */
template<template<typename...> class Base, typename... Args>
struct get_templates {
    /** @brief Tuple of extracted template parameters. */
    typedef decltype(_get_templates<Base>::of(std::declval<Args>()...)) types_tuple;
};

/** @brief Single-argument specialization exposing both tuple and first type. */
template<template<typename...> class Base, typename T>
struct get_templates<Base, T> {
    /** @brief Tuple with one extracted template parameter type. */
    typedef decltype(_get_templates<Base>::of(std::declval<T>())) types_tuple;
    /** @brief Convenience alias to the first extracted type. */
    typedef std::tuple_element_t<0, types_tuple> type;
};

/**
 * @brief Polymorphic interface for all argument generators.
 *
 * @tparam T Produced value type.
 *
 * Any generator participating in `advance(...)`, `Join`, `Combine`, or
 * container population derives from this interface.
 *
 * Contract:
 * 1. `Next()` updates internal state and returns current generated value.
 * 2. `Clone()` returns a heap-allocated deep copy; caller owns the pointer.
 *
 * Typical usage:
 * - Base of @ref Argument, @ref Random, @ref SequenceArgument, @ref Generator.
 */
template<class T>
class NextType {
public:
    /** @brief Produce and return the next generated value. */
    virtual T& Next() = 0;
    /** @brief Clone generator polymorphically. Caller owns returned pointer. */
    virtual NextType<T>* Clone() const = 0;
    /** @brief Virtual destructor for safe polymorphic cleanup. */
    virtual ~NextType() {}
};

/** @brief Marker interface for continuous domains with measurable choice width. */
template<typename T>
struct Continuous : public NextType<T> {
    /** @brief Returns effective measure/width of available values. */
    virtual double ChoiceLength() = 0;
};
/** @brief Marker interface for discrete domains with countable choices. */
template<typename T>
struct Discrete : public NextType<T> {
    /** @brief Returns number of available discrete choices. */
    virtual size_t ChoiceLength() = 0;
};

/**
 * @brief Base class for random distributions used by @ref Random.
 *
 * @tparam A Generated value type.
 */
template<typename A>
class Distribution {
protected:
    /** @brief Underlying random engine. */
    std::default_random_engine generator_;
public:
    /** @brief Construct with random-device seed. */
    Distribution() : generator_(std::random_device()()) {}
    /** @brief Construct with fixed seed (`UINT32_MAX` means random-device). */
    Distribution(uint32_t seed) : generator_(seed == UINT32_MAX ? std::random_device()() : seed) {}

    //virtual Distribution Clone() = 0;
    /** @brief Sample a value from the distribution. */
    virtual A operator()() = 0;
    /** @brief Virtual destructor for safe polymorphic deletion via base pointer. */
    virtual ~Distribution() = default;
};

/**
 * @brief Discrete distribution selecting elements from an explicit choice set.
 *
 * @tparam A Value type.
 *
 * Typical usage:
 * - Edge-value testing from a finite set.
 * - Reproducible discrete tests with explicit seed.
 */
template<typename A>
class ChoiceDistribution : public Distribution<A> {
public:
    /** @brief Construct from vector of choices. */
    ChoiceDistribution(const std::vector<A>& choices, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), choices_(choices), distribution_(0, choices.size()-1) {}
    /** @brief Construct from initializer list of choices. */
    ChoiceDistribution(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), choices_(choices), distribution_(0, choices_.size()-1) {}
    /** @brief Sample one choice uniformly by index. */
    A operator()() { return choices_[distribution_(Distribution<A>::generator_)]; }
private:
    /** @brief Source value set sampled by index. */
    std::vector<A> choices_;
    /** @brief Uniform index sampler over the choices vector. */
    std::uniform_int_distribution<int> distribution_;
};

// A class that randomly selects a value from a range
template<typename A, typename Enable = void>
class RangeDistribution;

/**
 * @brief Uniform range distribution for arithmetic types.
 *
 * @tparam A Integral or floating-point type.
 */
template<typename A>
class RangeDistribution<A, typename std::enable_if<std::is_floating_point<A>::value || std::is_integral<A>::value>::type> : public Distribution<A> {
public:

    /** @brief Construct with `[start, end]` bounds and optional seed. */
    RangeDistribution(const A& start = std::numeric_limits<A>::min(), const A& end = std::numeric_limits<A>::max(), uint32_t seed = UINT32_MAX)
        : Distribution<A>(seed), start_(start), end_(end), distribution_(start, end) {}
    /** @brief Copy constructor. */
    RangeDistribution(const RangeDistribution<A>& dist) : Distribution<A>(dist), start_(dist.start_), end_(dist.end_), distribution_(dist.distribution_) {}
    /** @brief Sample a value from the configured numeric interval. */
    A operator()() { return distribution_(Distribution<A>::generator_); }
private:
    /** @brief Lower bound of generated values. */
    A start_;
    /** @brief Upper bound of generated values. */
    A end_;
    // Use int distribution for integral types and real distribution for floating point types
    typename std::conditional<std::is_integral<A>::value,
        std::uniform_int_distribution<A>,
        std::uniform_real_distribution<A>
        >::type distribution_;
};

/**
 * @brief Integer-offset based range distribution for non-arithmetic types.
 *
 * @tparam A Type supporting `end-start` and `start + offset`.
 *
 * Typical usage:
 * - Custom numeric-like wrappers where arithmetic primitives are overloaded.
 */
template<typename A>
class RangeDistribution<A, typename std::enable_if<!std::is_floating_point<A>::value && !std::is_integral<A>::value>::type> : public Distribution<A> {
public:

    /** @brief Construct with bounds and optional seed. */
    RangeDistribution(const A& start, const A& end, uint32_t seed = UINT32_MAX) : Distribution<A>(seed), start_(start), end_(end), distribution_(0, (dist_t)(end-start)) {}
    /** @brief Sample by drawing offset and adding to `start`. */
    A operator()() { return start_ + distribution_(Distribution<A>::generator_); }
private:
    typedef long long dist_t;

    A start_;
    A end_;
    /** @brief Integer offset sampler mapped to `[start_, end_]`. */
    std::uniform_int_distribution<dist_t> distribution_;
};

/** @brief Factory helper creating a choice distribution from vector values. */
template<typename T>
std::shared_ptr<Distribution<T>> MakeDistribution(const std::vector<T>& choices, uint32_t seed = UINT32_MAX) {
    return std::shared_ptr<Distribution<T>>(new ChoiceDistribution<T>(choices, seed));
}
/** @brief Factory helper creating a choice distribution from initializer list values. */
template<typename T>
std::shared_ptr<Distribution<T>> MakeDistribution(const std::initializer_list<T>& choices, uint32_t seed = UINT32_MAX) {
    return std::shared_ptr<Distribution<T>>(new ChoiceDistribution<T>(choices, seed));
}
/** @brief Factory helper creating a range distribution from bounds. */
template<typename T>
std::shared_ptr<Distribution<T>> MakeDistribution(const T& start, const T& end, uint32_t seed = UINT32_MAX) {
    return std::shared_ptr<Distribution<T>>(new RangeDistribution<T>(start, end, seed));
}

/**
 * @brief Base value-holder generator for argument sources.
 *
 * @tparam A Produced value type.
 *
 * Typical usage:
 * - Constant argument source (`Next()` returns the same value).
 * - Base for random/sequence/container/generator argument types.
 */
template <typename A>
class Argument : public NextType<A> {
public:
    /** @brief Construct with default-initialized value. */
    Argument() : value_() {};
    /** @brief Construct with explicit initial value. */
    Argument(A value) : value_(value) {};

    /** @brief Implicit conversion to stored value. */
    operator A() const { return value_; };
    /** @brief Call operator returning current value by copy. */
    A operator ()() const { return value_; };

    template<size_t index>
    auto get() -> std::enable_if_t<is_base_of_template<A, std::tuple>::value, std::tuple_element_t<index, A>> {
        return std::get<index>(value_);
    }

    /** @brief Return current value without advancing generation state. */
    virtual A& Next() {
        return value_;
    }
    /** @brief Clone argument polymorphically. Caller owns returned pointer. */
    virtual NextType<A>* Clone() const {
        return new Argument(*this);
    }
protected:
    /** @brief Stored current value. */
    A value_;
};

/**
 * @brief Argument source producing random values from a configured distribution.
 *
 * @tparam A Produced value type.
 *
 * Typical discrete usage:
 * - `Random<int> x({0, 1, 2});`
 * - `Random<char> c({'a','b','c'}, 1234);`
 */
template <typename A>
class Random : public Argument<A> {
public:
    /** @brief Construct from range bounds. */
    Random(const A& start, const A& end, uint32_t seed = UINT32_MAX)
            : distribution_(new RangeDistribution<A>(start, end, seed)) {}
    /** @brief Construct from explicit choice set. */
    Random(const std::vector<A>& choices, uint32_t seed = UINT32_MAX)
            : distribution_(new ChoiceDistribution<A>(choices, seed)) {}
    /** @brief Construct from externally provided distribution implementation. */
    Random(std::shared_ptr<Distribution<A>> distribution) : distribution_(distribution) {}
    /** @brief Copy constructor; shares distribution pointer. */
    Random(const Random<A>& rnd) : Argument<A>(rnd()), distribution_(rnd.distribution_) {}

    /** @brief Draw next random sample and store/return it. */
    A& Next() { return this->value_ = (*distribution_)(); };
    NextType<A>* Clone() const {
        return new Random(*this);
    }
private:
    /** @brief Shared distribution backend used for sampling. */
    std::shared_ptr<Distribution<A>> distribution_;
};
/** @brief Deduction guide for discrete random source from initializer list. */
template<typename A>
Random(const std::initializer_list<A>& choices, uint32_t seed = UINT32_MAX) -> Random<A>;

/** @brief Converts plain values into `Argument<T>` while preserving argument-like inputs. */
template<typename T>
struct argumentize {
    typedef typename std::conditional<is_base_of_template<T, Argument>::value, T, Argument<T>>::type type;
};

/** @brief Strip `NextType` wrappers recursively to access underlying value type. */
template<typename T, class = void>
struct strip_next {
    typedef T type;
};
/** @brief Specialization for `NextType`-derived wrappers. */
template<typename T>
struct strip_next<T, std::enable_if_t<is_base_of_template<T, NextType>::value>> {
    typedef typename strip_next<decltype(std::declval<T>().Next())>::type type;
};
/**
 * @brief Argument source that generates containers with configurable size/content generators.
 *
 * @tparam T Element source type (value or generator-like type).
 * @tparam ContainerT Container template type (defaults to `std::vector`).
 *
 * Behavior:
 * 1. Size is produced by a `NextType<size_t>` source.
 * 2. Elements are produced by cycling through `source_` generators.
 * 3. If no element source is set, container uses default value.
 *
 * Typical usage:
 * - Fixed-size random elements.
 * - Random-size containers via @ref RandomSizeContainer helpers.
 */
template <typename T, template <typename...> class ContainerT = std::vector>
class Container : public Argument<ContainerT<typename strip_next<T>::type>> {
    typedef ContainerT<typename strip_next<T>::type> ReturnType;
    typedef std::vector<NextType<T>*> SourceType;
public:
    /** @brief Construct with dynamic size source and default element value. */
    Container(const Argument<size_t>& size, T def = T())
            : Argument<ReturnType>(ReturnType((size_t)size, def)) {
        Resize(size, def);
    }
    /** @brief Construct with fixed size and default element value. */
    Container(size_t size, T def = T())
            : Argument<ReturnType>(ReturnType(size, def)) {
        Resize(size, def);
    }
    template <template<typename...> class S, typename K, class = std::enable_if_t<is_base_of_template<S<K>, Argument>::value && !std::is_same_v<size_t, K>>>
    Container(S<K> size, T def) = delete; // disallow converting of size paramerer
    /** @brief Copy constructor with deep clone of size/element sources. */
    Container(const Container<T, ContainerT>& container)
            : Argument<ReturnType>(container), size_(container.size_->Clone()) {
        SetSource(container.source_);
    }
    /** @brief Destructor releasing owned source generators. */
    ~Container() {
        delete size_;
        for(auto& item : source_) {
            delete item;
        }
    }

    /** @brief Set dynamic size source and default fill value. */
    void Resize(const NextType<size_t>& size, T val = T()) {
        default_ = val;
        size_ = size.Clone();
    }
    /** @brief Set fixed size and default fill value. */
    void Resize(size_t size, T val = T()) {
        default_ = val;
        size_ = new Argument(size);
    }
    /** @brief Return currently materialized container size. */
    size_t GetSize() {
        return this->value_.size();
    }
    /** @brief Clear element source generators (defaults will be used). */
    void SetSource() {
        source_.clear();
    }
    /** @brief Replace element source generators by deep-cloning from references. */
    void SetSource(const std::vector<NextType<T>>& source) {
        source_.resize(source.size(), nullptr);
        std::transform(source.begin(), source.end(), source_.begin(), [](const NextType<T>& in){ return in.Clone(); });
    }
    /** @brief Replace element source generators by deep-cloning from pointers. */
    void SetSource(const std::vector<NextType<T>*>& source) {
        source_.resize(source.size(), nullptr);
        std::transform(source.begin(), source.end(), source_.begin(), [](const NextType<T>* in){ return in->Clone(); });
    }
    /** @brief Append one element source (value or generator-like object). */
    template<typename S>
    Container& operator<<(const S& item) {
        source_.push_back(((typename argumentize<S>::type*)&item)->Clone());
        return *this;
    }
    /** @brief Generate next container by size and cyclic element-source sampling. */
    ReturnType& Next() {
        this->value_.resize(size_->Next(), default_);
        auto it2 = source_.begin();
        if(it2 != source_.end()) {
            for(auto it = this->value_.begin(); it != this->value_.end(); ++it, ++it2) {
                if(it2 == source_.end())
                    it2 = source_.begin();
                *it = (*it2)->Next();
            }
        }
        return this->value_;
    }

    NextType<ContainerT<strip_next<T>>>* Clone() {
        return new Container(*this);
    }
private:
    /** @brief Element source generators sampled cyclically. */
    SourceType source_;
    /** @brief Size source generator. */
    NextType<size_t>* size_;
    /** @brief Default fill value used when sources are absent/short. */
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



/**
 * @brief Cyclic sequence argument source.
 *
 * @tparam T Value type.
 *
 * Produces elements from a vector in order and wraps at end.
 *
 * Typical usage:
 * - Deterministic input replay.
 * - Round-robin category/edge-case injection.
 */
template <class T>
class SequenceArgument : public Argument<T> {
public:
    /** @brief Construct cyclic sequence generator from vector values. */
    SequenceArgument(const std::vector<T>& seq) : Argument<T>(seq[0]), sequence_(seq), it_(sequence_.begin()) {}
    /** @brief Copy constructor resetting iterator to beginning. */
    SequenceArgument(const SequenceArgument<T>& seq) : Argument<T>(seq) {
        sequence_ = seq.sequence_;
        Reset();
    }

    /** @brief Emit next sequence value and wrap at end. */
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
    /** @brief Number of values in cycle. */
    size_t GetSize() { return sequence_.size(); }
    /** @brief Reset sequence cursor to first element. */
    void Reset() { it_ = sequence_.begin(); };
private:
    std::vector<T> sequence_;
    typename std::vector<T>::iterator it_;
};

template <class T>
SequenceArgument(const std::vector<T>& v) -> SequenceArgument<T>;
template <class T>
SequenceArgument(const std::initializer_list<T>& v) -> SequenceArgument<T>;

template<typename T, typename F>
class Generator : public Argument<T> {
    static T CallF(const F& functor, size_t index = 0) {
        if constexpr(std::is_invocable<F, size_t>::value) {
            return functor(index);
        } else {
            (void)index;
            return functor();
        }
    }
public:
    /**
     * @brief Construct from callable.
     *
     * Callable forms supported:
     * - `T f()`
     * - `T f(size_t index)`
     *
     * @param functor Callable used to generate values.
     * @param index Initial index for indexed functors.
     */
    Generator(const F& functor, size_t index = 0) : Argument<T>(CallF(functor, index)), index_(index), functor_(functor) {}

    /** @brief Access underlying functor. */
    const F& Functor() {
        return functor_;
    }

    /** @brief Generate next value by invoking callable and incrementing index. */
    T& Next() override {
        return this->value_ = CallF(functor_, index_++);
    }
    NextType<T>* Clone() const override {
        return new Generator(functor_, index_);
    }
private:
    /** @brief Call index passed to index-aware functors. */
    size_t index_ = 0;
    /** @brief Stored callable. */
    F functor_;
};

/** @brief Deduction guide for zero-argument callable generator. */
template<typename F>
Generator(F&& functor) -> Generator<typename std::invoke_result<F>::type, F>;

/** @brief Deduction guide for index-argument callable generator. */
template<typename F>
Generator(F&& functor) -> Generator<typename std::invoke_result<F, size_t>::type, F>;

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
    /** @brief Construct from component generators. */
    CombineBase(const Args&... args) : parts_(args...) {}

    /** @brief Uniform random selector in `[0,1]` used for component selection. */
    RangeDistribution<double> rnd_ = RangeDistribution<double>(0.0, 1.0);
    /** @brief Component generators participating in combination. */
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
    /** @brief Construct discrete combination from component generators. */
    Combine(const Args&... args) : CombineBase<Args...>(args...) {}

    /** @brief Add another discrete component to this combination. */
    template<typename T>
    Combine<DiscreteCombine, Args..., T> Added(const T& n) {
        return std::make_from_tuple<Combine<DiscreteCombine>>(std::tuple_cat(parts_, std::tuple(n)));
    }
    /** @brief Convert into weighted combination by adding weighted component. */
    template<typename T>
    Combine<WeightedCombine, Args..., T> Added(const T& n, double weight) {
        return std::make_from_tuple<Combine<WeightedCombine>>(std::tuple_cat(parts_, std::tuple(n, weight)));
    }

    /** @brief Total number of discrete choices across components. */
    size_t ChoiceLength() {
        return std::apply([](auto&... t){ return (t.ChoiceLength()+...); }, parts_);
    }

    /** @brief Draw one sample proportionally to each component's `ChoiceLength`. */
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
    /** @brief Construct continuous combination from component generators. */
    Combine(const Continuous<Args>&... args) : CombineBase<Args...>(args...) {}

    /** @brief Add another continuous component to this combination. */
    template<typename T>
    Combine<ContinuousCombine, Args..., T> Added(const T& n) {
        return std::make_from_tuple<Combine<ContinuousCombine>>(std::tuple_cat(parts_, std::tuple(n)));
    }
    /** @brief Convert into weighted combination by adding weighted component. */
    template<typename T>
    Combine<WeightedCombine, Args..., T> Added(const T& n, double weight) {
        return std::make_from_tuple<Combine<WeightedCombine>>(std::tuple_cat(parts_, std::tuple(n, weight)));
    }

    double ChoiceLength() {
        return std::apply([](auto&... t){ return (t.ChoiceLength()+...); }, parts_);
    }

    /** @brief Draw one sample proportionally to each component's interval width. */
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
    /** @brief Construct weighted binary combination (`weight` applies to first source). */
    Combine(const NextType<T>& first, const NextType<T>& second, double weight) : CombineBase<T, S>(first, second), weight_(weight) {}

    /** @brief Add another weighted branch by nesting current weighted combiner. */
    template<typename K>
    Combine<WeightedCombine, Combine<WeightedCombine, T, S>, K> Added(const NextType<K>& n, double weight) {
        return Combine<WeightedCombine>(*this, n, weight);
    }

    /** @brief Draw from first source with probability `weight_`, else second source. */
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
    /** @brief Selection probability threshold for first branch. */
    double weight_;
};

/** @brief Append discrete component to an existing discrete combination. */
template<typename T, typename... Args, class = std::enable_if_t<is_base_of_template<T, Discrete>::value>>
Combine<DiscreteCombine, Args..., T> operator+(const Combine<DiscreteCombine, Args...>& l, const T& r) {
    return l.Added(r);
}
/** @brief Prepend discrete component to an existing discrete combination. */
template<typename T, typename... Args, class = std::enable_if_t<is_base_of_template<T, Discrete>::value>>
Combine<DiscreteCombine, Args..., T> operator+(const T& l, const Combine<DiscreteCombine, Args...>& r) {
    return r + l;
}

/** @brief Append continuous component to an existing continuous combination. */
template<typename T, typename... Args, class = std::enable_if_t<is_base_of_template<T, Continuous>::value>>
Combine<ContinuousCombine, Args..., T> operator+(const Combine<ContinuousCombine, Args...>& l, const T& r) {
    return l.Added(r);
}
/** @brief Prepend continuous component to an existing continuous combination. */
template<typename T, typename... Args, class = std::enable_if_t<is_base_of_template<T, Continuous>::value>>
Combine<ContinuousCombine, Args..., T> operator+(const T& l, const Combine<ContinuousCombine, Args...>& r) {
    return r + l;
}


template<typename... Args>
class Join : public Argument<typename get_templates<NextType, Args...>::types_tuple> {
    typedef typename get_templates<NextType, Args...>::types_tuple TypesTuple;
    using Argument<TypesTuple>::value_;
public:
    /** @brief Construct tuple-join from component argument sources. */
    Join(const Args&... args) : parts_(args...) {}

    /** @brief Return a new join with an appended component. */
    template<typename T>
    Join<Args..., T> Appended(const T& n) const {
        return std::make_from_tuple<Join<Args..., T>>(std::tuple_cat(parts_, std::tuple(n)));
    }
    /** @brief Return a new join with a prepended component. */
    template<typename T>
    Join<T, Args...> Prepended(const T& n) const {
        return std::make_from_tuple<Join<T, Args...>>(std::tuple_cat(std::tuple(n), parts_));
    }

    /** @brief Generate next tuple by advancing all component generators once. */
    TypesTuple& Next() {
        value_ = std::apply([](auto&... t){ return std::tuple(t.Next()...); }, parts_);
        return value_;
    }

    NextType<TypesTuple>* Clone() const {
        return std::apply([](auto... args){return new Join(args...);}, parts_);
    }
private:
    /** @brief Component generators to evaluate and pack into tuple. */
    std::tuple<Args...> parts_;
};

/** @brief Append a generator to a join expression (`join * x`). */
template<typename T, typename... Args>
Join<Args..., T> operator*(const Join<Args...>& l, const T& r) {
    return l.Appended(r);
}
/** @brief Prepend a generator to a join expression (`x * join`). */
template<typename T, typename... Args>
Join<T, Args...> operator*(const T& l, const Join<Args...>& r) {
    return r.Prepended(l);
}


/** @brief Build discrete/continuous combination from two compatible generator sources. */
template<typename T, typename S, class = std::enable_if_t<
        (is_base_of_template<T, Continuous>::value && is_base_of_template<S, Continuous>::value)
        || (is_base_of_template<T, Discrete>::value && is_base_of_template<S, Discrete>::value)>>
auto operator+(const T& l, const S& r) {
    return Combine(l, r);
}
/** @brief Build tuple join from two `NextType`-compatible sources. */
template<typename T, typename S, class = std::enable_if_t<is_base_of_template<T, NextType>::value && is_base_of_template<S, NextType>::value>>
auto operator*(const T& l, const S& r) {
    return Join(l, r);
}

/** @brief Build random-size container using explicit size random source and element argument source. */
template<template<typename...> class ContainerT = std::vector, typename T>
auto RandomSizeContainer(const Random<size_t>& rnd, const Argument<T>& next) {
    Container<T, ContainerT> cont(rnd);
    cont << next;
    return cont;
}

/**
 * @brief Convenience helper for random-size container generation.
 *
 * Overload family builds a @ref Container with:
 * - size source from discrete/range random generator,
 * - element source from random generator (range or explicit values).
 *
 * Effective discrete usage examples:
 * - `RandomSizeContainer(std::vector<size_t>{0,1,2}, std::vector<int>{-1,0,1})`
 * - `RandomSizeContainer(1, 5, std::vector<char>{'a','b','c'})`
 */
template<template<typename...> class ContainerT = std::vector, typename T>
auto RandomSizeContainer(size_t min_size, size_t max_size, T min_value, T max_value) {
    return RandomSizeContainer<ContainerT, T>(Random<size_t>(min_size, max_size), Random<T>(min_value, max_value));
}

template<template<typename...> class ContainerT = std::vector, typename T>
auto RandomSizeContainer(size_t min_size, size_t max_size, std::vector<T> values) {
    return RandomSizeContainer<ContainerT, T>(Random<size_t>(min_size, max_size), Random<T>(values));
}

template<template<typename...> class ContainerT = std::vector, typename T>
auto RandomSizeContainer(std::vector<size_t> sizes, T min_value, T max_value) {
    return RandomSizeContainer<ContainerT, T>(Random<size_t>(sizes), Random<T>(min_value, max_value));
}

template<template<typename...> class ContainerT = std::vector, typename T>
auto RandomSizeContainer(std::vector<size_t> sizes, std::vector<T> values) {
    return RandomSizeContainer<ContainerT, T>(Random<size_t>(sizes), Random<T>(values));
}

template<template<typename...> class ContainerT = std::vector, typename T>
auto RandomSizeContainer(const Random<size_t>& rnd, std::vector<T> values) {
    return RandomSizeContainer<ContainerT, T>(rnd, Random<T>(values));
}

/** @brief Trait true for types derived from @ref Argument template family. */
template<typename A>
struct is_Argument : public is_base_of_template<A, Argument> {};

/** @brief Advance a generator-like value (`NextType`) once. */
template<class T>
std::enable_if_t<is_base_of_template<T, NextType>::value>
advance(T& item) { item.Next(); }

/** @brief No-op overload for non-generator values. */
template<class T>
std::enable_if_t<!is_base_of_template<T, NextType>::value>
advance(T&) { }

/** @brief Variadic helper that advances all generator-like arguments in place. */
template<class T, class... Args>
void advance(T& first, Args&... rest) {
    gcheck::advance(first);
    gcheck::advance(rest...);
}

// TODO: these were causing trouble for some reason
// Theoretically improves compile times with precompiled gcheck TODO: benchmark
/*extern template class SequenceArgument<int>;
extern template class SequenceArgument<unsigned int>;
extern template class SequenceArgument<double>;
extern template class SequenceArgument<float>;
extern template class SequenceArgument<std::string>;*/

}
