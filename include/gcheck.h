#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include <variant>
#include <chrono>

#include "argument.h"
#include "json.h"
#include "user_object.h"
#include "sfinae.h"
#include "macrotools.h"
#include "multiprocessing.h"

namespace gcheck {

template<typename T, typename S = std::string, typename... Ss>
struct Result {
    typedef std::tuple<S, Ss...> InputT;

    T output;
    std::string error;

    Result() : output() { }
    Result(T out) : output(out) { }

    Result(const Result& r) = default;

    Result& operator=(const T& item) {
        output = item;
        return *this;
    }
    Result& operator=(const Result& r) = default;

    void SetInput(const S& first, const Ss&... args) { input = std::tuple(first, args...); }
    const std::optional<InputT>& GetInput() const { return input; }
private:
    std::optional<InputT> input;
};

template<template<typename> class allocator = std::allocator>
struct _EqualsData {
    typedef _UserObject<allocator> UO;
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> string;
    UO output_expected;
    UO output;
    string descriptor;
    bool result;

    _EqualsData() {}

    template<template<typename> class T>
    _EqualsData(const _EqualsData<T>& d) {
        *this = d;
    }
    template<template<typename> class T>
    _EqualsData& operator=(const _EqualsData<T>& d) {
        output_expected = d.output_expected;
        output = d.output;
        descriptor = d.descriptor;
        result = d.result;
        return *this;
    }
};
using EqualsData = _EqualsData<>;

template<template<typename> class allocator = std::allocator>
struct _TrueData {
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> string;
    bool value;
    string descriptor;
    bool result;

    _TrueData() {}

    template<template<typename> class T>
    _TrueData(const _TrueData<T>& d) {
        *this = d;
    }
    template<template<typename> class T>
    _TrueData& operator=(const _TrueData<T>& d) {
        value = d.value;
        descriptor = d.descriptor;
        result = d.result;
        return *this;
    }
};
using TrueData = _TrueData<>;

template<template<typename> class allocator = std::allocator>
struct _FalseData {
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> string;
    bool value;
    string descriptor;
    bool result;

    _FalseData() {}

    template<template<typename> class T>
    _FalseData(const _FalseData<T>& d) {
        *this = d;
    }
    template<template<typename> class T>
    _FalseData& operator=(const _FalseData<T>& d) {
        value = d.value;
        descriptor = d.descriptor;
        result = d.result;
        return *this;
    }
};
using FalseData = _FalseData<>;

template<template<typename> class allocator = std::allocator>
struct _CaseEntry {
    typedef _UserObject<allocator> UO;
    std::optional<UO> arguments; // arguments to test function
    std::optional<UO> input; // arguments to tested function
    std::optional<UO> output;
    std::optional<UO> output_expected;
    bool result;

    _CaseEntry() {}

    template<template<typename> class T>
    _CaseEntry(const _CaseEntry<T>& f) {
        *this = f;
    }
    template<template<typename> class T>
    _CaseEntry& operator=(const _CaseEntry<T>& ce) {
        arguments = ce.arguments;
        input = ce.input;
        output = ce.output;
        output_expected = ce.output_expected;
        result = ce.result;
        return *this;
    }
};
using CaseEntry = _CaseEntry<>;
template<template<typename> class allocator = std::allocator>
using _CaseData = std::vector<_CaseEntry<allocator>, allocator<_CaseEntry<allocator>>>;
using CaseData = _CaseData<>;

template<template<typename> class allocator = std::allocator>
struct _FunctionEntry {
    typedef _UserObject<allocator> UO;
    std::optional<UO> input;
    std::optional<UO> output;
    std::optional<UO> output_expected;
    std::optional<UO> error;
    std::optional<UO> error_expected;
    std::optional<UO> arguments;
    std::optional<UO> arguments_after;
    std::optional<UO> arguments_after_expected;
    std::optional<UO> return_value;
    std::optional<UO> return_value_expected;
    std::optional<UO> object;
    std::optional<UO> object_after;
    std::optional<UO> object_after_expected;
    std::optional<std::chrono::nanoseconds> max_run_time;
    std::chrono::nanoseconds run_time;
    std::chrono::duration<double> timeout;
    ForkStatus status = OK;
    bool result;

    _FunctionEntry() {}

    template<template<typename> class T>
    _FunctionEntry(const _FunctionEntry<T>& f) {
        *this = f;
    }
    template<template<typename> class T>
    _FunctionEntry& operator=(const _FunctionEntry<T>& fe) {
        input = fe.input;
        output = fe.output;
        output_expected = fe.output_expected;
        error = fe.error;
        error_expected = fe.error_expected;
        arguments = fe.arguments;
        arguments_after = fe.arguments_after;
        arguments_after_expected = fe.arguments_after_expected;
        return_value = fe.return_value;
        return_value_expected = fe.return_value_expected;
        object = fe.object;
        object_after = fe.object_after;
        object_after_expected = fe.object_after_expected;
        max_run_time = fe.max_run_time;
        run_time = fe.run_time;
        timeout = fe.timeout;
        status = fe.status;
        result = fe.result;
        return *this;
    }
};
using FunctionEntry = _FunctionEntry<>;
template<template<typename> class allocator = std::allocator>
using _FunctionData = std::vector<_FunctionEntry<allocator>, allocator<_FunctionEntry<allocator>>>;
using FunctionData = _FunctionData<>;

template<template<typename> class allocator = std::allocator>
struct _TestReport {
    typedef std::basic_stringstream<char, std::char_traits<char>, allocator<char>> stringstream;
    stringstream info_stream;

    std::variant<_EqualsData<allocator>, _TrueData<allocator>, _FalseData<allocator>, _CaseData<allocator>, _FunctionData<allocator>> data;

    _TestReport(const _TestReport& r) : data(r.data) { info_stream << r.info_stream.str(); }
    template<typename T>
    _TestReport(const T& d) : data(d) {}
    template<template<typename> class T>
    _TestReport(const _TestReport<T>& r) {
        switch (r.data.index()) {
        case 0:
            data = std::get<0>(r.data);
            break;
        case 1:
            data = std::get<1>(r.data);
            break;
        case 2:
            data = std::get<2>(r.data);
            break;
        case 3: {
            auto& vec = std::get<3>(r.data);
            data = _CaseData<allocator>(vec.begin(), vec.end());
            break;
        } case 4: {
            auto& vec = std::get<4>(r.data);
            data = _FunctionData<allocator>(vec.begin(), vec.end());
            break;
        } default:
            break;
        }
    }

    template<typename T>
    static _TestReport Make(T item = T()) {
        return _TestReport(item);
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
using TestReport = _TestReport<>;

enum GradingMethod {
    Partial,
    AllOrNothing,
    Most,
    StrictMost
};
enum TestStatus : int {
    NotStarted,
    Started,
    TimedOut,
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

template<template<typename> class allocator = std::allocator>
struct _TestData {
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> string;
    typedef _TestReport<allocator> TReport;
    Prerequisite prerequisite;

    std::vector<TReport, allocator<TReport>> reports;
    GradingMethod grading_method = Partial;
    string output_format = "vertical";
    TestStatus status = NotStarted;

    double points = 0;
    double max_points = 0;

    string sout = "";
    string serr = "";

    int correct = 0;
    int incorrect = 0;

    _TestData(double points, Prerequisite prerequisite) : prerequisite(prerequisite), max_points(points) {}
    template<template<typename> class T>
    _TestData(const _TestData<T>& td) {
        reports = std::vector<TReport, allocator<TReport>>(td.reports.begin(), td.reports.end());
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
    template<template<typename> class T>
    _TestData& operator=(const _TestData<T>& td) {
        reports = std::vector<TReport, allocator<TReport>>(td.reports.begin(), td.reports.end());
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
using TestData = _TestData<>;

/*
    Abstract base class for tests. Keeps track of the test's results and options.
*/
class Test {
    // Contains all the tests. It's a function to get around the static initialization order problem
    static std::vector<Test*>& test_list_() {
        static std::unique_ptr<std::vector<Test*>> list(new std::vector<Test*>());
        return *list;
    }

    virtual void ActualTest() = 0; // The test function specified by inheritor

    void RunTest(); // Runs the test and takes care of result logging to Formatter
protected:
    TestData data_;
    std::string suite_;
    std::string test_;

    TestReport& AddReport(TestReport& report);
    void SetGradingMethod(GradingMethod method);
    void OutputFormat(std::string format);

public:
    Test(const TestInfo& info);

    bool IsPassed() const;
    const std::string& GetSuite() const { return suite_; }
    const std::string& GetTest() const { return test_; }

    static bool do_safe_run_;

    static bool RunTests();
    static Test* FindTest(std::string suite, std::string test);
};

}
