# gcheck - Code grading and testing framework
Gcheck is a framework for testing and grading C/C++ code. It is used by making a C++ testing file that includes the `gcheck.h` header.

## Using gcheck
1. gcheck contains a main function that will run the tests specified. This means that there cannot be a main function in the sources to be tested or in the testing source described later.
2. Create a C++ source file, which will contain the tests and include `gcheck.h`.
3. Using the `Test(suite, name)` and `Test(suite, name, max_points)` macros, define the test functions.
4.  a) With make: Compile the gcheck library using `make gcheck.o` and link `gcheck.o` to your code.
    b) Without make: Compile the gcheck library sources `gcheck.cpp`, `utility.cpp`, `argument.cpp` and link them to your code.
    If the the code to be tested is C, include it to the test source as below and compile it as C and the library and test source as C++.
    ```
    extern "C" {
        //insert includes here
    }
    ```
5. Voluntary: Use the `beautify.py` script to create a html file from the json report gotten from gcheck.