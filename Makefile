CXX = g++
CXXFLAGS = -std=c++0x -Wall -lm -g -pg

BINARY := main
SOURCES := $(wildcard *.cpp)
HEADERS := $(wildcard *.h)
OBJS := ${SOURCES:.cpp=.o}

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clang:
	clang++ $(CXXFLAGS) -o $(BINARY) $(SOURCES)

clean:
	rm -fr $(OBJS) $(BINARY)
