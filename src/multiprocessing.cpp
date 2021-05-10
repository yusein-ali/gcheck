#include "multiprocessing.h"

namespace gcheck {

#if defined(__linux__)
ForkStatus wait_timeout(pid_t pid, std::chrono::duration<double> time) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        return ERROR;
    }

    auto secs = std::chrono::floor<std::chrono::seconds>(time);
    auto nsecs = std::chrono::ceil<std::chrono::nanoseconds>(time-secs);

    struct timespec timeout;
    timeout.tv_sec = secs.count();
    timeout.tv_nsec = nsecs.count();

    siginfo_t info;
    while(true) {
        int val = sigtimedwait(&mask, &info, &timeout);
        if(val >= 0) {
            if(info.si_pid != pid) continue;
            else break;
        } else if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN) {
            kill(pid, SIGKILL);
            return TIMEDOUT;
        }
        break;
    }

    return info.si_status == 0 ? OK : ERROR;
}
#endif

} // gcheck