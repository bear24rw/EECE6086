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

BENCHMARKS = 1 2 3 4 5 6 7 8 9 10
BENCHMARK_RESULTS = $(patsubst %, benchmarks/%.log, $(BENCHMARKS))
benchmarks: $(BINARY) $(BENCHMARK_RESULTS)
benchmarks/%.log: benchmarks/%
	./main $< > $@.tmp && mv $@.tmp $@

clean:
	rm -fr $(OBJS) $(BINARY) benchmarks/*.{log,tmp}
