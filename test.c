#include <stdio.h>
#include <stdlib.h>
#include "bloom.h"

int main(int argc, char** argv)
{
	bloom_t *b;
	char *fn = "/tmp/test_bloom_filter";

	bloom_create(fn, 4096 * 128);
	b = bloom_open(fn);
	printf("loaded bloom filter of size %d\n", b->len);
	bloom_close(b);

	return 0;
}
