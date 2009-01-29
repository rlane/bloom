#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

int main(int argc, char** argv)
{
	bloom_t *b;
	char *fn = "/tmp/test_bloom_filter";

	bloom_create(fn, 4096 * 128);
	b = bloom_open(fn);
	printf("loaded bloom filter of size %d\n", b->len);

	char *key = "foo1234567";
	bloom_insert(b, key, strlen(key));
	if (bloom_check(b, key, strlen(key)) != 1) printf("lookup of %s failed\n", key);;

	bloom_close(b);

	return 0;
}
