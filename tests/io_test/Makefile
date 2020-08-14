include ../../vars.make
include ../vars.make

EXECNAME=io_test
SOURCES=io_test.cpp
HEADERS=

OBJECTS=$(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I$(GCHECK_INCLUDE_DIR)
CPPFLAGS = -L$(GCHECK_LIB_DIR) -l$(GCHECK_LIB)

ifeq ($(OS),Windows_NT)
	RM=del /f /q
	EXECUTABLE = $(BIN_DIR)/$(EXECNAME).exe
else
	RM=rm -f
	EXECUTABLE = $(BIN_DIR)/$(EXECNAME)
endif

.PHONY: clean all clean-all debug set-debug run $(GCHECK_LIB_DIR)/lib$(GCHECK_LIB).a

all: | $(EXECUTABLE) run

run:
	$(EXECUTABLE)

debug: | set-debug $(EXECUTABLE)

set-debug:
	$(eval CXXFLAGS += -g)

$(BUILD_DIR)/%.o : %.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE): $(GCHECK_LIB_DIR)/lib$(GCHECK_LIB).a $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE) output.html report.json

clean-all: clean
	$(MAKE) -C $(GCHECK_DIR)/ clean

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
$(BIN_DIR):
	mkdir $(BIN_DIR)

$(GCHECK_LIB_DIR)/lib$(GCHECK_LIB).a:
	$(MAKE) -C $(GCHECK_DIR)/ debug

