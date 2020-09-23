GCHECK_HEADERS=gcheck.h user_object.h argument.h redirectors.h json.h sfinae.h stringify.h macrotools.h function_test.h io_test.h ptr_tools.h method_test.h method_io_test.h deleter.h multiprocessing.h customtest.h
GCHECK_INCLUDE_DIR=include
GCHECK_LIB_DIR=lib

ifeq ($(OS),Windows_NT)
    GCHECK_LIB=gcheck.lib
    GCHECK_LIB_NAME=gcheck.lib
else
    GCHECK_LIB=gcheck
    GCHECK_LIB_NAME=libgcheck.a
endif