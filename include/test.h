#pragma once

/**
 * @file test.h
 * @brief Core orchestration types for test metadata, prerequisites, state, and execution.
 *
 * This header defines the core building blocks that higher-level test
 * frameworks (`FunctionTest`, `IOTest`, `MethodTest`, `CustomTest`) rely on:
 * 1. @ref Prerequisite for dependency constraints between tests.
 * 2. @ref TestData for mutable runtime/scoring/report state.
 * 3. @ref TestInfo for immutable-style metadata at construction time.
 * 4. @ref Test for lifecycle, registration, and global execution flow.
 *
 * How these pieces work together:
 * 1. A derived test constructs `TestInfo` with names/points/prerequisites.
 * 2. `Test` constructor copies metadata into `TestData` and registers itself.
 * 3. `Test::RunTests()` executes tests when @ref Prerequisite conditions are met.
 * 4. `AddReport(...)` appends @ref TestReport entries into @ref TestData::reports.
 * 5. `TestData::CalculatePoints()` computes final points from report outcomes.
 *
 * Minimal derived usage sketch:
 * @code{.cpp}
 * class MyCase : public gcheck::Test {
 * public:
 *     MyCase() : Test(gcheck::TestInfo("suite", "name", 2.0)) {}
 * private:
 *     void ActualTest() override {
 *         auto r = gcheck::TestReport::Make<gcheck::TrueData>();
 *         auto& d = r.Get<gcheck::TrueData>();
 *         d.result = true;
 *         d.value = true;
 *         d.descriptor = "sanity";
 *         AddReport(r);
 *         data_.status = gcheck::Finished;
 *     }
 * };
 * static MyCase instance;
 * @endcode
 */

#include <cmath>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "core_types.h"
#include "report_types.h"

namespace gcheck {

class Test;
/**
 * @brief Represents prerequisite dependencies for one test.
 *
 * Typical usage scenario:
 * 1. Create from a default suite plus a prerequisite string
 *    (for example `"setup case1.other_test"`).
 * 2. Attach it to @ref TestInfo.
 * 3. Runner checks @ref IsFulfilled before executing the dependent test.
 *
 * Dependency syntax:
 * - `"test_name"` resolves to `(default_suite, test_name)`
 * - `"suite_name.test_name"` resolves explicitly
 * - tokens are whitespace separated
 */
class Prerequisite {
public:
	/** @brief Construct with no prerequisites. */
	Prerequisite() {}
	/**
	 * @brief Parse prerequisite tokens into internal `(suite, test)` names.
	 *
	 * @param default_suite Suite used when a token omits explicit suite.
	 * @param prereqs Whitespace-separated prerequisite tokens.
	 */
	Prerequisite(std::string default_suite, std::string prereqs);

	/**
	 * @brief Resolve prerequisite names to test pointers (lazy) and evaluate fulfillment.
	 *
	 * @return `true` when all resolved prerequisite tests exist and have passed.
	 */
	bool IsFulfilled();
	/**
	 * @brief Evaluate fulfillment using already resolved tests.
	 *
	 * @return `true` when all resolved prerequisite tests have passed.
	 */
	bool IsFulfilled() const;
	/**
	 * @brief Return detailed prerequisite status rows for reporting/JSON.
	 *
	 * @return Vector of `(suite, test, is_passed)` tuples.
	 */
	std::vector<std::tuple<std::string, std::string, bool>> GetFulfillmentData() const;
private:
	/** @brief Populate @ref tests_ from @ref names_ when needed. */
	void FetchTests();

	/** @brief Parsed prerequisite names as `(suite, test)` pairs. */
	std::vector<std::pair<std::string, std::string>> names_;
	/** @brief Resolved test pointers matching @ref names_. */
	std::vector<Test*> tests_;
};

/**
 * @brief Mutable runtime state for a single test instance.
 *
 * Typical usage scenario:
 * 1. Constructed by @ref Test from @ref TestInfo.
 * 2. Receives report entries during `ActualTest()` via @ref Test::AddReport.
 * 3. Runner updates status and captured streams.
 * 4. @ref CalculatePoints computes final score from correctness counts.
 */
struct TestData {
	/** @brief String storage type for textual fields. */
	typedef std::string string;
	/** @brief Report item type stored per assertion/case batch. */
	typedef TestReport TReport;
	/** @brief Prerequisite policy controlling execution eligibility. */
	Prerequisite prerequisite;

	/** @brief Collected report entries emitted by test logic. */
	std::vector<TReport> reports;
	/** @brief Points aggregation policy. */
	GradingMethod grading_method = Partial;
	/** @brief Formatter output style hint (`vertical`, `horizontal`, ...). */
	string output_format = "vertical";
	/** @brief Execution lifecycle status. */
	TestStatus status = NotStarted;

	/** @brief Computed awarded points after evaluation. */
	double points = 0;
	/** @brief Maximum achievable points. */
	double max_points = 0;

	/** @brief Captured stdout from test run wrapper. */
	string sout = "";
	/** @brief Captured stderr from test run wrapper. */
	string serr = "";

	/** @brief Number of passing checks/cases. */
	int correct = 0;
	/** @brief Number of failing checks/cases. */
	int incorrect = 0;

	/**
	 * @brief Construct runtime state with score cap and prerequisite policy.
	 *
	 * @param points Maximum points for the test.
	 * @param prerequisite Parsed prerequisite policy.
	 */
	TestData(double points, Prerequisite prerequisite) : prerequisite(prerequisite), max_points(points) {}
	/**
	 * @brief Copy constructor.
	 *
	 * Performs deep copy of report vector and scalar state.
	 *
	 * @param td Source state.
	 */
	TestData(const TestData& td) {
		reports = std::vector<TReport>(td.reports.begin(), td.reports.end());
		grading_method = td.grading_method;
		output_format = td.output_format;
		status = td.status;
		points = td.points;
		max_points = td.max_points;
		sout = td.sout;
		serr = td.serr;
		correct = td.correct;
		incorrect = td.incorrect;
		prerequisite = td.prerequisite;
	}
	/**
	 * @brief Copy assignment.
	 *
	 * Copies all runtime/scoring fields and reports.
	 *
	 * @param td Source state.
	 * @return Reference to `*this`.
	 */
	TestData& operator=(const TestData& td) {
		reports = std::vector<TReport>(td.reports.begin(), td.reports.end());
		grading_method = td.grading_method;
		output_format = td.output_format;
		status = td.status;
		points = td.points;
		max_points = td.max_points;
		sout = td.sout;
		serr = td.serr;
		correct = td.correct;
		incorrect = td.incorrect;
		return *this;
	}

	/**
	 * @brief Compute awarded points from status, grading method, and counters.
	 *
	 * Rules:
	 * - Non-finished test gets `0` points.
	 * - `Partial` uses ratio `correct/(correct+incorrect)`.
	 * - Other modes apply threshold/all-or-nothing logic.
	 * - NaN/inf fallbacks to `max_points` (zero-checks edge case).
	 */
	void CalculatePoints() {
		if(status != Finished) {
			points = 0;
			return;
		}

		if(grading_method == Partial)
			points = correct/double(correct+incorrect)*max_points;
		else if(grading_method == AllOrNothing)
			points = incorrect == 0 ? max_points : 0;
		else if(grading_method == Most)
			points = incorrect <= correct ? max_points : 0;
		else if(grading_method == StrictMost)
			points = incorrect < correct ? max_points : 0;
		else
			points = 0;

		if(std::isinf(points) || std::isnan(points))
			points = max_points;
	}
};

/**
 * @brief Immutable-like metadata record used to configure a test instance.
 *
 * `TestInfo` captures identity and grading setup:
 * - suite/test name pair
 * - max points
 * - prerequisites
 *
 * Typical usage scenario:
 * 1. A derived test constructor builds `TestInfo("suite", "test", points, prereq)`.
 * 2. Base @ref Test constructor consumes it and initializes @ref TestData.
 */
struct TestInfo {
	/** @brief Default point value used by constructors without explicit points. */
	static double default_points;

	/** @brief Suite/group name. */
	std::string suite;
	/** @brief Test case name (unique within suite). */
	std::string test;
	/** @brief Maximum points assignable to this test. */
	double max_points;
	/** @brief Prerequisite constraints required before execution. */
	Prerequisite prerequisite;

	/**
     * @brief Full constructor.
     *
     * @param suite Suite name.
     * @param test Test name.
     * @param points Max points.
     * @param prerequisite Parsed prerequisite object.
     */
	TestInfo(std::string suite, std::string test, double points, Prerequisite prerequisite = Prerequisite());
	/**
     * @brief Constructor using @ref default_points.
     *
     * @param suite Suite name.
     * @param test Test name.
     * @param prerequisite Parsed prerequisite object.
     */
	TestInfo(std::string suite, std::string test, Prerequisite prerequisite = Prerequisite());
	/**
     * @brief Constructor from prerequisite string.
     *
     * @param suite Suite name.
     * @param test Test name.
     * @param points Max points.
     * @param prerequisite Space-separated prerequisite string.
     */
	TestInfo(std::string suite, std::string test, double points, std::string prerequisite);
	/**
     * @brief Constructor using @ref default_points and prerequisite string.
     *
     * @param suite Suite name.
     * @param test Test name.
     * @param prerequisite Space-separated prerequisite string.
     */
	TestInfo(std::string suite, std::string test, std::string prerequisite);
};

/**
 * @brief Abstract base class that all concrete gcheck tests ultimately derive from.
 *
 * Responsibilities:
 * 1. Auto-register each test instance into global registry.
 * 2. Own shared per-test data (`TestData`) and report list.
 * 3. Wrap execution with stdout/stderr capture and point calculation.
 * 4. Provide static APIs for global test run and test lookup.
 *
 * Relationship to higher-level test classes:
 * - `FunctionTest` adds argument/return/multi-run semantics.
 * - `IOTest` extends `FunctionTest` with stdin/stdout/stderr expectations.
 * - `MethodTest`/`MethodIOTest` add object-state semantics.
 * - `CustomTest` exposes expectation helpers for custom logic.
 *
 * Lifecycle:
 * 1. Derived constructor calls `Test(TestInfo)`.
 * 2. User/runner calls `RunTests()` globally.
 * 3. `RunTest()` invokes `ActualTest()` with output capture active.
 * 4. Derived class pushes report records via `AddReport(...)`.
 * 5. Points and pass/fail status are computed from collected records.
 *
 * Typical usage scenario:
 * 1. Define a static derived test object.
 * 2. Override `ActualTest()` and emit reports.
 * 3. Execute all tests by calling `Test::RunTests()` from main.
 */
class Test {
	// Contains all the tests. It's a function to get around the static initialization order problem
	/** @brief Global test registry storage (function-local static for init-order safety). */
	static std::vector<Test *> &test_list_() {
		static std::unique_ptr<std::vector<Test *>> list(new std::vector<Test *>());
		return *list;
	}

	/** @brief Derived test body implementation hook. */
	virtual void ActualTest() = 0; // The test function specified by inheritor

	/** @brief Internal execution wrapper used by global runner. */
	void RunTest(); // Runs the test and takes care of result logging to Formatter
protected:
	/** @brief Collected report/status/scoring data for this test. */
	TestData data_;
	/** @brief Suite name copied from @ref TestInfo. */
	std::string suite_;
	/** @brief Test name copied from @ref TestInfo. */
	std::string test_;

	/**
     * @brief Append one report record and update correct/incorrect counters.
     *
     * @param report Report entry to store.
     * @return Reference to stored report instance in internal vector.
     */
	TestReport &AddReport(TestReport &report);
	/**
     * @brief Set scoring policy for this test.
     *
     * @param method Grading method (`Partial`, `AllOrNothing`, ...).
     */
	void SetGradingMethod(GradingMethod method);
	/**
     * @brief Set output display format hint.
     *
     * @param format Output format string used by formatter.
     */
	void OutputFormat(std::string format);

public:
	/**
     * @brief Construct and register test instance.
     *
     * @param info Test metadata.
     */
	Test(const TestInfo &info);

	/**
     * @brief Return whether this test fully passed (full points, finished status).
     */
	bool IsPassed() const;
	/** @brief Suite name accessor. */
	const std::string &GetSuite() const {
		return suite_;
	}
	/** @brief Test name accessor. */
	const std::string &GetTest() const {
		return test_;
	}

	/** @brief Global flag enabling safe-run mode where supported. */
	static bool do_safe_run_;

	/**
     * @brief Execute all registered tests respecting prerequisite constraints.
     *
     * @return `true` when all registered tests were executed.
     */
	static bool RunTests();
	/**
     * @brief Find registered test by suite and test name.
     *
     * @param suite Suite name.
     * @param test Test name.
     * @return Matching test pointer or `nullptr`.
     */
	static Test *FindTest(std::string suite, std::string test);
};

} // namespace gcheck
