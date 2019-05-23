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
public:

    static bool html_output_;

    static JSON& GetTest(std::string test_class);
    static void AddReport(JSON& data);
    static void WriteReport(std::string filename);
    static void AddTest(std::string suite, std::string test, double points);
    static void Init();
    static void SetGradingMethod(std::string test_class, std::string method);
    static void SetFormat(std::string test_class, std::string format);
};

class Test {

    static std::vector<Test*> test_list_;

    virtual void RunTest() = 0;
protected:

    double points_ = 1;
public:

    Test();

    static void RunTests();
};

#define DEFAULT_POINTS(points) \

#define ADD_REPORT_(json, type, condition, result) \
    json.Set("type", type); \
    json.Set("condition", condition); \
    json.Set("test_class", typeid(*this).name() ); \
    Formatter::AddReport(json.Set("result", result)); \

#define ADD_REPORT(type, condition, result) \
    JSON json; \
    ADD_REPORT_(json, type, condition, result)

#define TEST_POINTS(suitename, testname, points) \
    class GCHECK_TEST_##suitename##_GCHECK_TEST_##testname : Test { \
        void RunTest(); \
    public: \
        GCHECK_TEST_##suitename##_GCHECK_TEST_##testname() { \
            points_ = points; \
            Formatter::AddTest(#suitename, #testname, points_); \
        } \
    }; \
    GCHECK_TEST_##suitename##_GCHECK_TEST_##testname GCHECK_TESTVAR_##suitename##_GCHECK_TEST_##testname; \
    void GCHECK_TEST_##suitename##_GCHECK_TEST_##testname::RunTest() 

#define TEST(suitename, testname) \
    TEST_POINTS(suitename, testname, points_)

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

#define GRADING_METHOD(method) \
    { \
        Formatter::SetGradingMethod(typeid(*this).name(), #method); \
    }

#define FORMAT(format) \
    { \
        Formatter::SetFormat(typeid(*this).name(), #format); \
    }

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
        Formatter::AddReport(json.Set("cases", results)); \
    }
