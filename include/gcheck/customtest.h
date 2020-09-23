#pragma once

#include <string>
#include <sstream>

#include "macrotools.h"
#include "gcheck.h"
#include "sfinae.h"
#include "user_object.h"
#include "multiprocessing.h"

namespace gcheck {

class CustomTest : public Test {
    void ActualTest() override;
    virtual void TheTest() = 0; // The test function specified by user

protected:
    double timeout_ = 0;
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
    CustomTest(const TestInfo& info) : Test(info) {}
};

template <class T, class S, class... Args>
void CustomTest::CompareWithAnswer(int num, const T& correct, const S& under_test, Args&... args) {
    auto forwarder = [&correct](auto...) -> T { return correct; };
    CompareWithCallable(num, forwarder, under_test, args...);
    //TODO: not tested.
}

template <class T, class S, class... Args>
void CustomTest::CompareWithAnswer(int num, const std::vector<T>& correct, const S& under_test, Args&... args) {
    int index = 0;
    auto forwarder = [&index, &correct](auto...) -> T { return correct[index++]; };
    CompareWithCallable(num, forwarder, under_test, args...);
    //TODO: not tested.
}

template<typename T, class = std::enable_if_t<!is_base_of_template<T, Argument>::value>>
auto extract_argument(T&& n) {
    return std::forward<T>(n);
}

template<typename T>
auto extract_argument(const Argument<T>& n) {
    return n();
}

template <class F, class S, class... Args>
void CustomTest::CompareWithCallable(int num, const F& correct, const S& under_test, Args&... args) {

    TestReport report = TestReport::Make<CaseData>();
    auto& data = report.Get<CaseData>();

    data.resize(num);

    for(auto it = data.begin(); it != data.end(); it++) {

        gcheck::advance(args...);

        it->arguments = UserObject(std::tuple(extract_argument(args)...));
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

template <class T, class S>
std::stringstream& CustomTest::ExpectEqual(T left, S right, std::string descriptor) {

    TestReport report = TestReport::Make<EqualsData>();
    auto& data = report.Get<EqualsData>();

    data.output_expected = left;
    data.output = right;
    data.descriptor = descriptor;
    data.result = left == right;

    return AddReport(report).info_stream;
}

template <class T, class S>
std::stringstream& CustomTest::ExpectInequal(T left, S right, std::string descriptor) {

    TestReport report = TestReport::Make<EqualsData>();
    auto& data = report.Get<EqualsData>();

    data.output_expected = left;
    data.output = right;
    data.descriptor = descriptor;
    data.result = left != right;

    return AddReport(report).info_stream;
}

// Theoretically improves compile times with precompiled gcheck TODO: benchmark
extern template std::stringstream& CustomTest::ExpectEqual(unsigned int left, unsigned int right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(int left, int right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(float left, float right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(double left, double right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(std::string left, std::string right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(std::string left, const char* right, std::string descriptor);
extern template std::stringstream& CustomTest::ExpectEqual(const char* left, std::string right, std::string descriptor);

} // gcheck

#define _TEST3(suitename, testname, points) _TEST4(suitename, testname, points, "")
#define _TEST4(suitename, testname, points, prerequisites) _TEST5(suitename, testname, points, prerequisites, 0)
#define _TEST5(suitename, testname, points, prerequisites, timeoutval) \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::CustomTest { \
        void TheTest(); \
    public: \
        GCHECK_TEST_##suitename##_##testname() : CustomTest(gcheck::TestInfo(#suitename, #testname, points, prerequisites)) { timeout_ = timeoutval; } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname; \
    void GCHECK_TEST_##suitename##_##testname::TheTest()
#define _TEST2(suitename, testname) \
    class GCHECK_TEST_##suitename##_##testname : public gcheck::CustomTest { \
        void TheTest(); \
    public: \
        GCHECK_TEST_##suitename##_##testname() : CustomTest(gcheck::TestInfo(#suitename, #testname)) { } \
    }; \
    GCHECK_TEST_##suitename##_##testname GCHECK_TESTVAR_##suitename##_##testname; \
    void GCHECK_TEST_##suitename##_##testname::TheTest()

// params: suite name, test name, points (optional), prerequisites (optional)
#define TEST(...) \
    VFUNC(_TEST, __VA_ARGS__)


#define EXPECT_TRUE(b) \
    ExpectTrue(b, #b)

#define EXPECT_FALSE(b) \
    ExpectFalse(b, #b)

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
