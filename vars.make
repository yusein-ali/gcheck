GCHECK_HEADERS=gcheck.h user_object.h argument.h redirectors.h json.h sfinae.h stringify.h macrotools.h function_test.h io_test.h ptr_tools.h
GCHECK_INCLUDE_DIR=include/gcheck
GCHECK_LIB_DIR=lib

ifeq ($(OS),Windows_NT)
    GCHECK_LIB=gcheck.lib
else
    GCHECK_LIB=gcheck
endif