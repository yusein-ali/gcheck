CC=g++
binaries=testtest testtest-d
objects=gcheck.o gcheck_.o testtest.o utility.o
debug_objects=gcheck.od gcheck_.od testtest.od utility.od
std=-std=c++17

.PHONY: clean

testtest-run: testtest
	./testtest
	python3 beautify.py -o output.html

testtest-debug: testtest-d
	gdb ./testtest-d

testtest-d: gcheck_.od utility.od testtest.od
	$(CC) -g $(std) $^ -o $@

testtest: gcheck.o
	$(CC) $(std) $^ -o $@

gcheck.o: gcheck_.o utility.o testtest.o
	ld -r gcheck_.o utility.o testtest.o -o gcheck.o
	
gcheck_.od: gcheck.cpp gcheck.h
	$(CC) -g $(std) -Wall -c $< -o $@

gcheck_.o: gcheck.cpp gcheck.h
	$(CC) $(std) -Wall  -c $< -o $@

%.od: %.cpp gcheck.h
	$(CC) -g $(std) -Wall -c $< -o $@

%.o: %.cpp gcheck.h
	$(CC) $(std) -Wall  -c $< -o $@

clean:
	rm $(objects) $(debug_objects) $(binaries) output.html report.json > /dev/null 2>&1