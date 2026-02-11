#include <string>
#include <cmath>
#include <sstream>

#include "matrix.hpp"
#include <gcheck/argument.h>
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

// BEGIN SERVER INTERNAL
struct Wrapper : public Matrix {
    template<class... Args>
    Wrapper(Args... args) : Matrix(args...) {}
};

typedef gcheck::Result<Wrapper> Result;

class CMatrix {
public:
    CMatrix(const std::vector<int>& in) : data_((int)sqrt(in.size())) {

        auto it = in.begin();
        for(auto& v : data_)
            for(int i = 0; i < (int)sqrt(in.size()); i++)
                v.push_back(*(it++));
    }

    CMatrix Rotate90Deg() {
        Matrix v(data_.size());

        for (size_t i = 0; i < data_.size(); ++i)
            v[i] = std::vector<int>(data_.size());

        for (size_t i = 0; i < data_.size(); ++i)
            for (size_t j = 0; j < data_.size(); ++j)
                v[j][data_.size()-i-1] = data_[i][j];

        data_ = v;
        return *this;
    }

    std::string ToString() {
        std::stringstream ss;
        ss << "Printing out a " << data_.size() << " x " << data_.size() << " matrix:" << std::endl;

        for (auto &i : data_){
            for (auto &j : i)
                ss << j << " ";
            ss << std::endl;
        }
        return ss.str();
    }

    operator Wrapper() {
        return data_;
    }
    operator Matrix() {
        return data_;
    }
private:
    Matrix data_;
};

Matrix MakeMatrix(const std::vector<int>& in) {
    Matrix out(sqrt(in.size()));
    auto it = in.begin();
    for(auto& v : out)
        for(int i = 0; i < (int)sqrt(in.size()); i++)
            v.push_back(*(it++));
    return out;
}

std::string GetInputString(const std::vector<int>& input) {

    std::string input_str;
    for(auto& i : input)
        input_str += std::to_string(i) + ' ';

    if(input.size() != 0)
        input_str.pop_back();
    input_str += '\n';

    return input_str;
}
// END SERVER INTERNAL
auto rnd = gcheck::RandomSizeContainer({ 0, 1, 4, 9, 16, 25 }, 0, 9);
gcheck::SequenceArgument<std::vector<int>> seq(
    { { 0, 1, 2,
        2, 3, 4,
        2, 1, 1 },
      { 7, 5,
        9, 0 },
      { 7, 1, 3, 5,
        9, 2, 3, 2,
        0, 2, 3, 8,
        8, 4, 5, 6 },
    });

std::string to_string(const Matrix& matrix) {
    std::stringstream ss;
    for (auto &i : matrix){
        for (auto &j : i)
            ss << j << " ";
        ss << '\n';
    }
    return ss.str();
}

bool operator==(const Matrix& m1, const Matrix& m2) {
    if(m1.size() != m2.size())
        return false;

    for(auto it1 = m1.begin(), it2 = m2.begin(); it1 != m1.end(); it1++, it2++) {
        for(auto it12 = it1->begin(), it22 = it2->begin(); it12 != it1->end(); it12++, it22++) {
            if(*it12 != *it22)
                return false;
        }
    }
    return true;
}

Matrix TRead(int size) {
    return ReadMatrix(size);
}

// BEGIN SERVER INTERNAL
Matrix CRead(const std::vector<int>& input) {
    return CMatrix(input);
}
// END SERVER INTERNAL

namespace {

class MatrixReadTest : public gcheck::IOTest<Matrix, int> {
public:
    MatrixReadTest() : gcheck::IOTest<Matrix, int>(gcheck::TestInfo("Matrix", "Read", 2.5), (int)seq.GetSize(), TRead) {}

private:
    void SetInputsAndOutputs() override {
        SetTimeout(1);
        auto input = seq.Next();
        SetArguments((int)sqrt(input.size()));
        SetInput(GetInputString(input));
        SetReturn(CRead(input));
    }
};
MatrixReadTest matrix_read_test;

// BEGIN SERVER INTERNAL
class MatrixReadRandomTest : public gcheck::IOTest<Matrix, int> {
public:
    MatrixReadRandomTest() : gcheck::IOTest<Matrix, int>(gcheck::TestInfo("Matrix", "Read_random", 5), 10, TRead) {}

private:
    void SetInputsAndOutputs() override {
        SetTimeout(1);
        auto input = rnd.Next();
        SetArguments((int)sqrt(input.size()));
        SetInput(GetInputString(input));
        SetReturn(CRead(input));
    }
};
MatrixReadRandomTest matrix_read_random_test;
// END SERVER INTERNAL

/* STUDENT:
Matrix TRotate(Matrix input) {
    return Rotate90Deg(input);
}*/

// BEGIN SERVER INTERNAL
Matrix TRotate(const std::vector<int>& input) {
    Matrix m = MakeMatrix(input);
    return Rotate90Deg(m);
}
gcheck::Result<Matrix, Matrix> CRotate(const std::vector<int>& input) {
    gcheck::Result<Matrix, Matrix> res;

    auto cm = CMatrix(input);
    res.SetInput(cm);

    Matrix m = cm.Rotate90Deg();
    res.output = m;

    std::vector<int> flat;
    for(auto v : m)
        for(auto i : v)
            flat.push_back(i);

    return res;
}
// END SERVER INTERNAL

class MatrixRotateTest : public gcheck::CustomTest {
public:
    MatrixRotateTest() : gcheck::CustomTest(gcheck::TestInfo("Matrix", "Rotate", 2.5)) {}

private:
    void TheTest() override {
        seq.Reset();
        CompareWithCallable((int)seq.GetSize(), CRotate, TRotate, seq);
    }
};
MatrixRotateTest matrix_rotate_test;
// BEGIN SERVER INTERNAL
class MatrixRotateRandomTest : public gcheck::CustomTest {
public:
    MatrixRotateRandomTest() : gcheck::CustomTest(gcheck::TestInfo("Matrix", "Rotate_random", 5)) {}

private:
    void TheTest() override {
        CompareWithCallable(10, CRotate, TRotate, rnd);
    }
};
MatrixRotateRandomTest matrix_rotate_random_test;
// END SERVER INTERNAL



/* STUDENT:
std::string TPrint(Matrix input) {
    gcheck::StdoutCapturer tout;

    Print(input);

    return tout.str();
}*/

// BEGIN SERVER INTERNAL
std::string TPrint(const std::vector<int>& input) {
    Matrix m = MakeMatrix(input);

    gcheck::StdoutCapturer tout;

    Print(m);

    return tout.str();
}

gcheck::Result<std::string, Matrix> CPrint(const std::vector<int>& input) {

    gcheck::Result<std::string, Matrix> res;
    auto cm = CMatrix(input);
    res.SetInput(cm);

    res.output = cm.ToString();
    return res;
}
// END SERVER INTERNAL

class MatrixPrintTest : public gcheck::CustomTest {
public:
    MatrixPrintTest() : gcheck::CustomTest(gcheck::TestInfo("Matrix", "Print", 2)) {}

private:
    void TheTest() override {
        seq.Reset();
        CompareWithCallable((int)seq.GetSize(), CPrint, TPrint, seq);
    }
};
MatrixPrintTest matrix_print_test;
// BEGIN SERVER INTERNAL
class MatrixPrintRandomTest : public gcheck::CustomTest {
public:
    MatrixPrintRandomTest() : gcheck::CustomTest(gcheck::TestInfo("Matrix", "Print_random", 3)) {}

private:
    void TheTest() override {
        CompareWithCallable(10, CPrint, TPrint, rnd);
    }
};
MatrixPrintRandomTest matrix_print_random_test;
// END SERVER INTERNAL

} // namespace
