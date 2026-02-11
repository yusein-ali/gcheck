#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iterator>

#include "list.hpp"
#include <gcheck/argument.h>
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

size_t num_runs = 10;
gcheck::SequenceArgument<int> seq_num({0, 1, 5});
gcheck::Random<int> rnd_num(0, 10);
gcheck::Random<std::string> rnd_str({"", " ", "some text", "some more", "asd", "what else?", "even more text", "im out of ideas"});
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
std::list<std::string> CGetLines(const std::string& input) {
    std::istringstream is(input);

    std::list<std::string> l;
    for (std::string line; std::getline(is, line); l.push_back(line)) {}

    return l;
}
// END SERVER INTERNAL

void TGetLines(const std::string& input, std::list<std::string>& l) {
    std::istringstream f(input);
    GetLines(f, l);
}

FUNCTIONTEST(List, GetLines, num_runs, TGetLines) {
    SetTimeout(1);
    SetArguments(rnd.Next(), rnd3.Next());
    SetArgumentsAfter(rnd(), CGetLines(rnd()));
}


// BEGIN SERVER INTERNAL
std::string CPrint(const std::list<std::string>& lines) {
    std::stringstream ss;
    for (std::list<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it){
        ss << *it << std::endl;
    }
    return ss.str();
}
// END SERVER INTERNAL

std::string TPrint(const std::list<std::string>& lines) {
    gcheck::StdoutCapturer tout;

    Print(lines);

    return tout.str();
}

IOTEST(List, Print, num_runs, Print) {
    SetTimeout(1);
    SetArguments(rnd3.Next());
    SetArgumentsAfter(rnd3());
    SetOutput(CPrint(rnd3()));
}


// BEGIN SERVER INTERNAL
std::list<std::string> CSortAndUnique(std::list<std::string> lines) {
    lines.sort();
    lines.unique();
    return lines;
}
// END SERVER INTERNAL

std::vector<std::string> TSortAndUnique(std::list<std::string> lines) {
    SortAndUnique(lines);

    return std::vector<std::string>(lines.begin(), lines.end());
}

FUNCTIONTEST(List, SortAndUnique, num_runs, SortAndUnique) {
    SetTimeout(1);
    SetArguments(rnd3.Next());
    SetArgumentsAfter(CSortAndUnique(rnd3()));
}
