#pragma once

#include <type_traits>
#include <functional>
#include <chrono>

#include "macrotools.h"
#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"
#include "function_test.h"

namespace gcheck {

template<typename ObjectType>
class MethodPlugin {
public:
    typedef std::tuple<bool, UserObject, UserObject> StateDiff;
    template<typename... Args>
    MethodPlugin(FunctionTest<Args...>* test) {
        test->AddResetTest([this](){ this->ResetTestVars(); });
        test->AddPreRun([this](auto&&... args){ this->PreRun(std::forward<decltype(args)>(args)...); });
        test->AddPostRun([this](auto&&... args){ this->PostRun(std::forward<decltype(args)>(args)...); });
    }
    ~MethodPlugin() {
        FreeObject();
        FreeObjectAfter();
    }
protected:
    ObjectType* object_ = nullptr;
    bool owns_object_ = false;
    const ObjectType* object_after_ = nullptr;
    bool owns_object_after_ = false;

    void SetObject(ObjectType* ptr, bool ownership = false) {
        FreeObject();
        object_ = ptr;
        owns_object_ = ownership;
    }
    void SetObject(ObjectType& obj) { SetObject(&obj); }
    void SetObjectAfter(const ObjectType* ptr, bool ownership = false) {
        FreeObjectAfter();
        object_after_ = ptr;
        owns_object_after_ = ownership;
    }
    void SetObjectAfter(const ObjectType& obj) { SetObjectAfter(&obj); }
    void SetStateComparer(const std::function<StateDiff(const ObjectType&)>& func) {
        state_comparer_ = func;
    }
    void ResetTestVars() {
        state_comparer_ = nullptr;
        FreeObject();
        FreeObjectAfter();
    }
private:
    std::function<StateDiff(const ObjectType&)> state_comparer_;

    void FreeObject() {
        if(owns_object_) {
            delete object_;
            object_ = nullptr;
            owns_object_ = false;
        }
    }
    void FreeObjectAfter() {
        if(owns_object_after_) {
            delete object_after_;
            object_after_ = nullptr;
            owns_object_after_ = false;
        }
    }

    void PreRun(size_t, FunctionEntry& entry) {
        entry.object = *object_;
    }
    void PostRun(size_t, FunctionEntry& entry) {
        entry.object_after = *object_;

        if(state_comparer_) {
            StateDiff diff = state_comparer_(*object_);
            entry.result = entry.result && std::get<0>(diff);
        }
        if(object_after_) {
            entry.object_after_expected = *object_after_;
            entry.result = entry.result && *object_ == *object_after_;
        }
    }
};

/*
    Base class for testing class methods.
*/
template<typename ReturnT, typename ObjectType, typename... Args>
class MethodTest : public FunctionTest<ReturnT, Args...>, public MethodPlugin<ObjectType> {
public:
    typedef std::tuple<bool, UserObject, UserObject> StateDiff;
    MethodTest(const TestInfo& info, int num_runs, const std::function<ReturnT(ObjectType*, Args...)>& func)
            : FunctionTest<ReturnT, Args...>(info, num_runs, [this, func](Args&&... args){ return func(this->object_, std::forward<Args>(args)...); }),
            MethodPlugin<ObjectType>(this) { }

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

#define _METHODTEST7(...) _METHODTEST5(__VA_ARGS__)
#define _METHODTEST6(...) _METHODTEST5(__VA_ARGS__)
#define _METHODTEST5(suitename, testname, num_runs, ...) \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::MethodTest<ReturnT, ObjectType, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPreRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPostRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        using gcheck::MethodTest<ReturnT, ObjectType, Args...>::SetObject; \
        using gcheck::MethodTest<ReturnT, ObjectType, Args...>::SetObjectAfter; \
        using gcheck::MethodTest<ReturnT, ObjectType, Args...>::SetStateComparer; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(const std::function<ReturnT(ObjectType*, Args...)>& func) : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(ObjectType*, Args...)) : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...) const) \
                : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...)) \
                : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(__HEAD(__VA_ARGS__)); \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, ObjectType, Args...>::SetInputsAndOutputs()

#define _METHODTEST4(suitename, testname, num_runs, tobetested) \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::MethodTest<ReturnT, ObjectType, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPreRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPostRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        using gcheck::MethodTest<ReturnT, ObjectType, Args...>::SetObject; \
        using gcheck::MethodTest<ReturnT, ObjectType, Args...>::SetObjectAfter; \
        using gcheck::MethodTest<ReturnT, ObjectType, Args...>::SetStateComparer; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(const std::function<ReturnT(ObjectType*, Args...)>& func) : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(ObjectType*, Args...)) : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...) const) \
                : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(ObjectType::*func)(Args...)) \
                : gcheck::MethodTest<ReturnT, ObjectType, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, std::function<ReturnT(ObjectType*, Args...)>(func)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(tobetested); \
    template<typename ReturnT, typename ObjectType, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, ObjectType, Args...>::SetInputsAndOutputs()

// params: suite name, test name, number of runs, method to be tested, points (optional), prerequisites (optional)
#define METHODTEST(...) \
    VFUNC(_METHODTEST, __VA_ARGS__)
