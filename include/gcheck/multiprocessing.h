#pragma once

#include <chrono>
#include <iostream>
#include "shared_allocator.h"

namespace gcheck {

enum ForkStatus : unsigned int {
    OK,
    TIMEDOUT,
    ERROR
};

#if defined(__linux__)
ForkStatus wait_timeout(pid_t pid, std::chrono::duration<double> time);

template<template<template<typename...> class> class T, typename F, typename... Args>
ForkStatus RunForked(std::chrono::duration<double> timeout, T<std::allocator>& data_out, size_t mem_size, F&& function, Args&&... args) {
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
        ForkStatus status = wait_timeout(pid, timeout);
        if(status == TIMEDOUT || status == ERROR) {
            return status;
        }
    } else {
        int status;
        waitpid(pid, &status, WUNTRACED);
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            return ERROR;
    }
    (void)timeout;

    data_out = *(T<shared_allocator>*)sm.Memory();
    return OK;
}
#endif

} // gcheck