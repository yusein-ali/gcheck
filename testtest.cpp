#include "gcheck.h"
#include <iostream>
#include <list>
#include <string>
#include "argument.h"

using namespace gcheck;

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
    std::cerr << "err" << std::endl;
    std::cout << "out" << std::endl;
    std::cerr << "err" << std::endl;
    std::cout << "out" << std::endl;
    std::string str;
    for(auto it = a.begin(); it != a.end(); ++it)
        (str += *it) += ' ';
    std::cerr << "err" << std::endl;
    std::cout << "out" << std::endl;
    return str;
}

std::string asd4(std::list<char> a) {
    std::cerr << "err" << std::endl;
    std::string str;
    for(auto it = a.begin(); it != a.end(); ++it)
        (str += *it) += ' ';
    return str;
}

TEST(suite1, test1) {
    
    RangeDistribution<char> dist('a', 'z');
    ChoiceDistribution<char> dist2({'a', 'b', 'd', 'k'});
    Random<char> rng(dist);
    RandomContainer<char, std::list, ChoiceDistribution> rng2(10, dist2);
    Constant cnt(1);
    //TestCase(10, asd, asd3, Random<char>(dist), cnt);
    TestCase(1, asd2, asd4, rng2);
    //EXPECT_EQ(true, true);
}
/*
TEST(suite1, test2) {
    Rnd<int> asds;
    RangeDistribution<char> dist('a', 'z');
    ChoiceDistribution<char> dist2({'a', 'b', 'd', 'k'});
    Random<char> rng(dist);
    RandomContainer<char, std::list, ChoiceDistribution> rng2(100, dist2);
    TestCase(1, asd2, asd4, rng2);
}*/
/*
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
}*/