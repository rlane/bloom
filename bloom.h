#include <stdint.h>

#ifndef BLOOM_H
#define BLOOM_H

typedef struct bloom
{
	int fd;
	uint8_t *map;
} bloom_t;

int bloom_create(const char *fn);
bloom_t *bloom_open(const char *fn);
void bloom_insert(bloom_t *b, const uint8_t *key, size_t key_len);
int bloom_check(bloom_t *b, const uint8_t *key, size_t key_len);
void bloom_close(bloom_t *b);

#endif
