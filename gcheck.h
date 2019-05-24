#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <sstream>
#include <tuple>
#include "utility.h"
#include "argument.h"

#define DEFAULT_REPORT_NAME "report.json"

/*
    Static class for keeping track of and logging test results.
*/
class Formatter {
    static std::vector<std::pair<std::string, JSON>> tests_;

    static bool show_input_;
    static bool highlight_difference_;
    static std::string default_format_;

    static bool swapped_;

    static std::streambuf* cout_buf_;
    static std::streambuf* cerr_buf_;

    static std::stringstream cout_;
    static std::stringstream cerr_;

    Formatter() {}; //Disallows instantation of this class
public:

    static bool html_output_;

    static void SetTestData(std::string suite, std::string test, JSON& data);
    // Writes the test results to a file
    static void WriteReport(std::string filename);
    static void Init();
};

/*
    Abstract base class for tests. Keeps track of the test's results and options.
*/
class Test {

    static std::vector<Test*> test_list_; // Contains all the tests

    virtual void ActualTest() = 0; // The test function specified by user

    void RunTest(); // Runs the test and takes care of result logging to Formatter
protected:

    JSON data_;

    double points_ = default_points_; // Maximum points given for this test
    std::string grading_method_ = "partial";
    std::string output_format_ = "horizontal";

    std::string suite_;
    std::string test_;

    int correct_ = 0;
    int incorrect_ = 0;

    void AddReport(JSON data);
    void GradingMethod(std::string method);
    void OutputFormat(std::string format);
public:

    Test(std::string suite, std::string test, double points);

    static double default_points_;

    static void RunTests();
};

#define ADD_REPORT_(json, type, condition, result) \
    json.Set("type", type); \
    json.Set("condition", condition); \
    AddReport(json.Set("result", result));

#define ADD_REPORT(type, condition, result) \
    JSON json; \
    ADD_REPORT_(json, type, condition, result)

// Creates a class for a test with specific maximum points
#define TEST_(suitename, testname, points) \
    class GCHECK_TEST_##suitename##_GCHECK_TEST_##testname : Test { \
        void ActualTest(); \
    public: \
        GCHECK_TEST_##suitename##_GCHECK_TEST_##testname() : Test(#suitename, #testname, points) { } \
    }; \
    GCHECK_TEST_##suitename##_GCHECK_TEST_##testname GCHECK_TESTVAR_##suitename##_GCHECK_TEST_##testname; \
    void GCHECK_TEST_##suitename##_GCHECK_TEST_##testname::ActualTest() 

// Calls TEST_ with the default maximum points
#define TEST(suitename, testname) \
    TEST_(suitename, testname, points_)

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
        JSON json; \
        json.Set("left", b); \
        json.Set("right", false); \
        ADD_REPORT(json, "ET", #b " = true", b) \
    }
    
#define EXPECT_FALSE(b) \
    { \
        JSON json; \
        json.Set("left", b); \
        json.Set("right", false); \
        ADD_REPORT(json, "EF", #b " = false", !b) \
    }

#define EXPECT_EQ(left, right) \
    { \
        JSON json; \
        json.Set("left", left); \
        json.Set("right", right); \
        ADD_REPORT_(json, "EE", #left " = " #right, left == right) \
    }

/* Runs num tests with correct(__VA_ARGS__) giving correct answer
and under_test(__VA_ARGS__) being the testing code and adds the results to test data */
#define TEST_CASE(num, correct, under_test, ...) \
    { \
        JSON json; \
        json.Set("type", "TC"); \
        json.Set("test_class", typeid(*this).name() ); \
        std::vector<JSON> results; \
        for(int i = 0; i < num; i++) { \
            JSON result; \
            advance(__VA_ARGS__); \
            result.Set("input", MakeAnyList(__VA_ARGS__)); \
            result.Set("correct", correct(__VA_ARGS__)); \
            result.Set("output", under_test(__VA_ARGS__)); \
            results.push_back(result); \
        } \
        AddReport(json.Set("cases", results)); \
    }
