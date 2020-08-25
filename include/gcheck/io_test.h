#pragma once

#include <functional>
#include <optional>
#include <string>

#include "macrotools.h"
#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"
#include "redirectors.h"
#include "function_test.h"

namespace gcheck {

class IOPlugin {
public:
    template<typename... Args>
    IOPlugin(FunctionTest<Args...>* test) : tout_(false), terr_(false), tin_(false) {
        test->AddResetTest([this](){ this->ResetTestVars(); });
        test->AddPreRun([this](auto&&... args){ this->PreRun(std::forward<decltype(args)>(args)...); });
        test->AddPostRun([this](auto&&... args){ this->PostRun(std::forward<decltype(args)>(args)...); });
    }
protected:
    StdoutCapturer tout_;
    StderrCapturer terr_;
    StdinInjecter tin_;

    std::optional<std::string> input_;
    std::optional<std::string> expected_output_;
    std::optional<std::string> expected_error_;
    bool do_close = false;

    // Sets the input (stdin) given to the tested function
    void SetInput(const std::string& str, bool close_stream = false) { input_ = str; do_close = close_stream; }
    // Sets the expected output (stdout) of tested function
    void SetOutput(const std::string& str) { expected_output_ = str; }
    // Sets the expected error (stderr) of tested function
    void SetError(const std::string& str) { expected_error_ = str; }

    void ResetTestVars() {
        input_.reset();
        expected_output_.reset();
        expected_error_.reset();
        do_close = false;
    }
private:
    void PreRun(size_t, FunctionEntry&) {
        //tin_.Capture();
        //if(input_) tin_.Write(*input_);
        //if(do_close) tin_.Close();

        tout_.Capture();
        terr_.Capture();
    }
    void PostRun(size_t, FunctionEntry& entry) {
        tout_.Restore();
        terr_.Restore();
        tin_.Restore();

        if(input_) entry.input = *input_;

        std::string outstr = tout_.str();
        std::string errstr = terr_.str();
        entry.output = outstr;
        if(expected_output_)
            entry.output_expected = *expected_output_;
        entry.error = errstr;
        if(expected_error_)
            entry.error_expected = *expected_error_;
        entry.result = entry.result && (!expected_output_ || *expected_output_ == outstr) && (!expected_error_ || *expected_error_ == errstr);
    }
};

/*
    Base class for testing functions, their response to standard input and output in standard output and error.
*/
template<typename ReturnT, typename... Args>
class IOTest : public FunctionTest<ReturnT, Args...>, public IOPlugin {
public:
    IOTest(const TestInfo& info, int num_runs, const std::function<ReturnT(Args...)>& func) : FunctionTest<ReturnT, Args...>(info, num_runs, func), IOPlugin(this) { }

private:
    using FunctionTest<ReturnT, Args...>::SetInputsAndOutputs;
    using FunctionTest<ReturnT, Args...>::AddReport;
    using FunctionTest<ReturnT, Args...>::RunOnce;
    using FunctionTest<ReturnT, Args...>::args_;
    using FunctionTest<ReturnT, Args...>::args_after_;
    using FunctionTest<ReturnT, Args...>::expected_return_value_;
    using FunctionTest<ReturnT, Args...>::num_runs_;
    using FunctionTest<ReturnT, Args...>::run_index_;
    using FunctionTest<ReturnT, Args...>::check_arguments_;
};

} // gcheck

#define _IOTEST7(...) _IOTEST5(__VA_ARGS__)
#define _IOTEST6(...) _IOTEST5(__VA_ARGS__)
#define _IOTEST5(suitename, testname, num_runs, ...) \
    template<typename ReturnT, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::IOTest<ReturnT, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPreRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPostRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        using gcheck::IOTest<ReturnT, Args...>::SetInput; \
        using gcheck::IOTest<ReturnT, Args...>::SetOutput; \
        using gcheck::IOTest<ReturnT, Args...>::SetError; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(std::function<ReturnT(Args...)> func) : gcheck::IOTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(Args...)) : gcheck::IOTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname, __TAIL(__VA_ARGS__)), num_runs, func) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(__HEAD(__VA_ARGS__)); \
    template<typename ReturnT, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, Args...>::SetInputsAndOutputs()

#define _IOTEST4(suitename, testname, num_runs, tobetested) \
    template<typename ReturnT, typename... Args> \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::IOTest<ReturnT, Args...> { \
        using gcheck::FunctionTest<ReturnT, Args...>::SetTimeout; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::IgnoreArgumentsAfter; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetReturn; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetLastArguments; \
        using gcheck::FunctionTest<ReturnT, Args...>::GetRunIndex; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPreRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::AddPostRun; \
        using gcheck::FunctionTest<ReturnT, Args...>::SetMaxRunTime; \
        using gcheck::IOTest<ReturnT, Args...>::SetInput; \
        using gcheck::IOTest<ReturnT, Args...>::SetOutput; \
        using gcheck::IOTest<ReturnT, Args...>::SetError; \
        void SetInputsAndOutputs(); \
    public: \
        GCHECK_TEST_##suitename##_##testname(std::function<ReturnT(Args...)> func) : gcheck::IOTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
        GCHECK_TEST_##suitename##_##testname(ReturnT(&func)(Args...)) : gcheck::IOTest<ReturnT, Args...>(gcheck::TestInfo(#suitename, #testname), num_runs, func) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname(tobetested); \
    template<typename ReturnT, typename... Args> \
    void GCHECK_TEST_##suitename##_##testname<ReturnT, Args...>::SetInputsAndOutputs()

// params: suite name, test name, number of runs, function to be tested, points (optional), prerequisites (optional)
#define IOTEST(...) \
    VFUNC(_IOTEST, __VA_ARGS__)
