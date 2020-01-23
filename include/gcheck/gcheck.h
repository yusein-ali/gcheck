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

namespace gcheck {

template<typename T, typename S = std::string>
struct Result {
    T output;
    std::string error;
    
    Result() : output() { }
    Result(T out) : output(out) { }
    
    Result(const Result<T, S>& r) 
        : output(r.output), error(r.error), input(r.input), input_set(r.input_set),
        input_params(r.input_params), input_params_set(r.input_params_set), 
        output_params(r.output_params), output_params_set(r.output_params_set) { 
    }
    
    Result<T, S>& operator=(const T& item) {
        output = item;
        return *this;
    }
    Result<T, S>& operator=(const Result<T, S>& r) {
        output = r.output;
        error = r.error;
        input = r.input;
        input_set = r.input_set;
        input_params = r.input_params;
        input_params_set = r.input_params_set; 
        output_params = r.output_params;
        output_params_set = r.output_params_set;
        return *this;
    }
    
    void SetInput(S data) { input = data; input_set = true; }
    const S& GetInput() const { return input; }
    void SetInputParams(std::string data) { input_params = data; input_params_set = true; }
    std::string GetInputParams() const { return input_params; }
    void SetOutputParams(std::string data) { output_params = data; output_params_set = true; }
    std::string GetOutputParams() const { return output_params; }
    
    bool IsSet() { return input_set; }
    bool IsInputParamsSet() { return input_params_set; }
    bool IsOutputParamsSet() { return output_params_set; }
private:
    S input;
    bool input_set = false;
    std::string input_params;
    bool input_params_set = false;
    std::string output_params;
    bool output_params_set = false;
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
        JSON input;
        std::string input_params;
        std::string output_params;
        std::string error;
        std::string output;
        JSON output_json;
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

enum GradingMethod {
    Partial,
    AllOrNothing,
    Most,
    StrictMost
};

struct TestData {
    std::vector<TestReport> reports;
    GradingMethod grading_method = Partial;
    std::string output_format = "horizontal";
    bool finished = false;
    
    double points = 0;
    double max_points = 0;
    
    std::string sout = "";
    std::string serr = "";

    int correct = 0;
    int incorrect = 0;
    
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
            
        finished = true;
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
    std::stringstream& ExpectEqual(T left, S right, std::string descriptor);

public:

    Test(std::string suite, std::string test, double points, int priority);

    static double default_points_;

    static bool RunTests();
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
    PREREQ_TEST(suitename, testname, points, std::numeric_limits<int>::max())
    
// Calls TEST_ with the default maximum points
#define TEST(suitename, testname) \
    TEST_(suitename, testname, default_points_)

#define EXPECT_TRUE(b) \
    ExpectTrue(b, #b)
    
#define EXPECT_FALSE(b) \
    ExpectFalse(b, "!" #b)

#define EXPECT_EQ(left, right) \
    ExpectEqual(left, right, #left " == " #right)

#define ASSERT_TRUE(b) \
    ExpectTrue(b, #b); \
    if(!(b)) return;
    
#define ASSERT_FALSE(b) \
    ExpectFalse(b, #b); \
    if(b) return;

#define FAIL() \
    GradingMethod(gcheck::AllOrNothing); \
    ExpectTrue(false, "FAIL")

template <class F, class S, class... Args>
void Test::CompareWithCallable(int num, const F& correct, const S& under_test, Args&... args) {
    
    TestReport report = TestReport::Make<TestReport::CaseData>();
    auto& data = report.Get<TestReport::CaseData>();
    
    data.resize(num);
    
    for(auto it = data.begin(); it != data.end(); it++) { 
        
        gcheck::advance(args...); 
        
        it->input_args = MakeUserObjectList(args...);
        auto correct_res = Result(correct(args...));
        auto correct_ans = correct_res.output;
        it->correct = UserObject(correct_ans).string();
        
        if(correct_res.IsSet())
            it->input = toJSON(correct_res.GetInput());
        else
            it->input = toJSON(it->input_args);
        
        if(correct_res.IsInputParamsSet())
            it->input_params = correct_res.GetInputParams();
        
        Result res(under_test(args...));
        it->error = res.error;
        it->output = UserObject(res.output).string();
        it->output_json = UserObject(res.output).json();
        it->result = res.output == correct_ans;
            
        if(correct_res.IsOutputParamsSet())
            it->output_params = correct_res.GetOutputParams();
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
    
    TestReport report = TestReport::Make<TestReport::EqualsData>();
    auto& data = report.Get<TestReport::EqualsData>();
    
    data.left = UserObject(left);
    data.right = UserObject(right);
    data.descriptor = descriptor;
    data.result = left == right;
    
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