#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

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

TEST(Iterator, ReadVector) {
    CompareWithCallable(10, CRead, TRead, rnd);
}


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

TEST(Iterator, PrintSum1) {
    CompareWithCallable(10, CPrintSum1, TPrintSum1, rnd);
}


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

TEST(Iterator, PrintSum2) {
    CompareWithCallable(10, CPrintSum2, TPrintSum2, rnd);
}
