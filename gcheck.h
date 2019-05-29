#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <sstream>
#include <tuple>
#include <memory>
#include "utility.h"
#include "argument.h"

namespace gcheck {

#define DEFAULT_REPORT_NAME "report.json"

/*
    Static class for keeping track of and logging test results.
*/
class Formatter {
    static std::vector<std::pair<std::string, JSON>> tests_;

    static double total_points_;
    static double total_max_points_;

    static bool show_input_;
    static bool highlight_difference_;
    static std::string default_format_;

    static bool swapped_;
    static std::streambuf* cout_buf_;
    static std::streambuf* cerr_buf_;
    static std::stringstream cout_;
    static std::stringstream cerr_;

    Formatter() {}; //Disallows instantiation of this class

public:

    static bool pretty_;
    static std::string filename_;

    static void SetTestData(std::string suite, std::string test, JSON& data);
    static void SetTotal(double points, double max_points);
    // Writes the test results to stdout
    static void WriteReport(bool is_finished = true, std::string suite = "", std::string test = "");
    static void Init();
};

/*
    Abstract base class for tests. Keeps track of the test's results and options.
*/
class Test {

    static std::vector<Test*>& test_list_() { 
        static std::unique_ptr<std::vector<Test*>> list(new std::vector<Test*>());
        return *list;
    }; // Contains all the tests. It's a function to get around the static initialization order problem

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
    void TestCase(int num, std::string (*correct)(Args2...), std::string (*under_test)(Args2...), Args... args) {
        gcheck::JSON json; 
        json.Set("type", "TC"); 
        json.Set("test_class", typeid(*this).name() ); 
        std::vector<gcheck::JSON> results; 
        for(int i = 0; i < num; i++) { 
            gcheck::JSON result; 
            gcheck::advance(args...); 
            result.Set("input", MakeAnyList(args...)); 
            result.Set("correct", correct(args...)); 
            result.Set("output", under_test(args...)); 
            results.push_back(result); 
        }
        gcheck::Test::AddReport(json.Set("cases", results)); 
    }
    
    /* Runs num tests with correct being the correct answer
    and under_test(arg...) giving the testing answer and adds the results to test data */
    template <class... Args, class... Args2>
    void TestCase(int num, std::string correct, std::string (*under_test)(Args2...), Args... args) {
        gcheck::JSON json; 
        json.Set("type", "TC"); 
        json.Set("test_class", typeid(*this).name() ); 
        std::vector<gcheck::JSON> results; 
        for(int i = 0; i < num; i++) { 
            gcheck::JSON result; 
            gcheck::advance(args...); 
            result.Set("input", MakeAnyList(args...)); 
            result.Set("correct", correct); 
            result.Set("output", under_test(args...)); 
            results.push_back(result); 
        }
        gcheck::Test::AddReport(json.Set("cases", results)); 
    }

public:

    Test(std::string suite, std::string test, double points, int priority);

    static double default_points_;

    static void RunTests();
};
}

#define ADD_REPORT_(json, type, condition, result) \
    json.Set("type", type); \
    json.Set("condition", condition); \
    AddReport(json.Set("result", result));

#define ADD_REPORT(type, condition, result) \
    gcheck::JSON json; \
    ADD_REPORT_(json, type, condition, result)

// Creates a class for a test with specific maximum points
#define TEST_(suitename, testname, points) \
    PREREQ_TEST(suitename, testname, points, -1)

// Creates a class for a prerequisite test with specific maximum points and prerequisite priority; higher goes first
#define PREREQ_TEST(suitename, testname, points, priority) \
    class GCHECK_TEST_##suitename##_##testname : gcheck::Test { \
        void ActualTest(); \
    public: \
        GCHECK_TEST_##suitename##_##testname() : Test(#suitename, #testname, points, priority) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname; \
    void GCHECK_TEST_##suitename##_##testname::ActualTest() 

// Calls TEST_ with the default maximum points
#define TEST(suitename, testname) \
    TEST_(suitename, testname, default_points_)

#define EXPECT_TRUE_(b, msg) \
    { \
        ADD_REPORT(msg, "ET", #b, b) \
    }

#define EXPECT_EQ_(left, right, msg) \
    { \
        msg.Set("left", left); \
        msg.Set("right", right); \
        ADD_REPORT(msg, "EE", #left " = " #right, left == right) \
    }

#define EXPECT_TRUE(b) \
    { \
        gcheck::JSON json; \
        json.Set("left", b); \
        json.Set("right", false); \
        ADD_REPORT(json, "ET", #b " = true", b) \
    }
    
#define EXPECT_FALSE(b) \
    { \
        gcheck::JSON json; \
        json.Set("left", b); \
        json.Set("right", false); \
        ADD_REPORT(json, "EF", #b " = false", !b) \
    }

#define EXPECT_EQ(left, right) \
    { \
        gcheck::JSON json; \
        json.Set("left", left); \
        json.Set("right", right); \
        ADD_REPORT_(json, "EE", #left " = " #right, left == right) \
    }

