#include <gcheck/gcheck.h>
#include <gcheck/function_test.h>

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

FUNCTIONTEST(basic, VoidAndEmpty, 3, VoidAndEmpty, 4) {

}
FUNCTIONTEST(basic, IntAndEmpty, 3, IntAndEmpty, 4) {
    SetReturn(0);
}
FUNCTIONTEST(basic, VoidAndIntInt, 3, VoidAndIntInt, 4) {
    SetArguments(0,2);
}
FUNCTIONTEST(basic, IntAndIntInt, 3, IntAndIntInt, 4) {
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
int IntAndIntInt2(int,int b) {
    return b+1;
}

FUNCTIONTEST(values, IntAndEmpty2, 3, IntAndEmpty2, 4) {
    SetReturn(1);
}
FUNCTIONTEST(values, VoidAndIntInt2, 3, VoidAndIntInt2, 4) {
    SetArguments(1, 2);
    SetArgumentsAfter(3, 5);
}
FUNCTIONTEST(values, IntAndIntInt2, 3, IntAndIntInt2, 4) {
    SetArguments(2, (int)GetRunIndex()-1);
    SetReturn(GetRunIndex());
}
FUNCTIONTEST(values, VoidAndIntInt2_fail, 3, VoidAndIntInt2, 4) {
    SetArguments(1, 2);
    SetArgumentsAfter(3, 4);
}
FUNCTIONTEST(values, IntAndIntInt2_fail, 3, IntAndIntInt2, 4) {
    SetArguments(2, (int)GetRunIndex());
    SetReturn(GetRunIndex());
}