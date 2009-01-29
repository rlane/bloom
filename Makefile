test: test.c bloom.c bloom.h
	$(CC) test.c bloom.c -lssl -o test

clean:
	rm -rf test
