#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

#include "poly.hpp"
#include <gcheck/argument.h>
#include <gcheck/tests.h>

// variables for tests

// BEGIN SERVER INTERNAL
gcheck::Container rnd = gcheck::RandomSizeContainer(0, 5, -100, 100);
gcheck::Container rnd2 = gcheck::RandomSizeContainer(0, 5, -100, 100);
gcheck::Container rnd3 = gcheck::RandomSizeContainer(1, 5, -100, 100);
// END SERVER INTERNAL

gcheck::SequenceArgument<std::vector<int>> seq({ {}, {1}, {1, 2}, {5, 3, 3, -2}, {0, 5, -10, 15, 20},
    });
gcheck::SequenceArgument<std::vector<int>> seq2({ {1, -4}, {0, 5, -2}, {1, 2}, {1, -3, 3, -2}, {4, 5, -1, 5},
    });

// BEGIN SERVER INTERNAL
class Wrapper {
public:
    std::vector<int> vector_;

    Wrapper(const Poly& p) {
        for(auto it = p.cbegin(); it != p.cend(); it++) {
            if(vector_.size() < (unsigned)it->first+1)
                vector_.resize(it->first+1, 0);
            vector_[it->first] = it->second;
        }
    }
    Wrapper(const std::vector<int>& v) : vector_(v) {}

    bool operator<(const Wrapper& w) const {

        int s1 = vector_.size()-1, s2 = w.vector_.size()-1;
        for(; s1 >= 0; s1--)
            if(vector_[s1] != 0)
                break;
        for(; s2 >= 0; s2--)
            if(w.vector_[s2] != 0)
                break;

        return s1 < s2;
    }
    bool operator>(const Wrapper& w) const {
        return !(*this < w || *this == w);
    }

    bool operator==(const Wrapper& w) const {
        return !(*this != w);
    }
    bool operator!=(const Wrapper& w) const {
        return *this < w || w < *this;
    }
};

std::istream& operator>>(std::istream& is, Wrapper& p) {
    std::string token;
    if (!(is >> token)) return is; // Couldn't extract the token?
    std::istringstream iss(token);
    Poly tmp; // A temporary in order to avoid modifying p if an error occurs
    bool fail = false;
    for (int m, exp; iss >> m; tmp[exp] += m) {
        if (iss.get() == 'x' && iss >> exp) continue; // All OK, add to tmp
        fail = true;
        break;
    }
    if (!iss.eof()) fail = true; // Fail if iss >> m failed due to garbage rather than EOF
    if (fail) is.clear((is.rdstate() & ~std::ios::goodbit) | std::ios::failbit); // Set iostate fail
    else p = tmp; // All OK, output the result
    return is;
}

std::ostream& operator<<(std::ostream& os, const Wrapper& p) {
    bool first = true;
    auto& v = p.vector_;
    for(int i = v.size()-1; i >= 0; i--) {
        if (v[i] == 0) continue; // Skip zero-valued terms
        // Print plus sign if needed
        if (!first && v[i] > 0) os << '+';
        first = false;
        // Print the term
        os << v[i] << "x" << i;
    }
    return os;
}

std::string to_string(const Wrapper& w) {
    std::stringstream ss;
    ss << w;
    return ss.str();
}
// END SERVER INTERNAL

/* STUDENT:
class Wrapper {
public:
    std::vector<int> vector_;

    Wrapper(const Poly& p) {
        for(auto it = p.begin(); it != p.end(); it++) {
            if(vector_.size() < (unsigned)it->first+1)
                vector_.resize(it->first+1, 0);
            vector_[it->first] = it->second;
        }
    }
    Wrapper(const std::vector<int>& v) : vector_(v) {}

    bool operator==(const Poly& p) const {

        int non_zeros = 0;
        for(auto& i : vector_) {
            if(i != 0) non_zeros++;
        }

        int non_zeros_p = 0;
        for(auto& i : p) {
            size_t index = i.first;
            if(i.second != 0) {
                non_zeros_p++;
                if(index >= vector_.size() || vector_[index] != i.second)
                    return false;
            }
        }
        return non_zeros == non_zeros_p;
    }
};
*/

std::string to_construct(const Wrapper& w) {
    return "Wrapper(" + gcheck::toConstruct(w.vector_) + ")";
}

bool operator==(const Poly& p, const Wrapper& w) {
    int non_zeros = 0;
    for(auto& i : w.vector_) {
        if(i != 0) non_zeros++;
    }

    int non_zeros_p = 0;
     for(auto& i : p) {
        size_t index = i.first;
        if(i.second != 0) {
            non_zeros_p++;
            if(index >= w.vector_.size() || w.vector_[index] != i.second)
                return false;
        }
    }
    return non_zeros == non_zeros_p;
}
/*STUDENT:
std::string to_string(const Wrapper& w) {
    std::string out;

    const std::vector<int>& v = w.vector_;

    for(int i = v.size()-1; i >= 0; i--) {
        if(i != 0) {
            out += v[i] > 0 ? " +" : " ";
        }
        out += std::to_string(v[i]);
        out += "x^";
        out += std::to_string(i);
    }

    return out;
}*/

Poly MakePoly(const std::vector<int>& v, int mult = 1) {
    Poly p;
    for(unsigned i = 0; i < v.size(); i++)
        p[i] = v[i] * mult;
    return p;
}


// BEGIN SERVER INTERNAL
Wrapper CNegate(const std::vector<int>& v) {
    return v;
}
// END SERVER INTERNAL
Poly TNegate(const std::vector<int>& v) {
    return -MakePoly(v, -1);
}

TEST(Poly, negate) {
    seq.Reset();
    auto p = TNegate(seq);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(seq.GetSize(), CNegate, TNegate, seq);
}

// BEGIN SERVER INTERNAL
TEST(Poly, negate_random) {
    auto p = TNegate(rnd);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(10, CNegate, TNegate, rnd);
}



Wrapper CAdd(std::vector<int> v1, std::vector<int> v2) {
    size_t size = std::max(v1.size(), v2.size());
    v1.resize(size, 0);
    v2.resize(size, 0);
    std::vector<int> v;
    for(unsigned i = 0; i < v1.size(); i++)
        v.push_back(v1[i] + v2[i]);

    return v;
}
// END SERVER INTERNAL
Poly TAdd(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) + MakePoly(v2);
}

Poly TAdd2(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) += MakePoly(v2);
}

TEST(Poly, add) {
    seq.Reset();
    seq2.Reset();
    auto p = TAdd(seq, seq2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(seq.GetSize(), CAdd, TAdd, seq, seq2);
}

TEST(Poly, add2) {
    seq.Reset();
    seq2.Reset();
    auto p = TAdd2(seq, seq2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(seq.GetSize(), CAdd, TAdd2, seq, seq2);
}

// BEGIN SERVER INTERNAL
TEST(Poly, add_random) {
    auto p = TAdd(rnd, rnd2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(10, CAdd, TAdd, rnd, rnd2);
}

TEST(Poly, add2_random) {
    auto p = TAdd2(rnd, rnd2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(10, CAdd, TAdd2, rnd, rnd2);
}


Wrapper CSubstract(std::vector<int> v1, std::vector<int> v2) {
    size_t size = std::max(v1.size(), v2.size());
    v1.resize(size, 0);
    v2.resize(size, 0);
    std::vector<int> v;
    for(unsigned i = 0; i < v1.size(); i++)
        v.push_back(v1[i] - v2[i]);

    return v;
}
// END SERVER INTERNAL
Poly TSubstract(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) - MakePoly(v2);
}

Poly TSubstract2(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) -= MakePoly(v2);
}

TEST(Poly, substract) {
    seq.Reset();
    seq2.Reset();
    auto p = TSubstract(seq, seq2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(seq.GetSize(), CSubstract, TSubstract, seq, seq2);
}

TEST(Poly, substract2) {
    seq.Reset();
    seq2.Reset();
    auto p = TSubstract2(seq, seq2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(seq.GetSize(), CSubstract, TSubstract2, seq, seq2);
}

// BEGIN SERVER INTERNAL
TEST(Poly, substract_random) {
    auto p = TSubstract(rnd, rnd2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(10, CSubstract, TSubstract, rnd, rnd2);
}

TEST(Poly, substract2_random) {
    auto p = TSubstract2(rnd, rnd2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(10, CSubstract, TSubstract2, rnd, rnd2);
}



bool CEqual(const std::vector<int>& v1, const std::vector<int>& v2) {
    return Wrapper(v1) == Wrapper(v2);
}
// END SERVER INTERNAL
bool TEqual(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) == MakePoly(v2);
}

TEST(Poly, equal) {
    seq.Reset();
    seq2.Reset();
    CompareWithCallable(seq.GetSize(), CEqual, TEqual, seq, seq2);
}

// BEGIN SERVER INTERNAL
TEST(Poly, equal_random) {
    CompareWithCallable(10, CEqual, TEqual, rnd, rnd2);
}



bool CNEqual(const std::vector<int>& v1, const std::vector<int>& v2) {
    return Wrapper(v1) != Wrapper(v2);
}
// END SERVER INTERNAL
bool TNEqual(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) != MakePoly(v2);
}

TEST(Poly, not_equal) {
    seq.Reset();
    seq2.Reset();
    CompareWithCallable(seq.GetSize(), CNEqual, TNEqual, seq, seq2);
}

// BEGIN SERVER INTERNAL
TEST(Poly, not_equal_random) {
    CompareWithCallable(10, CNEqual, TNEqual, rnd, rnd2);
}



bool CSmaller(const std::vector<int>& v1, const std::vector<int>& v2) {
    return Wrapper(v1) < Wrapper(v2);
}
// END SERVER INTERNAL
bool TSmaller(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) < MakePoly(v2);
}

TEST(Poly, lesser_than) {
    seq.Reset();
    seq2.Reset();
    CompareWithCallable(seq.GetSize(), CSmaller, TSmaller, seq, seq2);
}

// BEGIN SERVER INTERNAL
TEST(Poly, lesser_than_random) {
    CompareWithCallable(10, CSmaller, TSmaller, rnd, rnd2);
}



bool CBigger(const std::vector<int>& v1, const std::vector<int>& v2) {
    return Wrapper(v1) > Wrapper(v2);
}
// END SERVER INTERNAL
bool TBigger(const std::vector<int>& v1, const std::vector<int>& v2) {
    return MakePoly(v1) > MakePoly(v2);
}

TEST(Poly, greater_than) {
    seq.Reset();
    seq2.Reset();
    CompareWithCallable(seq.GetSize(), CBigger, TBigger, seq, seq2);
}

// BEGIN SERVER INTERNAL
TEST(Poly, greater_than_random) {
    CompareWithCallable(10, CBigger, TBigger, rnd, rnd2);
}



std::string CWrite(const std::vector<int>& v) {
    std::stringstream ss;
    ss << Wrapper(v);
    return ss.str();
}
// END SERVER INTERNAL

std::string TWrite(const std::vector<int>& v) {
    std::stringstream ss;
    ss << MakePoly(v);
    return ss.str();
}

TEST(Poly, write_operator) {
    seq.Reset();
    CompareWithCallable(seq.GetSize(), CWrite, TWrite, seq);
}
// BEGIN SERVER INTERNAL
TEST(Poly, write_operator_random) {
    CompareWithCallable(10, CWrite, TWrite, rnd);
}



gcheck::Result<Wrapper> CRead(const std::vector<int>& v) {
    gcheck::Result<Wrapper> res((Wrapper(v)));
    std::stringstream ss;
    ss << res.output;
    res.SetInput(ss.str());
    return res;
}

Poly TRead(const std::vector<int>& v) {
    std::stringstream ss;
    ss << Wrapper(v);
    Poly out;
    ss >> out;
    return out;
}
// END SERVER INTERNAL
/*STUDENT:
Poly TRead(const std::string& str) {
    std::stringstream ss(str);
    Poly out;
    ss >> out;
    return out;
}
*/

TEST(Poly, read_operator) {
    seq2.Reset();
    auto p = TRead(seq2);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(seq2.GetSize(), CRead, TRead, seq2);
}
//BEGIN SERVER INTERNAL
TEST(Poly, read_operator_random) {
    auto p = TRead(rnd3);
    if(p.cbegin()->first < 0) {
        FAIL() << "Negative exponents not allowed!";
    } else
        CompareWithCallable(10, CRead, TRead, rnd3);
}
//END SERVER INTERNAL
