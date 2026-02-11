#include <gcheck.h>
#include <function_test.h>

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

class BasicVoidAndEmptyTest : public gcheck::FunctionTest<void> {
public:
    BasicVoidAndEmptyTest() : gcheck::FunctionTest<void>(gcheck::TestInfo("basic", "VoidAndEmpty"), 3, VoidAndEmpty) {}

private:
    void SetInputsAndOutputs() override {}
};
BasicVoidAndEmptyTest basic_void_and_empty_test;

class BasicIntAndEmptyTest : public gcheck::FunctionTest<int> {
public:
    BasicIntAndEmptyTest() : gcheck::FunctionTest<int>(gcheck::TestInfo("basic", "IntAndEmpty", 3), 2, IntAndEmpty) {}

private:
    void SetInputsAndOutputs() override {
        this->SetReturn(0);
    }
};
BasicIntAndEmptyTest basic_int_and_empty_test;

class BasicVoidAndIntIntTest : public gcheck::FunctionTest<void, int, int> {
public:
    BasicVoidAndIntIntTest() : gcheck::FunctionTest<void, int, int>(gcheck::TestInfo("basic", "VoidAndIntInt", 4), 4, VoidAndIntInt) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(0, 2);
    }
};
BasicVoidAndIntIntTest basic_void_and_int_int_test;

class BasicIntAndIntIntTest : public gcheck::FunctionTest<int, int, int> {
public:
    BasicIntAndIntIntTest() : gcheck::FunctionTest<int, int, int>(gcheck::TestInfo("basic", "IntAndIntInt", 5), 3, IntAndIntInt) {}

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
int IntAndIntInt2(int, int b) {
    return b + 1;
}

class ValuesIntAndEmpty2Test : public gcheck::FunctionTest<int> {
public:
    ValuesIntAndEmpty2Test() : gcheck::FunctionTest<int>(gcheck::TestInfo("values", "IntAndEmpty2", 4), 3, IntAndEmpty2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetReturn(1);
    }
};
ValuesIntAndEmpty2Test values_int_and_empty_2_test;

class ValuesVoidAndIntInt2Test : public gcheck::FunctionTest<void, int&, int&> {
public:
    ValuesVoidAndIntInt2Test() : gcheck::FunctionTest<void, int&, int&>(gcheck::TestInfo("values", "VoidAndIntInt2", 4), 3, VoidAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(1, 2);
        this->SetArgumentsAfter(3, 5);
    }
};
ValuesVoidAndIntInt2Test values_void_and_int_int_2_test;

class ValuesIntAndIntInt2Test : public gcheck::FunctionTest<int, int, int> {
public:
    ValuesIntAndIntInt2Test() : gcheck::FunctionTest<int, int, int>(gcheck::TestInfo("values", "IntAndIntInt2", 4), 3, IntAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(2, static_cast<int>(this->GetRunIndex()) - 1);
        this->SetReturn(this->GetRunIndex());
    }
};
ValuesIntAndIntInt2Test values_int_and_int_int_2_test;

class ValuesVoidAndIntInt2FailTest : public gcheck::FunctionTest<void, int&, int&> {
public:
    ValuesVoidAndIntInt2FailTest() : gcheck::FunctionTest<void, int&, int&>(gcheck::TestInfo("values", "VoidAndIntInt2_fail", 4), 3, VoidAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(1, 2);
        this->SetArgumentsAfter(3, 4);
    }
};
ValuesVoidAndIntInt2FailTest values_void_and_int_int_2_fail_test;

class ValuesIntAndIntInt2FailTest : public gcheck::FunctionTest<int, int, int> {
public:
    ValuesIntAndIntInt2FailTest() : gcheck::FunctionTest<int, int, int>(gcheck::TestInfo("values", "IntAndIntInt2_fail", 4), 3, IntAndIntInt2) {}

private:
    void SetInputsAndOutputs() override {
        this->SetArguments(2, static_cast<int>(this->GetRunIndex()));
        this->SetReturn(this->GetRunIndex());
    }
};
ValuesIntAndIntInt2FailTest values_int_and_int_int_2_fail_test;

} // anonymous namespace
