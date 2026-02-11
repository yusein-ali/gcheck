MAIN_FILES = main.cpp main.o
SOURCES=main.cpp aviary.cpp
HEADERS= aviary.hpp duck.hpp owl.hpp bird.hpp
OBJECTS=$(SOURCES:cpp=o)
EXECNAME=main
CLEAN_FILES=out.txt