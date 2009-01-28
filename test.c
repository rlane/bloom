#include <stdio.h>
#include <stdlib.h>
#include "bloom.h"

int main(int argc, char** argv)
{
	bloom_t *b;

	b = bloom_open("/tmp/test_bloom_filter");
	bloom_close(b);

	return 0;
}
