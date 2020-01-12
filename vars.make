GCHECK_HEADERS=gcheck.h user_object.h argument.h redirectors.h json.h sfinae.h
GCHECK_INCLUDE_DIR=include/gcheck
GCHECK_LIB_DIR=lib

ifeq ($(OS),Windows_NT)
    GCHECK_LIB=gcheck.lib
else
    GCHECK_LIB=gcheck
endif