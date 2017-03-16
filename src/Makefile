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
	   wireManager.o

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
