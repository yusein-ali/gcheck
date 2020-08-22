#pragma once

#include <type_traits>
#include <functional>

#include "macrotools.h"
#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"

namespace gcheck {

namespace {

    template<typename... Args>
    struct TupleWrapper : public std::tuple<Args...> {
        typedef std::tuple<Args...> tuple_type;
        typedef std::tuple<std::remove_const_t<Args>...> constless_tuple_type;
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
        typedef std::tuple<> constless_tuple_type;
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
    typedef typename ConditionalT<ReturnT>::type ReturnType;
    typedef TupleWrapper<typename std::remove_reference<Args>::type...> TupleWrapperType;
    typedef typename TupleWrapperType::constless_tuple_type ConstlessTupleType;
    typedef typename TupleWrapperType::tuple_type TupleType;
    typedef void(*CompareFunc)(void);

    FunctionTest(const TestInfo& info, int num_runs, const std::function<ReturnT(Args...)>& func) : Test(info), num_runs_(num_runs), function_(func) { }
protected:
    CompareFunc comp_function_ = nullptr;

    std::optional<ConstlessTupleType> args_;
    std::optional<ConstlessTupleType> args_after_;
    std::optional<ReturnType> expected_return_value_;

    std::optional<ConstlessTupleType> last_args_;
    int num_runs_;
    size_t run_index_ = 0;
    bool check_arguments_ = true;

    // Sets the input arguments given to the tested function
    void SetArguments(const typename std::remove_reference<Args>::type&... args) { args_ = std::tuple(args...);}
    void SetArguments(const TupleType& args) {
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

    const std::optional<TupleType>& GetLastArguments() const { return last_args_; }
    size_t GetRunIndex() { return run_index_; }

    virtual void SetInputsAndOutputs() = 0;

    void RunOnce(FunctionEntry& data);
    void ResetTestVars();
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
                std::apply(function_, args);
                data.result = true;
            } else {
                auto ret = std::apply(function_, args);
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
        function_();
        data.result = !args_after_ && !args_;
    } else {
        auto ret = function_();
        data.return_value = ret;
        if(expected_return_value_)
            data.return_value_expected = *expected_return_value_;
        data.result = expected_return_value_ == ret && (!args_after_ && !args_);
    }
}

template<typename ReturnT, typename... Args>
void FunctionTest<ReturnT, Args...>::ActualTest() {

    TestReport report = TestReport::Make<FunctionData>();
    auto& data = report.Get<FunctionData>();

    data.resize(num_runs_);

    run_index_ = 0;
    for(auto it = data.begin(); it != data.end(); it++, run_index_++) {
        ResetTestVars();

        SetInputsAndOutputs();

        RunOnce(*it);
    }
    AddReport(report);
}

} // gcheck

#define _FUNCTIONTEST7(...) _FUNCTIONTEST5(__VA_ARGS__)
#define _FUNCTIONTEST6(...) _FUNCTIONTEST5(__VA_ARGS__)
#define _FUNCTIONTEST5(suitename, testname, num_runs, ...) \
    template<typename ReturnT, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::FunctionTest<ReturnT, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
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
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
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
