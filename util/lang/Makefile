CHANGING_LIBS = ../../src/libisomatch.a
LIBPATH = -L../../src
INCLUDE_PATH = -I../../src
CXX = g++
CXXFLAGS = $(INCLUDE_PATH) -Wall -Wextra -O0 -g -std=c++14
CXXLIBS = $(LIBPATH) -lisomatch
LEX = flex
YACC = bison
YACCFLAGS =

PARSER = parser.y
MAINS = dot sig serialize
OBJS = aux.o parser.tab.o lexer.yy.o parseTools.o

##########################################################################

BINS = $(addsuffix .bin,$(MAINS))

all: build test

build: $(BINS)

%.bin: main_%.o $(OBJS) $(CHANGING_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ main_$*.o $(OBJS) $(CXXLIBS)

aux.o: aux.cpp parser.tab.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.yy.cpp: %.l $(PARSER:.y=.tab.hpp)
	$(LEX) -o $@ $<

%.tab.cpp: %.y
	$(YACC) $(YACCFLAGS) -d -o $@ $<

%.tab.hpp: %.y
	$(YACC) $(YACCFLAGS) -d -o $(@:.hpp=.cpp) $<

clean:
	rm -rf *.bin *.o *.yy.{cpp,c} *.tab.{cpp,c,h,hpp}

test: sig.bin dot.bin serialize.bin
	./run_sigtests.py
	./dot.bin circ/processor.circ > /dev/null
	valgrind -q ./dot.bin circ/processor.circ > /dev/null
	./sig.bin circ/processor.circ > /dev/null
	valgrind -q ./sig.bin circ/processor.circ > /dev/null
	./serialize.bin circ/processor.circ > /dev/null
	@echo -e "\e[92m>> All tests passed! :)\e[0m"

.PHONY: all build clean test
