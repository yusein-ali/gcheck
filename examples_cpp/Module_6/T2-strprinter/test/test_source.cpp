#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <iterator>

#include "diagonal_printer.hpp"
#include "printers.hpp"
#include "standard_printer.hpp"
#include "string_printer.hpp"
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

const std::vector<std::string> strings_v{"test print", "second test", "testing the third", "mooore", "idk"};
gcheck::SequenceArgument<std::string> strings(strings_v);
gcheck::SequenceArgument<std::string> starting_lines({"", "first line", "mumbo jumbo"});
gcheck::SequenceArgument<std::string> ending_lines({"last line", "", ":D"});
const std::vector<std::string> names{"mumbo", "a name", "second name", "something", "John Cena"};
const std::vector<std::string> prints{"mumbo", "a name", "second name", "something", "John Cena"};

class ReversePrinter : public StringPrinter {
public:
    /* Construct a new object - test default parameter value*/
    ReversePrinter() : StringPrinter() { }
    /* Construct a new object - pass the std::ostream& to the base class */
    ReversePrinter(std::ostream &os) : StringPrinter(os) { }
    /* Copy this object */
    ReversePrinter* Clone() const {
        return new ReversePrinter(os_);
    }
    virtual StringPrinter& operator()(const std::string& str) {
        /* os is a protected member of the base class */
        std::copy(str.rbegin(), str.rend(), std::ostream_iterator<char>(os_));
        os_ << std::endl;
        return *this;
    }
private:
};

// BEGIN SERVER INTERNAL
std::string CStandard(std::string str) {
    return str + "\n";
}
// END SERVER INTERNAL
std::string TStandard(std::string str) {
    std::stringstream ss;
    StandardPrinter sp(ss);
    sp(str);
    return ss.str();
}
TEST(StandardPrinter, function_call_operator_and_constructor, 1) {
    strings.Reset();
    CompareWithCallable(strings.GetSize(), CStandard, TStandard, strings);
}

// BEGIN SERVER INTERNAL
std::string CDiagonal(std::string str, std::string first, std::string last) {
    std::string out((str.length()+1)*str.length(), ' ');
    for(size_t i = 0; i < str.length(); ++i) {
        out[i*(str.length()+1) + str.length()] = '\n';
        out[i*(str.length()+2)] = str[i];
    }
    if(first.length() > 0) out = first + '\n' + out;
    if(last.length() > 0) out = out + last + '\n';
    return out;
}
// END SERVER INTERNAL
std::string TDiagonal(std::string str, std::string first, std::string last) {
    std::stringstream ss;
    DiagonalPrinter sp(first, last, ss);
    sp(str);
    return ss.str();
}
std::string TDiagonal2(std::string str) {
    gcheck::StdoutCapturer tout;
    DiagonalPrinter sp;
    sp(str);
    return tout.str();
}
TEST(DiagonalPrinter, function_call_operator_and_constructor, 1) {
    strings.Reset();
    starting_lines.Reset();
    ending_lines.Reset();
    CompareWithCallable(strings.GetSize(), [](std::string str) { return CDiagonal(str, "", ""); }, TDiagonal2, strings);
    strings.Reset();
    CompareWithCallable(starting_lines.GetSize(), CDiagonal, TDiagonal, strings, starting_lines, ending_lines);
}

TEST(StringPrinter, test_members, 1) {
    std::ostringstream oss;
    StringPrinter* p = new ReversePrinter;
    StringPrinter* p2 = new ReversePrinter(oss);

    gcheck::StdoutCapturer tout;

    (*p)("Testing the two printers");

    std::string function_call_operator_out = tout.str();

    (*p2)("Seems to work");

    std::string correct = "sretnirp owt eht gnitseT\n";
    EXPECT_EQ(correct, function_call_operator_out) << "default stream";
    correct = "krow ot smeeS\n";
    function_call_operator_out = oss.str();
    EXPECT_EQ(correct, function_call_operator_out) << "given a stream";

    delete p;
    delete p2;
}


// BEGIN SERVER INTERNAL
std::string CAddIndexing(std::vector<std::string>, std::vector<std::string> print) {
    std::stringstream ss;
    std::string out;
    out += CStandard(print[0]);

    (ReversePrinter(ss))(print[1]);
    out += ss.str();
    ss.str(""); ss.clear();

    out += CDiagonal(print[2], "", "");

    (ReversePrinter(ss))(print[3]);
    out += ss.str();;

    return out;
}
// END SERVER INTERNAL
std::string TAddIndexing(std::vector<std::string> names, std::vector<std::string> print) {
    Printers printers;
    std::stringstream ss;
    StringPrinter* p = new StandardPrinter(ss);
    StringPrinter* p2 = new ReversePrinter(ss);
    StringPrinter* p3 = new DiagonalPrinter("", "", ss);
    StringPrinter* p4 = new ReversePrinter(ss);
    printers.Add(names[1], p2);
    printers.Add(names[0], p);
    printers.Add(names[3], p4);
    printers.Add(names[2], p3);

    try {
        for(size_t i = 0; i < 4; ++i)
            printers[names[i]](print[i]);

    } catch(std::invalid_argument& e) {
        return "Indexing operator threw an exception when it shouldn't have.";
    }
    return ss.str();
}

TEST(Printers, Add_and_indexing, 1, "StandardPrinter.function_call_operator_and_constructor DiagonalPrinter.function_call_operator_and_constructor StringPrinter.test_members") {
    CompareWithCallable(3, CAddIndexing, TAddIndexing, names, prints);
}

TEST(Diagonal_Standard, chaining, 1, "StandardPrinter.function_call_operator_and_constructor DiagonalPrinter.function_call_operator_and_constructor") {
    std::ostringstream ss;
    StringPrinter* p = new StandardPrinter(ss);
    StringPrinter* p2 = new DiagonalPrinter("","", ss);

    (*p)("Testing the standard printer")("With some chaining")("Whoo");

    std::string standard_chain_out = ss.str();

    std::string correct = "Testing the standard printer\n"
        "With some chaining\n"
        "Whoo\n";

    EXPECT_EQ(correct, standard_chain_out);

    ss.str("");
    ss.clear();

    (*p2)("works?")("yes");

    std::string diagonal_chain_out = ss.str();

    correct =
            "w     \n"
            " o    \n"
            "  r   \n"
            "   k  \n"
            "    s \n"
            "     ?\n"
            "y  \n"
            " e \n"
            "  s\n";

    EXPECT_EQ(correct, diagonal_chain_out);

    delete p;
    delete p2;
}

TEST(Printers, test_copyassign, 2, "Printers.Add_and_indexing Diagonal_Standard.chaining") {
    std::ostringstream ss;
    auto p = new StandardPrinter(ss);
    auto p2 = new ReversePrinter(ss);

    Printers& ps = *(new Printers);
    ps.Add("n1", p);
    ps.Add("r2", p2);

    Printers ps2(ps);
    Printers ps3;
    ps3 = ps;

    delete &ps;

    ps2["n1"]("Testing a copy constructed Printers");
    ps2["n1"]("With some chaining")("Whoo");

    std::string copy_constructed_out = ss.str();

    std::string correct =
        "Testing a copy constructed Printers\n"
        "With some chaining\n"
        "Whoo\n";
    EXPECT_EQ(correct, copy_constructed_out);

    ss.str("");
    ss.clear();

    ps3["r2"]("Seems to work");
    ps3["r2"]("Yes, pretty well");

    std::string copy_assigned_out = ss.str();

    correct = "krow ot smeeS\n"
        "llew ytterp ,seY\n";
    EXPECT_EQ(correct, copy_assigned_out);
}

TEST(Printers, exceptions, 1, "Printers.Add_and_indexing") {
    auto p = new StandardPrinter;
    auto p2 = new DiagonalPrinter;

    Printers ps;
    ps.Add("n1", p);
    ps.Add("r2", p2);

    std::string exception_what;
    bool exception_type_is_invalid_argument = false;
    bool exception_thrown = false;
    try {
        ps.Add("n3", nullptr);
    } catch(const std::invalid_argument& err) {
        exception_what = err.what();
        exception_type_is_invalid_argument = true;
        exception_thrown = true;
    } catch(const std::exception& err) {
        exception_what = err.what();
        exception_type_is_invalid_argument = false;
        exception_thrown = true;
    }
    if(exception_thrown) {
        EXPECT_EQ(std::string("Invalid printer"), exception_what) << "exception.what()";
        EXPECT_TRUE(exception_type_is_invalid_argument) << "Add nullptr: Exception type should be invalid_argument";
    } else {
        bool Add_with_nullptr_throws = false;
        EXPECT_TRUE(Add_with_nullptr_throws) << "Add should throw with a nullptr parameter";
    }

    exception_thrown = false;
    try {
        ps["pinkey"]("Hih");
    } catch(const std::invalid_argument& err) {
        exception_what = err.what();
        exception_type_is_invalid_argument = true;
        exception_thrown = true;
    } catch(const std::exception& err) {
        exception_what = err.what();
        exception_type_is_invalid_argument = false;
        exception_thrown = true;
    }
    if(exception_thrown) {
        EXPECT_EQ(std::string("Unknown index"), exception_what) << "exception.what()";
        EXPECT_TRUE(exception_type_is_invalid_argument) << "Indexing: Exception type should be invalid_argument";
    } else {
        bool Add_with_nullptr_throws = false;
        EXPECT_TRUE(Add_with_nullptr_throws) << "Indexing should throw when the printer doesn't exist";
    }

    exception_thrown = false;
    try {
        ps.Add("n1", p);
    } catch(const std::invalid_argument& err) {
        exception_what = err.what();
        exception_type_is_invalid_argument = true;
        exception_thrown = true;
    } catch(const std::exception& err) {
        exception_what = err.what();
        exception_type_is_invalid_argument = false;
        exception_thrown = true;
    }
    if(exception_thrown) {
        EXPECT_EQ(std::string("Duplicate index"), exception_what) << "exception.what()";
        EXPECT_TRUE(exception_type_is_invalid_argument) << "Add duplicate name: Exception type should be invalid_argument";
    } else {
        bool Add_with_duplicate_name_throws = false;
        EXPECT_TRUE(Add_with_duplicate_name_throws) << "Add should throw with a duplicate name";
    }
}
