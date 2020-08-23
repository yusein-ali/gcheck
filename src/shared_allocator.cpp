#include "shared_allocator.h"

namespace gcheck {

shared_manager* shared_manager::manager = nullptr;

shared_manager::shared_manager(size_t size) {
    memory_ = (void**)mmap(NULL, sizeof(void*), PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *memory_ = nullptr;

    free_.emplace_back(nullptr, 0);

    if(size != 0)
        Realloc(size);
}
shared_manager::~shared_manager() {
    Free();
    if(manager == this) {
        manager = nullptr;
    }
}

void* shared_manager::allocate(size_t n, const void *) {
    auto it = std::find_if(free_.begin(), free_.end(), [n](auto& a){ return a.second >= n; });
    if(it == free_.end()) {
        return nullptr;
    }
    void* ptr = it->first;
    if(n == it->second) {
        free_.erase(it);
    } else {
        *it = std::pair<void*, size_t>((uint8_t*)it->first+n, it->second-n);
    }
    return ptr;
}

void shared_manager::deallocate(void* p, size_t n) {
    if (!p) return;

    free_.emplace_back(p, n);
}

void shared_manager::Realloc(size_t n) {
    if(*memory_)
        throw std::exception(); // not implemented
    else {
        *memory_ = mmap(NULL, n, PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        free_.clear();
        free_.emplace_back(*memory_, n);
    }
    size_ = n;
}
void shared_manager::FreeMemory() {
    if(!memory_) return;

    if(*memory_)
        munmap(*memory_, size_);

    size_ = 0;
    *memory_ = nullptr;
}
void shared_manager::Free() {
    if(memory_) {
        FreeMemory();
        munmap(memory_, sizeof(void*));
    }
    memory_ = nullptr;
}

} // gcheck