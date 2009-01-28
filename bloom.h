#ifndef BLOOM_H
#define BLOOM_H

typedef struct bloom
{
	int fd;
	void *map;
	size_t len;
} bloom_t;

int bloom_create(const char *fn, size_t len);
bloom_t *bloom_open(const char *fn);
void bloom_close(bloom_t *b);

#endif
