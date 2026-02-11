#if !defined(GCHECK_BUILD_TESTS)
#include <gcheck/gcheck.h>
#include <gcheck/io_test.h>
#else
#include "gcheck.h"
#include "io_test.h"
#endif

void VoidAndEmpty() {
}
int IntAndEmpty() {
    return 0;
}
void VoidAndIntInt(int, int) {
}
int IntAndIntInt(int, int) {
    return 0;
}

namespace {

class BasicVoidAndEmptyTest : public gcheck::IOTest<void> {
public:
    BasicVoidAndEmptyTest() : gcheck::IOTest<void>(gcheck::TestInfo("basic", "VoidAndEmpty", 4), 3, VoidAndEmpty) {}

private:
    void SetInputsAndOutputs() override {}
};
BasicVoidAndEmptyTest basic_void_and_empty_test;

class BasicIntAndEmptyTest : public gcheck::IOTest<int> {
public:
    BasicIntAndEmptyTest() : gcheck::IOTest<int>(gcheck::TestInfo("basic", "IntAndEmpty", 4), 3, IntAndEmpty) {}

private:
    void SetInputsAndOutputs() override {
        this->SetReturn(0);
    }
};
BasicIntAndEmptyTest basic_int_and_empty_test;

class BasicVoidAndIntIntTest : public gcheck::IOTest<void, int, int> {
public:
    BasicVoidAndIntIntTest() : gcheck::IOTest<void, int, int>(gcheck::TestInfo("basic", "VoidAndIntInt", 4), 3, VoidAndIntInt) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(0, 2);
    }
};
BasicVoidAndIntIntTest basic_void_and_int_int_test;

class BasicIntAndIntIntTest : public gcheck::IOTest<int, int, int> {
public:
    BasicIntAndIntIntTest() : gcheck::IOTest<int, int, int>(gcheck::TestInfo("basic", "IntAndIntInt", 4), 3, IntAndIntInt) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(0, 2);
        this->SetReturn(0);
    }
};
BasicIntAndIntIntTest basic_int_and_int_int_test;

int IntAndEmpty2() {
    return 1;
}
void VoidAndIntInt2(int& a, int& b) {
    a = 3;
    b = 5;
}
int IntAndIntInt2(std::string, int b) {
    return b + 1;
}

class ValuesIntAndEmpty2Test : public gcheck::IOTest<int> {
public:
    ValuesIntAndEmpty2Test() : gcheck::IOTest<int>(gcheck::TestInfo("values", "IntAndEmpty2", 4), 3, IntAndEmpty2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetReturn(1);
    }
};
ValuesIntAndEmpty2Test values_int_and_empty_2_test;

class ValuesVoidAndIntInt2Test : public gcheck::IOTest<void, int&, int&> {
public:
    ValuesVoidAndIntInt2Test() : gcheck::IOTest<void, int&, int&>(gcheck::TestInfo("values", "VoidAndIntInt2", 4), 3, VoidAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(1, 2);
        this->SetArgumentsAfter(3, 5);
    }
};
ValuesVoidAndIntInt2Test values_void_and_int_int_2_test;

class ValuesIntAndIntInt2Test : public gcheck::IOTest<int, std::string, int> {
public:
    ValuesIntAndIntInt2Test() : gcheck::IOTest<int, std::string, int>(gcheck::TestInfo("values", "IntAndIntInt2", 4), 3, IntAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments("asd", static_cast<int>(this->GetRunIndex()) - 1);
        this->SetReturn(this->GetRunIndex());
    }
};
ValuesIntAndIntInt2Test values_int_and_int_int_2_test;

class ValuesVoidAndIntInt2FailTest : public gcheck::IOTest<void, int&, int&> {
public:
    ValuesVoidAndIntInt2FailTest() : gcheck::IOTest<void, int&, int&>(gcheck::TestInfo("values", "VoidAndIntInt2_fail", 4), 3, VoidAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(1, 2);
        this->SetArgumentsAfter(3, 4);
    }
};
ValuesVoidAndIntInt2FailTest values_void_and_int_int_2_fail_test;

class ValuesIntAndIntInt2FailTest : public gcheck::IOTest<int, std::string, int> {
public:
    ValuesIntAndIntInt2FailTest() : gcheck::IOTest<int, std::string, int>(gcheck::TestInfo("values", "IntAndIntInt2_fail", 4), 3, IntAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments("asd", static_cast<int>(this->GetRunIndex()));
        this->SetReturn(this->GetRunIndex());
    }
};
ValuesIntAndIntInt2FailTest values_int_and_int_int_2_fail_test;

int IntAndEmpty2AndWriteOut() {
    std::cout << "asd";
    return 1;
}
void VoidAndIntInt2AndWriteErr(int& a, int& b) {
    char c;
    std::cin >> c;
    std::cerr << c << "err";
    a = 3;
    b = 5;
}
int IntAndIntInt2AndWriteErrAndOut(std::string, int b) {
    std::string str;
    std::cin >> str;
    std::cout << str;
    std::cerr << str << "err";
    return b + 1;
}

class StdIntAndEmpty2AndWriteOutTest : public gcheck::IOTest<int> {
public:
    StdIntAndEmpty2AndWriteOutTest()
            : gcheck::IOTest<int>(gcheck::TestInfo("std", "IntAndEmpty2AndWriteOut", 4), 3, IntAndEmpty2AndWriteOut) {}

private:
    void SetInputsAndOutputs() override {
        this->SetReturn(1);
        this->SetOutput("asd");
    }
};
StdIntAndEmpty2AndWriteOutTest std_int_and_empty_2_and_write_out_test;

class StdVoidAndIntInt2AndWriteErrTest : public gcheck::IOTest<void, int&, int&> {
public:
    StdVoidAndIntInt2AndWriteErrTest()
            : gcheck::IOTest<void, int&, int&>(gcheck::TestInfo("std", "VoidAndIntInt2AndWriteErr", 4), 3, VoidAndIntInt2AndWriteErr) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(1, 2);
        this->SetArgumentsAfter(3, 5);
        this->SetInput("asd");
        this->SetError("aerr");
    }
};
StdVoidAndIntInt2AndWriteErrTest std_void_and_int_int_2_and_write_err_test;

class StdIntAndIntInt2AndWriteErrAndOutTest : public gcheck::IOTest<int, std::string, int> {
public:
    StdIntAndIntInt2AndWriteErrAndOutTest()
            : gcheck::IOTest<int, std::string, int>(gcheck::TestInfo("std", "IntAndIntInt2AndWriteErrAndOut", 4), 3, IntAndIntInt2AndWriteErrAndOut) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments("asd", static_cast<int>(this->GetRunIndex()) - 1);
        this->SetReturn(this->GetRunIndex());
        this->SetInput("asd", true);
        this->SetOutput("asd");
        this->SetError("asderr");
    }
};
StdIntAndIntInt2AndWriteErrAndOutTest std_int_and_int_int_2_and_write_err_and_out_test;

class StdVoidAndIntInt2AndWriteErrFailTest : public gcheck::IOTest<void, int&, int&> {
public:
    StdVoidAndIntInt2AndWriteErrFailTest()
            : gcheck::IOTest<void, int&, int&>(gcheck::TestInfo("std", "VoidAndIntInt2AndWriteErr_fail", 4), 3, VoidAndIntInt2AndWriteErr) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(1, 2);
        this->SetArgumentsAfter(3, 5);
        this->SetInput("asd");
        this->SetError("aer");
    }
};
StdVoidAndIntInt2AndWriteErrFailTest std_void_and_int_int_2_and_write_err_fail_test;

class StdIntAndIntInt2AndWriteErrAndOutFailTest : public gcheck::IOTest<int, std::string, int> {
public:
    StdIntAndIntInt2AndWriteErrAndOutFailTest()
            : gcheck::IOTest<int, std::string, int>(gcheck::TestInfo("std", "IntAndIntInt2AndWriteErrAndOut_fail", 4), 3, IntAndIntInt2AndWriteErrAndOut) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments("asd", static_cast<int>(this->GetRunIndex()) - 1);
        this->SetReturn(this->GetRunIndex());
        this->SetInput("asd", true);
        this->SetOutput("assd");
        this->SetError("asderr");
    }
};
StdIntAndIntInt2AndWriteErrAndOutFailTest std_int_and_int_int_2_and_write_err_and_out_fail_test;

} // anonymous namespace
