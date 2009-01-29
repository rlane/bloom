#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include "bloom.h"

#define PAGE_SIZE 4096
#define TABLE_SIZE (1<<27)
#define BYTE_SIZE 8
#define TABLE_BYTES TABLE_SIZE/BYTE_SIZE
#define NUM_IDXS 4

int bloom_create(const char *fn)
{
	int fd = creat(fn, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		perror("creat");
		abort();
	}

	lseek(fd, TABLE_BYTES, SEEK_SET);

	char z = 0;
	if (write(fd, &z, 1) < 0) {
		perror("write");
		abort();
	}

	close(fd);

	return 0;
}

bloom_t *bloom_open(const char *fn)
{
	bloom_t *b;
	
	b = malloc(sizeof(*b));
	if (!b) abort();

	b->fd = open(fn, O_RDWR);
	if (b->fd < 0) {
		perror("open");
		abort();
	}

	b->map = mmap(NULL, TABLE_BYTES, PROT_READ|PROT_WRITE, MAP_SHARED, b->fd, 0);
	if (b->map == MAP_FAILED) {
		perror("mmap");
		abort();
	}

#ifdef USE_MLOCK
	if (mlock(b->map, TABLE_BYTES) < 0) {
		perror("mlock");
		abort();
	}
#endif

	return b;
}

static void key2idxs(const uint8_t *key, size_t key_len, uint32_t *idxs, int n)
{
	uint8_t md5[MD5_DIGEST_LENGTH];
	MD5_CTX c;
	MD5_Init(&c);
	MD5_Update(&c, key, key_len);
	MD5_Final(md5, &c);

	int i;
	for (i = 0; i < n; i++) {
		idxs[i] = ((uint32_t*)md5)[i];
	}
}

void bloom_insert(bloom_t *b, const uint8_t *key, size_t key_len)
{
	int i;
	uint32_t idxs[NUM_IDXS];
	key2idxs(key, key_len, idxs, NUM_IDXS);
	for (i = 0; i < NUM_IDXS; i++) {
		uint32_t wrapped = idxs[i] % TABLE_SIZE;
		uint32_t byte_idx = wrapped / BYTE_SIZE;
		uint32_t bit_idx = wrapped % BYTE_SIZE;
		b->map[byte_idx] |= (1 << bit_idx);
	}
}

int bloom_check(bloom_t *b, const uint8_t *key, size_t key_len)
{
	int i;
	uint32_t idxs[NUM_IDXS];
	key2idxs(key, key_len, idxs, NUM_IDXS);
	for (i = 0; i < NUM_IDXS; i++) {
		uint32_t wrapped = idxs[i] % TABLE_SIZE;
		uint32_t byte_idx = wrapped / BYTE_SIZE;
		uint32_t bit_idx = wrapped % BYTE_SIZE;
		if (!(b->map[byte_idx] & (1 << bit_idx)))
			return 0;
	}
	return 1;
}

void bloom_close(bloom_t *b)
{
#ifdef USE_MLOCK
	if (mlock(b->map, TABLE_BYTES) < 0) {
		perror("munlock");
		abort();
	}
#endif

	munmap(b->map, TABLE_BYTES);
	close(b->fd);
	free(b);
}
