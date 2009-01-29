#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bloom.h"

#define PAGE_SIZE 4096
#define HEADER_LEN 64
#define NUM_IDXS 8

int bloom_create(const char *fn, size_t len)
{
	int fd = creat(fn, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		perror("creat");
		abort();
	}

	lseek(fd, len + PAGE_SIZE, SEEK_SET);

	char z = 0;
	if (write(fd, &z, 1) < 0) {
		perror("write");
		abort();
	}

	lseek(fd, 0, SEEK_SET);

	if (write(fd, &len, sizeof(len)) < 0) {
		perror("write");
		abort();
	}

	close(fd);

	return 0;
}

bloom_t *bloom_open(const char *fn)
{
	char hdr[HEADER_LEN];
	bloom_t *b;
	
	b = malloc(sizeof(*b));
	if (!b) abort();

	b->fd = open(fn, O_RDWR);
	if (b->fd < 0) {
		perror("open");
		abort();
	}

	if (read(b->fd, hdr, sizeof(hdr)) < sizeof(hdr)) {
		perror("read");
		abort();
	}

	b->len = *(size_t*)(hdr);

	b->map = mmap(NULL, b->len + PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, b->fd, PAGE_SIZE);
	if (b->map == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	return b;
}

static void key2idxs(const char *key, size_t key_len, int *idxs, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		idxs[i] = (i < key_len) ? key[i] : 0;
	}
}

void bloom_insert(bloom_t *b, const char *key, size_t key_len)
{
	int i;
	int idxs[NUM_IDXS];
	key2idxs(key, key_len, idxs, NUM_IDXS);
	for (i = 0; i < NUM_IDXS; i++) {
		b->map[idxs[i] % b->len] = 1;
	}
}

int bloom_check(bloom_t *b, char *key, size_t key_len)
{
	int i;
	int idxs[NUM_IDXS];
	key2idxs(key, key_len, idxs, NUM_IDXS);
	for (i = 0; i < NUM_IDXS; i++) {
		if (b->map[idxs[i] % b->len] != 1)
			return 0;
	}
	return 1;
}

void bloom_close(bloom_t *b)
{
	munmap(b->map, b->len + PAGE_SIZE);
	close(b->fd);
	free(b);
}
