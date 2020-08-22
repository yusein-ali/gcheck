#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include <variant>

#include "argument.h"
#include "json.h"
#include "user_object.h"
#include "sfinae.h"
#include "macrotools.h"

namespace gcheck {

template<typename T, typename S = std::string>
struct Result {
    T output;
    std::string error;

    Result() : output() { }
    Result(T out) : output(out) { }

    Result(const Result<T, S>& r) = default;

    Result<T, S>& operator=(const T& item) {
        output = item;
        return *this;
    }
    Result<T, S>& operator=(const Result<T, S>& r) = default;

    void SetInput(const S& data) { input = data; }
    const std::optional<S>& GetInput() const { return input; }
private:
    std::optional<S> input;
};

struct EqualsData {
    UserObject output_expected;
    UserObject output;
    std::string descriptor;
    bool result;
};

struct TrueData {
    bool value;
    std::string descriptor;
    bool result;
};

struct FalseData {
    bool value;
    std::string descriptor;
    bool result;
};

struct CaseEntry {
    std::optional<UserObject> arguments; // arguments to test function
    std::optional<UserObject> input; // arguments to tested function
    std::optional<UserObject> output;
    std::optional<UserObject> output_expected;
    bool result;
};
typedef std::vector<CaseEntry> CaseData;

struct FunctionEntry {
    std::optional<UserObject> input;
    std::optional<UserObject> output;
    std::optional<UserObject> output_expected;
    std::optional<UserObject> error;
    std::optional<UserObject> error_expected;
    std::optional<UserObject> arguments;
    std::optional<UserObject> arguments_after;
    std::optional<UserObject> arguments_after_expected;
    std::optional<UserObject> return_value;
    std::optional<UserObject> return_value_expected;
    std::optional<UserObject> object;
    std::optional<UserObject> object_after;
    std::optional<UserObject> object_after_expected;
    bool result;
};
typedef std::vector<FunctionEntry> FunctionData;

struct TestReport {

    std::shared_ptr<std::stringstream> info_stream;

    std::variant<EqualsData, TrueData, FalseData, CaseData, FunctionData> data;

    template<typename T>
    TestReport(const T& d) : info_stream(std::make_shared<std::stringstream>()), data(d) {}

    TestReport(const TestReport& r) : info_stream(r.info_stream), data(r.data) {}

    template<typename T>
    static TestReport Make(T item = T()) {
        return TestReport(item);
    }

    template<typename T>
    T& Get() {
        return std::get<T>(data);
    }

    template<typename T>
    const T& Get() const {
        return std::get<T>(data);
    }
};

enum GradingMethod {
    Partial,
    AllOrNothing,
    Most,
    StrictMost
};
enum TestStatus : int {
    NotStarted,
    Started,
    Finished
};

class Test;
class Prerequisite {
public:
    Prerequisite() {}
    Prerequisite(std::string default_suite, std::string prereqs);

    bool IsFulfilled();
    bool IsFulfilled() const;
    std::vector<std::tuple<std::string, std::string, bool>> GetFullfillmentData() const;
private:
    void FetchTests();

    std::vector<std::pair<std::string, std::string>> names_;
    std::vector<Test*> tests_;
};

struct TestInfo {
    static double default_points;

    std::string suite;
    std::string test;
    double max_points;
    Prerequisite prerequisite;

    TestInfo(std::string suite, std::string test, double points, Prerequisite prerequisite = Prerequisite()) : suite(suite), test(test), max_points(points), prerequisite(prerequisite) {}
    TestInfo(std::string suite, std::string test, Prerequisite prerequisite = Prerequisite()) : TestInfo(suite, test, default_points, prerequisite) {}
    TestInfo(std::string suite, std::string test, double points, std::string prerequisite) : TestInfo(suite, test, points, Prerequisite(suite, prerequisite)) {}
    TestInfo(std::string suite, std::string test, std::string prerequisite) : TestInfo(suite, test, default_points, prerequisite) {}
};

struct TestData {
    Prerequisite prerequisite;

    std::vector<TestReport> reports;
    GradingMethod grading_method = Partial;
    std::string output_format = "horizontal";
    TestStatus status = NotStarted;

    double points = 0;
    double max_points = 0;

    std::string sout = "";
    std::string serr = "";

    int correct = 0;
    int incorrect = 0;

    TestData(double points, Prerequisite prerequisite) : prerequisite(prerequisite), max_points(points) {}

    void CalculatePoints() {

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

        status = Finished;
    }
};

/*
    Abstract base class for tests. Keeps track of the test's results and options.
*/
class Test {
    // Contains all the tests. It's a function to get around the static initialization order problem
    static std::vector<Test*>& test_list_() {
        static std::unique_ptr<std::vector<Test*>> list(new std::vector<Test*>());
        return *list;
    }

    virtual void ActualTest() = 0; // The test function specified by user

    double RunTest(); // Runs the test and takes care of result logging to Formatter
protected:
    TestData data_;
    std::string suite_;
    std::string test_;

    TestReport& AddReport(TestReport& report);
    void SetGradingMethod(GradingMethod method);
    void OutputFormat(std::string format);

    /* Runs num tests with correct(args...) giving correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class F, class S, class... Args>
    void CompareWithCallable(int num, const F& correct, const S& under_test, Args&... args);

    /* Runs num tests with correct being the correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class T, class S, class... Args>
    void CompareWithAnswer(int num, const T& correct, const S& under_test, Args&... args);

    /* Runs num tests with the item with corresponding index in correct vector being the correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class T, class S, class... Args>
    void CompareWithAnswer(int num, const std::vector<T>& correct, const S& under_test, Args&... args);

    std::stringstream& ExpectTrue(bool b, std::string descriptor);
    std::stringstream& ExpectFalse(bool b, std::string descriptor);
    template <class T, class S>
    std::stringstream& ExpectEqual(T expected, S output, std::string descriptor);
    template <class T, class S>
    std::stringstream& ExpectInequal(T expected, S output, std::string descriptor);

public:
    Test(const TestInfo& info);

    bool IsPassed() const;
    const std::string& GetSuite() const { return suite_; }
    const std::string& GetTest() const { return test_; }

    static bool RunTests();
    static Test* FindTest(std::string suite, std::string test);
};

template <class F, class S, class... Args>
void Test::CompareWithCallable(int num, const F& correct, const S& under_test, Args&... args) {

    TestReport report = TestReport::Make<CaseData>();
    auto& data = report.Get<CaseData>();

    data.resize(num);

    for(auto it = data.begin(); it != data.end(); it++) {

        gcheck::advance(args...);

        it->arguments = UserObject(std::tuple(args...));
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

template <class T, class S, class... Args>
void Test::CompareWithAnswer(int num, const T& correct, const S& under_test, Args&... args) {
    auto forwarder = [correct](auto...) -> T { return correct; };
    TestCase(num, forwarder, under_test, args...);
    //TODO: not tested.
}

template <class T, class S, class... Args>
void Test::CompareWithAnswer(int num, const std::vector<T>& correct, const S& under_test, Args&... args) {
    int index = 0;
    auto forwarder = [&index, &correct](auto...) -> T { return correct[index++]; };
    TestCase(num, forwarder, under_test, args...);
    //TODO: not tested.
}

template <class T, class S>
std::stringstream& Test::ExpectEqual(T left, S right, std::string descriptor) {

    TestReport report = TestReport::Make<EqualsData>();
    auto& data = report.Get<EqualsData>();

    data.output_expected = left;
    data.output = right;
    data.descriptor = descriptor;
    data.result = left == right;

    return *AddReport(report).info_stream;
}

template <class T, class S>
std::stringstream& Test::ExpectInequal(T left, S right, std::string descriptor) {

    TestReport report = TestReport::Make<EqualsData>();
    auto& data = report.Get<EqualsData>();

    data.output_expected = left;
    data.output = right;
    data.descriptor = descriptor;
    data.result = left != right;

    return *AddReport(report).info_stream;
}

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template std::stringstream& Test::ExpectEqual(unsigned int left, unsigned int right, std::string descriptor);
extern template std::stringstream& Test::ExpectEqual(int left, int right, std::string descriptor);
extern template std::stringstream& Test::ExpectEqual(float left, float right, std::string descriptor);
extern template std::stringstream& Test::ExpectEqual(double left, double right, std::string descriptor);
extern template std::stringstream& Test::ExpectEqual(std::string left, std::string right, std::string descriptor);
extern template std::stringstream& Test::ExpectEqual(std::string left, const char* right, std::string descriptor);
extern template std::stringstream& Test::ExpectEqual(const char* left, std::string right, std::string descriptor);

}

#define _TEST3(suitename, testname, points) _TEST4(suitename, testname, points, "")
#define _TEST4(suitename, testname, points, prerequisites) \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::Test { \
        void ActualTest(); \
    public: \
        GCHECK_TEST_##suitename##_##testname() : Test(gcheck::TestInfo(#suitename, #testname, points, prerequisites)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname; \
    void GCHECK_TEST_##suitename##_##testname::ActualTest()
#define _TEST2(suitename, testname) \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::Test { \
        void ActualTest(); \
    public: \
        GCHECK_TEST_##suitename##_##testname() : Test(gcheck::TestInfo(#suitename, #testname)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname; \
    void GCHECK_TEST_##suitename##_##testname::ActualTest()

// params: suite name, test name, points (optional), prerequisites (optional)
#define TEST(...) \
    VFUNC(_TEST, __VA_ARGS__)


#define EXPECT_TRUE(b) \
    ExpectTrue(b, #b)

#define EXPECT_FALSE(b) \
    ExpectFalse(b, "!" #b)

#define EXPECT_EQ(left, right) \
    ExpectEqual(left, right, #left " == " #right)
#define EXPECT_INEQ(left, right) \
    ExpectInequal(left, right, #left " != " #right)

#define ASSERT_TRUE(b) \
    ExpectTrue(b, #b); \
    if(!(b)) return;

#define ASSERT_FALSE(b) \
    ExpectFalse(b, #b); \
    if(b) return;

#define FAIL() \
    GradingMethod(gcheck::AllOrNothing); \
    ExpectTrue(false, "FAIL")
