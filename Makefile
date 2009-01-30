CFLAGS=-O3 -Wall

all: test test-big test-multi sweep

test: test.c bloom.c bloom.h
	$(CC) test.c bloom.c -lssl -o test

test-big: test-big.c bloom.c bloom.h
	$(CC) test-big.c bloom.c -lssl -o test-big

test-multi: test-multi.c bloom.c bloom.h
	$(CC) test-multi.c bloom.c -lssl -o test-multi

sweep: sweep.c bloom.c bloom.h
	$(CC) sweep.c bloom.c -o sweep

clean:
	rm -rf test test-big test-multi bench sweep
