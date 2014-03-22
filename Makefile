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
	rm -fr $(OBJS) $(BINARY) benchmarks/*.{log,tmp,mag,svg,png}

#
# BENCHMARKS
#

BENCHMARKS = 1 2 3 4 5 6 7 8 9 10
BENCHMARKS_LOG = $(patsubst %, benchmarks/%.log, $(BENCHMARKS))

benchmarks: $(BENCHMARKS_LOG)

benchmarks/%.log: benchmarks/% $(BINARY)
	./main $< > $@.tmp && mv $@.tmp $@

#
# PNG
#

BENCHMARKS_PNG = $(patsubst benchmarks/%.svg, benchmarks/%.png, $(wildcard benchmarks/*.svg))

pngs: $(BENCHMARKS_PNG)

benchmarks/%.png: benchmarks/%.svg
	convert -trim $< $@

