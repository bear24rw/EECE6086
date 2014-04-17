CC = gcc
CFLAGS = -std=c99 -Wall -g -pg -Ofast -Wno-unused-result
LIBS = -lm -lpthread

BINARY := main
SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)

all: tc cc

tc: tc.o heur.o flags.o shared.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

cc: cc.o shared.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr *.o tc cc benchmarks/*.{log,tmp,png,jpg}

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

benchmarks/%.log: benchmarks/%.txt $(BINARY)
	(time -v ./main $<) &> $@.tmp && mv $@.tmp $@ #&& mv /tmp/mem_log.txt $@.mem
	#(time -v ./main -h $<) &> $@.tmp && mv $@.tmp $@.heur && mv /tmp/mem_log.txt $@.heur.mem
	#(time -v ./main -f $<) &> $@.tmp && mv $@.tmp $@.flag && mv /tmp/mem_log.txt $@.flag.mem

pngs: $(BENCHMARKS_PNG)

benchmarks/%.png: benchmarks/%.log
	gnuplot -e "filename='$<.mem'"      -e "titlename='Either'" -e "outfilename='benchmarks/$*.png'" plot.gpi
	gnuplot -e "filename='$<.heur.mem'" -e "titlename='Heur'" 	-e "outfilename='benchmarks/$*.heur.png'" plot.gpi
	gnuplot -e "filename='$<.flag.mem'" -e "titlename='Both'" 	-e "outfilename='benchmarks/$*.flag.png'" plot.gpi
	gnuplot -e "either='$<.mem'" \
			-e "heur='$<.heur.mem'" \
			-e "flag='$<.flag.mem'" \
			-e "titlename='All'" \
			-e "outfilename='benchmarks/$*.all.png'" \
			plot_all.gpi

