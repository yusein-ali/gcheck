EXECUTABLES=testtest
SOURCES=gcheck.cpp  json.cpp
HEADERS=gcheck.h utility.h json.h argument.h
OBJECTS=gcheck.o gcheck_.o testtest.o json.o
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

.PHONY: clean

all: testtest gcheck.o

run: testtest
	./testtest --json report.json
	python3 beautify.py -o output.html

testtest: gcheck.o testtest.o
	$(CXX) $(CPPFLAGS) $(LOADLIBES) $(LDLIBS) $^ -o $@

gcheck.o: gcheck_.o utility.o
	ld -r gcheck_.o utility.o -o gcheck.o
	
gcheck_.o: gcheck.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

get-report: testtest
	./testtest --json 2>&1
	
clean:
	rm -f $(OBJECTS) $(EXECUTABLES) output.html report.json