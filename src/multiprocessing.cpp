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
#elif defined(__unix__) || defined(__APPLE__)
// sigtimedwait does not exist on every Unix (macOS has none), so the child
// is polled until it ends or the time is up.
ForkStatus wait_timeout(pid_t pid, std::chrono::duration<double> time) {
    auto deadline = std::chrono::steady_clock::now() + time;

    int status = 0;
    while(true) {
        pid_t res = waitpid(pid, &status, WNOHANG);
        if(res == pid) {
            break;
        } else if(res < 0 && errno != EINTR) {
            return ERROR;
        } else if(std::chrono::steady_clock::now() >= deadline) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            return TIMEDOUT;
        }
        usleep(1000);
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? OK : ERROR;
}
#endif

} // gcheck