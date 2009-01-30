CFLAGS=-O2

all: test test-big

test: test.c bloom.c bloom.h
	$(CC) test.c bloom.c -lssl -o test

test-big: test-big.c bloom.c bloom.h
	$(CC) test-big.c bloom.c -lssl -o test-big

clean:
	rm -rf test test-big bench
