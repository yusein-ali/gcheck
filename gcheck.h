#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>

#include "utility.h"
#include "json.h"

namespace gcheck {

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

    JSON data_;

    double points_; // Maximum points given for this test
    std::string grading_method_ = "partial";
    std::string output_format_ = "horizontal";

    std::string suite_;
    std::string test_;

    int correct_ = 0;
    int incorrect_ = 0;

    int priority_;

    void AddReport(JSON data);
    void GradingMethod(std::string method);
    void OutputFormat(std::string format);
    
    /* Runs num tests with correct(args...) giving correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class... Args, class... Args2>
    void TestCase(int num, std::string (*correct)(Args2...), std::string (*under_test)(Args2...), Args... args);
    
    /* Runs num tests with correct being the correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class... Args, class... Args2>
    void TestCase(int num, std::string correct, std::string (*under_test)(Args2...), Args... args);

    std::stringstream& ExpectTrue(bool b, std::string descriptor, JSON json = JSON());
    std::stringstream& ExpectFalse(bool b, std::string descriptor, JSON json = JSON());
    template <class T, class S>
    std::stringstream& ExpectEqual(T left, S right, std::string descriptor, JSON json = JSON());

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


#define EXPECT_TRUE_(b, msg) \
    ExpectTrue(b, #b, msg)
#define EXPECT_TRUE(b) \
    ExpectTrue(b, #b)
    
#define EXPECT_FALSE_(b, msg) \
    ExpectFalse(b, #b, msg)
#define EXPECT_FALSE(b) \
    ExpectFalse(b, #b)

#define EXPECT_EQ_(left, right, msg) \
    ExpectEqual(left, right, #left " = " #right, msg)
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

template <class... Args, class... Args2>
void Test::TestCase(int num, std::string (*correct)(Args2...), std::string (*under_test)(Args2...), Args... args) {
    JSON json; 
    json.Set("type", "TC"); 
    json.Set("test_class", typeid(*this).name() ); 
    std::vector<JSON> results; 
    for(int i = 0; i < num; i++) { 
        JSON result; 
        advance(args...); 
        result.Set("input", MakeAnyList(args...)); 
        result.Set("correct", correct(args...)); 
        result.Set("output", under_test(args...)); 
        results.push_back(result); 
    }
    Test::AddReport(json.Set("cases", results)); 
}

template <class... Args, class... Args2>
void Test::TestCase(int num, std::string correct, std::string (*under_test)(Args2...), Args... args) {
    JSON json; 
    json.Set("type", "TC"); 
    json.Set("test_class", typeid(*this).name() ); 
    std::vector<JSON> results; 
    for(int i = 0; i < num; i++) { 
        JSON result; 
        advance(args...); 
        result.Set("input", MakeAnyList(args...)); 
        result.Set("correct", correct); 
        result.Set("output", under_test(args...)); 
        results.push_back(result); 
    }
    Test::AddReport(json.Set("cases", results)); 
}

template <class T, class S>
std::stringstream& Test::ExpectEqual(T left, S right, std::string descriptor, JSON json) {
    
    auto ss = std::make_shared<std::stringstream>();
    json.Set("info_stream", ss);
    json.Set("left", left);
    json.Set("right", right);
    json.Set("type", "EE");
    json.Set("condition", descriptor);
    AddReport(json.Set("result", left == right));
    
    return *ss;
}
}