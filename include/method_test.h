#pragma once

/**
 * @file method_test.h
 * @brief Method-oriented test helpers built on top of @ref gcheck::FunctionTest.
 *
 * This header provides:
 * - @ref gcheck::MethodPlugin, which attaches object-state capture/comparison
 *   behavior to a function-style test.
 * - @ref gcheck::MethodTest, a base class for testing member-like callables of
 *   the form `ReturnT(ObjectType*, Args...)`.
 *
 * Typical usage flow:
 * 1. Derive from `MethodTest<ReturnT, ObjectType, Args...>`.
 * 2. Configure object-under-test with @ref MethodPlugin::SetObject.
 * 3. Optionally configure expected final object with @ref MethodPlugin::SetObjectAfter
 *    or custom state checker with @ref MethodPlugin::SetStateComparer.
 * 4. Configure per-run expectations in `SetInputsAndOutputs()`.
 */

#include <type_traits>
#include <functional>
#include <chrono>

#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"
#include "function_test.h"

namespace gcheck {

/**
 * @brief Plugin mixin that adds object-state tracking to @ref FunctionTest.
 *
 * `MethodPlugin` registers reset/pre/post hooks into a `FunctionTest` instance:
 * - Pre-run: stores object state before call in @ref FunctionEntry::object.
 * - Post-run: stores object state after call and validates expected state.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * // Inside a MethodTest-derived class:
 * SetObject(&obj);
 * SetObjectAfter(expected_obj);
 * SetStateComparer([](const MyObj& current) {
 *     return std::tuple<bool, gcheck::UserObject, gcheck::UserObject>(current.ok(), {}, {});
 * });
 * @endcode
 *
 * @tparam ObjectType Type of tested object.
 */
template<typename ObjectType>
class MethodPlugin {
public:
    /** @brief State comparison result tuple `(ok, expected_repr, actual_repr)`. */
    typedef std::tuple<bool, UserObject, UserObject> StateDiff;
    /**
     * @brief Construct plugin and register lifecycle hooks into the test.
     *
     * @tparam Args FunctionTest template arguments.
     * @param test Target test instance receiving hook callbacks.
     */
    template<typename... Args>
    MethodPlugin(FunctionTest<Args...>* test) {
        test->AddResetTest([this](){ this->ResetTestVars(); });
        test->AddPreRun([this](auto&&... args){ this->PreRun(std::forward<decltype(args)>(args)...); });
        test->AddPostRun([this](auto&&... args){ this->PostRun(std::forward<decltype(args)>(args)...); });
    }
    /**
     * @brief Destructor that releases owned object pointers.
     */
    ~MethodPlugin() {
        FreeObject();
        FreeObjectAfter();
    }
protected:
    /** @brief Pointer to object under test for current run. */
    ObjectType* object_ = nullptr;
    /** @brief Ownership flag for @ref object_. */
    bool owns_object_ = false;
    /** @brief Pointer to expected object state after call (optional). */
    const ObjectType* object_after_ = nullptr;
    /** @brief Ownership flag for @ref object_after_. */
    bool owns_object_after_ = false;

    /**
     * @brief Set object under test pointer.
     *
     * Frees previous owned object if needed.
     *
     * @param ptr Object pointer for method invocation.
     * @param ownership Whether plugin owns and deletes the pointer.
     */
    void SetObject(ObjectType* ptr, bool ownership = false) {
        FreeObject();
        object_ = ptr;
        owns_object_ = ownership;
    }
    /**
     * @brief Set object under test from reference (non-owning).
     *
     * @param obj Object reference to use.
     */
    void SetObject(ObjectType& obj) { SetObject(&obj); }
    /**
     * @brief Set expected object state after method call.
     *
     * Frees previous owned expected-object if needed.
     *
     * @param ptr Expected object pointer.
     * @param ownership Whether plugin owns and deletes the pointer.
     */
    void SetObjectAfter(const ObjectType* ptr, bool ownership = false) {
        FreeObjectAfter();
        object_after_ = ptr;
        owns_object_after_ = ownership;
    }
    /**
     * @brief Set expected object state from reference (non-owning).
     *
     * @param obj Expected object reference.
     */
    void SetObjectAfter(const ObjectType& obj) { SetObjectAfter(&obj); }
    /**
     * @brief Set custom object-state comparer callback.
     *
     * Callback result boolean is AND-ed into run result.
     *
     * @param func State comparer callable.
     */
    void SetStateComparer(const std::function<StateDiff(const ObjectType&)>& func) {
        state_comparer_ = func;
    }
    /**
     * @brief Reset plugin per-run state.
     *
     * Clears custom comparer and releases owned pointers.
     */
    void ResetTestVars() {
        state_comparer_ = nullptr;
        FreeObject();
        FreeObjectAfter();
    }
private:
    /** @brief Optional custom state-comparison callback. */
    std::function<StateDiff(const ObjectType&)> state_comparer_;

    /** @brief Delete owned @ref object_ and clear ownership flags. */
    void FreeObject() {
        if(owns_object_) {
            delete object_;
            object_ = nullptr;
            owns_object_ = false;
        }
    }
    /** @brief Delete owned @ref object_after_ and clear ownership flags. */
    void FreeObjectAfter() {
        if(owns_object_after_) {
            delete object_after_;
            object_after_ = nullptr;
            owns_object_after_ = false;
        }
    }

    /**
     * @brief Pre-run hook: store current object snapshot into report entry.
     *
     * @param Unused run index.
     * @param entry Run entry being populated.
     */
    void PreRun(size_t, FunctionEntry& entry) {
        entry.object = *object_;
    }
    /**
     * @brief Post-run hook: store final object snapshot and validate state expectations.
     *
     * Validations applied:
     * - Optional custom comparer result.
     * - Optional equality against @ref object_after_.
     *
     * @param Unused run index.
     * @param entry Run entry being finalized.
     */
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

/**
 * @brief Base class for testing callables that operate on an object pointer.
 *
 * `MethodTest` adapts a callable `ReturnT(ObjectType*, Args...)` into
 * @ref FunctionTest by binding the object pointer from @ref MethodPlugin.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * class MyMethodCase : public gcheck::MethodTest<int, MyObj, int> {
 * public:
 *     MyMethodCase()
 *         : MethodTest(gcheck::TestInfo("suite", "method_case"), 5,
 *                      [](MyObj* o, int x){ return o->Compute(x); }) {}
 * private:
 *     void SetInputsAndOutputs() override {
 *         SetObject(my_obj_);
 *         SetArguments(3);
 *         SetReturn(9);
 *     }
 *     MyObj my_obj_;
 * };
 * @endcode
 *
 * @tparam ReturnT Callable return type.
 * @tparam ObjectType Tested object type.
 * @tparam Args Callable argument types (excluding object pointer).
 */
template<typename ReturnT, typename ObjectType, typename... Args>
class MethodTest : public FunctionTest<ReturnT, Args...>, public MethodPlugin<ObjectType> {
public:
    /** @brief State comparison result tuple `(ok, expected_repr, actual_repr)`. */
    typedef std::tuple<bool, UserObject, UserObject> StateDiff;
    /**
     * @brief Construct method test adapter.
     *
     * Wraps `func(ObjectType*, Args...)` into a callable that uses plugin-managed
     * `this->object_` and forwards remaining args.
     *
     * @param info Test metadata.
     * @param num_runs Number of configured runs/cases.
     * @param func Method-like callable under test.
     */
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
