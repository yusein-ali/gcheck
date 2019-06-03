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

std::string asd4(std::list<char> a) {
    std::string str;
    for(auto it = a.begin(); it != a.end(); ++it)
        (str += *it) += ' ';
    return str;
}

TEST(suite1, test1) {
    GradingMethod("partial");
    gcheck::RangeDistribution<char> dist('a', 'z');
    gcheck::ChoiceDistribution<char> dist2({'a', 'b', 'd', 'k'});
    gcheck::RandomArgument<char> rng(dist);
    gcheck::RandomContainerArgument<std::list, char> rng2(std::list<char>(10), dist2);
    gcheck::ConstantArgument cnt(1);
    TestCase(10, asd, asd3, gcheck::RandomArgument<char>(dist), cnt);
    TestCase(10, asd2, asd4, rng2);
    std::cout << "err";
    EXPECT_EQ(true, true);
}

TEST(suite1, test2) {
    OutputFormat("vertical");
    GradingMethod("binary");
    std::cout << "err2";
    EXPECT_EQ(true, true) << "true is indeed true";
    std::cout << "err3";
    EXPECT_EQ(false, true);
}

TEST_(suite2, test1, 3) {
    GradingMethod("binary");
    std::cout << "err4";
    std::cerr << "err";
    EXPECT_EQ(true, true);
}