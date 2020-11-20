include vars.make

GCHECK_INCLUDE_DIR:=$(GCHECK_INCLUDE_DIR)/gcheck

GCHECK_SOURCES=gcheck.cpp user_object.cpp redirectors.cpp json.cpp console_writer.cpp argument.cpp stringify.cpp shared_allocator.cpp multiprocessing.cpp customtest.cpp
GCHECK_OBJECTS=$(GCHECK_SOURCES:cpp=o)

SOURCES=$(GCHECK_SOURCES:%=src/%)
HEADERS=$(GCHECK_HEADERS:%=$(GCHECK_INCLUDE_DIR)/%) src/console_writer.h
OBJECTS:=$(GCHECK_OBJECTS:%=build/%)
PIC_OBJECTS:=$(OBJECTS:o=pic.o)

LIBNAME=$(GCHECK_LIB_NAME)

CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I$(GCHECK_INCLUDE_DIR) -Isrc

ifeq ($(OS),Windows_NT)
	RM=del /f /q
	FixPath = $(subst /,\,$1)
else
	RM=rm -f
	FixPath = $1
endif

.PHONY: clean all debug set-debug shared

all: $(GCHECK_LIB_DIR)/$(LIBNAME)

with-construct: | set-construct $(GCHECK_LIB_DIR)/$(LIBNAME)

debug-construct: | set-debug with-construct

debug: | set-debug $(GCHECK_LIB_DIR)/$(LIBNAME)

shared: $(GCHECK_LIB_DIR)/$(GCHECK_SHARED_LIB_NAME)

set-construct:
	$(eval CPPFLAGS += -DGCHECK_CONSTRUCT_DATA)

set-debug:
	$(eval CXXFLAGS += -g)

build/%.o : src/%.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/%.pic.o : src/%.cpp $(HEADERS) | build
	$(CXX) -fPIC $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(GCHECK_LIB_DIR)/$(LIBNAME): | $(OBJECTS) $(GCHECK_LIB_DIR)
	ar rcs $(call FixPath, $@ $(OBJECTS))

$(GCHECK_LIB_DIR)/$(GCHECK_SHARED_LIB_NAME): $(PIC_OBJECTS) | $(GCHECK_LIB_DIR)
	$(CXX) -shared $(CPPFLAGS) $(CXXFLAGS) $(PIC_OBJECTS) -o $@

get-report: $(EXECUTABLE)
	$(call FixPath, ./$(EXECUTABLE)) --json 2>&1

clean:
	$(RM) $(call FixPath, build/* $(GCHECK_LIB_DIR)/* $(EXECUTABLE) output.html report.json)

build:
	mkdir build

$(GCHECK_LIB_DIR):
	mkdir $(GCHECK_LIB_DIR)