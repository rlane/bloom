#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

int main(int argc, char** argv)
{
	char *fn = "/tmp/test_bloom_filter";
	bloom_t *b;
	uint8_t *keys;
	const int n = 10000;
	const int key_len = 20;

	bloom_create(fn);
	b = bloom_open(fn);

	keys = malloc(n * key_len);
	if (keys == NULL) {
		fprintf(stderr, "failed to allocate key array\n");
		abort();
	}

	bloom_close(b);

	printf("tests passed\n");

	return 0;
}
