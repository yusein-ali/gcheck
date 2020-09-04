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
#include <iostream>

namespace gcheck {

FileInjecter::FileInjecter(FILE* stream, std::string str, std::istream* associate)
        : FileInjecter(stream, true, associate) {
    if(str.length() != 0) {
        Write(str);
    }
}

FileInjecter::FileInjecter(FILE* stream, bool capture, std::istream* associate)
        : swapped_(false), closed_(true), associate_(associate) {
    original_ = stream;
    save_ = dup(fileno(stream));

    if(capture)
        Capture();
}

FileInjecter::~FileInjecter() {
    Restore();
    close(save_);
}

FileInjecter& FileInjecter::Write(std::string str) {
    if(!swapped_ || closed_) Capture();

    fputs(str.c_str(), out_);
    fflush(out_);

    return *this;
}

FileInjecter& FileInjecter::Capture() {
    if(swapped_) {
        if(!closed_) return *this;

        Restore();
    } else if(!closed_) {
        Close();
    }

    if(associate_ != nullptr)
        original_state_ = associate_->rdstate();

    int fds[2];
    pipe(fds);
    out_ = fdopen(fds[1], "w");
    dup2(fds[0], fileno(original_));
    close(fds[0]);

    swapped_ = true;
    closed_ = false;

    return *this;
}

FileInjecter& FileInjecter::Restore() {
    if(!closed_) Close();
    if(!swapped_) return *this;

    char buffer[1024];
    while(fgets(buffer, 1024, original_) != NULL);
    clearerr(original_);

    dup2(save_, fileno(original_));

    swapped_ = false;
    if(associate_ != nullptr)
        associate_->clear(original_state_);

    return *this;
}

FileInjecter& FileInjecter::Close() {
    if(closed_) return *this;

    fclose(out_);
    closed_ = true;

    return *this;
}



FileCapturer::FileCapturer(FILE* stream, bool capture) : is_swapped_(false), last_pos_(0), fileno_(fileno(stream)), original_(stream) {
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

    if(capture)
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

    fseek(new_, last_pos_, SEEK_SET);

    std::stringstream ss;

    int c = EOF+1;
    while((c = fgetc(new_)) != EOF) {
        ss.put(c);
    }

    return ss.str();
}

FileCapturer& FileCapturer::Restore() {
    if(!is_swapped_) return *this;
    is_swapped_ = false;

    fflush(original_);
    dup2(save_, fileno_);
    close(save_);

    return *this;
}

FileCapturer& FileCapturer::Capture() {
    if(new_ == NULL) throw; // TODO: better exception
    if(is_swapped_) return *this;
    is_swapped_ = true;

    fflush(original_);
    save_ = dup(fileno_);
    dup2(fileno(new_), fileno_);

    fseek(new_, 0, SEEK_END);
    last_pos_ = ftell(new_);

    return *this;
}

StdinInjecter::StdinInjecter(std::string str) : FileInjecter(stdin, str, &std::cin) {}
StdinInjecter::StdinInjecter(const char* str) : StdinInjecter((std::string)str) {}
StdinInjecter::StdinInjecter(bool capture) : FileInjecter(stdin, capture, &std::cin) {}
StdoutCapturer::StdoutCapturer(bool capture) : FileCapturer(stdout, capture) {}
StderrCapturer::StderrCapturer(bool capture) : FileCapturer(stderr, capture) {}

}