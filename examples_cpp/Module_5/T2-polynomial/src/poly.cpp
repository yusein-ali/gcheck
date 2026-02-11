#include "poly.hpp"

#include <sstream>

int Poly::operator[](int exp) const {
    auto it = values_.find(exp);
    return it == values_.end() ? 0 : it->second;
}

// BEGIN SOLUTION
Poly& Poly::operator+=(const Poly& b) {
    for (auto it = b.crbegin(), end = b.crend(); it != end; ++it) {
        values_[it->first] += it->second;
    }
    return *this;
}

Poly& Poly::operator-=(const Poly& b) {
    return *this += -b;
}

std::istream& operator>>(std::istream& is, Poly& p) {
    std::string str;

    // Couldn't extract the token?
    if (!(is >> str)) 
        return is; 

    std::istringstream iss(str);

    // A temporary in order to avoid modifying p if an error occurs
    Poly tmp; 
    
    bool fail = false;
    for (int m, exp; iss >> m; ) {
        if (iss.get() == 'x' && iss >> exp) {
            // All OK, add to tmp
            tmp[exp] += m;
            continue; 
        }

        fail = true;
        break;
    }

    // Fail if iss >> m failed due to garbage rather than EOF
    if (!iss.eof()) 
        fail = true; 

    // Set iostate fail
    if (fail) 
        is.clear((is.rdstate() & ~std::ios::goodbit) | std::ios::failbit); 
    else 
        p = tmp; // All OK, store the result

    return is;
}

std::ostream& operator<<(std::ostream& os, const Poly& p) {
    bool first = true;
    for (auto beg = p.crbegin(), it = beg, end = p.crend(); it != end; ++it) {
        if (it->second == 0) continue; // Skip zero-valued terms
        // Print plus sign if needed
        if (!first && it->second > 0) os << '+';
        first = false;
        // Print the term
        os << it->second << "x" << it->first;
    }
    return os;
}

Poly operator+(const Poly& a, const Poly& b) {
    return Poly(a) += b;
}

Poly operator-(const Poly& a, const Poly& b) {
    return Poly(a) -= b;
}

Poly operator-(const Poly& p) {
    Poly ret;
    for (auto it = p.crbegin(), end = p.crend(); it != end; ++it) ret[it->first] = -it->second;
    return ret;
}

bool operator<(const Poly& a, const Poly& b) {
    auto ai = a.crbegin(), bi = b.crbegin(), ae = a.crend(), be = b.crend();
    // Skip zero-valued terms
    while (ai != ae && ai->second == 0) ++ai;
    while (bi != be && bi->second == 0) ++bi;
    // If either is empty, a < b only if a is empty and b isn't.
    if (ai == ae || bi == be) return ai == ae && bi != be;
    // Compare the exponents
    return ai->first < bi->first;
}

bool operator==(const Poly& a, const Poly& b) {
    return !(a < b) && !(b < a);
}

bool operator>(const Poly& a, const Poly& b) {
    return b < a;
}

bool operator!=(const Poly& a, const Poly& b) {
    return !(a == b);
}

// END SOLUTION
