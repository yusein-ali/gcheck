#pragma once

#include <chrono>

#include "shared_allocator.h"

namespace gcheck {

#if !defined(_WIN32) && !defined(WIN32)
bool wait_timeout(pid_t pid, std::chrono::duration<double> time) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        return false;
    }

    auto secs = std::chrono::floor<std::chrono::seconds>(time);
    auto nsecs = std::chrono::ceil<std::chrono::nanoseconds>(time-secs);

    struct timespec timeout;
    timeout.tv_sec = secs.count();
    timeout.tv_nsec = nsecs.count();

    while(sigtimedwait(&mask, NULL, &timeout) < 0) {
        if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN) {
            kill(pid, SIGKILL); // TODO: not working in WSL. test in linux
            return false;
        }
        break;
    }
    return true;
}
#endif

template<template<template<typename...> class> class T, typename F, typename... Args>
bool RunForked(std::chrono::duration<double> timeout, T<std::allocator>& data_out, size_t mem_size, F&& function, Args&&... args) {
    shared_manager sm;
    shared_manager::manager = &sm;
    sm.Realloc(mem_size);

    pid_t pid = fork();
    if(pid == 0) {
        auto allocator = shared_allocator<T<shared_allocator>>();
        auto data = allocator.allocate(1);

        function(std::forward<Args>(args)...);

        allocator.construct(data, data_out);
        exit(0);
    } else if(timeout != timeout.zero()) {
        if(!wait_timeout(pid, timeout)) {
            sm.Free();
            return false;
        }
    } else {
        waitpid(pid, NULL, WUNTRACED);
    }
    data_out = *(T<shared_allocator>*)sm.Memory();
    sm.Free();
    return true;
}

} // gcheck