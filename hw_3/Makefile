CXX = gcc
CXXFLAGS = -std=c99 -Wall -g -pg -Ofast
LIBS = -lm -lpthread

BINARY := main
SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)
OBJS := ${SOURCES:.c=.o}

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

debug:
	$(CXX) $(CXXFLAGS) -o $(BINARY) $(SOURCES) $(LIBS) -DDEBUG

clang:
	clang++ $(CXXFLAGS) -o $(BINARY) $(SOURCES) $(LIBS)

clean:
	rm -fr $(OBJS) $(BINARY) benchmarks/*.{log,tmp,png,jpg}

#
# BENCHMARKS
#

BENCHMARKS = $(wildcard benchmarks/*.txt)
BENCHMARKS_LOG = $(patsubst %.txt, %.log, $(BENCHMARKS))
BENCHMARKS_PNG = $(patsubst %.txt, %.png, $(BENCHMARKS))

BENCHMARKS_BIG = $(wildcard benchmarks/big/*.txt)
BENCHMARKS_BIG_LOG = $(patsubst %.txt, %.log, $(BENCHMARKS_BIG))
BENCHMARKS_BIG_PNG = $(patsubst %.txt, %.png, $(BENCHMARKS_BIG))

benchmarks: $(BENCHMARKS_LOG)
benchmarks_big: $(BENCHMARKS_BIG)

benchmarks/%.log: benchmarks/%.txt
	./main $< > $@.tmp && mv $@.tmp $@ && mv /tmp/mem_log.txt $@.mem

pngs: $(BENCHMARKS_PNG)

benchmarks/%.png: benchmarks/%.log
	gnuplot -e "filename='$<.mem'" -e "outfilename='$@'" plot.gpi
