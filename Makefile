test: test.c bloom.c bloom.h
	$(CC) test.c bloom.c -o test

clean:
	rm -rf test
