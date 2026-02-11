#pragma once

/**
 * @file method_io_test.h
 * @brief Combined method + IO test base class.
 *
 * This header provides @ref gcheck::MethodIOTest, which composes:
 * - @ref gcheck::FunctionTest execution/report pipeline
 * - @ref gcheck::MethodPlugin object-state tracking
 * - @ref gcheck::IOPlugin stdin/stdout/stderr capture and checks
 *
 * Typical usage flow:
 * 1. Derive from `MethodIOTest<ReturnT, ObjectType, Args...>`.
 * 2. In `SetInputsAndOutputs()`, configure:
 *    - object-under-test via `SetObject(...)`
 *    - method arguments via `SetArguments(...)`
 *    - expected return and IO via `SetReturn(...)`, `SetInput(...)`,
 *      `SetOutput(...)`, `SetError(...)`
 * 3. Let inherited execution run and produce @ref gcheck::FunctionData entries.
 */

#include "gcheck.h"
#include "method_test.h"
#include "io_test.h"

namespace gcheck {

/**
 * @brief Base class for testing object methods with stdin/stdout/stderr behavior.
 *
 * `MethodIOTest` adapts a callable of signature
 * `ReturnT(ObjectType*, Args...)` and combines method-state validation with IO
 * capture/comparison in one test type.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * class MyMethodIOCase : public gcheck::MethodIOTest<void, MyObj, int> {
 * public:
 *     MyMethodIOCase()
 *         : MethodIOTest(gcheck::TestInfo("suite", "method_io_case"), 1,
 *                        [](MyObj* o, int x){ o->Run(x); }) {}
 * private:
 *     void SetInputsAndOutputs() override {
 *         SetObject(obj_);
 *         SetArguments(5);
 *         SetInput("10\n");
 *         SetOutput("ok\n");
 *     }
 *     MyObj obj_;
 * };
 * @endcode
 *
 * @tparam ReturnT Callable return type.
 * @tparam ObjectType Tested object type.
 * @tparam Args Callable argument types (excluding object pointer).
 */
template<typename ReturnT, typename ObjectType, typename... Args>
class MethodIOTest : public FunctionTest<ReturnT, Args...>, public MethodPlugin<ObjectType>, public IOPlugin {
public:
    /** @brief State comparison result tuple `(ok, expected_repr, actual_repr)`. */
    typedef std::tuple<bool, UserObject, UserObject> StateDiff;
    /**
     * @brief Construct combined method+IO test adapter.
     *
     * Wraps `func(ObjectType*, Args...)` into a callable bound to plugin-managed
     * `this->object_`, and initializes MethodPlugin and IOPlugin hooks.
     *
     * @param info Test metadata.
     * @param num_runs Number of configured runs.
     * @param func Method-like callable under test.
     */
    MethodIOTest(const TestInfo& info, int num_runs, const std::function<ReturnT(ObjectType*, Args...)>& func)
            : FunctionTest<ReturnT, Args...>(info, num_runs, [this, func](Args&&... args){ return func(this->object_, std::forward<Args>(args)...); }),
            MethodPlugin<ObjectType>(this), IOPlugin(this) { }

private:
    /** @brief Keep `SetInputsAndOutputs` private in this layer (implemented by derived tests). */
    using FunctionTest<ReturnT, Args...>::SetInputsAndOutputs;
    /** @brief Hide base report append helper from public interface of this adapter. */
    using FunctionTest<ReturnT, Args...>::AddReport;
    /** @brief Hide single-run executor from public interface. */
    using FunctionTest<ReturnT, Args...>::RunOnce;
    /** @brief Hide mutable expected-arguments storage. */
    using FunctionTest<ReturnT, Args...>::args_;
    /** @brief Hide mutable expected-arguments-after storage. */
    using FunctionTest<ReturnT, Args...>::args_after_;
    /** @brief Hide mutable expected-return storage. */
    using FunctionTest<ReturnT, Args...>::expected_return_value_;
    /** @brief Hide run-count storage. */
    using FunctionTest<ReturnT, Args...>::num_runs_;
    /** @brief Hide current run index storage. */
    using FunctionTest<ReturnT, Args...>::run_index_;
    /** @brief Hide argument-after checking flag. */
    using FunctionTest<ReturnT, Args...>::check_arguments_;
};

} // gcheck
