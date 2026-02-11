/**
 * @file test_source.cpp
 * @brief Function/IO tests for list utilities in Module 3 (Task 2).
 *
 * This file defines randomized test fixtures for:
 * 1. `GetLines(std::istream&, std::list<std::string>&)` through wrapper
 *    `TGetLines(const std::string&, std::list<std::string>&)`.
 * 2. `Print(const std::list<std::string>&)`.
 * 3. `SortAndUnique(std::list<std::string>&)`.
 *
 * Design notes:
 * - Randomized generators are shared globally to diversify inputs.
 * - For each run, tests set both immediate arguments and generated
 *   argument/expectation suppliers (`SetArgumentsAfter`, `SetOutput`).
 * - "Server internal" helpers (`CGetLines`, `CPrint`, `CSortAndUnique`)
 *   define the expected reference behavior used for comparison.
 */
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

#include "list.hpp"
#include <argument.h>
#include <redirectors.h>
#include <tests.h>

/** @brief Number of executions performed per test instance. */
size_t num_runs = 10;
/** @brief Deterministic short sequence used for edge-case run sizes. */
gcheck::SequenceArgument<int> seq_num({0, 1, 5});
/** @brief Random integer count generator for generalized run sizes. */
gcheck::Random<int> rnd_num(0, 10);
/** @brief Random line-content generator including empty/space-heavy strings. */
gcheck::Random<std::string> rnd_str({"", " ", "some text", "some more", "asd", "what else?", "even more text", "im out of ideas"});
/**
 * @brief Random multiline-text generator.
 *
 * Produces a single string containing `amount` lines with `\\n` separators.
 * For the first few runs (`index % num_runs < 3`), uses deterministic values
 * from @ref seq_num to guarantee edge-coverage. Otherwise uses @ref rnd_num.
 */
gcheck::Generator rnd(
    [](size_t index) -> std::string {
        std::string input;
        size_t amount;
        if (index % num_runs < 3) {
            amount = seq_num.Next();
        } else {
            amount = rnd_num.Next();
        }
        for(size_t i = 0; i < amount; i++)
            input += rnd_str.Next() + '\n';
        return input;
    }
);
/**
 * @brief Random list-of-strings generator with index-aware run sizing.
 *
 * Uses the same size policy as @ref rnd and fills each entry with text from
 * @ref rnd (therefore each list element may itself contain multiple lines).
 */
gcheck::Generator rnd2(
    [](size_t index) -> std::list<std::string> {
        std::list<std::string> l;
        size_t amount;
        if (index % num_runs < 3) {
            amount = seq_num.Next();
        } else {
            amount = rnd_num.Next();
        }
        for(size_t i = 0; i < amount; i++)
            l.push_back(rnd.Next());
        return l;
    }
);
/**
 * @brief Random list-of-strings generator without deterministic warmup sizes.
 *
 * This is used where fully random list sizes are acceptable.
 */
gcheck::Generator rnd3(
    []() -> std::list<std::string> {
        std::list<std::string> l;
        size_t amount;
        amount = rnd_num.Next();
        for(size_t i = 0; i < amount; i++)
            l.push_back(rnd.Next());
        return l;
    }
);


// BEGIN SERVER INTERNAL
/**
 * @brief Reference implementation of line splitting.
 *
 * @param input Multiline input text.
 * @return List of lines extracted with `std::getline`.
 */
std::list<std::string> CGetLines(const std::string& input) {
    std::istringstream is(input);

    std::list<std::string> l;
    for (std::string line; std::getline(is, line); l.push_back(line)) {}

    return l;
}
// END SERVER INTERNAL

/**
 * @brief Adapter that matches `FunctionTest<void, const std::string&, std::list<std::string>&>`.
 *
 * Converts a raw string into an input stream, then forwards to student-facing
 * `GetLines` implementation.
 *
 * @param input Raw multiline input.
 * @param l Output list populated by `GetLines`.
 */
void TGetLines(const std::string& input, std::list<std::string>& l) {
    std::istringstream f(input);
    GetLines(f, l);
}

namespace {

/**
 * @brief Property-style randomized test for `GetLines`.
 *
 * Usage scenario:
 * - Verifies that parsing behavior over random multiline strings matches
 *   reference logic from @ref CGetLines.
 * - Also validates in-place mutation of the output list argument.
 */
class ListGetLinesTest : public gcheck::FunctionTest<void, const std::string&, std::list<std::string>&> {
public:
    /** @brief Construct and register the test with suite/name and run count. */
    ListGetLinesTest() : gcheck::FunctionTest<void, const std::string&, std::list<std::string>&>(
        gcheck::TestInfo("List", "GetLines"), num_runs, TGetLines) {}

private:
    /**
     * @brief Configure one run's inputs, expectations, and timeout.
     *
     * Sets:
     * - timeout to 1 second
     * - direct arguments for immediate run data
     * - expected post-call arguments from @ref CGetLines
     */
    void SetInputsAndOutputs() override {
        SetTimeout(1);
        SetArguments(rnd.Next(), rnd3.Next());
        SetArgumentsAfter(rnd(), CGetLines(rnd()));
    }
};
ListGetLinesTest list_get_lines_test;


// BEGIN SERVER INTERNAL
/**
 * @brief Reference implementation for list printing.
 *
 * Produces the expected stdout format for `Print`, where each element is
 * followed by `std::endl`.
 *
 * @param lines Input list.
 * @return Expected printed representation.
 */
std::string CPrint(const std::list<std::string>& lines) {
    std::stringstream ss;
    for (std::list<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it){
        ss << *it << std::endl;
    }
    return ss.str();
}
// END SERVER INTERNAL

/**
 * @brief Randomized IO test for `Print`.
 *
 * Usage scenario:
 * - Confirms stdout emitted by `Print` matches @ref CPrint across random list
 *   values and sizes.
 */
class ListPrintTest : public gcheck::IOTest<void, const std::list<std::string>&> {
public:
    /** @brief Construct and register test metadata and callable under test. */
    ListPrintTest() : gcheck::IOTest<void, const std::list<std::string>&>(
        gcheck::TestInfo("List", "Print"), num_runs, Print) {}

private:
    /**
     * @brief Configure one run's arguments/output expectation and timeout.
     *
     * Sets:
     * - timeout to 1 second
     * - randomized list input
     * - expected stdout from @ref CPrint
     */
    void SetInputsAndOutputs() override {
        SetTimeout(1);
        SetArguments(rnd3.Next());
        SetArgumentsAfter(rnd3());
        SetOutput(CPrint(rnd3()));
    }
};
ListPrintTest list_print_test;


// BEGIN SERVER INTERNAL
/**
 * @brief Reference implementation for sort+unique list normalization.
 *
 * @param lines Input list copied by value.
 * @return Sorted list with consecutive duplicates removed.
 */
std::list<std::string> CSortAndUnique(std::list<std::string> lines) {
    lines.sort();
    lines.unique();
    return lines;
}
// END SERVER INTERNAL

/**
 * @brief Randomized mutation test for `SortAndUnique`.
 *
 * Usage scenario:
 * - Validates in-place list transformation against reference behavior from
 *   @ref CSortAndUnique for random list inputs.
 */
class ListSortAndUniqueTest : public gcheck::FunctionTest<void, std::list<std::string>&> {
public:
    /** @brief Construct and register test metadata and target function. */
    ListSortAndUniqueTest() : gcheck::FunctionTest<void, std::list<std::string>&>(
        gcheck::TestInfo("List", "SortAndUnique"), num_runs, SortAndUnique) {}

private:
    /**
     * @brief Configure one run's input/expected post-state and timeout.
     *
     * Sets:
     * - timeout to 1 second
     * - randomized list input
     * - expected mutated list from @ref CSortAndUnique
     */
    void SetInputsAndOutputs() override {
        SetTimeout(1);
        SetArguments(rnd3.Next());
        SetArgumentsAfter(CSortAndUnique(rnd3()));
    }
};
ListSortAndUniqueTest list_sort_and_unique_test;

} // namespace
