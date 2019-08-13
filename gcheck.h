#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include <variant>

#include "utility.h"
#include "json.h"
#include "user_object.h"

namespace gcheck {

template<typename T, typename S = std::string>
struct Result {
    T output;
    std::string error;
    
    Result(T out) { output = out; }
    
    Result(const Result<T, S>& r) { 
        output = r.output;
        error = r.error;
        input = r.input;
        input_set = r.input_set;
    }
    
    Result<T, S>& operator=(const T& item) {
        output = item;
        return *this;
    }
    
    void SetInput(S data) { input = data; input_set = true; }
    const S& GetInput() const { return input; }
    
    bool IsSet() { return input_set; }
private:
    S input;
    bool input_set = false;
};

struct TestReport {
    
    std::string test_class;
    std::shared_ptr<std::stringstream> info_stream;
    
    enum Type {
        Equals,
        ExpectTrue,
        ExpectFalse,
        Case,
    };
    
    struct EqualsData {
        UserObject left;
        UserObject right;
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
        std::vector<UserObject> input_args;
        std::string correct;
        std::string input;
        std::string error;
        std::string output;
        bool result;
    };
    typedef std::vector<CaseEntry> CaseData;
    
    std::variant<EqualsData, TrueData, FalseData, CaseData> data;
    
    template<typename T>
    TestReport(const T& d) : info_stream(std::make_shared<std::stringstream>()), data(d) {}
    
    TestReport(const TestReport& r) : test_class(r.test_class), info_stream(r.info_stream), data(r.data) {}
    
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

struct TestData {
    std::vector<TestReport> reports;
    std::string grading_method = "partial";
    std::string output_format = "horizontal";
    
    double points = 0;
    double max_points;
    
    std::string sout;
    std::string serr;

    int correct = 0;
    int incorrect = 0;
    
    void CalculatePoints() {
        
        if(grading_method == "partial")
            points = correct/double(correct+incorrect)*max_points;
        else if(grading_method == "binary")
            points = incorrect == 0 ? points : 0;
        else if(grading_method == "most")
            points = incorrect <= correct ? points : 0;
        else if(grading_method == "strict_most")
            points = incorrect < correct ? points : 0;
        else
            points = 0;
        
        if(std::isinf(points) || std::isnan(points))
            points = max_points;
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

    int priority_;

    TestReport& AddReport(TestReport& report);
    void GradingMethod(std::string method);
    void OutputFormat(std::string format);
    
    /* Runs num tests with correct(args...) giving correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class T, class S, class... Args, class... Args2>
    void TestCase(int num, T (*correct)(Args2...), S (*under_test)(Args2...), Args... args);
    
    /* Runs num tests with correct being the correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class T, class S, class... Args, class... Args2>
    void TestCase(int num, const T& correct, S (*under_test)(Args2...), Args... args);
    
    /* Runs num tests with the item with corresponding index in correct vector being the correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class T, class S, class... Args, class... Args2>
    void TestCase(int num, const std::vector<T>& correct, S (*under_test)(Args2...), Args... args);

    std::stringstream& ExpectTrue(bool b, std::string descriptor);
    std::stringstream& ExpectFalse(bool b, std::string descriptor);
    template <class T, class S>
    std::stringstream& ExpectEqual(T left, S right, std::string descriptor);

public:

    Test(std::string suite, std::string test, double points, int priority);

    static double default_points_;

    static void RunTests();
};

// Creates a class for a prerequisite test with specific maximum points and prerequisite priority; higher goes first
#define PREREQ_TEST(suitename, testname, points, priority) \
    class GCHECK_TEST_##suitename##_##testname : gcheck::Test { \
        void ActualTest(); \
    public: \
        GCHECK_TEST_##suitename##_##testname() : Test(#suitename, #testname, points, priority) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname; \
    void GCHECK_TEST_##suitename##_##testname::ActualTest() 
    
// Creates a class for a test with specific maximum points
#define TEST_(suitename, testname, points) \
    PREREQ_TEST(suitename, testname, points, -1)
    
// Calls TEST_ with the default maximum points
#define TEST(suitename, testname) \
    TEST_(suitename, testname, default_points_)

#define EXPECT_TRUE(b) \
    ExpectTrue(b, #b)
    
#define EXPECT_FALSE(b) \
    ExpectFalse(b, #b)

#define EXPECT_EQ(left, right) \
    ExpectEqual(left, right, #left " = " #right)

#define ASSERT_TRUE(b) \
    ExpectTrue(b, #b); \
    if(!(b)) return;
    
#define ASSERT_FALSE(b) \
    ExpectFalse(b, #b); \
    if(b) return;

#define FAIL() \
    GradingMethod("binary"); \
    ExpectTrue(false, "FAIL")

template <class T, class S, class... Args, class... Args2>
void Test::TestCase(int num, T (*correct)(Args2...), S (*under_test)(Args2...), Args... args) {
    
    TestReport report = TestReport::Make<TestReport::CaseData>();
    auto& data = report.Get<TestReport::CaseData>();
    
    data.resize(num);
    
    for(auto it = data.begin(); it != data.end(); it++) { 
        
        gcheck::advance(args...); 
        
        it->input_args = MakeUserObjectList(args...);
        auto correct_ans = Result(correct(args...)).output;
        it->correct = MakeUserObject(correct_ans).string();
        
        Result res(under_test(args...));
        if(res.IsSet())
            it->input = res.GetInput();
        else
            it->input = toJSON(it->input_args);
        
        it->error = res.error;
        it->output = MakeUserObject(res.output).string();
        it->result = res.output == correct_ans;
    }
    AddReport(report);
}

template <class T, class S, class... Args, class... Args2>
void Test::TestCase(int num, const T& correct, S (*under_test)(Args2...), Args... args) {
    auto forwarder = [correct](auto... params) -> T { return correct; };
    TestCase(num, forwarder, under_test, args...);
    //TODO: not tested.
    }

template <class T, class S, class... Args, class... Args2>
void Test::TestCase(int num, const std::vector<T>& correct, S (*under_test)(Args2...), Args... args) {
    int index = 0;
    auto forwarder = [&index, &correct](auto... params) -> T { return correct[index++]; };
    TestCase(num, forwarder, under_test, args...);
    //TODO: not tested.
}

template <class T, class S>
std::stringstream& Test::ExpectEqual(T left, S right, std::string descriptor) {
    
    TestReport report = TestReport::Make<TestReport::EqualsData>();
    auto& data = report.Get<TestReport::EqualsData>();
    
    data.left = MakeUserObject(left);
    data.right = MakeUserObject(right);
    data.descriptor = descriptor;
    data.result = left == right;
    
    return *AddReport(report).info_stream;
}

}