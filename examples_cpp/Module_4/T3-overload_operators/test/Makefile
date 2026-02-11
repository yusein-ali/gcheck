include ../src/vars.make

TESTNAME = test
TEST_SOURCE = test_source.cpp

CPPFLAGS += -I../src/

SOURCES := $(filter-out $(MAIN_FILES), $(SOURCES))
OBJECTS := $(filter-out $(MAIN_FILES), $(OBJECTS))
HEADERS := $(filter-out $(MAIN_FILES), $(HEADERS))

include ../../../_scripts/test.make