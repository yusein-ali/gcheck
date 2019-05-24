#include "gcheck.h"
#include <iostream>
#include <list>
#include <string>

std::string asd(char a, int b) {
    std::string str;
    str += a;
    str += ' ';
    str += std::to_string(b);
    str += '\n';
    return str;
}
std::string asd3(char a, int b) {
    std::string str;
    str += a;
    str += 'k';
    str += std::to_string(b);
    str += '\n';
    return str;
}

std::string asd2(std::list<char> a) {
    std::string str;
    for(auto it = a.begin(); it != a.end(); ++it)
        (str += *it) += ' ';
    return str;
}

TEST(suite1, test1) {
    GradingMethod("partial");
    RangeDistribution<char> dist('a', 'z');
    ChoiceDistribution<char> dist2({'a', 'b', 'd', 'k'});
    RandomArgument<char> rng(dist);
    RandomContainerArgument<std::list, char> rng2(std::list<char>(10), dist2);
    ConstantArgument cnt(1);
    TEST_CASE(10, asd, asd3, rng, cnt);
    TEST_CASE(10, asd2, asd2, rng2);
    std::cout << "err";
    EXPECT_EQ(true, true);
}

TEST(suite1, test2) {
    OutputFormat("vertical");
    GradingMethod("binary");
    std::cout << "err2";
    EXPECT_EQ(true, true);
    std::cout << "err3";
    EXPECT_EQ(false, true);
}

TEST_(suite2, test1, 3) {
    GradingMethod("binary");
    std::cout << "err4";
    std::cerr << "err";
    EXPECT_EQ(true, true);
}