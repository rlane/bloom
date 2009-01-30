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

#define FILE_SIZE (1<<25)
#define PAGE_SIZE (1<<12)
#define BYTE_SIZE 8
#define NUM_BUCKETS (FILE_SIZE/PAGE_SIZE)
#define BITS_PER_BUCKET (PAGE_SIZE*BYTE_SIZE)
#define NUM_IDXS 4

int bloom_create(const char *fn)
{
	int fd;
	char z;
	
	fd = creat(fn, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		perror("creat");
		goto fail_creat;
	}

	if (lseek(fd, FILE_SIZE, SEEK_SET) < 0) {
		perror("seek");
		goto fail_lseek;
	}

	z = 0;
	if (write(fd, &z, 1) < 0) {
		perror("write");
		goto fail_write;
	}

	close(fd);

	return 0;

fail_write:
fail_lseek:
fail_creat:
	return -1;
}

bloom_t *bloom_open(const char *fn)
{
	bloom_t *b;
	
	b = malloc(sizeof(*b));
	if (!b) goto fail_malloc;

	b->fd = open(fn, O_RDWR);
	if (b->fd < 0) {
		perror("open");
		goto fail_open;
	}

	b->map = mmap(NULL, FILE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, b->fd, 0);
	if (b->map == MAP_FAILED) {
		perror("mmap");
		goto fail_mmap;
	}

#ifdef USE_MLOCK
	if (mlock(b->map, FILE_SIZE) < 0) {
		perror("mlock");
		goto fail_mlock;
	}
#endif

	return b;

#ifdef USE_MLOCK
fail_mlock:
	munmap(b->map, FILE_SIZE);
#endif

fail_mmap:
	close(b->fd);

fail_open:
	free(b);

fail_malloc:
	return NULL;
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
	uint32_t wrapped, byte_idx, bit_idx;
	uint8_t *bucket;

	key2idxs(key, key_len, idxs, NUM_IDXS);
	bucket = b->map + (idxs[0] % NUM_BUCKETS) * PAGE_SIZE;

	for (i = 1; i < NUM_IDXS; i++) {
		wrapped = idxs[i] % BITS_PER_BUCKET;
		byte_idx = wrapped / BYTE_SIZE;
		bit_idx = wrapped % BYTE_SIZE;
		bucket[byte_idx] |= (1 << bit_idx);
	}
}

int bloom_check(bloom_t *b, const uint8_t *key, size_t key_len)
{
	int i;
	uint32_t idxs[NUM_IDXS];
	uint32_t wrapped, byte_idx, bit_idx;
	uint8_t *bucket;

	key2idxs(key, key_len, idxs, NUM_IDXS);
	bucket = b->map + (idxs[0] % NUM_BUCKETS) * PAGE_SIZE;

	for (i = 1; i < NUM_IDXS; i++) {
		wrapped = idxs[i] % BITS_PER_BUCKET;
		byte_idx = wrapped / BYTE_SIZE;
		bit_idx = wrapped % BYTE_SIZE;
		if (!(bucket[byte_idx] & (1 << bit_idx)))
			return 0;
	}
	return 1;
}

void bloom_close(bloom_t *b)
{
#ifdef USE_MLOCK
	if (mlock(b->map, FILE_SIZE) < 0) {
		perror("munlock");
		abort();
	}
#endif

	munmap(b->map, FILE_SIZE);
	close(b->fd);
	free(b);
}
