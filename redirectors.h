#pragma once

#include <string>
#include <stdio.h>

namespace gcheck {
    
/* 
    Class for injecting input from a file (e.g. stdin).
*/
class FileInjecter {
    int save_;
    FILE* original_;
    FILE* out_;
public:
    /*
        Constructs an injector to stream 'stream'.
        FileInjecter(stdin) or StdinInjecter() for standard input injection.
    */
    FileInjecter(FILE* stream);
    ~FileInjecter();

    void Write(std::string str);
};
/*
    Injects to stdin.
    Equivalent to FileInjecter(stdin).
*/
class StdinInjecter : public FileInjecter {
public:
    StdinInjecter();
};


/* 
    Class for capturing output written to a file (e.g. stdout).
*/
class FileCapturer {
    bool is_swapped_;
    long last_pos_;
    int fileno_;
    int save_;
    FILE* new_;
    FILE* original_;
public:
    /*
        Captures output to 'stream'.
        Use FileCapturer(stdout) to capture standard output and FileCapturer(stderr) to capture standard error output
            or use the StdoutCapturer and StderrCapturer classes.
    */
    FileCapturer(FILE* stream);
    ~FileCapturer();
    
    std::string str();
    void Restore();
    void Capture();
};

/*
    Captures from standard output.
    Equivalent to FileCapturer(stdout).
*/
class StdoutCapturer : public FileCapturer {
public:
    StdoutCapturer();
};

/*
    Captures from standard error.
    Equivalent to FileCapturer(stderr).
*/
class StderrCapturer : public FileCapturer {
public:
    StderrCapturer();
};
}