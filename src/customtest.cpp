#include <chrono>

#include "customtest.h"
#include "multiprocessing.h"

namespace gcheck {

void CustomTest::ActualTest() {
    if(do_safe_run_) {
        auto status = RunForked(std::chrono::duration<double>(timeout_), data_, 1024*1024, std::bind(&CustomTest::TheTest, this));
        if(status == OK) {
            data_.status = Finished;
        } else if(status == TIMEDOUT) {
            data_.status = TimedOut;
        }
    } else {
        TheTest();
        data_.status = Finished;
    }
}

std::stringstream& CustomTest::ExpectTrue(bool b, std::string descriptor) {

    TestReport report = TestReport::Make<TrueData>();
    auto& data = report.Get<TrueData>();

    data.value = b;
    data.descriptor = descriptor;
    data.result = b;

    return AddReport(report).info_stream;
}

std::stringstream& CustomTest::ExpectFalse(bool b, std::string descriptor) {

    TestReport report = TestReport::Make<FalseData>();
    auto& data = report.Get<FalseData>();

    data.value = b;
    data.descriptor = descriptor;
    data.result = !b;

    return AddReport(report).info_stream;
}


template std::stringstream& CustomTest::ExpectEqual(unsigned int left, unsigned int right, std::string descriptor);
template std::stringstream& CustomTest::ExpectEqual(int left, int right, std::string descriptor);
template std::stringstream& CustomTest::ExpectEqual(float left, float right, std::string descriptor);
template std::stringstream& CustomTest::ExpectEqual(double left, double right, std::string descriptor);
template std::stringstream& CustomTest::ExpectEqual(std::string left, std::string right, std::string descriptor);
template std::stringstream& CustomTest::ExpectEqual(std::string left, const char* right, std::string descriptor);
template std::stringstream& CustomTest::ExpectEqual(const char* left, std::string right, std::string descriptor);

} // gcheck