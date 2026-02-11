#pragma once

/**
 * @file redirectors.h
 * @brief Stream redirection helpers for stdin injection and stdout/stderr capture.
 *
 * These utilities provide RAII wrappers around C file-descriptor operations:
 * - input injection (`FileInjecter`, `StdinInjecter`)
 * - output capture (`FileCapturer`, `StdoutCapturer`, `StderrCapturer`)
 *
 * Implementation overview (see `src/redirectors.cpp`):
 * 1. Injection uses `pipe + dup2` to replace the target stream descriptor
 *    with the read end of a pipe; injected content is written to the write end.
 * 2. Capture uses `tmpfile + dup2` to redirect writes from target descriptor to
 *    a temporary file.
 * 3. `Restore()` reinstates original file descriptors and stream state.
 *
 * Typical usage:
 * @code{.cpp}
 * using namespace gcheck;
 *
 * // Inject stdin
 * StdinInjecter in("12 34\n");
 * int a, b;
 * std::cin >> a >> b; // reads injected values
 * in.Restore();
 *
 * // Capture stdout
 * StdoutCapturer out;
 * std::cout << "hello";
 * std::string captured = out.str(); // "hello"
 * out.Restore();
 * @endcode
 */

#include <string>
#include <stdio.h>
#include <ios>

namespace gcheck {

/**
 * @brief Inject text into an input FILE stream by descriptor redirection.
 *
 * This class is designed for streams like `stdin` and can optionally keep an
 * associated C++ `std::istream` state in sync (for example `std::cin`).
 *
 * Implementation details:
 * 1. Saves original descriptor via `dup(fileno(stream))`.
 * 2. On `Capture()`, creates pipe; redirects target stream to pipe read end.
 * 3. `Write()` pushes content to pipe write end (`fputs` + `fflush`).
 * 4. `Restore()` drains/clears stream, restores original descriptor with `dup2`,
 *    and resets associated iostream state.
 *
 * Typical usage scenarios:
 * 1. Inject deterministic stdin input for parser/function tests.
 * 2. Repeatedly rewrite input stream within same test case.
 *
 * Example:
 * @code{.cpp}
 * gcheck::FileInjecter inj(stdin, "", &std::cin);
 * inj.Capture().Write("line1\nline2\n");
 * std::string a, b;
 * std::cin >> a >> b;
 * inj.Restore();
 * @endcode
 */
class FileInjecter {
    /** @brief Whether target stream descriptor is currently redirected. */
    bool swapped_;
    /** @brief Whether current injection output endpoint is closed. */
    bool closed_;
    /** @brief Saved copy of original target descriptor (`dup`). */
    int save_;
    /** @brief Original stream being redirected (e.g. `stdin`). */
    FILE* original_;
    /** @brief Writable endpoint used to push injected content. */
    FILE* out_;
    /** @brief Optional associated C++ input stream (e.g. `&std::cin`). */
    std::istream* associate_;
    /** @brief Saved associated iostream state restored by `Restore()`. */
    std::ios_base::iostate original_state_;
public:
    /**
     * @brief Construct and optionally preload injected data.
     *
     * @param stream Target FILE stream to redirect.
     * @param str Optional initial text to inject.
     * @param associate Optional associated C++ input stream.
     *
     * Starts capture immediately and writes `str` when non-empty.
     */
    FileInjecter(FILE* stream, std::string str = "", std::istream* associate = nullptr);
    /**
     * @brief Convenience constructor from C string.
     *
     * @param stream Target FILE stream.
     * @param str Initial injected content.
     * @param associate Optional associated C++ stream.
     */
    FileInjecter(FILE* stream, const char* str, std::istream* associate = nullptr) : FileInjecter(stream, (std::string)str, associate) {}
    /**
     * @brief Construct injector with optional immediate capture activation.
     *
     * @param stream Target FILE stream.
     * @param capture If true, immediately activate redirection.
     * @param associate Optional associated C++ stream.
     */
    FileInjecter(FILE* stream, bool capture, std::istream* associate = nullptr);
    /**
     * @brief Destructor restoring stream and releasing saved descriptor.
     */
    ~FileInjecter();

    /**
     * @brief Write injected content to active input pipe.
     *
     * If capture is not active or output endpoint is closed, reactivates capture.
     *
     * @param str Data to inject.
     * @return `*this`.
     */
    FileInjecter& Write(std::string str);
    /**
     * @brief Activate injection capture (pipe + descriptor swap).
     *
     * Safe to call multiple times; handles previously closed/redirection state.
     *
     * @return `*this`.
     */
    FileInjecter& Capture();
    /**
     * @brief Restore original descriptor and associated stream state.
     *
     * Also drains remaining pipe input and clears stream errors.
     *
     * @return `*this`.
     */
    FileInjecter& Restore();
    /**
     * @brief Close current writable injection endpoint.
     *
     * Useful to simulate EOF on injected stream.
     *
     * @return `*this`.
     */
    FileInjecter& Close();

    /**
     * @brief Stream-style write shortcut.
     *
     * @param str Data to inject.
     * @return `*this`.
     */
    FileInjecter& operator<<(std::string str) { return Write(str); }
};
/**
 * @brief Convenience injector specialized for `stdin` / `std::cin`.
 *
 * Equivalent to `FileInjecter(stdin, ..., &std::cin)`.
 *
 * Example:
 * @code{.cpp}
 * gcheck::StdinInjecter in("5 8\n");
 * int x, y;
 * std::cin >> x >> y;
 * @endcode
 */
class StdinInjecter : public FileInjecter {
public:
    /** @brief Construct with optional initial injected content. */
    StdinInjecter(std::string str = "");
    /** @brief Construct from C string content. */
    StdinInjecter(const char* str);
    /** @brief Construct with explicit immediate-capture toggle. */
    StdinInjecter(bool capture);
};


/**
 * @brief Capture output from a FILE stream into an internal temporary file.
 *
 * Implementation details:
 * 1. Creates `tmpfile()` storage on construction.
 * 2. `Capture()` saves original descriptor and redirects target descriptor to tmpfile.
 * 3. `str()` reads newly written bytes since previous `str()` call using `last_pos_`.
 * 4. `Restore()` reinstates original descriptor using `dup2`.
 *
 * Typical usage scenarios:
 * 1. Assert emitted stdout/stderr text in tests.
 * 2. Capture incremental output by calling `str()` multiple times.
 *
 * Example:
 * @code{.cpp}
 * gcheck::FileCapturer cap(stdout);
 * std::cout << "A";
 * auto a = cap.str(); // "A"
 * std::cout << "B";
 * auto b = cap.str(); // "B" (incremental)
 * cap.Restore();
 * @endcode
 */
class FileCapturer {
    /** @brief Whether descriptor is currently redirected to temp file. */
    bool is_swapped_;
    /** @brief Last read position in temporary file for incremental `str()`. */
    long last_pos_;
    /** @brief Descriptor number of original stream (e.g. fileno(stdout)). */
    int fileno_;
    /** @brief Saved original descriptor while capture is active. */
    int save_;
    /** @brief Temporary file receiving redirected output. */
    FILE* new_;
    /** @brief Original stream pointer being captured. */
    FILE* original_;
public:
    /**
     * @brief Construct capturer for a stream, optionally starting capture immediately.
     *
     * @param stream Target output stream (`stdout`, `stderr`, or other FILE*).
     * @param capture If true, call `Capture()` during construction.
     *
     * Throws runtime errors for `tmpfile()` failures with mapped errno details.
     */
    FileCapturer(FILE* stream, bool capture = true);
    /**
     * @brief Destructor restoring original stream and releasing temporary file.
     */
    ~FileCapturer();

    /**
     * @brief Read captured output since last read.
     *
     * This is incremental: each call returns only newly written content.
     *
     * @return Captured text segment.
     */
    std::string str();
    /**
     * @brief Restore original descriptor and stop capture.
     *
     * @return `*this`.
     */
    FileCapturer& Restore();
    /**
     * @brief Start capture by redirecting target descriptor to temporary file.
     *
     * @return `*this`.
     */
    FileCapturer& Capture();
};

/**
 * @brief Convenience capturer for `stdout`.
 *
 * Equivalent to `FileCapturer(stdout, capture)`.
 */
class StdoutCapturer : public FileCapturer {
public:
    /** @brief Construct stdout capturer. */
    StdoutCapturer(bool capture = true);
};

/**
 * @brief Convenience capturer for `stderr`.
 *
 * Equivalent to `FileCapturer(stderr, capture)`.
 */
class StderrCapturer : public FileCapturer {
public:
    /** @brief Construct stderr capturer. */
    StderrCapturer(bool capture = true);
};
} // gcheck
