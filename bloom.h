#ifndef BLOOM_H
#define BLOOM_H

typedef struct bloom
{
	int fd;
	unsigned char *map;
	size_t len;
} bloom_t;

int bloom_create(const char *fn, size_t len);
bloom_t *bloom_open(const char *fn);
void bloom_insert(bloom_t *b, const char *key, size_t key_len);
int bloom_check(bloom_t *b, char *key, size_t key_len);
void bloom_close(bloom_t *b);

#endif
