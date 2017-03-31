LIBPATH = -L../../src
INCLUDE_PATH = -I../../src
CXX = g++
CXXFLAGS = $(INCLUDE_PATH) -Wall -Wextra -O2 -std=c++14
CXXLIBS = $(LIBPATH) -lisomatch
LEX = flex
YACC = bison
YACCFLAGS =

TARGET = langtest
PARSER = parser.y
OBJS = main.o parser.tab.o lexer.yy.o parseTools.o

all: $(PARSER:.y=.tab.cpp) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(CXXLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.yy.cpp: %.l $(PARSER:.y=.tab.hpp)
	$(LEX) -o $@ $<

%.tab.cpp: %.y
	$(YACC) $(YACCFLAGS) -d -o $@ $<

%.tab.hpp: %.y
	$(YACC) $(YACCFLAGS) -d -o $(@:.hpp=.cpp) $<

clean:
	rm -rf $(TARGET) *.o *.yy.{cpp,c} *.tab.{cpp,c,h,hpp}

.PHONY: all clean
