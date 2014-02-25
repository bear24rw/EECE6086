all:
	clang -O3 -Wall -std=c99 -lm main.c -o main

test:
	clang -O3 -Wall -std=c99 -lm main.c test.c -o main -DTEST && ./main

clean:
	rm -fr main
