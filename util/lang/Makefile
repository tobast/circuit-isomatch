LIBPATH = -L../../src
INCLUDE_PATH = -I../../src
CXX = g++
CXXFLAGS = $(INCLUDE_PATH) -Wall -Wextra -O2 -std=c++14
CXXLIBS = $(LIBPATH) -lisomatch
LEX = flex
YACC = bison

TARGET = langtest
PARSER = parser.y
OBJS = main.o parser.tab.o lexer.yy.o parseTools.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(CXXLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.yy.cpp: %.l $(PARSER:.y=.tab.hpp)
	$(LEX) -o $@ $<

%.tab.cpp %.tab.hpp: %.y
	$(YACC) -d -o $@ $<

clean:
	rm -rf $(TARGET) *.o *.yy.{cpp,c} *.tab.{cpp,c,h,hpp}

.PHONY: all clean
