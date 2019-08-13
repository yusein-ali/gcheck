#include "redirectors.h"
#include <stdio.h>
#include <cerrno>
#if defined(WIN32) || defined(_WIN32)
    #include <fcntl.h>
    #include <io.h>
    #define dup(fd) _dup(fd)
    #define dup2(fd, fd2) _dup2(fd, fd2)
    #define fileno(file) _fileno(file)
    #define pipe(p) _pipe(p, 1024, _O_TEXT)
#else
    #include <unistd.h>
#endif
#include <string>
#include <sstream>

namespace gcheck {
    
FileInjecter::FileInjecter(FILE* stream, std::string str) {
    original_ = stream;
    save_ = dup(fileno(stream));
    int fds[2];
    pipe(fds);
    out_ = fdopen(fds[1], "w");
    dup2(fds[0], fileno(stream));
    close(fds[0]);
    
    if(str.length() != 0)
        Write(str);
}

FileInjecter::~FileInjecter() {
    dup2(save_, fileno(original_));
    fclose(out_);
    close(save_);
}

void FileInjecter::Write(std::string str) {
    fputs(str.c_str(), out_);
    fflush(out_);
}


FileCapturer::FileCapturer(FILE* stream) : is_swapped_(false), last_pos_(0), fileno_(fileno(stream)), original_(stream) {
    new_ = tmpfile();
    if(new_ == NULL) {
        int err = errno;
        std::string desc = "errno: " + std::to_string(err) + ", " + std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": ";
        switch(err) {
            case EACCES:
                throw std::runtime_error(desc + "No file permissions");
            case EEXIST:
                throw std::runtime_error(desc + "Unable to generate a unique filename");
            case EINTR:
                throw std::runtime_error(desc + "The call was interrupted by a signal");
            case EMFILE:
                throw std::runtime_error(desc + "Too many open files in process");
            case ENFILE:
                throw std::runtime_error(desc + "Too many files open in system");
            case ENOSPC:
                throw std::runtime_error(desc + "Directory full");
            case EROFS:
                throw std::runtime_error(desc + "File system is read-only");
            default:
                throw;
        }
    }
    Capture();
}

FileCapturer::~FileCapturer() {
    Restore();
    if(new_ == NULL) return;
    
    fclose(new_);
    close(save_);
    new_ = NULL;
}

std::string FileCapturer::str() {
    if(new_ == NULL) return "";
    
    fflush(new_);
    
    fseek(new_, last_pos_, 0);
    
    std::stringstream ss;
    
    int c = EOF+1;
    while((c = fgetc(new_)) != EOF) {
        ss.put(c);
    }
    
    last_pos_ = ftell(new_);
    
    return ss.str();
}

void FileCapturer::Restore() {
    if(!is_swapped_) return;
    is_swapped_ = false;

    fflush(original_);
    dup2(save_, fileno_);
    close(save_);
}

void FileCapturer::Capture() {
    if(new_ == NULL) throw; // TODO: better exception
    if(is_swapped_) return;
    is_swapped_ = true;
    
    fflush(original_);
    save_ = dup(fileno_);
    dup2(fileno(new_), fileno_);
}

StdinInjecter::StdinInjecter(std::string str) : FileInjecter(stdin, str) {}
StdoutCapturer::StdoutCapturer() : FileCapturer(stdout) {}
StderrCapturer::StderrCapturer() : FileCapturer(stderr) {}

}