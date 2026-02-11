#pragma once

/**
 * @file ptr_tools.h
 * @brief Pointer-oriented helper types and adapter utilities.
 *
 * This header contains lightweight helpers for tests that need to compare
 * pointer identity/value together or adapt pointer-returning functions into
 * value/container-returning callables.
 *
 * Typical usage scenarios:
 * 1. Wrapping legacy C APIs that return `T*` so tests can compare vectors/values.
 * 2. Preserving pointer address alongside pointee value for aliasing checks.
 * 3. Representing array-layout expectations with @ref ArrayType.
 */

#include <functional>
namespace gcheck {

/**
 * @brief Describes how pointer-based sequence outputs are interpreted.
 *
 * Typical usage:
 * 1. `NULLTerminated`: stop when sentinel/null marker is reached.
 * 2. `ConstantSize`: size is known/provided externally.
 * 3. `LinkedList`: follow next pointers instead of contiguous indexing.
 */
enum ArrayType {
    NULLTerminated,
    ConstantSize,
    LinkedList
};

/**
 * @brief Pair-like wrapper that keeps a pointer and compares both address and value.
 *
 * @tparam T Pointee type.
 *
 * Purpose:
 * - Useful when a test should assert not only pointee value equality but also
 *   that a function returned the exact expected address.
 *
 * Typical usage:
 * 1. Compare two pointer results for aliasing-sensitive APIs.
 * 2. Validate in-place mutation functions that must return original storage.
 */
template<typename T>
struct ValueAndAddress {
    /** @brief Pointer to compared object. */
    T* ptr;
    /**
     * @brief Compare with raw pointer by address and dereferenced value.
     *
     * @param r Raw pointer to compare against.
     * @return `true` if addresses match and `*ptr == *r`.
     */
    bool operator==(const T* r) const {
        return ptr == r && *ptr == *r;
    }
    /**
     * @brief Compare with another wrapper by delegating to raw-pointer overload.
     *
     * @param r Other wrapper.
     * @return `true` when wrapped pointers and pointed values match.
     */
    bool operator==(const ValueAndAddress& r) const {
        return r == ptr;
    }
};

/**
 * @brief Adapt a C-array-returning function into a vector-returning function.
 *
 * @tparam T Element type.
 * @tparam Args Function argument types.
 * @param func Source callable returning `T*`.
 * @param size_func Callable that computes number of elements from returned pointer.
 * @return Callable returning `std::vector<T>`.
 *
 * Typical usage:
 * 1. Wrap C APIs that return raw contiguous buffers.
 * 2. Convert pointer outputs into value containers for easier test assertions.
 */
template<typename T, typename... Args>
std::function<std::vector<T>(Args...)> CArray(std::function<T*(Args...)> func, std::function<size_t(T*)> size_func) {
    return [type](Args... args) {
            T* ptr = func(args...);
            size_t size = size_func(ptr);
            return std::vector(ptr, ptr + size);
        };
}

/**
 * @brief Adapt a pointer-returning function into one returning pointed value.
 *
 * @tparam T Value type.
 * @tparam Args Function argument types.
 * @param func Source callable returning `T*`.
 * @return Callable returning `T` by dereferencing pointer result.
 *
 * Typical usage:
 * 1. Simplify assertions when pointer identity is irrelevant and only value matters.
 */
template<typename T, typename... Args>
std::function<T(Args...)> PointerValue(std::function<T*(Args...)> func) {
    return [type](Args... args) {
            return *func(args...);
        };
}

/**
 * @brief Adapt a pointer-returning function into one returning address+value wrapper.
 *
 * @tparam T Value type.
 * @tparam Args Function argument types.
 * @param func Source callable returning `T*`.
 * @return Callable returning @ref ValueAndAddress.
 *
 * Typical usage:
 * 1. Assertions that require both pointer identity and pointee equality.
 */
template<typename T, typename... Args>
std::function<ValueAndAddress<T>(Args...)> PointerValueAndAddress(std::function<T*(Args...)> func) {
    return [type](Args... args) {
            return *func(args...);
        };
}

} // gcheck
