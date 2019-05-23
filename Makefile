CC=g++
objects=gcheck.o testtest.o utility.o
debug_objects=gcheck.od testtest.od utility.od
std=-std=c++17

.PHONY: clean beautify run-testtest

testtest-run: testtest run-testtest beautify
run-testtest:
	./testtest
	
testtest-debug: testtest-d
	gdb ./testtest-d

beautify:
	python3 beautify.py $(report) $(out)

testtest-d: gcheck.od utility.od testtest.od
	$(CC) -g $(std) $^ -o $@

testtest: gcheck.o utility.o testtest.o
	$(CC) $(std) $^ -o $@

%.od: %.cpp gcheck.h
	$(CC) -g $(std) -Wall -c $< -o $@

%.o: %.cpp gcheck.h
	$(CC) $(std) -Wall  -c $< -o $@

clean:
	rm $(objects) $(debug_objects) testtest testtest-d output.html report.json > /dev/null 2>&1