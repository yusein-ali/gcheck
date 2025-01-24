#pragma once

#include <vector>
#include <cstdlib>
#include <utility>

template<typename T>
class Deleter {
public:
    Deleter() {}
    ~Deleter() { Clear(); }
    template<typename... Args>
    T* New(Args&&... args) {
        auto ptr = new T(std::forward<Args>(args)...);
        Add(ptr);
        return ptr;
    }
    void Add(T* ptr) { pointers_.push_back(ptr); }
    void Clear() {
        for(auto ptr : pointers_)
            delete ptr;
        pointers_.clear();
    }
private:
    std::vector<T*> pointers_;
};

template<typename T>
class Deleter<T[]> {
public:
    Deleter() {}
    ~Deleter() {
        for(auto ptr : pointers_)
            delete[] ptr;
    }
    T* New(size_t n) {
        auto ptr = new T[n];
        Add(ptr);
        return ptr;
    }
    void Add(T* ptr) {
        pointers_.push_back(ptr);
    }
private:
    std::vector<T*> pointers_;
};

template<typename T>
class Freer {
public:
    Freer() {}
    ~Freer() { Clear(); }
    T* New(size_t n) {
        auto ptr = (T*)malloc(sizeof(T)*n);
        Add(ptr);
        return ptr;
    }
    void Add(T* ptr) { pointers_.push_back(ptr); }
    void Clear() {
        for(auto ptr : pointers_)
            free(ptr);
        pointers_.clear();
    }
private:
    std::vector<T*> pointers_;
};
