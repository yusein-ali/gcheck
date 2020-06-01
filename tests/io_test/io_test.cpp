#include "gcheck.h"
#include "io_test.h"
#include "gcheck.h"

void VoidAndEmpty() {
    
}
int IntAndEmpty() {
    return 0;
}
void VoidAndIntInt(int,int) {
    
}
int IntAndIntInt(int,int) {
    return 0;
}

IOTEST(basic, VoidAndEmpty, 3, VoidAndEmpty, 4) {
    
}
IOTEST(basic, IntAndEmpty, 3, IntAndEmpty, 4) {
    SetReturn(0);
}
IOTEST(basic, VoidAndIntInt, 3, VoidAndIntInt, 4) {
    SetArguments(0,2);
}
IOTEST(basic, IntAndIntInt, 3, IntAndIntInt, 4) {
    SetArguments(0,2);
    SetReturn(0);
}


int IntAndEmpty2() {
    return 1;
}
void VoidAndIntInt2(int& a, int& b) {
    a = 3;
    b = 5;
}
int IntAndIntInt2(std::string,int b) {
    return b+1;
}

IOTEST(values, IntAndEmpty2, 3, IntAndEmpty2, 4) {
    SetReturn(1);
}
IOTEST(values, VoidAndIntInt2, 3, VoidAndIntInt2, 4) {
    SetArguments(1, 2);
    SetArgumentsAfter(3, 5);
}
IOTEST(values, IntAndIntInt2, 3, IntAndIntInt2, 4) {
    SetArguments("asd", (int)GetRunIndex()-1);
    SetReturn(GetRunIndex());
}
IOTEST(values, VoidAndIntInt2_fail, 3, VoidAndIntInt2, 4) {
    SetArguments(1, 2);
    SetArgumentsAfter(3, 4);
}
IOTEST(values, IntAndIntInt2_fail, 3, IntAndIntInt2, 4) {
    SetArguments("asd", (int)GetRunIndex());
    SetReturn(GetRunIndex());
}



int IntAndEmpty2AndWriteOut() {
    std::cout << "asd";
    return 1;
}
void VoidAndIntInt2AndWriteErr(int& a, int& b) {
    char c;
    std::cin >> c;
    std::cerr << c << "err";
    a = 3;
    b = 5;
}
int IntAndIntInt2AndWriteErrAndOut(std::string,int b) {
    std::string str;
    std::cin >> str;
    std::cout << str;
    std::cerr << str << "err";
    return b+1;
}

IOTEST(std, IntAndEmpty2AndWriteOut, 3, IntAndEmpty2AndWriteOut, 4) {
    SetReturn(1);
    SetOutput("asd");
}
IOTEST(std, VoidAndIntInt2AndWriteErr, 3, VoidAndIntInt2AndWriteErr, 4) {
    SetArguments(1, 2);
    SetArgumentsAfter(3, 5);
    SetInput("asd");
    SetError("aerr");
}
IOTEST(std, IntAndIntInt2AndWriteErrAndOut, 3, IntAndIntInt2AndWriteErrAndOut, 4) {
    SetArguments("asd", (int)GetRunIndex()-1);
    SetReturn(GetRunIndex());
    SetInput("asd", true);
    SetOutput("asd");
    SetError("asderr");
}
IOTEST(std, VoidAndIntInt2AndWriteErr_fail, 3, VoidAndIntInt2AndWriteErr, 4) {
    SetArguments(1, 2);
    SetArgumentsAfter(3, 5);
    SetInput("asd");
    SetError("aer");
}
IOTEST(std, IntAndIntInt2AndWriteErrAndOut_fail, 3, IntAndIntInt2AndWriteErrAndOut, 4) {
    SetArguments("asd", (int)GetRunIndex()-1);
    SetReturn(GetRunIndex());
    SetInput("asd", true);
    SetOutput("assd");
    SetError("asderr");
}