#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

#include "triple.hpp"
#include <gcheck/tests.h>

// BEGIN SERVER INTERNAL
class Special
{
public:
    Special(const std::string &s, int n) : str_(s), num_(n) {}

    friend bool operator==(const Special &l, const Special &r);
    friend bool operator!=(const Special &l, const Special &r);
    friend std::ostream &operator<<(std::ostream &out, const Special &special);

private:
    std::string str_;
    int num_;
};

std::string to_string(const Special &s)
{
    std::stringstream ss;
    ss << s;
    return "Special" + ss.str();
}

template <class T1, class T2, class T3>
std::string to_string(const Triple<T1, T2, T3> &v)
{
    std::stringstream ss;
    ss << "[" << v.First() << ", " << v.Second() << ", " << v.Third() << "]";
    return ss.str();
}

bool operator==(const Special &l, const Special &r)
{
    return l.str_.compare(r.str_) == 0 && l.num_ == r.num_;
}

bool operator!=(const Special &l, const Special &r)
{
    return !(l == r);
}

std::ostream &operator<<(std::ostream &out, const Special &special)
{
    return out << "{" << special.str_ << ", " << special.num_ << "}";
}
// END SERVER INTERNAL

template <class T1, class T2, class T3>
class TripleSolution
{
public:
    // these aren't required but are a good thing to have
    typedef T1 Type1;
    typedef T2 Type2;
    typedef T3 Type3;

    // Constructor
    // Take the parameters as const references to avoid extra copies and
    // allow usage of consts as parameters
    TripleSolution(const T1 &param1, const T2 &param2, const T3 &param3)
        : val1_(param1), val2_(param2), val3_(param3) {}

    // Default, empty- and copy constructors will do, so no need to
    // declare them

    // Accessors
    const T1 &First() const { return val1_; }
    const T2 &Second() const { return val2_; }
    const T3 &Third() const { return val3_; }

private:
    T1 val1_;
    T2 val2_;
    T3 val3_;
};

template <typename T1, typename T2, typename T3>
bool operator==(const TripleSolution<T1, T2, T3> &l, const TripleSolution<T1, T2, T3> &r)
{
    return l.First() == r.First() && l.Second() == r.Second() && l.Third() == r.Third();
}

template <typename T1, typename T2, typename T3>
bool operator!=(const TripleSolution<T1, T2, T3> &l, const TripleSolution<T1, T2, T3> &r)
{
    return !(l == r);
}

template <typename T1, typename T2, typename T3>
bool operator==(const TripleSolution<T1, T2, T3> &l, const Triple<T1, T2, T3> &r)
{
    return l.First() == r.First() && l.Second() == r.Second() && l.Third() == r.Third();
}

template <typename T1, typename T2, typename T3>
std::ostream &operator<<(std::ostream &out, const TripleSolution<T1, T2, T3> &triple)
{
    return out << "[" << triple.First() << ", " << triple.Second() << ", " << triple.Third() << "]";
}

template <typename T1, typename T2, typename T3>
bool IsHomogenous(const TripleSolution<T1, T2, T3> &)
{
    return false;
}

template <typename T>
bool IsHomogenous(const TripleSolution<T, T, T> &)
{
    return true;
}

template <class T1, class T2, class T3>
std::string to_string(const TripleSolution<T1, T2, T3> &v)
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}

gcheck::Random rnd_str({"Templates", "are", "fun", "reb00t", "coffee"});
gcheck::Random rnd_int(-100, 100);
gcheck::Random rnd_double(-100.0, 100.0);
gcheck::Random rnd_char('a', 'z');

TEST(Triple, construct_and_access, 1)
{
    // Test the constructor
    std::vector<int> ints_(10);
    for (auto &i : ints_)
        i = rnd_int.Next();

    const auto &ints = ints_;

    double d1 = rnd_double.Next();
    char c1 = rnd_char.Next();

    Triple t1(ints[0], ints[1], ints[2]);
    TripleSolution ts1(ints[0], ints[1], ints[2]);

    Triple t2(ints[3], ints[4], ints[5]);
    TripleSolution ts2(ints[3], ints[4], ints[5]);

    Triple t3(ints[6], d1, c1);
    TripleSolution ts3(ints[6], d1, c1);

    // BEGIN SERVER INTERNAL
    std::string str1 = rnd_str.Next();
    std::string str2 = rnd_str.Next();
    Special s1 = Special(str1, ints[7]);
    const Special &rs1 = s1;
    Special s2 = Special(str2, ints[8]);
    const Special &rs2 = s2;

    Triple<Special, Special, size_t> t4(rs1, rs2, ints[9]);
    TripleSolution<Special, Special, size_t> ts4(rs1, rs2, ints[9]);

    // Test the accessors
    std::stringstream ss;
    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ts1) << " == " << to_string(t1);
    ExpectEqual(ts1, t1, ss.str());
    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ts2) << " == " << to_string(t2);
    ExpectEqual(ts2, t2, ss.str());
    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ts3) << " == " << to_string(t3);
    ExpectEqual(ts3, t3, ss.str());
    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ts4) << " == " << to_string(t4);
    ExpectEqual(ts4, t4, ss.str());
}

TEST(Triple, construct_and_assign_and_access, 1)
{
    // Test the constructor
    std::vector<int> ints_(10);
    for (auto &i : ints_)
        i = rnd_int.Next();

    const auto &ints = ints_;

    double d1 = rnd_double.Next();
    char c1 = rnd_char.Next();

    Triple t1(ints[0], ints[1], ints[2]);
    TripleSolution ts1(ints[0], ints[1], ints[2]);
    const auto &rt1 = t1;
    const auto &rts1 = ts1;

    Triple t2(ints[3], ints[4], ints[5]);
    TripleSolution ts2(ints[3], ints[4], ints[5]);

    Triple t3(ints[6], d1, c1);
    TripleSolution ts3(ints[6], d1, c1);
    const auto &rt3 = t3;
    const auto &rts3 = ts3;

    // BEGIN SERVER INTERNAL
    std::string str1 = rnd_str.Next();
    std::string str2 = rnd_str.Next();
    Special s1 = Special(str1, ints[7]);
    const Special &rs1 = s1;
    Special s2 = Special(str2, ints[8]);
    const Special &rs2 = s2;

    Triple<Special, Special, size_t> t4(rs1, rs2, ints[9]);
    TripleSolution<Special, Special, size_t> ts4(rs1, rs2, ints[9]);
    const Triple<Special, Special, size_t> &rt4 = t4;
    const TripleSolution<Special, Special, size_t> &rts4 = ts4;
    // END SERVER INTERNAL

    // Test the assignment operator
    Triple at1 = rt1;
    TripleSolution ats1 = rts1;
    t2 = t2;   // Self assignment
    ts2 = ts2; // Self assignment

    Triple at3 = rt3;
    TripleSolution ats3 = rts3;
    // BEGIN SERVER INTERNAL
    Triple at4 = rt4;
    TripleSolution ats4 = rts4;
    // END SERVER INTERNAL

    std::stringstream ss;
    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ats1) << " == " << to_string(at1) << " after assignment";
    ExpectEqual(ats1, at1, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ts2) << " == " << to_string(t2) << " after self-assignment";
    ExpectEqual(ts2, t2, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ats3) << " == " << to_string(at3) << " after assignment";
    ExpectEqual(ats3, at3, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(ats4) << " == " << to_string(at4) << " after assignment";
    ExpectEqual(ats4, at4, ss.str());
}

TEST(Triple, IsHomogenous, 1, "construct_and_access")
{
    std::vector<int> ints_(10);
    for (auto &i : ints_)
        i = rnd_int.Next();

    const auto &ints = ints_;

    double d1 = rnd_double.Next();
    char c1 = rnd_char.Next();

    Triple t1(ints[0], ints[1], ints[2]);
    TripleSolution ts1(ints[0], ints[1], ints[2]);

    Triple t2(ints[3], ints[4], ints[5]);
    TripleSolution ts2(ints[3], ints[4], ints[5]);

    Triple t3(ints[6], d1, c1);
    TripleSolution ts3(ints[6], d1, c1);

    // BEGIN SERVER INTERNAL
    std::string str1 = rnd_str.Next();
    std::string str2 = rnd_str.Next();
    Special s1 = Special(str1, ints[7]);
    const Special &rs1 = s1;
    Special s2 = Special(str2, ints[8]);
    const Special &rs2 = s2;

    Triple<Special, Special, size_t> t4(rs1, rs2, ints[9]);
    TripleSolution<Special, Special, size_t> ts4(rs1, rs2, ints[9]);
    // END SERVER INTERNAL

    // Test the IsHomogenous function
    std::stringstream ss;
    ss.str(std::string(""));
    ss.clear();
    ss << "IsHomogenous(" << to_string(ts1) << ") == IsHomogeneous(" << to_string(t1) << ") == True";
    ExpectEqual(IsHomogenous(ts1), IsHomogenous(t1), ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << "IsHomogenous(" << to_string(ts2) << ") == IsHomogeneous(" << to_string(t2) << ") == True";
    ExpectEqual(IsHomogenous(ts2), IsHomogenous(t2), ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << "IsHomogenous(" << to_string(ts3) << ") == IsHomogeneous(" << to_string(t3) << ") == True";
    ExpectEqual(IsHomogenous(ts3), IsHomogenous(t3), ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << "IsHomogenous(" << to_string(ts4) << ") == IsHomogeneous(" << to_string(t4) << ") == True";
    ExpectEqual(IsHomogenous(ts4), IsHomogenous(t4), ss.str());
}

TEST(Triple, operators, 1, "construct_and_access")
{
    std::vector<int> ints_(11);
    for (auto &i : ints_)
        i = rnd_int.Next();

    const auto &ints = ints_;

    double d1 = rnd_double.Next();
    double d2 = rnd_double.Next();
    char c1 = rnd_char.Next();
    char c2 = rnd_char.Next();

    Triple t1(ints[0], ints[1], ints[2]);
    TripleSolution ts1(ints[0], ints[1], ints[2]);
    const auto &rt1 = t1;

    Triple t2(ints[3], ints[4], ints[5]);
    TripleSolution ts2(ints[3], ints[4], ints[5]);
    const auto &rt2 = t2;

    Triple t3(ints[6], d1, c1);
    TripleSolution ts3(ints[6], d1, c1);
    const auto &rt3 = t3;

    Triple t4(ints[10], d2, c2);
    TripleSolution ts4(ints[10], d2, c2);
    const auto &rt4 = t4;

    // BEGIN SERVER INTERNAL
    std::string str1 = rnd_str.Next();
    std::string str2 = rnd_str.Next();
    Special s1 = Special(str1, ints[7]);
    const Special &rs1 = s1;
    Special s2 = Special(str2, ints[8]);
    const Special &rs2 = s2;

    Triple t5(rs1, rs2, ints[9]);
    TripleSolution ts5(rs1, rs2, ints[9]);
    const auto &rt5 = t5;

    Triple t6(t5);
    TripleSolution ts6(ts5);
    const auto &rt6 = t6;
    // END SERVER INTERNAL

    // Test the << -operator
    // use stringstream as output
    std::stringstream out_test;
    std::stringstream out_solution;
    std::stringstream ss;

    out_test << rt1 << std::endl;
    out_solution << ts1 << std::endl;

    out_test.str(std::string(""));
    out_solution.str(std::string(""));
    out_test << t1;
    out_solution << ts1;
    ss.str(std::string(""));
    ss.clear();
    ss << "ostream.operator<<(" << ts1 << ") == ostream.operator<<(" << t1 << ")";
    ExpectEqual(out_test.str(), out_solution.str(), ss.str());

    out_test.str(std::string(""));
    out_solution.str(std::string(""));
    out_test << t2;
    out_solution << ts2;
    ss.str(std::string(""));
    ss.clear();
    ss << "ostream.operator<<(" << ts2 << ") == ostream.operator<<(" << t2 << ")";
    ExpectEqual(out_test.str(), out_solution.str(), ss.str());

    out_test.str(std::string(""));
    out_solution.str(std::string(""));
    out_test << t3;
    out_solution << ts3;
    ss.str(std::string(""));
    ss.clear();
    ss << "ostream.operator<<(" << ts3 << ") == ostream.operator<<(" << t3 << ")";
    ExpectEqual(out_test.str(), out_solution.str(), ss.str());

    out_test.str(std::string(""));
    out_solution.str(std::string(""));
    out_test << t4;
    out_solution << ts4;
    ss.str(std::string(""));
    ss.clear();
    ss << "ostream.operator<<(" << ts4 << ") == ostream.operator<<(" << t4 << ")";
    ExpectEqual(out_test.str(), out_solution.str(), ss.str());

    // Test the == -operator
    ss.str(std::string(""));
    ss.clear();
    ss << to_string(t1) << ".operator==(" << to_string(t2) << ") == "
       << to_string(ts1) << ".operator==(" << to_string(ts2) << ")";
    ExpectEqual(rt1 == rt2, ts1 == ts2, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(t3) << ".operator==(" << to_string(t4) << ") == "
       << to_string(ts3) << ".operator==(" << to_string(ts4) << ")";
    ExpectEqual(rt3 == rt4, ts3 == ts4, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(t5) << ".operator==(" << to_string(t6) << ") == "
       << to_string(ts5) << ".operator==(" << to_string(ts6) << ")";
    ExpectEqual(rt5 == rt6, ts5 == ts6, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(t1) << ".operator!=(" << to_string(t2) << ") == "
       << to_string(ts1) << ".operator!=(" << to_string(ts2) << ")";
    ExpectEqual(rt1 != rt2, ts1 != ts2, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(t3) << ".operator!=(" << to_string(t4) << ") == "
       << to_string(ts3) << ".operator!=(" << to_string(ts4) << ")";
    ExpectEqual(rt3 != rt4, ts3 != ts4, ss.str());

    ss.str(std::string(""));
    ss.clear();
    ss << to_string(t5) << ".operator!=(" << to_string(t6) << ") == "
       << to_string(ts5) << ".operator!=(" << to_string(ts6) << ")";
    ExpectEqual(rt5 != rt6, ts5 != ts6, ss.str());
}
