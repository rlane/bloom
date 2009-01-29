#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include "bloom.h"

void *fill_random(uint8_t *p, int n)
{
	RAND_pseudo_bytes(p, n);
}

int main(int argc, char** argv)
{
	char *fn = "/tmp/test_bloom_filter";
	bloom_t *b;
	uint8_t *keys;
	const int n = 1000000;
	const int key_len = 20;
	int i;
	int stat_before = 0;

	fprintf(stderr, "allocating keys\n");
	keys = malloc(n * key_len);
	if (keys == NULL) {
		fprintf(stderr, "failed to allocate key array\n");
		abort();
	}

	fprintf(stderr, "generating keys\n");
	fill_random(keys, n * key_len);

	fprintf(stderr, "creating filter\n");
	bloom_create(fn);
	b = bloom_open(fn);

	fprintf(stderr, "inserting\n");
	for (i=0; i<n; i++) {
		uint8_t *k = keys + (i*key_len);
		int found = bloom_check(b, k, key_len);
		stat_before += found;
		bloom_insert(b, k, key_len);
	}

	fprintf(stderr, "checking\n");
	for (i=0; i<n; i++) {
		uint8_t *k = keys + (i*key_len);
		int found = bloom_check(b, k, key_len);
		if (!found) {
			fprintf(stderr, "missing key %d\n", i);
			abort();
		}
	}

	bloom_close(b);

	printf("tests passed\n");
	printf("before=%d\n", stat_before);

	return 0;
}
