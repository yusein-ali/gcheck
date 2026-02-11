#pragma once

/**
 * @file io_test.h
 * @brief IO-capable function test helpers for stdin/stdout/stderr validation.
 *
 * This header provides:
 * - @ref gcheck::IOPlugin, a mixin that captures stdout/stderr and injects stdin
 *   around each run.
 * - @ref gcheck::IOTest, a function-test base class combining
 *   @ref gcheck::FunctionTest with @ref gcheck::IOPlugin.
 *
 * Typical usage flow:
 * 1. Derive from `IOTest<ReturnT, Args...>`.
 * 2. In `SetInputsAndOutputs()`, configure inputs/expectations:
 *    - `SetInput(...)`, `SetOutput(...)`, `SetError(...)`
 *    - optional return/argument expectations from `FunctionTest`.
 * 3. Run tests normally; IOPlugin hooks capture/compare IO per run.
 */

#include <functional>
#include <optional>
#include <string>

#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"
#include "redirectors.h"
#include "function_test.h"

namespace gcheck {

/**
 * @brief Plugin mixin that adds stdin injection and stdout/stderr capture checks.
 *
 * `IOPlugin` registers reset/pre/post callbacks into a @ref FunctionTest:
 * - Pre-run: optionally inject stdin and start stream capture.
 * - Post-run: restore streams, store observed IO into @ref FunctionEntry,
 *   and evaluate output/error expectations.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * // Inside a FunctionTest/IOTest-derived class:
 * SetInput("4 5\n");
 * SetOutput("9\n");
 * SetError("");
 * @endcode
 */
class IOPlugin {
public:
    /**
     * @brief Construct plugin and register lifecycle hooks into a FunctionTest.
     *
     * @tparam Args FunctionTest template arguments.
     * @param test Target test instance receiving IO hooks.
     */
    template<typename... Args>
    IOPlugin(FunctionTest<Args...>* test) : tout_(false), terr_(false), tin_(false) {
        test->AddResetTest([this](){ this->ResetTestVars(); });
        test->AddPreRun([this](auto&&... args){ this->PreRun(std::forward<decltype(args)>(args)...); });
        test->AddPostRun([this](auto&&... args){ this->PostRun(std::forward<decltype(args)>(args)...); });
    }
protected:
    /** @brief Stdout capturer active during run execution. */
    StdoutCapturer tout_;
    /** @brief Stderr capturer active during run execution. */
    StderrCapturer terr_;
    /** @brief Stdin injector used to provide synthetic input. */
    StdinInjecter tin_;

    /** @brief Optional stdin content to feed into tested callable. */
    std::optional<std::string> input_;
    /** @brief Optional expected stdout content. */
    std::optional<std::string> expected_output_;
    /** @brief Optional expected stderr content. */
    std::optional<std::string> expected_error_;
    /** @brief Whether stdin stream should be closed after writing input. */
    bool do_close = false;

    /**
     * @brief Set stdin content for current run.
     *
     * @param str Input text to inject.
     * @param close_stream Whether to close stdin after write.
     */
    void SetInput(const std::string& str, bool close_stream = false) { input_ = str; do_close = close_stream; }
    /**
     * @brief Set expected stdout content for current run.
     *
     * @param str Expected stdout text.
     */
    void SetOutput(const std::string& str) { expected_output_ = str; }
    /**
     * @brief Set expected stderr content for current run.
     *
     * @param str Expected stderr text.
     */
    void SetError(const std::string& str) { expected_error_ = str; }

    /**
     * @brief Reset per-run IO expectations and input flags.
     */
    void ResetTestVars() {
        input_.reset();
        expected_output_.reset();
        expected_error_.reset();
        do_close = false;
    }
private:
    /**
     * @brief Pre-run hook: inject stdin (if configured) and start IO capture.
     *
     * @param Unused run index.
     * @param Unused run entry reference.
     */
    void PreRun(size_t, FunctionEntry&) {
        if(input_) {
            tin_.Capture();
            tin_.Write(*input_);
            if(do_close) tin_.Close();
        }

        tout_.Capture();
        terr_.Capture();
    }
    /**
     * @brief Post-run hook: restore streams, persist captured IO, and evaluate expectations.
     *
     * Side effects on @p entry:
     * - stores observed output/error and optional expected fields
     * - updates `entry.result` with output/error comparisons
     *
     * @param Unused run index.
     * @param entry Run entry being finalized.
     */
    void PostRun(size_t, FunctionEntry& entry) {
        tout_.Restore();
        terr_.Restore();
        tin_.Restore();

        if(input_) entry.input = *input_;

        std::string outstr = tout_.str();
        std::string errstr = terr_.str();
        entry.output = outstr;
        if(expected_output_)
            entry.output_expected = *expected_output_;
        entry.error = errstr;
        if(expected_error_)
            entry.error_expected = *expected_error_;
        entry.result = entry.result && (!expected_output_ || *expected_output_ == outstr) && (!expected_error_ || *expected_error_ == errstr);
    }
};

/**
 * @brief Base class for testing functions with IO expectations.
 *
 * `IOTest` composes @ref FunctionTest and @ref IOPlugin to validate:
 * - return values/arguments/runtime (FunctionTest features)
 * - stdin/stdout/stderr behavior (IOPlugin features)
 *
 * Typical usage scenario:
 * @code{.cpp}
 * class MyIOCase : public gcheck::IOTest<int, int, int> {
 * public:
 *     MyIOCase()
 *         : IOTest(gcheck::TestInfo("suite", "io_case"), 1,
 *                  [](int a, int b){ std::cout << a+b << "\n"; return a+b; }) {}
 * private:
 *     void SetInputsAndOutputs() override {
 *         SetArguments(2, 3);
 *         SetOutput("5\n");
 *         SetReturn(5);
 *     }
 * };
 * @endcode
 *
 * @tparam ReturnT Callable return type.
 * @tparam Args Callable argument types.
 */
template<typename ReturnT, typename... Args>
class IOTest : public FunctionTest<ReturnT, Args...>, public IOPlugin {
public:
    /**
     * @brief Construct IO-capable function test.
     *
     * @param info Test metadata.
     * @param num_runs Number of configured runs.
     * @param func Callable under test.
     */
    IOTest(const TestInfo& info, int num_runs, const std::function<ReturnT(Args...)>& func) : FunctionTest<ReturnT, Args...>(info, num_runs, func), IOPlugin(this) { }

private:
    /** @brief Keep run-configuration hook private in this adapter layer. */
    using FunctionTest<ReturnT, Args...>::SetInputsAndOutputs;
    /** @brief Hide base report append helper. */
    using FunctionTest<ReturnT, Args...>::AddReport;
    /** @brief Hide single-run executor from public interface. */
    using FunctionTest<ReturnT, Args...>::RunOnce;
    /** @brief Hide expected-arguments storage. */
    using FunctionTest<ReturnT, Args...>::args_;
    /** @brief Hide expected-arguments-after storage. */
    using FunctionTest<ReturnT, Args...>::args_after_;
    /** @brief Hide expected-return storage. */
    using FunctionTest<ReturnT, Args...>::expected_return_value_;
    /** @brief Hide run-count storage. */
    using FunctionTest<ReturnT, Args...>::num_runs_;
    /** @brief Hide current run index storage. */
    using FunctionTest<ReturnT, Args...>::run_index_;
    /** @brief Hide argument-after checking flag. */
    using FunctionTest<ReturnT, Args...>::check_arguments_;
};

} // gcheck
