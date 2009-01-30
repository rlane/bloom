#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bloom.h"

#define FILE_SIZE (1<<25)
#define PAGE_SIZE (1<<12)
#define BYTE_SIZE 8
#define NUM_BUCKETS (FILE_SIZE/PAGE_SIZE)
#define BITS_PER_BUCKET (PAGE_SIZE*BYTE_SIZE)
#ifndef NUM_IDXS
#define NUM_IDXS 16
#endif

typedef uint16_t idx_t;

unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed );

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

static void key2idxs(const uint8_t *key, size_t key_len, idx_t *idxs, int n)
{
	int i;
	int hash = 0xdeadbeef;
	for (i = 0; i < NUM_IDXS; i+=2) {
		hash = MurmurHash2(key, key_len, hash);
		idxs[i] = hash;
		if (i + 1 < NUM_IDXS)
			idxs[i+1] = hash >> 16;
	}
}

void bloom_insert(bloom_t *b, const uint8_t *key, size_t key_len)
{
	int i;
	idx_t idxs[NUM_IDXS];
	idx_t wrapped, byte_idx, bit_idx;
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
	idx_t idxs[NUM_IDXS];
	idx_t wrapped, byte_idx, bit_idx;
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

//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed )
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	unsigned int h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h *= m; 
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}
