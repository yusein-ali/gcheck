#pragma once

/**
 * @file shared_allocator.h
 * @brief Shared-memory manager and STL-compatible allocator used for fork-safe data transfer.
 *
 * This module is used by `RunForked(...)` (`include/multiprocessing.h`) to move
 * test result data from child process to parent process on Linux:
 *
 * 1. Parent creates `shared_manager sm`, assigns `shared_manager::manager = &sm`, and allocates shared region via `Realloc(...)`.
 * 2. Child allocates objects inside that region using `shared_allocator<T>`.
 * 3. Child constructs result object in shared memory and exits.
 * 4. Parent reads shared object through `sm.Memory()` and copies into normal allocator-backed data.
 *
 * Platform notes:
 * - Linux path uses `mmap(..., MAP_SHARED | MAP_ANONYMOUS, ...)`.
 * - Non-Linux build keeps manager APIs as no-op stubs (implemented in `src/shared_allocator.cpp`).
 */

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

/**
 * @brief Manages one shared-memory region and a simple free-list allocator state.
 *
 * `shared_manager` owns two mappings on Linux:
 * 1. A tiny mapping that stores a shared pointer slot (`memory_`), and
 * 2. The actual shared data block referenced by that slot (`*memory_`).
 *
 * Allocation strategy:
 * - First-fit search over `free_` blocks.
 * - No block coalescing on deallocation.
 *
 * Typical usage:
 * - Create stack instance in parent process before fork.
 * - Set `shared_manager::manager` to that instance.
 * - Use through @ref shared_allocator in child and parent.
 */
class shared_manager {
public:
    /**
     * @brief Active manager pointer used by @ref shared_allocator.
     *
     * This must point to a valid manager before allocator operations.
     */
    static shared_manager* manager;

    /**
     * @brief Construct manager and optionally allocate initial shared region.
     *
     * @param size Initial shared-memory size in bytes. `0` means deferred allocation.
     */
    shared_manager(size_t size = 0);

    /**
     * @brief Destructor; releases mappings and clears static manager pointer when matching.
     */
    ~shared_manager();

    /**
     * @brief Allocate a byte block from internal free list.
     *
     * @param n Requested size in bytes.
     * @param Unused placement hint.
     * @return Pointer to allocated block, or `nullptr` if no suitable free block exists.
     */
    void* allocate(size_t n, const void * = 0);

    /**
     * @brief Return a previously allocated block to free list.
     *
     * @param p Block pointer.
     * @param n Block size in bytes.
     */
    void deallocate(void* p, size_t n);

    /**
     * @brief Allocate/reallocate underlying shared memory region.
     *
     * @param n Requested region size in bytes.
     *
     * Current implementation expects no active region and throws if one exists.
     */
    void Realloc(size_t n);

    /**
     * @brief Unmap the data region while keeping manager metadata mapping alive.
     */
    void FreeMemory();

    /**
     * @brief Fully release all mappings managed by this instance.
     */
    void Free();

    /**
     * @brief Access pointer to beginning of active shared region.
     *
     * @return Shared memory base pointer (or `nullptr` when uninitialized).
     */
    void* Memory() { return *memory_;}
private:
    /** @brief Mapping that stores the shared data pointer slot. */
    void** memory_ = nullptr;
#if defined(__linux__)
    /** @brief Size (bytes) of currently mapped shared data region. */
    size_t size_ = 0;
#endif
    /** @brief Free-list blocks as `(ptr, size_bytes)`. */
    std::vector<std::pair<void*, size_t>> free_;
};

#if !defined(_WIN32) && !defined(WIN32)
/**
 * @brief STL-style allocator that allocates from @ref shared_manager shared region.
 *
 * @tparam T Allocated object type.
 *
 * This allocator enables constructing allocator-aware containers/objects inside
 * shared memory so child and parent processes can access the same bytes.
 *
 * Typical usage (internal):
 * - `shared_allocator<T<shared_allocator>>` in `RunForked(...)`.
 */
template <class T>
class shared_allocator {
public:
    /** @brief Standard allocator size type alias. */
    typedef size_t size_type;
    /** @brief Standard allocator difference type alias. */
    typedef ptrdiff_t difference_type;
    /** @brief Pointer to allocated value type. */
    typedef T* pointer;
    /** @brief Const pointer to allocated value type. */
    typedef const T* const_pointer;
    /** @brief Reference to allocated value type. */
    typedef T& reference;
    /** @brief Const reference to allocated value type. */
    typedef const T& const_reference;
    /** @brief Value type alias. */
    typedef T value_type;

    /** @brief Default constructor. */
    shared_allocator() {}
    /** @brief Copy constructor. */
    shared_allocator(const shared_allocator&) {}

    /**
     * @brief Allocate storage for `n` objects of type `T` from shared memory.
     *
     * @param n Number of objects.
     * @param Unused placement hint.
     * @return Pointer to allocated storage or `nullptr`.
     */
    pointer allocate(size_type n, const void * = 0) {
        n = n*sizeof(T);
        return (pointer)shared_manager::manager->allocate(n);
    }

    /**
     * @brief Deallocate storage for `n` objects of type `T`.
     *
     * @param p Block pointer.
     * @param n Number of objects in block.
     */
    void deallocate(void* p, size_type n) {
        n = n*sizeof(T);
        shared_manager::manager->deallocate(p, n);
    }

    /** @brief Address helper for allocator compatibility. */
    pointer address(reference x) const { return &x; }
    /** @brief Const-address helper for allocator compatibility. */
    const_pointer address(const_reference x) const { return &x; }
    /** @brief Copy assignment (stateless allocator semantics). */
    shared_allocator<T>& operator=(const shared_allocator&) { return *this; }

    /**
     * @brief Placement-construct object at allocated address.
     *
     * @param p Target address.
     * @param val Value to copy-construct.
     */
    void construct(pointer p, const T& val) { new ((T*) p) T(val); }
    /**
     * @brief Explicitly destroy object at address.
     *
     * @param p Object pointer.
     */
    void destroy(pointer p) { p->~T(); }

    /** @brief Maximum allocatable element count (theoretical). */
    size_type max_size() const { return size_t(-1); }

    /**
     * @brief Standard allocator rebind helper.
     *
     * @tparam U Rebound value type.
     */
    template <class U>
    struct rebind { typedef shared_allocator<U> other; };

    /** @brief Cross-type allocator copy constructor. */
    template <class U>
    shared_allocator(const shared_allocator<U>&) {}

    /** @brief Cross-type allocator assignment. */
    template <class U>
    shared_allocator& operator=(const shared_allocator<U>&) { return *this; }

    /** @brief Stateless allocator equality (always true). */
    bool operator==(const shared_allocator&) { return true; }
    /** @brief Stateless allocator inequality (always false). */
    bool operator!=(const shared_allocator&) { return false; }
};
#endif

} // gcheck
