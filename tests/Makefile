include ../vars.make

EXECNAME=test
SOURCES=testtest.cpp
HEADERS=
OBJECTS=$(SOURCES:%.cpp=build/%.o)
GCHECK_DIR=..

GCHECK_LIB_DIR:=$(GCHECK_DIR)/$(GCHECK_LIB_DIR)
GCHECK_INCLUDE_DIR:=$(GCHECK_DIR)/$(GCHECK_INCLUDE_DIR)

CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I$(GCHECK_INCLUDE_DIR)
CPPFLAGS = -L$(GCHECK_LIB_DIR) -l$(GCHECK_LIB)

ifeq ($(OS),Windows_NT)
	RM=del /f /q
	EXECUTABLE = bin/$(EXECNAME).exe
else
	RM=rm -f
	EXECUTABLE = bin/$(EXECNAME)
endif

.PHONY: clean all debug set-debug

all: $(EXECUTABLE)

debug: | set-debug $(EXECUTABLE)

set-debug:
	$(eval CXXFLAGS += -g)

build/%.o : %.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJECTS) $(GCHECK_LIB_DIR)/libgcheck.a | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) -o $@

clean:
	$(RM) build/* $(EXECUTABLE) output.html report.json
	
build:
	mkdir build
bin:
	mkdir bin

$(GCHECK_LIB_DIR)/libgcheck.a:
	$(MAKE) -C ../