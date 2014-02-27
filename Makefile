CXX = clang++
CXXFLAGS = -Ofast -std=c++0x -Wall -flto -funroll-loops -lm

all:
	$(CXX) $(CXXFLAGS) -o main main.cpp magic.cpp

test:
	$(CXX) $(CXXFLAGS) -o main main.cpp magic.cpp test.cpp -DTEST && ./main

clean:
	rm -fr main
