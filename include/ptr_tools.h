#pragma once

#include <functional>
namespace gcheck {

enum ArrayType {
    NULLTerminated,
    ConstantSize,
    LinkedList
};

template<typename T>
struct ValueAndAddress {
    T* ptr;
    bool operator==(const T* r) const {
        return ptr == r && *ptr == *r;
    }
    bool operator==(const ValueAndAddress& r) const {
        return r == ptr;
    }
};

// Turns a C-array returning function into a vector returning one
template<typename T, typename... Args>
std::function<std::vector<T>(Args...)> CArray(std::function<T*(Args...)> func, std::function<size_t(T*)> size_func) {
    return [type](Args... args) {
            T* ptr = func(args...);
            size_t size = size_func(ptr);
            return std::vector(ptr, ptr + size);
        };
}

template<typename T, typename... Args>
std::function<T(Args...)> PointerValue(std::function<T*(Args...)> func) {
    return [type](Args... args) {
            return *func(args...);
        };
}

template<typename T, typename... Args>
std::function<ValueAndAddress<T>(Args...)> PointerValueAndAddress(std::function<T*(Args...)> func) {
    return [type](Args... args) {
            return *func(args...);
        };
}

} // gcheck