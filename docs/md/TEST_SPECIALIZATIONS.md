# gcheck Test Specializations Guide

This document explains:
- the basic operation of `gcheck::Test`
- what each test specialization provides
- how each specialization must be constructed and configured

## 1. Basic Operation of `Test`

`gcheck::Test` is the abstract root for all test types.

Core responsibilities:
- registration: each constructed test instance auto-registers itself in a global list
- execution wrapper: captures stdout/stderr around test execution
- report accounting: accepts report entries and updates `correct`/`incorrect`
- score calculation: computes points from grading method and report results

### Lifecycle

1. Define a derived test class (or use a specialization like `FunctionTest`).
2. Construct it with `TestInfo` (suite, test name, points, prerequisites).
3. Create a static instance so it is registered before `RunTests()`.
4. Runner calls `Test::RunTests()`.
5. For each eligible test (prerequisites fulfilled), framework calls `ActualTest()`.
6. Derived implementation adds `TestReport` entries.
7. Framework computes final points/status and serializes output.

### `TestInfo` construction options

`TestInfo` supports these forms:
- `TestInfo(suite, test, points, Prerequisite)`
- `TestInfo(suite, test, Prerequisite)`
- `TestInfo(suite, test, points, prerequisite_string)`
- `TestInfo(suite, test, prerequisite_string)`

Prerequisite string format:
- `"test_name"` means `(current_suite, test_name)`
- `"other_suite.test_name"` means explicit suite
- multiple prerequisites are whitespace-separated

### Safe-run mode

`Test::do_safe_run_` enables fork-based safe execution where supported.
- Linux: supported
- macOS/Windows: safe-run path throws (not supported)

## 2. Shared Report Model (important for all specializations)

`TestReport` payload variant can contain:
- `EqualsData`
- `TrueData`
- `FalseData`
- `CaseData`
- `FunctionData`

JSON `type` mapping (from serializer):
- `EE` -> `EqualsData`
- `ET` -> `TrueData`
- `EF` -> `FalseData`
- `TC` -> `CaseData`
- `FC` -> `FunctionData`

Practical implication:
- `FunctionTest`/`IOTest`/`MethodTest`/`MethodIOTest` produce `FunctionData` (`type = FC`)
- `CustomTest` can produce `EE/ET/EF` and `TC` (when using case-comparison helpers)

## 3. `CustomTest`

Header: `include/customtest.h`

### What it provides

- full manual control over test body via `TheTest()`
- assertion helpers and macros (`EXPECT_*`, `ASSERT_*`, `FAIL`)
- helper methods for case-style callable comparisons (`CompareWithCallable`, `CompareWithAnswer`)

### How to construct

- derive from `gcheck::CustomTest`
- call base constructor in initializer list:
  - `CustomTest(TestInfo("Suite", "Name", points))`
- implement `void TheTest() override`
- create a static instance

### Typical usage

- complex multi-step workflows not fitting function/method wrappers
- bespoke assertions with custom control flow
- case-table or generator-based comparisons via `CompareWithCallable`

## 4. `FunctionTest<ReturnT, Args...>`

Header: `include/function_test.h`

### What it provides

- multi-run execution of a callable `ReturnT(Args...)`
- per-run setup hook: `SetInputsAndOutputs()`
- validation of:
  - input arguments snapshot
  - post-call argument state (`SetArgumentsAfter`)
  - return value (`SetReturn`)
  - runtime (`run_time`) and optional limit (`SetMaxRunTime`)
  - timeout metadata (`SetTimeout`)
- per-run hooks: reset/pre/post

### Constructor

```cpp
FunctionTest(const TestInfo& info,
             int num_runs,
             const std::function<ReturnT(Args...)>& func)
```

### Required derived implementation

```cpp
void SetInputsAndOutputs() override;
```

This is called once per run and should set expectations for that run.

### Minimal pattern

```cpp
class MyFnCase : public gcheck::FunctionTest<int, int, int> {
public:
    MyFnCase()
      : FunctionTest(gcheck::TestInfo("Math", "Add"), 10, Add) {}
private:
    void SetInputsAndOutputs() override {
        SetArguments(2, 3);
        SetReturn(5);
    }
};
static MyFnCase my_fn_case;
```

## 5. `IOTest<ReturnT, Args...>` (Function + IO)

Header: `include/io_test.h`

### What it provides

`IOTest` is `FunctionTest` plus `IOPlugin`.
It adds stdin/stdout/stderr handling per run:
- `SetInput(...)`
- `SetOutput(...)`
- `SetError(...)`

It still supports all `FunctionTest` expectations (`SetArguments`, `SetReturn`, etc.).

### Constructor

```cpp
IOTest(const TestInfo& info,
       int num_runs,
       const std::function<ReturnT(Args...)>& func)
```

### Required derived implementation

```cpp
void SetInputsAndOutputs() override;
```

### Typical usage

- console-style functions that read stdin / write stdout or stderr
- validating both value semantics and textual stream behavior

## 6. `MethodTest<ReturnT, ObjectType, Args...>`

Header: `include/method_test.h`

### What it provides

`MethodTest` is `FunctionTest` plus `MethodPlugin<ObjectType>`.
It adapts method-like callables:
- input callable signature: `ReturnT(ObjectType*, Args...)`

Object-state features:
- `SetObject(...)` sets object under test
- `SetObjectAfter(...)` sets expected final object
- `SetStateComparer(...)` custom state-diff checker
- records object snapshot before/after call

### Constructor

```cpp
MethodTest(const TestInfo& info,
           int num_runs,
           const std::function<ReturnT(ObjectType*, Args...)>& func)
```

### Required derived implementation

```cpp
void SetInputsAndOutputs() override;
```

In each run, you generally must set object + arguments.

### Typical usage

- member-function behavior where object mutation is important
- asserting full object state transitions, not just return values

## 7. `MethodIOTest<ReturnT, ObjectType, Args...>`

Header: `include/method_io_test.h`

### What it provides

`MethodIOTest` combines:
- `FunctionTest`
- `MethodPlugin<ObjectType>`
- `IOPlugin`

So it supports method-state validation and stdin/stdout/stderr expectations together.

### Constructor

```cpp
MethodIOTest(const TestInfo& info,
             int num_runs,
             const std::function<ReturnT(ObjectType*, Args...)>& func)
```

### Required derived implementation

```cpp
void SetInputsAndOutputs() override;
```

In each run, configure:
- object (`SetObject`)
- method args (`SetArguments`)
- optional return/state checks (`SetReturn`, `SetObjectAfter`, `SetStateComparer`)
- IO checks (`SetInput`, `SetOutput`, `SetError`)

### Typical usage

- object methods that also interact with terminal IO
- integration-style checks where state and stream output must both match

## 8. Choosing the right specialization

Use this rule of thumb:
- only callable args/return/runtimes: `FunctionTest`
- callable + stdin/stdout/stderr: `IOTest`
- object method + object state: `MethodTest`
- object method + object state + IO: `MethodIOTest`
- custom control flow / ad-hoc assertions / mixed logic: `CustomTest`

## 9. Construction checklist (applies to all)

1. Choose specialization and derive a class.
2. Call base constructor with `TestInfo`, run count, and callable where required.
3. Implement required hook (`SetInputsAndOutputs()` or `TheTest()`).
4. Create a static instance to auto-register.
5. Ensure prerequisites reference valid suite/test names.
6. Run via `Test::RunTests()`.

