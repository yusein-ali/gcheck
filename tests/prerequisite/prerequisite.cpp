
#include <gcheck.h>
#include <function_test.h>

void Void() {}

FUNCTIONTEST(shouldbecalled, after_first_and_after_first, 1, Void, "first after_first") {

}
FUNCTIONTEST(shouldbecalled2, after_first_and_after_first, 1, Void, "shouldbecalled.first shouldbecalled.after_first") {

}
FUNCTIONTEST(shouldbecalled, after_first, 1, Void, "first") {

}
FUNCTIONTEST(shouldbecalled, after_first2, 1, Void, "shouldbecalled.first") {

}
FUNCTIONTEST(shouldbecalled, first, 1, Void) {

}
FUNCTIONTEST(shouldbecalled2, after_first, 1, Void, "shouldbecalled.first") {

}
FUNCTIONTEST(shouldntbecalled, after_first, 1, Void, "first") {

}
FUNCTIONTEST(shouldntbecalled, random, 1, Void, "asd") {

}