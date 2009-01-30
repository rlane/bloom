CFLAGS=-O3 -Wall

all: test test-big sweep

test: test.c bloom.c bloom.h
	$(CC) test.c bloom.c -lssl -o test

test-big: test-big.c bloom.c bloom.h
	$(CC) test-big.c bloom.c -lssl -o test-big

sweep: sweep.c bloom.c bloom.h
	$(CC) sweep.c bloom.c -o sweep

clean:
	rm -rf test test-big bench sweep
