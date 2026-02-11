#pragma once

/**
 * @file function_test.h
 * @brief Function-oriented test base framework and utility helpers.
 *
 * This header provides:
 * - @ref gcheck::DeltaCompare for delta-tolerant floating comparisons.
 * - Factory/generator helpers for constructing objects in test scenarios.
 * - @ref gcheck::FunctionTest, the core template used by function, method,
 *   and IO-oriented test types.
 *
 * Typical usage flow:
 * 1. Derive a concrete test type from `FunctionTest<ReturnT, Args...>`.
 * 2. Implement @ref gcheck::FunctionTest::SetInputsAndOutputs to set expected
 *    return values, arguments, timing constraints, etc.
 * 3. Let @ref gcheck::FunctionTest::ActualTest execute configured runs and
 *    emit @ref gcheck::FunctionData report entries.
 */

#include <type_traits>
#include <functional>
#include <chrono>
#include <stdexcept>

#include "gcheck.h"
#include "test.h"
#include "sfinae.h"
#include "user_object.h"
#include "multiprocessing.h"

namespace gcheck {

/**
 * @brief Wrapper for value comparisons with an absolute delta tolerance.
 *
 * Typical usage scenario:
 * - Used automatically by @ref FunctionTest for floating-point return values
 *   and arguments, enabling comparisons like `expected +/- delta`.
 *
 * Example:
 * @code{.cpp}
 * gcheck::DeltaCompare<double> d(3.14, 0.01);
 * bool ok = (d == 3.141); // true
 * @endcode
 *
 * @tparam T Numeric value type.
 */
template<typename T>
class DeltaCompare {
public:
    /**
     * @brief Construct with value and optional tolerance.
     *
     * @param value Stored comparison value.
     * @param delta Allowed absolute deviation.
     */
    DeltaCompare(T value, T delta = 0) : value_(value), delta_(delta) {}

    /** @brief Return configured tolerance. */
    T Delta() const { return delta_; }

    /**
     * @brief Assign new base value.
     *
     * @param val New stored value.
     * @return Reference to `*this`.
     */
    T& operator=(T val) { value_ = val; return *this; }

    /** @brief Implicit mutable conversion to wrapped value. */
    operator T&() { return value_; }
    /** @brief Implicit const conversion to wrapped value. */
    operator T() const { return value_; }

    /** @brief Compare with another delta wrapper using min of two deltas. */
    bool operator==(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) == val.value_; }
    /** @brief Inequality compare with another delta wrapper. */
    bool operator!=(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) != val.value_; }
    /** @brief Less-than compare with another delta wrapper. */
    bool operator<(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) < val.value_; }
    /** @brief Greater-than compare with another delta wrapper. */
    bool operator>(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) > val.value_; }
    /** @brief Less-or-equal compare with another delta wrapper. */
    bool operator<=(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) <= val.value_; }
    /** @brief Greater-or-equal compare with another delta wrapper. */
    bool operator>=(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) >= val.value_; }

    /** @brief Equality compare against raw value with delta tolerance. */
    bool operator==(T val) const { return value_ + delta_ >= val && value_ - delta_ <= val; }
    /** @brief Inequality compare against raw value with delta tolerance. */
    bool operator!=(T val) const { return !(*this == val); }
    /** @brief Raw less-than compare (no tolerance window expansion). */
    bool operator<(T val) const { return value_ < val; }
    /** @brief Raw greater-than compare (no tolerance window expansion). */
    bool operator>(T val) const { return value_ > val; }
    /** @brief Less-or-equal compare against raw value with tolerance. */
    bool operator<=(T val) const { return value_ - delta_ <= val; }
    /** @brief Greater-or-equal compare against raw value with tolerance. */
    bool operator>=(T val) const { return value_ + delta_ >= val; }
private:
    /** @brief Stored base value for comparisons. */
    T value_;
    /** @brief Absolute tolerance used by comparisons. */
    T delta_;
};
/** @brief Symmetric equality overload for `raw == DeltaCompare`. */
template<typename T>
bool operator==(T val, const DeltaCompare<T>& dc) { return dc == val; }
/** @brief Symmetric inequality overload for `raw != DeltaCompare`. */
template<typename T>
bool operator!=(T val, const DeltaCompare<T>& dc) { return dc != val; }
/** @brief Symmetric less-than overload for `raw < DeltaCompare`. */
template<typename T>
bool operator<(T val, const DeltaCompare<T>& dc) { return dc < val; }
/** @brief Symmetric greater-than overload for `raw > DeltaCompare`. */
template<typename T>
bool operator>(T val, const DeltaCompare<T>& dc) { return dc > val; }
/** @brief Symmetric less-or-equal overload for `raw <= DeltaCompare`. */
template<typename T>
bool operator<=(T val, const DeltaCompare<T>& dc) { return dc <= val; }
/** @brief Symmetric greater-or-equal overload for `raw >= DeltaCompare`. */
template<typename T>
bool operator>=(T val, const DeltaCompare<T>& dc) { return dc >= val; }

namespace {

    /**
     * @brief Select delta-enabled storage type for floating-point inputs.
     *
     * Maps floating-point types to @ref DeltaCompare and leaves others unchanged.
     *
     * @tparam T Input type.
     */
    template<typename T>
    struct MakeDelta {
        typedef std::conditional_t<std::is_floating_point_v<T>, DeltaCompare<T>, T> type;
    };
    /** @brief Convenience alias for @ref MakeDelta result type. */
    template<typename T>
    using MakeDelta_t = typename MakeDelta<T>::type;

    /**
     * @brief Tuple wrapper that preserves original tuple type and storage tuple type.
     *
     * `storage_tuple_type` applies @ref DeltaCompare transformation for floating
     * element types.
     *
     * @tparam Args Tuple element types.
     */
    template<typename... Args>
    struct TupleWrapper : public std::tuple<Args...> {
        /** @brief Original tuple type. */
        typedef std::tuple<Args...> tuple_type;
        /** @brief Storage tuple with delta-aware floating element wrappers. */
        typedef std::tuple<MakeDelta_t<std::remove_const_t<Args>>...> storage_tuple_type;
        /** @brief Marker indicating non-empty tuple. */
        typedef std::false_type is_empty;
        /** @brief Default constructor. */
        TupleWrapper(){}
        /** @brief Construct from base tuple. */
        TupleWrapper(const std::tuple<Args...>& c) : std::tuple<Args...>(c) {}
        /** @brief Assign from base tuple. */
        TupleWrapper& operator=(const std::tuple<Args...>& val) {
            std::tuple<Args...>::operator=(val);
            return *this;
        }
    };
    /** @brief Empty tuple specialization. */
    template<>
    struct TupleWrapper<> : public std::tuple<> {
        /** @brief Original tuple type. */
        typedef std::tuple<> tuple_type;
        /** @brief Storage tuple type for empty tuple. */
        typedef std::tuple<> storage_tuple_type;
        /** @brief Marker indicating empty tuple. */
        typedef std::true_type is_empty;
        /** @brief Default constructor. */
        TupleWrapper(){}
        /** @brief Construct from base empty tuple. */
        TupleWrapper(const std::tuple<>& c) : std::tuple<>(c) {}
        /** @brief Assign from base empty tuple. */
        TupleWrapper& operator=(const std::tuple<>& val) {
            std::tuple<>::operator=(val);
            return *this;
        }
    };
    /** @brief `void` maps to empty tuple wrapper. */
    template<>
    struct TupleWrapper<void> : public TupleWrapper<> {};

    /**
     * @brief Conditional type wrapper used to substitute `void` with placeholder.
     *
     * @tparam T Candidate type.
     */
    template<typename T>
    struct ConditionalT {
        /** @brief Effective type when `T` is not `void`. */
        typedef T type;
        /** @brief Marker that type is defined. */
        typedef std::true_type is_defined;
    };
    /** @brief Specialization for `void` return/argument contexts. */
    template<>
    struct ConditionalT<void> {
        /** @brief Placeholder type for `void`. */
        typedef char type;
        /** @brief Marker that original type was undefined (`void`). */
        typedef std::false_type is_defined;
    };
    /** @brief Convenience alias for @ref ConditionalT result type. */
    template<typename T>
    using ConditionalT_t = typename ConditionalT<T>::type;

} // anonymous

/**
 * @brief Build a callable that stack-constructs objects of type `T`.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * auto make_obj = gcheck::constructor<MyType, int, std::string>();
 * MyType v = make_obj(1, "x");
 * @endcode
 *
 * @tparam T Constructed type.
 * @tparam Args Constructor argument types.
 * @return Function object calling `T(args...)`.
 */
template<typename T, typename... Args>
std::function<T(Args...)> constructor() {
    return [](auto... args){ return T(args...); };
}

/**
 * @brief Build a callable that heap-constructs objects of type `T`.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * auto make_ptr = gcheck::new_constructor<MyType, int>();
 * MyType* p = make_ptr(7);
 * @endcode
 *
 * @tparam T Constructed type.
 * @tparam Args Constructor argument types.
 * @return Function object calling `new T(args...)`.
 */
template<typename T, typename... Args>
std::function<T*(Args...)> new_constructor() {
    return [](auto... args){ return new T(args...); };
}

/**
 * @brief Create a generator that stack-constructs class instances from generated args.
 *
 * @tparam Class Constructed class type.
 * @tparam Args Constructor argument types for `Class`.
 * @tparam Args2 Types produced by `NextType`.
 * @param args Source argument generator.
 * @return Generator yielding `Class` values.
 */
template<typename Class, typename... Args, typename... Args2>
auto ClassGenerator(NextType<Args2...>& args) {
    auto c = constructor<Class, Args...>();
    return Generator([c, &args]() -> Class { return std::apply(c, args.Next()); });
}

/**
 * @brief Create a generator that heap-constructs class instances from generated args.
 *
 * @tparam Class Constructed class type.
 * @tparam Args Constructor argument types for `Class`.
 * @tparam Args2 Types produced by `NextType`.
 * @param args Source argument generator.
 * @return Generator yielding `Class*` values.
 */
template<typename Class, typename... Args, typename... Args2>
auto ClassPtrGenerator(NextType<Args2...>& args) {
    auto c = new_constructor<Class, Args...>();
    return Generator([c, &args]() -> Class* { return std::apply(c, args.Next()); });
}

/**
 * @brief Template base class for testing callable functions over one or more runs.
 *
 * `FunctionTest` drives repeated execution of a target callable, captures
 * arguments/return values/timing, and writes @ref FunctionData reports.
 *
 * Typical usage scenario:
 * 1. Derive from `FunctionTest<ReturnT, Args...>`.
 * 2. Implement @ref SetInputsAndOutputs to configure expected values per run.
 * 3. Optionally register hooks with @ref AddResetTest, @ref AddPreRun,
 *    @ref AddPostRun.
 * 4. Let inherited test lifecycle execute and grade results.
 *
 * @tparam ReturnT Target callable return type (`void` supported).
 * @tparam Args Target callable argument types.
 */
template<typename ReturnT, typename... Args>
class FunctionTest : public Test {
public:
    /** @brief Return storage type (delta-aware for floating-point values). */
    typedef MakeDelta_t<ConditionalT_t<ReturnT>> ReturnType;
    /** @brief Tuple wrapper for argument values. */
    typedef TupleWrapper<std::remove_cv_t<std::remove_reference_t<Args>>...> TupleWrapperType;
    /** @brief Internal storage tuple type for argument expectations. */
    typedef typename TupleWrapperType::storage_tuple_type StorageTupleType;
    /** @brief Plain tuple type for user-facing arguments. */
    typedef typename TupleWrapperType::tuple_type TupleType;
    /** @brief Legacy compare-function placeholder type. */
    typedef void(*CompareFunc)(void);

    /**
     * @brief Construct function test.
     *
     * @param info Test metadata.
     * @param num_runs Number of runs/cases to execute.
     * @param func Callable under test.
     */
    FunctionTest(const TestInfo& info, int num_runs, const std::function<ReturnT(Args...)>& func) : Test(info), num_runs_(num_runs), function_(func) { }

    /**
     * @brief Register callback invoked before each run's input/output setup.
     *
     * Typical usage:
     * - Reset external/global state between runs.
     *
     * @tparam F Callable type `void()`.
     * @param func Callback to store.
     */
    template<typename F>
    void AddResetTest(F&& func) {
        reset_vars_functions_.push_back(std::forward<F>(func));
    }
    /**
     * @brief Register callback invoked just before callable execution.
     *
     * Callback signature: `void(size_t run_index, FunctionEntry&)`.
     *
     * @tparam F Callable type.
     * @param func Callback to store.
     */
    template<typename F>
    void AddPreRun(F&& func) {
        pre_run_functions_.push_back(std::forward<F>(func));
    }
    /**
     * @brief Register callback invoked immediately after callable execution.
     *
     * Callback signature: `void(size_t run_index, FunctionEntry&)`.
     *
     * @tparam F Callable type.
     * @param func Callback to store.
     */
    template<typename F>
    void AddPostRun(F&& func) {
        post_run_functions_.push_back(std::forward<F>(func));
    }
protected:
    /** @brief Optional compare function hook (reserved/legacy). */
    CompareFunc comp_function_ = nullptr;

    /** @brief Expected input arguments for current run. */
    std::optional<StorageTupleType> args_;
    /** @brief Expected post-call argument values. */
    std::optional<StorageTupleType> args_after_;
    /** @brief Expected return value for current run. */
    std::optional<ReturnType> expected_return_value_;
    /** @brief Optional per-run maximum runtime constraint. */
    std::optional<std::chrono::nanoseconds> max_run_time_;
    /** @brief Optional process timeout used in safe-run mode. */
    std::chrono::duration<double> timeout_ = std::chrono::duration<double>::zero();

    /** @brief Arguments snapshot used in the latest executed run. */
    std::optional<StorageTupleType> last_args_;
    /** @brief Number of runs to execute. */
    int num_runs_;
    /** @brief Zero-based current run index. */
    size_t run_index_ = 0;
    /** @brief Whether to validate argument-after-call expectations. */
    bool check_arguments_ = true;

    /** @brief Registered pre-setup reset callbacks. */
    std::vector<std::function<void()>> reset_vars_functions_;
    /** @brief Registered pre-run callbacks. */
    std::vector<std::function<void(size_t, FunctionEntry&)>> pre_run_functions_;
    /** @brief Registered post-run callbacks. */
    std::vector<std::function<void(size_t, FunctionEntry&)>> post_run_functions_;

    /**
     * @brief Set expected input arguments for next run.
     *
     * @tparam Args2 Argument value types.
     * @param args Argument values.
     */
    template<typename... Args2>
    void SetArguments(Args2&&... args) {
        args_ = std::tuple(std::forward<Args2>(args)...);
    }
    /**
     * @brief Set expected input arguments from plain tuple.
     *
     * @param args Tuple of expected argument values.
     */
    void SetArguments(const TupleType& args) {
        std::apply([this](auto... x){this->SetArguments(x...);}, args);
    }
    /**
     * @brief Set expected input arguments from storage tuple.
     *
     * Enabled only when `TupleType == StorageTupleType`.
     *
     * @tparam A Internal SFINAE guard type.
     * @param args Storage tuple of expected argument values.
     */
    template<typename A = TupleType, class = std::enable_if_t<std::is_same_v<A, StorageTupleType>>>
    void SetArguments(const StorageTupleType& args) {
        std::apply([this](auto... x){this->SetArguments(x...);}, args);
    }
    /**
     * @brief Set expected argument values after callable execution.
     *
     * @param args Expected post-call argument values.
     */
    void SetArgumentsAfter(const typename std::remove_reference<Args>::type&... args) { args_after_ = std::tuple(args...);}
    /**
     * @brief Set expected argument values after callable execution from tuple.
     *
     * @param args Tuple of expected post-call argument values.
     */
    void SetArgumentsAfter(const TupleType& args) {
        std::apply([this](auto... x){this->SetArgumentsAfter(x...);}, args);
    }
    /** @brief Disable argument-after-call checking for current run. */
    void IgnoreArgumentsAfter() { check_arguments_ = false; }
    /** @brief Set expected return value for current run. */
    void SetReturn(const ReturnType& val) { expected_return_value_ = val; }
    /** @brief Set per-run max execution time constraint. */
    void SetMaxRunTime(std::chrono::nanoseconds ns) { max_run_time_ = ns; }
    /** @brief Set per-run max execution time constraint from nanoseconds count. */
    void SetMaxRunTime(unsigned long long ns) { max_run_time_ = std::chrono::nanoseconds(ns); }
    /** @brief Set safe-run process timeout duration. */
    void SetTimeout(std::chrono::duration<double> seconds) { timeout_ = seconds; }
    /** @brief Set safe-run process timeout (seconds). */
    void SetTimeout(double seconds) { timeout_ = std::chrono::duration<double>(seconds); }

    /** @brief Access last run's argument snapshot. */
    const std::optional<TupleType>& GetLastArguments() const { return last_args_; }
    /** @brief Access current run index. */
    size_t GetRunIndex() { return run_index_; }

    /**
     * @brief Derived hook to configure expected inputs/outputs per run.
     *
     * Called once per run after reset callbacks and before execution.
     */
    virtual void SetInputsAndOutputs() = 0;

    /**
     * @brief Execute one run and fill a @ref FunctionEntry.
     *
     * @param data Output run entry being populated.
     */
    void RunOnce(FunctionEntry& data);
    /**
     * @brief Reset per-run expectation state.
     *
     * Default implementation clears argument/return expectations and enables
     * argument-after checking. Derived classes may extend behavior.
     */
    virtual void ResetTestVars();
private:
    /**
     * @brief Execute all configured runs and emit one @ref FunctionData report.
     */
    virtual void ActualTest();

    /** @brief Target callable under test. */
    std::function<ReturnT(Args...)> function_;
};

/**
 * @brief Default run-state reset implementation for @ref FunctionTest.
 *
 * Clears per-run expectations before `SetInputsAndOutputs()` is called.
 */
template<typename ReturnT, typename... Args>
void FunctionTest<ReturnT, Args...>::ResetTestVars() {
    args_.reset();
    args_after_.reset();
    expected_return_value_.reset();
    check_arguments_ = true;
}

/**
 * @brief Execute one configured run and populate the provided @ref FunctionEntry.
 *
 * Populates:
 * - arguments / arguments-after snapshots
 * - return value and expected return value
 * - runtime and timeout-related fields
 * - final pass/fail result
 *
 * @param data Run entry to populate.
 */
template<typename ReturnT, typename... Args>
void FunctionTest<ReturnT, Args...>::RunOnce(FunctionEntry& data) {
    for(auto& f : pre_run_functions_)
        f(run_index_, data);

    last_args_ = args_;

    if(check_arguments_) {
        if(!args_after_ && args_)
            args_after_ = args_;
        if(args_after_)
            data.arguments_after_expected = (TupleType)*args_after_;
    }

    if constexpr(sizeof...(Args) != 0) { // if function takes arguments
        if(args_) {
            auto args = (FunctionTest::TupleType)*args_;
            data.arguments = args;

            if constexpr(std::is_same<ReturnT, void>::value) {
                auto t1 = std::chrono::high_resolution_clock::now();
                std::apply(function_, args);
                data.run_time = std::chrono::high_resolution_clock::now() - t1;

                data.result = true;
            } else {
                auto t1 = std::chrono::high_resolution_clock::now();
                auto ret = std::apply(function_, args);
                data.run_time = std::chrono::high_resolution_clock::now() - t1;

                data.return_value = ret;
                if(expected_return_value_)
                    data.return_value_expected = *expected_return_value_;
                data.result = (!expected_return_value_ || *expected_return_value_ == ret);
            }

            data.arguments_after = args;
            data.result = data.result && (!check_arguments_ || args == args_after_);
        } else {
            data.result = false;
        }
    } else if constexpr(std::is_same<ReturnT, void>::value) {
        auto t1 = std::chrono::high_resolution_clock::now();
        function_();
        data.run_time = std::chrono::high_resolution_clock::now() - t1;

        data.result = !args_after_ && !args_;
    } else {
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ret = function_();
        data.run_time = std::chrono::high_resolution_clock::now() - t1;

        data.return_value = ret;
        if(expected_return_value_)
            data.return_value_expected = *expected_return_value_;
        data.result = (!expected_return_value_ || *expected_return_value_ == ret) && (!args_after_ && !args_);
    }

    data.max_run_time = max_run_time_;
    if(max_run_time_)
        data.result = data.result && data.run_time <= max_run_time_.value();

    for(auto& f : post_run_functions_)
        f(run_index_, data);
}

/**
 * @brief Execute all runs for this test instance and append report data.
 *
 * Behavior:
 * 1. Allocates one @ref FunctionData report sized by `num_runs_`.
 * 2. For each run, resets state, applies hooks, configures expectations,
 *    executes directly or through safe-run wrapper, and stores timeout metadata.
 * 3. Appends report and marks test status as `Finished`.
 */
template<typename ReturnT, typename... Args>
void FunctionTest<ReturnT, Args...>::ActualTest() {
    TestReport report = TestReport::Make<FunctionData>();
    auto& data = report.Get<FunctionData>();

    data.resize(num_runs_);

    run_index_ = 0;
    for(auto it = data.begin(); it != data.end(); it++, run_index_++) {
        ResetTestVars();

        for(auto& f : reset_vars_functions_)
            f();

        SetInputsAndOutputs();

        if(do_safe_run_) {
#if defined(__linux__)
            it->status = gcheck::RunForked(timeout_, *it, 1024*1024, std::bind(&FunctionTest::RunOnce, this, std::placeholders::_1), *it);
            it->result = it->result && it->status == OK;
#else
            throw std::runtime_error("Safe running is only supported on linux.");
#endif
        } else {
            RunOnce(*it);
        }
        it->timeout = timeout_;
    }
    AddReport(report);
    data_.status = Finished;
}

} // gcheck
