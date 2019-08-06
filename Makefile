include vars.make

EXECNAME=testtest
SOURCES=$(GCHECK_SOURCES)
HEADERS=$(GCHECK_HEADERS)
OBJECTS=testtest.o $(GCHECK_OBJECTS)
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

ifeq ($(OS),Windows_NT)
	RM=del /f /q
	EXECUTABLE:=$(EXECNAME).exe
else
	RM=rm -f
	EXECUTABLE:=$(EXECNAME)
endif

.PHONY: clean all debug set-debug

all: $(EXECUTABLE) gcheck.o

run: $(EXECUTABLE)
	./$(EXECUTABLE) --json report.json
	python3 beautify.py -o output.html

debug: set-debug $(EXECUTABLE)

set-debug:
	$(eval CXXFLAGS += -g)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LOADLIBES) $(LDLIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

get-report: $(EXECUTABLE)
	./$(EXECUTABLE) --json 2>&1
	
clean:
	$(RM) *.o $(EXECNAME) $(EXECNAME).exe output.html report.json