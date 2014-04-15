CXX = gcc
#CXXFLAGS = -std=c++0x -Wall -g -pg -Ofast
CXXFLAGS = -std=c99 -Wall -g -pg -Ofast
LIBS = -lm

BINARY := main
SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)
OBJS := ${SOURCES:.c=.o}

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

debug:
	$(CXX) $(CXXFLAGS) $(LIBS) -o $(BINARY) $(SOURCES) -DDEBUG

clang:
	clang++ $(CXXFLAGS) $(LIBS) -o $(BINARY) $(SOURCES)

clean:
	rm -fr $(OBJS) $(BINARY) benchmarks/*.{log,tmp,mag,svg,png,jpg}

#
# BENCHMARKS
#

BENCHMARKS = 1 2 3 4 5 6 7 8 9 10
BENCHMARKS_LOG = $(patsubst %, benchmarks/%.log, $(BENCHMARKS))

benchmarks: $(BENCHMARKS_LOG)

benchmarks/%.log: benchmarks/% $(BINARY)
	./main $< > $@.tmp && mv $@.tmp $@
