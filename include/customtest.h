#pragma once

/**
 * @file customtest.h
 * @brief High-level custom test base class and assertion macros for author-defined checks.
 *
 * This header provides:
 * - @ref gcheck::CustomTest, a `Test`-derived base where users implement
 *   arbitrary test logic in @ref gcheck::CustomTest::TheTest.
 * - Helper comparison/expectation methods that append rich @ref gcheck::TestReport
 *   entries to the current test.
 * - Convenience macros (`EXPECT_*`, `ASSERT_*`, `FAIL`) for concise test code.
 *
 * Typical usage flow:
 * 1. Derive from @ref gcheck::CustomTest.
 * 2. Implement @ref gcheck::CustomTest::TheTest and call `EXPECT_*` helpers.
 * 3. Instantiate a static object so the framework auto-registers the test.
 * 4. Execute via `Test::RunTests()`.
 */

#include <string>
#include <sstream>

#include "test.h"
#include "sfinae.h"
#include "user_object.h"
#include "multiprocessing.h"

namespace gcheck {

/**
 * @brief Base class for fully custom test bodies with expectation helpers.
 *
 * `CustomTest` is used when built-in function/method/io test patterns are not
 * sufficient and the test author wants full control over execution logic.
 *
 * Typical usage scenario:
 * @code{.cpp}
 * class MyCustomCase : public gcheck::CustomTest {
 * public:
 *     MyCustomCase() : CustomTest(gcheck::TestInfo("suite", "name", 2.0)) {}
 * private:
 *     void TheTest() override {
 *         int x = 2 + 2;
 *         EXPECT_EQ(x, 4) << "basic arithmetic";
 *         EXPECT_TRUE(x > 0);
 *     }
 * };
 * static MyCustomCase my_case;
 * @endcode
 *
 * Execution model:
 * - @ref ActualTest is framework-invoked and handles optional safe-run mode.
 * - User-defined assertions belong in @ref TheTest.
 */
class CustomTest : public Test {
    /**
     * @brief Framework execution wrapper.
     *
     * Runs @ref TheTest directly or in safe-run mode (Linux fork path) when
     * `Test::do_safe_run_` is enabled.
     *
     * Status effects:
     * - sets @ref TestData::status to `Finished` on success
     * - sets `TimedOut` on timeout in safe mode
     */
    void ActualTest() override;
    /**
     * @brief User-implemented test body.
     *
     * Typical usage:
     * - call `EXPECT_*` / `ASSERT_*` macros and helper methods
     * - add optional extra info with returned stream (`<< "details"`)
     * - avoid manually setting final status; wrapper handles it
     */
    virtual void TheTest() = 0;

protected:
    /** @brief Timeout in seconds used by safe-run mode (`RunForked`). */
    double timeout_ = 0;
    /**
     * @brief Compare tested callable output against a reference callable over multiple runs.
     *
     * For each run:
     * - advance argument generators (`gcheck::advance(args...)`)
     * - evaluate `correct(args...)` and `under_test(args...)`
     * - append one @ref CaseEntry inside a @ref CaseData report
     *
     * Typical usage scenario:
     * @code{.cpp}
     * CompareWithCallable(20, reference_fn, student_fn, arg_gen1, arg_gen2);
     * @endcode
     *
     * @tparam F Reference callable type.
     * @tparam S Tested callable type.
     * @tparam Args Argument or generator argument types.
     * @param num Number of runs/cases.
     * @param correct Reference callable.
     * @param under_test Callable under test.
     * @param args Arguments/generators forwarded per run.
     */
    template <class F, class S, class... Args>
    void CompareWithCallable(int num, const F& correct, const S& under_test, Args&... args);

    /**
     * @brief Compare tested callable against one constant expected answer across runs.
     *
     * Internally wraps constant `correct` into a callable and delegates to
     * @ref CompareWithCallable.
     *
     * Typical usage scenario:
     * @code{.cpp}
     * CompareWithAnswer(10, 42, student_fn, arg_gen);
     * @endcode
     *
     * @tparam T Expected answer type.
     * @tparam S Tested callable type.
     * @tparam Args Argument or generator argument types.
     * @param num Number of runs/cases.
     * @param correct Constant expected value for every run.
     * @param under_test Callable under test.
     * @param args Arguments/generators forwarded per run.
     */
    template <class T, class S, class... Args>
    void CompareWithAnswer(int num, const T& correct, const S& under_test, Args&... args);

    /**
     * @brief Compare tested callable against per-run expected answers from a vector.
     *
     * For run `i`, expected value is `correct[i]`.
     * Internally delegates to @ref CompareWithCallable.
     *
     * Typical usage scenario:
     * @code{.cpp}
     * std::vector<int> expected = {1, 4, 9, 16};
     * CompareWithAnswer(4, expected, student_fn, input_gen);
     * @endcode
     *
     * @tparam T Expected answer element type.
     * @tparam S Tested callable type.
     * @tparam Args Argument or generator argument types.
     * @param num Number of runs/cases.
     * @param correct Expected values indexed by run.
     * @param under_test Callable under test.
     * @param args Arguments/generators forwarded per run.
     */
    template <class T, class S, class... Args>
    void CompareWithAnswer(int num, const std::vector<T>& correct, const S& under_test, Args&... args);

    /**
     * @brief Record `b` as an expectation that should be true.
     *
     * Appends a @ref TrueData report and returns its info stream so callers can
     * add custom text via stream insertion.
     *
     * @param b Evaluated boolean.
     * @param descriptor Human-readable expression/description.
     * @return Stream attached to stored report item.
     */
    std::stringstream& ExpectTrue(bool b, std::string descriptor);
    /**
     * @brief Record `b` as an expectation that should be false.
     *
     * Appends a @ref FalseData report and returns its info stream.
     *
     * @param b Evaluated boolean.
     * @param descriptor Human-readable expression/description.
     * @return Stream attached to stored report item.
     */
    std::stringstream& ExpectFalse(bool b, std::string descriptor);
    /**
     * @brief Record equality expectation between two values.
     *
     * Appends an @ref EqualsData report with `left == right` result.
     *
     * @tparam T Expected value type.
     * @tparam S Actual value type.
     * @param expected Left/expected value.
     * @param output Right/actual value.
     * @param descriptor Human-readable comparison text.
     * @return Stream attached to stored report item.
     */
    template <class T, class S>
    std::stringstream& ExpectEqual(T expected, S output, std::string descriptor);
    /**
     * @brief Record inequality expectation between two values.
     *
     * Appends an @ref EqualsData report with `left != right` result.
     *
     * @tparam T Left value type.
     * @tparam S Right value type.
     * @param expected Left value.
     * @param output Right value.
     * @param descriptor Human-readable comparison text.
     * @return Stream attached to stored report item.
     */
    template <class T, class S>
    std::stringstream& ExpectInequal(T expected, S output, std::string descriptor);

public:
    /**
     * @brief Construct custom test and register through base @ref Test.
     *
     * Typical usage scenario:
     * - Called from derived-class constructor initializer list.
     *
     * @param info Test metadata (suite, name, points, prerequisites).
     */
    CustomTest(const TestInfo& info) : Test(info) {}
};

/**
 * @brief Compare against a constant answer by forwarding to callable comparison.
 *
 * @tparam T Constant expected value type.
 * @tparam S Tested callable type.
 * @tparam Args Argument or generator argument types.
 * @param num Number of runs/cases.
 * @param correct Constant expected value.
 * @param under_test Callable under test.
 * @param args Arguments/generators forwarded per run.
 */
template <class T, class S, class... Args>
void CustomTest::CompareWithAnswer(int num, const T& correct, const S& under_test, Args&... args) {
    auto forwarder = [&correct](auto...) -> T { return correct; };
    CompareWithCallable(num, forwarder, under_test, args...);
    //TODO: not tested.
}

/**
 * @brief Compare against a sequence of expected answers by forwarding to callable comparison.
 *
 * @tparam T Expected answer element type.
 * @tparam S Tested callable type.
 * @tparam Args Argument or generator argument types.
 * @param num Number of runs/cases.
 * @param correct Expected values, indexed by run.
 * @param under_test Callable under test.
 * @param args Arguments/generators forwarded per run.
 */
template <class T, class S, class... Args>
void CustomTest::CompareWithAnswer(int num, const std::vector<T>& correct, const S& under_test, Args&... args) {
    int index = 0;
    auto forwarder = [&index, &correct](auto...) -> T { return correct[index++]; };
    CompareWithCallable(num, forwarder, under_test, args...);
    //TODO: not tested.
}

/**
 * @brief Extract plain argument value for non-`Argument<T>` inputs.
 *
 * Typical usage:
 * - Internal helper for serializing case arguments before invocation.
 *
 * @tparam T Argument type not derived from `Argument<...>`.
 * @param n Input argument.
 * @return Forwarded value.
 */
template<typename T, class = std::enable_if_t<!is_base_of_template<std::remove_cv_t<std::remove_reference_t<T>>, Argument>::value>>
auto extract_argument(T&& n) {
    return std::forward<T>(n);
}

/**
 * @brief Materialize runtime value from `Argument<T>` generator wrapper.
 *
 * Typical usage:
 * - Internal helper to store current generated argument value in reports.
 *
 * @tparam T Generated value type.
 * @param n Generator wrapper.
 * @return Generated value from `n()`.
 */
template<typename T>
auto extract_argument(const Argument<T>& n) {
    return n();
}

/**
 * @brief Run multiple callable comparisons and append one aggregate case report.
 *
 * Report behavior:
 * - Creates one @ref TestReport containing @ref CaseData with `num` entries.
 * - Each entry stores arguments, expected output, actual output, input snapshot,
 *   and pass/fail result.
 *
 * @tparam F Reference callable type.
 * @tparam S Tested callable type.
 * @tparam Args Argument/generator argument types.
 * @param num Number of runs/cases.
 * @param correct Reference callable.
 * @param under_test Callable under test.
 * @param args Arguments/generators forwarded each run.
 */
template <class F, class S, class... Args>
void CustomTest::CompareWithCallable(int num, const F& correct, const S& under_test, Args&... args) {

    TestReport report = TestReport::Make<CaseData>();
    auto& data = report.Get<CaseData>();

    data.resize(num);

    for(auto it = data.begin(); it != data.end(); it++) {

        gcheck::advance(args...);

        it->arguments = UserObject(std::tuple(extract_argument(args)...));
        auto correct_res = Result(correct(args...));
        auto correct_ans = correct_res.output;
        it->output_expected = UserObject(correct_ans);

        if(correct_res.GetInput())
            it->input = *correct_res.GetInput();
        else
            it->input = it->arguments;

        Result res(under_test(args...));
        it->output = UserObject(res.output);
        it->result = res.output == correct_ans;
    }
    AddReport(report);
}

/**
 * @brief Record equality expectation and return report info stream.
 *
 * Typical usage:
 * @code{.cpp}
 * ExpectEqual(a, b, "a == b") << "extra context";
 * @endcode
 *
 * @tparam T Expected value type.
 * @tparam S Actual value type.
 * @param left Expected value.
 * @param right Actual value.
 * @param descriptor Description shown in output.
 * @return Stream for additional info text.
 */
template <class T, class S>
std::stringstream& CustomTest::ExpectEqual(T left, S right, std::string descriptor) {

    TestReport report = TestReport::Make<EqualsData>();
    auto& data = report.Get<EqualsData>();

    data.output_expected = left;
    data.output = right;
    data.descriptor = descriptor;
    data.result = left == right;

    return AddReport(report).info_stream;
}

/**
 * @brief Record inequality expectation and return report info stream.
 *
 * Typical usage:
 * @code{.cpp}
 * ExpectInequal(a, b, "a != b") << "values should differ";
 * @endcode
 *
 * @tparam T Left value type.
 * @tparam S Right value type.
 * @param left Left value.
 * @param right Right value.
 * @param descriptor Description shown in output.
 * @return Stream for additional info text.
 */
template <class T, class S>
std::stringstream& CustomTest::ExpectInequal(T left, S right, std::string descriptor) {

    TestReport report = TestReport::Make<EqualsData>();
    auto& data = report.Get<EqualsData>();

    data.output_expected = left;
    data.output = right;
    data.descriptor = descriptor;
    data.result = left != right;

    return AddReport(report).info_stream;
}

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template std::stringstream& CustomTest::ExpectEqual(unsigned int left, unsigned int right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(int left, int right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(float left, float right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(double left, double right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(std::string left, std::string right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(std::string left, const char* right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(const char* left, std::string right, std::string descriptor);

} // gcheck


/** @brief Assert-style helper that records `b` should be true. */
#define EXPECT_TRUE(b) \
    ExpectTrue(b, #b)

/** @brief Assert-style helper that records `b` should be false. */
#define EXPECT_FALSE(b) \
    ExpectFalse(b, #b)

/** @brief Equality expectation helper: records `left == right`. */
#define EXPECT_EQ(left, right) \
    ExpectEqual(left, right, #left " == " #right)
/** @brief Inequality expectation helper: records `left != right`. */
#define EXPECT_INEQ(left, right) \
    ExpectInequal(left, right, #left " != " #right)

/** @brief Record `b` should be true and abort current test body on failure. */
#define ASSERT_TRUE(b) \
    ExpectTrue(b, #b); \
    if(!(b)) return;

/** @brief Record `b` should be false and abort current test body on failure. */
#define ASSERT_FALSE(b) \
    ExpectFalse(b, #b); \
    if(b) return;

/** @brief Mark test failed immediately using all-or-nothing grading. */
#define FAIL() \
    SetGradingMethod(gcheck::AllOrNothing); \
    ExpectTrue(false, "FAIL")
