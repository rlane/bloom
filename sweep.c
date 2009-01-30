#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

int main(int argc, char** argv)
{
	bloom_t *b;
	unsigned long i = 0;
	unsigned int us;
	char *fn;
	char *p;

	if (argc != 3) {
		printf("usage: %s <filename> <sleep time (us)>\n", argv[0]);
		exit(1);
	}

	fn = argv[1];
	us = strtol(argv[2], &p, 0);

	if (p == argv[2] || *p != 0) {
		printf("failed to parse sleep time\n");
		exit(1);
	}

	b = bloom_open(fn);
	while (1) {
		bloom_sweep(b, i);
		usleep(us);
		i++;
	}
}
