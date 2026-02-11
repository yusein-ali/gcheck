#pragma once

#include <type_traits>

#include "customtest.h"
#include "function_test.h"
#include "io_test.h"
#include "method_io_test.h"
#include "method_test.h"

namespace gcheck::compat {

template <typename T>
struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T>
struct function_pointer_traits;

template <typename R, typename... Args>
struct function_pointer_traits<R(*)(Args...)> {
    using function_test_type = FunctionTest<R, Args...>;
    using io_test_type = IOTest<R, Args...>;
};

template <typename T>
struct method_pointer_traits;

template <typename R, typename Obj, typename... Args>
struct method_pointer_traits<R(*)(Obj*, Args...)> {
    using method_test_type = MethodTest<R, Obj, Args...>;
    using method_io_test_type = MethodIOTest<R, Obj, Args...>;
};

} // namespace gcheck::compat

#define GCHECK_COMPAT_CAT_I(a, b) a##b
#define GCHECK_COMPAT_CAT(a, b) GCHECK_COMPAT_CAT_I(a, b)

#define GCHECK_COMPAT_ARG_N(_1, _2, _3, _4, _5, _6, N, ...) N
#define GCHECK_COMPAT_RSEQ_N() 6, 5, 4, 3, 2, 1, 0
#define GCHECK_COMPAT_NARG_(...) GCHECK_COMPAT_ARG_N(__VA_ARGS__)
#define GCHECK_COMPAT_NARG(...) GCHECK_COMPAT_NARG_(__VA_ARGS__, GCHECK_COMPAT_RSEQ_N())
#define GCHECK_COMPAT_DISPATCH(name, ...) GCHECK_COMPAT_CAT(name, GCHECK_COMPAT_NARG(__VA_ARGS__))(__VA_ARGS__)

#define GCHECK_TEST_CLASS(suite, name) GCHECK_COMPAT_CAT(gcheck_test_, GCHECK_COMPAT_CAT(suite, GCHECK_COMPAT_CAT(_, name)))
#define GCHECK_FUNCTIONTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(gcheck_function_test_, GCHECK_COMPAT_CAT(suite, GCHECK_COMPAT_CAT(_, name)))
#define GCHECK_IOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(gcheck_io_test_, GCHECK_COMPAT_CAT(suite, GCHECK_COMPAT_CAT(_, name)))
#define GCHECK_METHODTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(gcheck_method_test_, GCHECK_COMPAT_CAT(suite, GCHECK_COMPAT_CAT(_, name)))
#define GCHECK_METHODIOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(gcheck_method_io_test_, GCHECK_COMPAT_CAT(suite, GCHECK_COMPAT_CAT(_, name)))

#define GCHECK_TESTINFO_2(suite, name) gcheck::TestInfo(#suite, #name)
#define GCHECK_TESTINFO_3(suite, name, a1) gcheck::TestInfo(#suite, #name, a1)
#define GCHECK_TESTINFO_4(suite, name, a1, a2) gcheck::TestInfo(#suite, #name, a1, a2)

#define TEST(...) GCHECK_COMPAT_DISPATCH(GCHECK_TEST_, __VA_ARGS__)

#define GCHECK_TEST_2(suite, name) \
class GCHECK_TEST_CLASS(suite, name) : public gcheck::CustomTest { \
public: \
    GCHECK_TEST_CLASS(suite, name)() : gcheck::CustomTest(gcheck::TestInfo(#suite, #name)) {} \
private: \
    void TheTest() override; \
}; \
static GCHECK_TEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_TEST_CLASS(suite, name), _instance); \
void GCHECK_TEST_CLASS(suite, name)::TheTest()

#define GCHECK_TEST_3(suite, name, a1) \
class GCHECK_TEST_CLASS(suite, name) : public gcheck::CustomTest { \
public: \
    GCHECK_TEST_CLASS(suite, name)() : gcheck::CustomTest(gcheck::TestInfo(#suite, #name, a1)) {} \
private: \
    void TheTest() override; \
}; \
static GCHECK_TEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_TEST_CLASS(suite, name), _instance); \
void GCHECK_TEST_CLASS(suite, name)::TheTest()

#define GCHECK_TEST_4(suite, name, a1, a2) \
class GCHECK_TEST_CLASS(suite, name) : public gcheck::CustomTest { \
public: \
    GCHECK_TEST_CLASS(suite, name)() : gcheck::CustomTest(gcheck::TestInfo(#suite, #name, a1, a2)) {} \
private: \
    void TheTest() override; \
}; \
static GCHECK_TEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_TEST_CLASS(suite, name), _instance); \
void GCHECK_TEST_CLASS(suite, name)::TheTest()

#define FUNCTIONTEST(...) GCHECK_COMPAT_DISPATCH(GCHECK_FUNCTIONTEST_, __VA_ARGS__)
#define IOTEST(...) GCHECK_COMPAT_DISPATCH(GCHECK_IOTEST_, __VA_ARGS__)
#define METHODTEST(...) GCHECK_COMPAT_DISPATCH(GCHECK_METHODTEST_, __VA_ARGS__)
#define METHODIOTEST(...) GCHECK_COMPAT_DISPATCH(GCHECK_METHODIOTEST_, __VA_ARGS__)

#define GCHECK_FUNCTIONTEST_4(suite, name, num_runs, func) \
class GCHECK_FUNCTIONTEST_CLASS(suite, name) : public gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::function_test_type { \
public: \
    GCHECK_FUNCTIONTEST_CLASS(suite, name)() : gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::function_test_type(gcheck::TestInfo(#suite, #name), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_FUNCTIONTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_FUNCTIONTEST_CLASS(suite, name), _instance); \
void GCHECK_FUNCTIONTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_FUNCTIONTEST_5(suite, name, num_runs, func, a1) \
class GCHECK_FUNCTIONTEST_CLASS(suite, name) : public gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::function_test_type { \
public: \
    GCHECK_FUNCTIONTEST_CLASS(suite, name)() : gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::function_test_type(gcheck::TestInfo(#suite, #name, a1), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_FUNCTIONTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_FUNCTIONTEST_CLASS(suite, name), _instance); \
void GCHECK_FUNCTIONTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_FUNCTIONTEST_6(suite, name, num_runs, func, a1, a2) \
class GCHECK_FUNCTIONTEST_CLASS(suite, name) : public gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::function_test_type { \
public: \
    GCHECK_FUNCTIONTEST_CLASS(suite, name)() : gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::function_test_type(gcheck::TestInfo(#suite, #name, a1, a2), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_FUNCTIONTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_FUNCTIONTEST_CLASS(suite, name), _instance); \
void GCHECK_FUNCTIONTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_IOTEST_4(suite, name, num_runs, func) \
class GCHECK_IOTEST_CLASS(suite, name) : public gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::io_test_type { \
public: \
    GCHECK_IOTEST_CLASS(suite, name)() : gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::io_test_type(gcheck::TestInfo(#suite, #name), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_IOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_IOTEST_CLASS(suite, name), _instance); \
void GCHECK_IOTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_IOTEST_5(suite, name, num_runs, func, a1) \
class GCHECK_IOTEST_CLASS(suite, name) : public gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::io_test_type { \
public: \
    GCHECK_IOTEST_CLASS(suite, name)() : gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::io_test_type(gcheck::TestInfo(#suite, #name, a1), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_IOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_IOTEST_CLASS(suite, name), _instance); \
void GCHECK_IOTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_IOTEST_6(suite, name, num_runs, func, a1, a2) \
class GCHECK_IOTEST_CLASS(suite, name) : public gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::io_test_type { \
public: \
    GCHECK_IOTEST_CLASS(suite, name)() : gcheck::compat::function_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::io_test_type(gcheck::TestInfo(#suite, #name, a1, a2), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_IOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_IOTEST_CLASS(suite, name), _instance); \
void GCHECK_IOTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_METHODTEST_4(suite, name, num_runs, func) \
class GCHECK_METHODTEST_CLASS(suite, name) : public gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_test_type { \
public: \
    GCHECK_METHODTEST_CLASS(suite, name)() : gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_test_type(gcheck::TestInfo(#suite, #name), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_METHODTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_METHODTEST_CLASS(suite, name), _instance); \
void GCHECK_METHODTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_METHODTEST_5(suite, name, num_runs, func, a1) \
class GCHECK_METHODTEST_CLASS(suite, name) : public gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_test_type { \
public: \
    GCHECK_METHODTEST_CLASS(suite, name)() : gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_test_type(gcheck::TestInfo(#suite, #name, a1), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_METHODTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_METHODTEST_CLASS(suite, name), _instance); \
void GCHECK_METHODTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_METHODTEST_6(suite, name, num_runs, func, a1, a2) \
class GCHECK_METHODTEST_CLASS(suite, name) : public gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_test_type { \
public: \
    GCHECK_METHODTEST_CLASS(suite, name)() : gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_test_type(gcheck::TestInfo(#suite, #name, a1, a2), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_METHODTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_METHODTEST_CLASS(suite, name), _instance); \
void GCHECK_METHODTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_METHODIOTEST_4(suite, name, num_runs, func) \
class GCHECK_METHODIOTEST_CLASS(suite, name) : public gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_io_test_type { \
public: \
    GCHECK_METHODIOTEST_CLASS(suite, name)() : gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_io_test_type(gcheck::TestInfo(#suite, #name), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_METHODIOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_METHODIOTEST_CLASS(suite, name), _instance); \
void GCHECK_METHODIOTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_METHODIOTEST_5(suite, name, num_runs, func, a1) \
class GCHECK_METHODIOTEST_CLASS(suite, name) : public gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_io_test_type { \
public: \
    GCHECK_METHODIOTEST_CLASS(suite, name)() : gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_io_test_type(gcheck::TestInfo(#suite, #name, a1), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_METHODIOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_METHODIOTEST_CLASS(suite, name), _instance); \
void GCHECK_METHODIOTEST_CLASS(suite, name)::SetInputsAndOutputs()

#define GCHECK_METHODIOTEST_6(suite, name, num_runs, func, a1, a2) \
class GCHECK_METHODIOTEST_CLASS(suite, name) : public gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_io_test_type { \
public: \
    GCHECK_METHODIOTEST_CLASS(suite, name)() : gcheck::compat::method_pointer_traits<gcheck::compat::remove_cvref_t<decltype(+func)>>::method_io_test_type(gcheck::TestInfo(#suite, #name, a1, a2), num_runs, func) {} \
private: \
    void SetInputsAndOutputs() override; \
}; \
static GCHECK_METHODIOTEST_CLASS(suite, name) GCHECK_COMPAT_CAT(GCHECK_METHODIOTEST_CLASS(suite, name), _instance); \
void GCHECK_METHODIOTEST_CLASS(suite, name)::SetInputsAndOutputs()
