CXX = g++
CXXFLAGS = -std=c++0x -Wall -g -pg -Ofast
LIBS = -lm

BINARY := main
SOURCES := $(wildcard *.cpp)
HEADERS := $(wildcard *.h)
OBJS := ${SOURCES:.cpp=.o}

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clang:
	clang++ $(CXXFLAGS) $(LIBS) -o $(BINARY) $(SOURCES)

clean:
	rm -fr $(OBJS) $(BINARY)
