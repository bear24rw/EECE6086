CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -pg -Ofast -Wno-unused-result
LIBS = -lm -lpthread

SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)

all: tc cc

tc: tc.o tc_heur.o flags.o shared.o mem_log.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

cc: cc.o cc_heur.o flags.o shared.o mem_log.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr *.o tc cc benchmarks/*.{log,tmp,png,jpg,mem,dep,heur,tc,cc}

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

benchmarks/%.log: benchmarks/%.txt tc cc
	@touch $@
	@echo "TC: "$<
	@-(/usr/bin/time -v ./tc    -m $<) &> $@.tc      ; mv /tmp/mem.log $@.tc.mem      && mv /tmp/dep.log $@.tc.dep
	@#-(/usr/bin/time -v ./tc -h -m $<) &> $@.tc.heur ; mv /tmp/mem.log $@.tc.heur.mem && mv /tmp/dep.log $@.tc.heur.dep
	@#-(/usr/bin/time -v ./tc -f -m $<) &> $@.tc.flag ; mv /tmp/mem.log $@.tc.flag.mem && mv /tmp/dep.log $@.tc.flag.dep
	@echo "CC: "$<
	@-(/usr/bin/time -v ./cc    -m $<) &> $@.cc      ; mv /tmp/mem.log $@.cc.mem       && mv /tmp/dep.log $@.cc.dep
	@#-(/usr/bin/time -v ./cc -h -m $<) &> $@.cc.heur ; mv /tmp/mem.log $@.cc.heur.mem  && mv /tmp/dep.log $@.cc.heur.dep
	@#-(/usr/bin/time -v ./cc -f -m $<) &> $@.cc.flag ; mv /tmp/mem.log $@.cc.flag.mem  && mv /tmp/dep.log $@.cc.flag.dep

pngs: $(BENCHMARKS_PNG)

benchmarks/%.png: benchmarks/%.log
	@echo "Plot: "$<
	@-gnuplot -e "name='TC: $*'" -e "memfile='$<.tc.mem'" -e "depthfile='$<.tc.dep" -e "titlename='Either'" -e "outfilename='$<.tc.png'" plot.gpi
	@-gnuplot -e "name='CC: $*'" -e "memfile='$<.cc.mem'" -e "depthfile='$<.cc.dep" -e "titlename='Either'" -e "outfilename='$<.cc.png'" plot.gpi
	@#echo "Heur Plot: "$<
	@#-gnuplot -e "name='TC: $*'" -e "memfile='$<.tc.heur.mem'" -e "depthfile='$<.tc.heur.dep" -e "titlename='Either'" -e "outfilename='$<.tc.heur.png'" plot.gpi
	@#-gnuplot -e "name='CC: $*'" -e "memfile='$<.cc.heur.mem'" -e "depthfile='$<.cc.heur.dep" -e "titlename='Either'" -e "outfilename='$<.cc.heur.png'" plot.gpi

	@#gnuplot -e "filename='$<.heur.mem'" -e "titlename='Heur'" 	-e "outfilename='benchmarks/$*.heur.png'" plot.gpi
	@#gnuplot -e "filename='$<.flag.mem'" -e "titlename='Both'" 	-e "outfilename='benchmarks/$*.flag.png'" plot.gpi
	@#gnuplot -e "either='$<.mem'" \
#			-e "heur='$<.heur.mem'" \
#			-e "flag='$<.flag.mem'" \
#			-e "titlename='All'" \
#			-e "outfilename='benchmarks/$*.all.png'" \
#			plot_all.gpi

