include vars.make

GCHECK_SOURCES=gcheck.cpp user_object.cpp redirectors.cpp json.cpp console_writer.cpp argument.cpp stringify.cpp
GCHECK_OBJECTS=$(GCHECK_SOURCES:cpp=o)

SOURCES=$(GCHECK_SOURCES:%=src/%)
HEADERS=$(GCHECK_HEADERS:%=$(GCHECK_INCLUDE_DIR)/%) src/console_writer.h
OBJECTS:=$(GCHECK_OBJECTS:%=build/%)

CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I$(GCHECK_INCLUDE_DIR) -Isrc

ifeq ($(OS),Windows_NT)
	RM=del /f /q
	LIBNAME=$(GCHECK_LIB)
else
	RM=rm -f
	LIBNAME=lib$(GCHECK_LIB).a
endif

.PHONY: clean all debug set-debug

all: $(GCHECK_LIB_DIR)/$(LIBNAME)

debug: | set-debug $(GCHECK_LIB_DIR)/$(LIBNAME)

set-debug:
	$(eval CXXFLAGS += -g)

build/%.o : src/%.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(GCHECK_LIB_DIR)/$(LIBNAME): $(OBJECTS) | $(GCHECK_LIB_DIR)
	ar rcs $@ $(OBJECTS)

get-report: $(EXECUTABLE)
	./$(EXECUTABLE) --json 2>&1

clean:
	$(RM) build/* $(GCHECK_LIB_DIR)/* $(EXECUTABLE) output.html report.json
	
build:
	mkdir build
$(GCHECK_LIB_DIR):
	mkdir $(GCHECK_LIB_DIR)