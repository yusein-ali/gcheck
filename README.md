# GCheck - Code grading and testing framework

GCheck is a framework for use in C and C++ courses. It contains a C++ library for testing and grading C/C++ code, and tools for compiling HTML output and administrative work.

It has the following parts:

1. C++ library for testing and grading C/C++ code
2. beautify.py script for compiling the test results into HTML
3. report_parser.py for loading the test results to python objects
4. filter.py for compiling a student version of the sources (more about this below)

The library is designed to be make writing tests easy and fast given that you already have a working example implementation of the code to be tested. You can also write tests without one using the `CUSTOMTEST` macro or with a custom test class.

The python scripts provided are designed to make the administrative work easy. They allow easy deployment of the tests and integration to custom python scripts. The `filter.py` script also makes it easy to compile a version of the assignments for students without including the answers.

## Using the C++ library

### Overall idea

GCheck contains a main function that will run the tests specified. This means that there shouldn't be a main function in the sources to be tested or in the testing source described later. You can compile the library with `GCHECK_NOMAIN` defined if you want to specify your own main function but then you need to take care of the settings and running the tests yourself.

The tests are written in C++ and generally using the macros defined in the library. These macros create a new class for each test that automatically adds itself to the list of tests. Custom test types and classes are possible, just look at the `*test.cpp/.h` files for a reference. There are different macros for different purposes, listed below with their usages.

The vars.make in the root of this repository contains the information needed for linking and compiling against this library. Just copy the information from there or include the file in your makefile.

### Test class macros

There are 5 test class macros provided:

1. FUNCTIONTEST
   - This is for testing functions
2. IOTEST
   - This is for testing functions and their input and output through the standard streams.
3. METHODTEST
   - This is for testing class methods.
4. METHODIOTEST
   - This is for testing class methods and their input and output through the standard streams.
5. TEST
   - This is for implementing tests using a sequential method of explicit comparisons.

### Prerequisite tests

Each test macro allows the listing of prerequisite tests that must be passed with full points before the test in question can be run. If the prerequisites aren't fulfilled, the test is assumed to not pass and gives 0 points. This allows making tests that require some other functionality from the tested code. E.g. it can be verified that constructors, getters and setters work correctly before running more complicated tests.

The prerequisites are passed to the test macros as a string in the format `<suite name 1>.<test name 1> <suite name 2>.<test name 2> ...`. If the suitename (and the period) is omitted, the suite is assumed to be the same as the test being specified.

### Grading method

All test types allow setting the grading method with the `SetGradingMethod` class method. The possible values are in the `GradingMethod` enum.

### FUNCTIONTEST(suitename, testname, num_runs, tobetested, points (optional, default 1), prerequisites (optional, default empty))

`suitename` is the name of the test suite, `testname` is the name of the test in the suite (the pair (suitename, testname) identifies the test; it must be unique), `num_runs` is the number of times the function to be tested is called, `tobetested` is the function to be tested, `points` is the number of points given from the test, and `prerequisites` is a string listing the prerequisite tests. E.g. `FUNCTIONTEST(classname, somefunction, 3, hello_world, "classname.otherfunction")`.

This class calls the function `tobetested` a number of times defined by `num_runs` using the options defined in the test body. The following class methods are available:

- SetTimeout
- SetArguments
- SetArgumentsAfter
- IgnoreArgumentsAfter
- SetReturn
- GetLastArguments
- GetRunIndex
- SetMaxRunTime
- OutputFormat

### IOTEST(suitename, testname, num_runs, tobetested, points (optional, default 1), prerequisites (optional, default empty))

`suitename` is the name of the test suite, `testname` is the name of the test in the suite (the pair (suitename, testname) identifies the test; it must be unique), `num_runs` is the number of times the function to be tested is called, `tobetested` is the function to be tested, `points` is the number of points given from the test, and `prerequisites` is a string listing the prerequisite tests. E.g. `IOTEST(classname, somefunction, 3, hello_world, "classname.otherfunction")`.

This is equivalent to `FUNCTIONTEST` but for class methods and the addition of comparing the object state before and after method invocation. The following additional class methods are available:

- SetObject
- SetObjectAfter
- SetStateComparer

### METHODTEST(suitename, testname, num_runs, tobetested, points (optional, default 1), prerequisites (optional, default empty))

`suitename` is the name of the test suite, `testname` is the name of the test in the suite (the pair (suitename, testname) identifies the test; it must be unique), `num_runs` is the number of times the function to be tested is called, `tobetested` is the function to be tested, `points` is the number of points given from the test, and `prerequisites` is a string listing the prerequisite tests. E.g. `METHODTEST(classname, somefunction, 3, hello_world, "classname.otherfunction")`.

This is equivalent to `FUNCTIONTEST` but with the addition of standard input, output and error stream manipulation. The following additional class methods are available:

- SetInput
- SetOutput
- SetError

### METHODIOTEST(suitename, testname, num_runs, tobetested, points (optional, default 1), prerequisites (optional, default empty))

`suitename` is the name of the test suite, `testname` is the name of the test in the suite (the pair (suitename, testname) identifies the test; it must be unique), `num_runs` is the number of times the function to be tested is called, `tobetested` is the function to be tested, `points` is the number of points given from the test, and `prerequisites` is a string listing the prerequisite tests. E.g. `METHODIOTEST(classname, somefunction, 3, hello_world, "classname.otherfunction")`.

This combines the capabilities of `IOTEST` and `METHODTEST`.

### TEST(suitename, testname, points (optional, default 1), prerequisites (optional, default empty))

`suitename` is the name of the test suite, `testname` is the name of the test in the suite (the pair (suitename, testname) identifies the test; it must be unique), `points` is the number of points given from the test, and `prerequisites` is a string listing the prerequisite tests. E.g. `METHODIOTEST(classname, somefunction, 3, hello_world, "classname.otherfunction")`.

With this class the test body is called once, and the points are given by the fraction of the passed comparisons. The following class methods are available:

- CompareWithCallable
- CompareWithAnswer
- ExpectTrue
- ExpectFalse
- ExpectEqual
- ExpectInequal

### Testing C code

The whole process is the same as with C++ code but the headers of the code to be tested should be included with
```
extern "C" {
    //insert includes here
}
```
and the code to be tested should be compiled as C, one then links all the object files together.

### Running the tests

The tests are run by running the executable gotten from linking the library with the test source and the test target code.

Command line args for the executable:

- "--json"
  - whether to output a JSON file
- "--pretty"
  - whether to output a human readable format to stdout
- "--no-confirm"
  - skip the confirmation after running the tests if pretty output is enabled
- "--safe"
  - whether to run the tests in a separate process. This needs to be enabled for timeouts to work. Only available on linux.
- "--width <width>"
  - the line length of the pretty output. The program tries to figure out the console width if this isn't specified.
- <filename>
  - where to save the JSON. `report.json` by default

## beautify.py

This script compiles the test result JSON into HTML using templates. The templates directory contains example templates and instructions. The script can be used as a standalone program or it can be used as an import with the `Beautify` class. When used standalone, the following arguments are available:

- "-o", default="stdout"
  - The output file (or "stdout")
- "-i", default="report.json"
  - The test results JSON file
- "-p", default=-1
  - The maximum points are scaled to this number if it's not -1.
- "-t", default=<location of this script>/../templates
  - The template directory

## filter.py

This script copies the files in the specified location to another location according to the commandline arguments while ignoring the files defined in `.filterignore`. It also has filtering functionality for the tests and solutions:
- Portions marked with `BEGIN SOLUTION` and `END SOLUTION` comments are removed (if --keep-solution is not specified)
- Comments starting with `STUB:` or `STUDENT:` are uncommented and included. E.g. `//STUDENT: return NULL;` becomes `return NULL;`
- Portions in test sources marked with `BEGIN SERVER INTERNAL` and `END SERVER INTERNAL` comments are removed.

The following arguments are possible:

- "-o", default=""
  - Output file/directory
- "-i", default=""
  - Input file/directory
- "-r"
  - Recursively
- "-c"
  - Clear output directory before copying any files
- "-v"
  - Verbose output
- "-q"
  - Quiet output
- "--keep-solution"
  - Do not remove solutions from files
- "--skip-tests"
  - Do not copy test sources

## report_parser.py

This module contains auxiliary classes for parsing the JSON test output. Call instantiate the `Report` class with `Report(<filename>)` to load a JSON report.
