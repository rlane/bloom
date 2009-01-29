#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

bloom_t *b;

static void insert(char *key)
{
	bloom_insert(b, key, strlen(key));
}

static void check(char *key)
{
	if (bloom_check(b, key, strlen(key)) != 1) {
		printf("did not find key %s\n", key);
	}
}

static void checknot(char *key)
{
	if (bloom_check(b, key, strlen(key)) != 0) {
		printf("unexpectedly found key %s\n", key);
	}
}

static void nic(char *key)
{
	checknot(key);
	insert(key);
	check(key);
}

static void ic(char *key)
{
	insert(key);
	check(key);
}

int main(int argc, char** argv)
{
	char *key;
	char *fn = "/tmp/test_bloom_filter";

	bloom_create(fn);
	b = bloom_open(fn);

	nic("foo1234567");
	ic("foo1234567");
	nic("bazhydyusfhkjds");
	ic("bazhydyusfhkjds");
	check("foo1234567");

	bloom_close(b);

	printf("tests passed\n");

	return 0;
}
