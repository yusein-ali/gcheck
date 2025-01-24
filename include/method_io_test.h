#pragma once

#include "gcheck.h"
#include "method_test.h"
#include "io_test.h"

namespace gcheck {

/*
    Base class for testing methods, their response to standard input, and output in standard output and error.
*/
template<typename ReturnT, typename ObjectType, typename... Args>
class MethodIOTest : public FunctionTest<ReturnT, Args...>, public MethodPlugin<ObjectType>, public IOPlugin {
public:
    typedef std::tuple<bool, UserObject, UserObject> StateDiff;
    MethodIOTest(const TestInfo& info, int num_runs, const std::function<ReturnT(ObjectType*, Args...)>& func)
            : FunctionTest<ReturnT, Args...>(info, num_runs, [this, func](Args&&... args){ return func(this->object_, std::forward<Args>(args)...); }),
            MethodPlugin<ObjectType>(this), IOPlugin(this) { }

private:
    using FunctionTest<ReturnT, Args...>::SetInputsAndOutputs;
    using FunctionTest<ReturnT, Args...>::AddReport;
    using FunctionTest<ReturnT, Args...>::RunOnce;
    using FunctionTest<ReturnT, Args...>::args_;
    using FunctionTest<ReturnT, Args...>::args_after_;
    using FunctionTest<ReturnT, Args...>::expected_return_value_;
    using FunctionTest<ReturnT, Args...>::num_runs_;
    using FunctionTest<ReturnT, Args...>::run_index_;
    using FunctionTest<ReturnT, Args...>::check_arguments_;
};

} // gcheck

#define _METHODIOTEST7(...) _METHODIOTEST5(__VA_ARGS__)
#define _METHODIOTEST6(...) _METHODIOTEST5(__VA_ARGS__)
#define _METHODIOTEST5(suitename, testname, num_runs, ...) \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::MethodIOTest<ReturnT, ObjectType, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetObject; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetObjectAfter; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetStateComparer; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetInput; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetOutput; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetError; \
        using gcheck::Test::OutputFormat; \
        using gcheck::Test::SetGradingMethod; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(const std::function<ReturnT(ObjectType*, Args...)>& func) : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(ObjectType*, Args...)) : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...) const) \
                : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...)) \
                : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(__HEAD(__VA_ARGS__)); \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, ObjectType, Args...>::SetInputsAndOutputs()

#define _METHODIOTEST4(suitename, testname, num_runs, tobetested) \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::MethodIOTest<ReturnT, ObjectType, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetObject; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetObjectAfter; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetStateComparer; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetInput; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetOutput; \
        using gcheck::MethodIOTest<ReturnT, ObjectType, Args...>::SetError; \
        using gcheck::Test::OutputFormat; \
        using gcheck::Test::SetGradingMethod; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(const std::function<ReturnT(ObjectType*, Args...)>& func) : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(ObjectType*, Args...)) : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...) const) \
                : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...)) \
                : gcheck::MethodIOTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(tobetested); \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, ObjectType, Args...>::SetInputsAndOutputs()

// params: suite name, test name, number of runs, method to be tested, points (optional), prerequisites (optional)
#define METHODIOTEST(...) \
    VFUNC(_METHODIOTEST, __VA_ARGS__)
