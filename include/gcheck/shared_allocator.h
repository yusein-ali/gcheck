#pragma once

#include <vector>
#include <algorithm>
#include <tuple>
#if !defined(_WIN32) && !defined(WIN32)
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/mman.h>
    #include <cstring>
#endif

namespace gcheck {

class shared_manager {
public:
    static shared_manager* manager;

    shared_manager(size_t size = 0);
    ~shared_manager();

    void* allocate(size_t n, const void * = 0);
    void deallocate(void* p, size_t n);

    void Realloc(size_t n);
    void FreeMemory();
    void Free();

    void* Memory() { return *memory_;}
private:
    void** memory_ = nullptr;
    size_t size_ = 0;
    std::vector<std::pair<void*, size_t>> free_;
};

#if !defined(_WIN32) && !defined(WIN32)
template <class T>
class shared_allocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    shared_allocator() {}
    shared_allocator(const shared_allocator&) {}

    pointer allocate(size_type n, const void * = 0) {
        n = n*sizeof(T);
        return (pointer)shared_manager::manager->allocate(n);
    }

    void deallocate(void* p, size_type n) {
        n = n*sizeof(T);
        shared_manager::manager->deallocate(p, n);
    }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }
    shared_allocator<T>& operator=(const shared_allocator&) { return *this; }

    void construct(pointer p, const T& val) { new ((T*) p) T(val); }
    void destroy(pointer p) { p->~T(); }

    size_type max_size() const { return size_t(-1); }

    template <class U>
    struct rebind { typedef shared_allocator<U> other; };

    template <class U>
    shared_allocator(const shared_allocator<U>&) {}

    template <class U>
    shared_allocator& operator=(const shared_allocator<U>&) { return *this; }

    bool operator==(const shared_allocator&) { return true; }
    bool operator!=(const shared_allocator&) { return false; }
};
#endif

} // gcheck