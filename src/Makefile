CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -Werror -std=c++14
CXXLIBS =
AR = ar
DOXYGEN = doxygen
DOXYGEN_FLAGS =

NAME = isomatch
TARGET = lib$(NAME).a
OBJS = \
	   wireId.o \
	   wireManager.o \
	   dotPrint.o \
	   circuitTree.o \
	   circuitGroup.o \
	   circuitTristate.o \
	   circuitDelay.o \
	   circuitAssert.o \
	   circuitComb.o

###############################################################################

all: $(TARGET) docs

$(TARGET): $(OBJS)
	$(AR) rs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXXLIBS) -o $@ -c $<

###############################################################################

docs: Doxyfile
	$(DOXYGEN) $(DOXYGEN_FLAGS)

###############################################################################

clean:
	rm -f *.o
	rm -f $(TARGET)

###############################################################################

.PHONY: all clean
