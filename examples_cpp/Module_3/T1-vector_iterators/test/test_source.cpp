#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include "vector_it.hpp"
#include <gcheck/argument.h>
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

auto rnd = gcheck::RandomSizeContainer(1, 20, 0, 1000);

// BEGIN SERVER INTERNAL
gcheck::Result<std::vector<int>> CRead(const std::vector<int>& v) {
    gcheck::Result<std::vector<int>> res(v);
    std::string ints = " ";
    for(auto i : v)
        ints += std::to_string(i) + ",";
    ints.pop_back();
    //res.SetOutputParams("std::vector<int>{" + ints + "}");
    return res;
}
// END SERVER INTERNAL

std::vector<int> TRead(const std::vector<int>& v) {
    std::string input;
    for(auto i : v) {
        input += std::to_string(i) + " ";
    }
    input += "!\n";
    gcheck::StdinInjecter tin(input);

    std::vector<int> out = ReadVector();

    return out;
}

namespace {

class IteratorReadVectorTest : public gcheck::CustomTest {
public:
    IteratorReadVectorTest() : gcheck::CustomTest(gcheck::TestInfo("Iterator", "ReadVector")) {}

private:
    void TheTest() override {
        CompareWithCallable(10, CRead, TRead, rnd);
    }
};
IteratorReadVectorTest iterator_read_vector_test;


// BEGIN SERVER INTERNAL
std::string CPrintSum1(const std::vector<int> v) {
    std::stringstream ss;
    for(auto it = v.begin(); it < v.end()-1; it++)
        ss << *it+*(it+1) << " ";
    ss << std::endl;
    return ss.str();
}
// END SERVER INTERNAL

std::string TPrintSum1(const std::vector<int> v) {
    gcheck::StdoutCapturer tout;

    PrintSum1(v);

    return tout.str();
}

class IteratorPrintSum1Test : public gcheck::CustomTest {
public:
    IteratorPrintSum1Test() : gcheck::CustomTest(gcheck::TestInfo("Iterator", "PrintSum1")) {}

private:
    void TheTest() override {
        CompareWithCallable(10, CPrintSum1, TPrintSum1, rnd);
    }
};
IteratorPrintSum1Test iterator_print_sum1_test;


// BEGIN SERVER INTERNAL
std::string CPrintSum2(const std::vector<int> v) {
    std::stringstream ss;
    auto b = v.begin(), e = v.end()-1;
    for (; e > b; ++b, --e)
        ss << *b + *e << " ";
    ss << std::endl;
    return ss.str();
}
// END SERVER INTERNAL

std::string TPrintSum2(const std::vector<int> v) {
    gcheck::StdoutCapturer tout;

    PrintSum2(v);

    return tout.str();
}

class IteratorPrintSum2Test : public gcheck::CustomTest {
public:
    IteratorPrintSum2Test() : gcheck::CustomTest(gcheck::TestInfo("Iterator", "PrintSum2")) {}

private:
    void TheTest() override {
        CompareWithCallable(10, CPrintSum2, TPrintSum2, rnd);
    }
};
IteratorPrintSum2Test iterator_print_sum2_test;

} // namespace
