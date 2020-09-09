#pragma once

#include <type_traits>
#include <functional>
#include <chrono>
#include <stdexcept>

#include "macrotools.h"
#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"
#include "multi_processing.h"

namespace gcheck {

template<typename T>
class DeltaCompare {
public:
    DeltaCompare(T value, T delta = 0) : value_(value), delta_(delta) {}

    T Delta() const { return delta_; }

    T& operator=(T val) { value_ = val; return *this; }

    operator T&() { return value_; }
    operator T() const { return value_; }

    bool operator==(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) == val.value_; }
    bool operator!=(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) != val.value_; }
    bool operator<(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) < val.value_; }
    bool operator>(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) > val.value_; }
    bool operator<=(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) <= val.value_; }
    bool operator>=(const DeltaCompare& val) const { return DeltaCompare(value_, std::min(delta_, val.delta_)) >= val.value_; }

    bool operator==(T val) const { return value_ + delta_ >= val && value_ - delta_ <= val; }
    bool operator!=(T val) const { return !(*this == val); }
    bool operator<(T val) const { return value_ < val; }
    bool operator>(T val) const { return value_ > val; }
    bool operator<=(T val) const { return value_ - delta_ <= val; }
    bool operator>=(T val) const { return value_ + delta_ >= val; }
private:
    T value_;
    T delta_;
};
template<typename T>
bool operator==(T val, const DeltaCompare<T>& dc) { return dc == val; }
template<typename T>
bool operator!=(T val, const DeltaCompare<T>& dc) { return dc != val; }
template<typename T>
bool operator<(T val, const DeltaCompare<T>& dc) { return dc < val; }
template<typename T>
bool operator>(T val, const DeltaCompare<T>& dc) { return dc > val; }
template<typename T>
bool operator<=(T val, const DeltaCompare<T>& dc) { return dc <= val; }
template<typename T>
bool operator>=(T val, const DeltaCompare<T>& dc) { return dc >= val; }

namespace {

    template<typename T>
    struct MakeDelta {
        typedef std::conditional_t<std::is_floating_point_v<T>, DeltaCompare<T>, T> type;
    };
    template<typename T>
    using MakeDelta_t = typename MakeDelta<T>::type;

    template<typename... Args>
    struct TupleWrapper : public std::tuple<Args...> {
        typedef std::tuple<Args...> tuple_type;
        typedef std::tuple<MakeDelta_t<std::remove_const_t<Args>>...> storage_tuple_type;
        typedef std::false_type is_empty;
        TupleWrapper(){}
        TupleWrapper(const std::tuple<Args...>& c) : std::tuple<Args...>(c) {}
        TupleWrapper& operator=(const std::tuple<Args...>& val) {
            std::tuple<Args...>::operator=(val);
            return *this;
        }
    };
    template<>
    struct TupleWrapper<> : public std::tuple<> {
        typedef std::tuple<> tuple_type;
        typedef std::tuple<> storage_tuple_type;
        typedef std::true_type is_empty;
        TupleWrapper(){}
        TupleWrapper(const std::tuple<>& c) : std::tuple<>(c) {}
        TupleWrapper& operator=(const std::tuple<>& val) {
            std::tuple<>::operator=(val);
            return *this;
        }
    };
    template<>
    struct TupleWrapper<void> : public TupleWrapper<> {};

    template<typename T>
    struct ConditionalT {
        typedef T type;
        typedef std::true_type is_defined;
    };
    template<>
    struct ConditionalT<void> {
        typedef char type;
        typedef std::false_type is_defined;
    };
    template<typename T>
    using ConditionalT_t = typename ConditionalT<T>::type;

} // anonymous

/* Creates a function for creating an object on the stack
Usage: constructor<class_name, args1 type, arg2 type, ...>()
Return value is a lambda that calls the constructor
*/
template<typename T, typename... Args>
std::function<T(Args...)> constructor() {
    return [](auto... args){ return T(args...); };
}

/* Creates a function for creating an object on the heap
Usage: new_constructor<class_name, args1 type, arg2 type, ...>()
Return value is a lambda that calls the constructor with new
*/
template<typename T, typename... Args>
std::function<T*(Args...)> new_constructor() {
    return [](auto... args){ return new T(args...); };
}

/* Creates a Generator object for creating an object on the stack
Usage: ClassGenerator<class_name, args1 type, arg2 type, ...>(NextType that generates args for the constructor)
*/
template<typename Class, typename... Args, typename... Args2>
auto ClassGenerator(NextType<Args2...>& args) {
    auto c = constructor<Class, Args...>();
    return Generator([c, &args]() -> Class { return std::apply(c, args.Next()); });
}

/* Creates a Generator object for creating an object on the heap
Usage: ClassGenerator<class_name, args1 type, arg2 type, ...>(NextType that generates args for the constructor)
*/
template<typename Class, typename... Args, typename... Args2>
auto ClassPtrGenerator(NextType<Args2...>& args) {
    auto c = new_constructor<Class, Args...>();
    return Generator([c, &args]() -> Class* { return std::apply(c, args.Next()); });
}

/*
    Base class for testing functions.
*/
template<typename ReturnT, typename... Args>
class FunctionTest : public Test {
public:
    typedef MakeDelta_t<ConditionalT_t<ReturnT>> ReturnType;
    typedef TupleWrapper<std::remove_cv_t<std::remove_reference_t<Args>>...> TupleWrapperType;
    typedef typename TupleWrapperType::storage_tuple_type StorageTupleType;
    typedef typename TupleWrapperType::tuple_type TupleType;
    typedef void(*CompareFunc)(void);

    FunctionTest(const TestInfo& info, int num_runs, const std::function<ReturnT(Args...)>& func) : Test(info), num_runs_(num_runs), function_(func) { }

    template<typename F>
    void AddResetTest(F&& func) {
        reset_vars_functions_.push_back(std::forward<F>(func));
    }
    template<typename F>
    void AddPreRun(F&& func) {
        pre_run_functions_.push_back(std::forward<F>(func));
    }
    template<typename F>
    void AddPostRun(F&& func) {
        post_run_functions_.push_back(std::forward<F>(func));
    }
protected:
    CompareFunc comp_function_ = nullptr;

    std::optional<StorageTupleType> args_;
    std::optional<StorageTupleType> args_after_;
    std::optional<ReturnType> expected_return_value_;
    std::optional<std::chrono::nanoseconds> max_run_time_;
    std::chrono::duration<double> timeout_ = std::chrono::duration<double>::zero();

    std::optional<StorageTupleType> last_args_;
    int num_runs_;
    size_t run_index_ = 0;
    bool check_arguments_ = true;

    std::vector<std::function<void()>> reset_vars_functions_;
    std::vector<std::function<void(size_t, FunctionEntry&)>> pre_run_functions_;
    std::vector<std::function<void(size_t, FunctionEntry&)>> post_run_functions_;

    // Sets the input arguments given to the tested function
    template<typename... Args2>
    void SetArguments(Args2&&... args) {
        args_ = std::tuple(std::forward<Args2>(args)...);
    }
    void SetArguments(const TupleType& args) {
        std::apply([this](auto... x){this->SetArguments(x...);}, args);
    }
    template<typename A = ReturnT, class = std::enable_if_t<std::is_same_v<TupleType, StorageTupleType>>>
    void SetArguments(const StorageTupleType& args) {
        std::apply([this](auto... x){this->SetArguments(x...);}, args);
    }
    // Sets what the input arguments given to the tested function should be after the call
    void SetArgumentsAfter(const typename std::remove_reference<Args>::type&... args) { args_after_ = std::tuple(args...);}
    void SetArgumentsAfter(const TupleType& args) {
        std::apply([this](auto... x){this->SetArgumentsAfter(x...);}, args);
    }
    void IgnoreArgumentsAfter() { check_arguments_ = false; }
    // Sets the expected output (stdout) of tested function
    void SetReturn(const ReturnType& val) { expected_return_value_ = val; }
    void SetMaxRunTime(std::chrono::nanoseconds ns) { max_run_time_ = ns; }
    void SetMaxRunTime(unsigned long long ns) { max_run_time_ = std::chrono::nanoseconds(ns); }
    void SetTimeout(std::chrono::duration<double> seconds) { timeout_ = seconds; }
    void SetTimeout(double seconds) { timeout_ = std::chrono::duration<double>(seconds); }

    const std::optional<TupleType>& GetLastArguments() const { return last_args_; }
    size_t GetRunIndex() { return run_index_; }

    virtual void SetInputsAndOutputs() = 0;

    void RunOnce(FunctionEntry& data);
    virtual void ResetTestVars();
private:
    virtual void ActualTest();

    std::function<ReturnT(Args...)> function_;
};

template<typename ReturnT, typename... Args>
void FunctionTest<ReturnT, Args...>::ResetTestVars() {
    args_.reset();
    args_after_.reset();
    expected_return_value_.reset();
    check_arguments_ = true;
}

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
                data.result = expected_return_value_ == ret;
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
        data.result = expected_return_value_ == ret && (!args_after_ && !args_);
    }

    data.max_run_time = max_run_time_;
    if(max_run_time_)
        data.result = data.result && data.run_time <= max_run_time_.value();

    for(auto& f : post_run_functions_)
        f(run_index_, data);
}

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
            if(!gcheck::RunForked(timeout_, *it, 1024*1024, std::bind(&FunctionTest::RunOnce, this, std::placeholders::_1), *it)) {
                it->timed_out = true;
                it->result = false;
            }
#else
            throw std::runtime_error("Safe running is only supported on linux.");
#endif
        } else {
            RunOnce(*it);
        }
        it->timeout = timeout_;
    }
    AddReport(report);
}

} // gcheck

#define _FUNCTIONTEST7(...) _FUNCTIONTEST5(__VA_ARGS__)
#define _FUNCTIONTEST6(...) _FUNCTIONTEST5(__VA_ARGS__)
#define _FUNCTIONTEST5(suitename, testname, num_runs, ...) \
    template<typename ReturnT, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::FunctionTest<ReturnT, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(std::function<ReturnT(Args...)> func) : gcheck::FunctionTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(Args...)) : gcheck::FunctionTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(__HEAD(__VA_ARGS__)); \
    template<typename ReturnT, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, Args...>::SetInputsAndOutputs()

#define _FUNCTIONTEST4(suitename, testname, num_runs, tobetested) \
    template<typename ReturnT, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::FunctionTest<ReturnT, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(std::function<ReturnT(Args...)> func) : gcheck::FunctionTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(Args...)) : gcheck::FunctionTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(tobetested); \
    template<typename ReturnT, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, Args...>::SetInputsAndOutputs()

// params: suite name, test name, number of runs, function to be tested, points (optional), prerequisites (optional)
#define FUNCTIONTEST(...) \
    VFUNC(_FUNCTIONTEST, __VA_ARGS__)
