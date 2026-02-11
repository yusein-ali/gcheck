#include <gcheck.h>
#include <function_test.h>

void Void() {}

namespace {

class ShouldBeCalledAfterFirstAndAfterFirst : public gcheck::FunctionTest<void> {
public:
    ShouldBeCalledAfterFirstAndAfterFirst()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldbecalled", "after_first_and_after_first", 1, "first after_first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldBeCalledAfterFirstAndAfterFirst should_be_called_after_first_and_after_first;

class ShouldBeCalled2AfterFirstAndAfterFirst : public gcheck::FunctionTest<void> {
public:
    ShouldBeCalled2AfterFirstAndAfterFirst()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldbecalled2", "after_first_and_after_first", 1, "shouldbecalled.first shouldbecalled.after_first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldBeCalled2AfterFirstAndAfterFirst should_be_called_2_after_first_and_after_first;

class ShouldBeCalledAfterFirst : public gcheck::FunctionTest<void> {
public:
    ShouldBeCalledAfterFirst()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldbecalled", "after_first", 1, "first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldBeCalledAfterFirst should_be_called_after_first;

class ShouldBeCalledAfterFirst2 : public gcheck::FunctionTest<void> {
public:
    ShouldBeCalledAfterFirst2()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldbecalled", "after_first2", 1, "shouldbecalled.first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldBeCalledAfterFirst2 should_be_called_after_first_2;

class ShouldBeCalledFirst : public gcheck::FunctionTest<void> {
public:
    ShouldBeCalledFirst() : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldbecalled", "first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldBeCalledFirst should_be_called_first;

class ShouldBeCalled2AfterFirst : public gcheck::FunctionTest<void> {
public:
    ShouldBeCalled2AfterFirst()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldbecalled2", "after_first", 1, "shouldbecalled.first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldBeCalled2AfterFirst should_be_called_2_after_first;

class ShouldNotBeCalledAfterFirst : public gcheck::FunctionTest<void> {
public:
    ShouldNotBeCalledAfterFirst()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldntbecalled", "after_first", 1, "first"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldNotBeCalledAfterFirst should_not_be_called_after_first;

class ShouldNotBeCalledRandom : public gcheck::FunctionTest<void> {
public:
    ShouldNotBeCalledRandom()
            : gcheck::FunctionTest<void>(gcheck::TestInfo("shouldntbecalled", "random", 1, "asd"), 1, Void) {}

private:
    void SetInputsAndOutputs() override {}
};
ShouldNotBeCalledRandom should_not_be_called_random;

} // anonymous namespace
