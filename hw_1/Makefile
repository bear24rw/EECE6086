CXX = g++
CXXFLAGS = -Ofast -std=c++0x -Wall -flto -funroll-loops

all: default

default:
	$(CXX) $(CXXFLAGS) -march=native -o parprog parprog.cpp

cheat:
	$(CXX) $(CXXFLAGS) -march=native -o parprog parprog.cpp -DCHEAT

generic:
	$(CXX) $(CXXFLAGS) -mtune=generic -o parprog parprog.cpp -DGENERIC

clang:
	clang++ -O3 -std=c++11 -g -Wall -o parprog parprog.cpp

clean:
	rm -f parprog

profile:
	sudo opcontrol --deinit
	sudo opcontrol --separate=kernel
	sudo opcontrol --init
	sudo opcontrol --reset
	sudo opcontrol --start
	sudo ./parprog < BM/B4
	sudo opcontrol --stop
	sudo opreport -l ./parprog

# ==========================
# 		 BENCHMARKS
# ==========================

BENCHMARKS = $(wildcard BM/B*)
BENCHMARK_RESULTS = $(patsubst BM/B%,BM/R%,$(BENCHMARKS))

benchmarks: parprog $(BENCHMARK_RESULTS)

BM/R%: BM/B%
	./parprog < $< > $@.tmp && mv $@.tmp $@

# ==========================
# 			TESTS
# ==========================

TESTS = $(wildcard tests/B*)
TEST_RESULTS = $(patsubst tests/B%,tests/R%,$(TESTS))

run_tests: $(TEST_RESULTS)

tests/R%: tests/B%
	./parprog < $< > $@.tmp && mv $@.tmp $@

plot_tests:
	cd tests && python process.py
	cd tests && gnuplot plot.gpi
