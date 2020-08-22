#pragma once

#include <string>
#include <stdio.h>
#include <ios>

namespace gcheck {

/*
    Class for injecting input from a file (e.g. stdin).
    State of associate istream gets reset back to the original after Restore().
*/
class FileInjecter {
    bool swapped_;
    bool closed_;
    int save_;
    FILE* original_;
    FILE* out_;
    std::istream* associate_;
    std::ios_base::iostate original_state_;
public:
    /*
        Constructs an injector to stream 'stream'.
        FileInjecter(stdin) or StdinInjecter() for standard input injection.
    */
    FileInjecter(FILE* stream, std::string str = "", std::istream* associate = nullptr);
    FileInjecter(FILE* stream, bool capture, std::istream* associate = nullptr);
    ~FileInjecter();

    FileInjecter& Write(std::string str);
    FileInjecter& Capture();
    FileInjecter& Restore();
    FileInjecter& Close();

    FileInjecter& operator<<(std::string str) { return Write(str); }
};
/*
    Injects to stdin.
    Equivalent to FileInjecter(stdin, str, &std::cin).
*/
class StdinInjecter : public FileInjecter {
public:
    StdinInjecter(std::string str = "");
    StdinInjecter(bool capture);
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
    FileCapturer(FILE* stream, bool capture = true);
    ~FileCapturer();

    std::string str();
    FileCapturer& Restore();
    FileCapturer& Capture();
};

/*
    Captures from standard output.
    Equivalent to FileCapturer(stdout).
*/
class StdoutCapturer : public FileCapturer {
public:
    StdoutCapturer(bool capture = true);
};

/*
    Captures from standard error.
    Equivalent to FileCapturer(stderr).
*/
class StderrCapturer : public FileCapturer {
public:
    StderrCapturer(bool capture = true);
};
}