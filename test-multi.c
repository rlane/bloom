#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

#define INITIAL_SLEEP_TIME (3*1000*1000)

unsigned int key_seed;

void generate_key(uint8_t *key, int key_len)
{
	long result;
	int c, i = 0;

	while (i < key_len) {
		result = rand_r(&key_seed);
		c = (i + sizeof(result) > key_len) ? (key_len - i) : sizeof(result);
		memcpy(key+i, &result, c);
		i += sizeof(result);
	}
}

int insert_all(bloom_t *b, int n, int key_len, int sleep_max)
{
	int i, inserted = 0;
	uint8_t k[key_len];
	for (i=0; i<n; i++) {
		generate_key(k, key_len);
		if (bloom_check(b, k, key_len) == 0) {
			inserted += 1;
			bloom_insert(b, k, key_len);
		}
		if (sleep_max > 0)
			usleep(rand() % sleep_max);
	}
	return inserted;
}

int check_all(bloom_t *b, int n, int key_len)
{
	int i, missing = 0;
	uint8_t k[key_len];
	for (i=0; i<n; i++) {
		generate_key(k, key_len);
		if (bloom_check(b, k, key_len) == 0) {
			missing += 1;
		}
	}
	return missing;
}

int main(int argc, char** argv)
{
	char *fn = "/tmp/test_bloom_filter";
	bloom_t *b;
	const int n = 10000000;
	const int key_len = 20;
	const int num_children = 100;
	const int sleep_max = 0;
	int i;
	int child_inserted, inserted, missing;
	int ret;

	bloom_create(fn);
	b = bloom_open(fn);

	key_seed = 0xdeadbeef;
	int pipefd[2];
	pipe(pipefd);

	for (i=0; i<num_children; i++) {
		int pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(1);
		} else if (pid == 0) {
			close(pipefd[0]);
			srand(i);
			fprintf(stderr, "child %d spawned\n", getpid());
			usleep(INITIAL_SLEEP_TIME);
			fprintf(stderr, "child %d starting\n", getpid());
			child_inserted = insert_all(b, n, key_len, sleep_max);
			fprintf(stderr, "child %d finished with subtotal %d\n", getpid(), child_inserted);
			write(pipefd[1], &child_inserted, sizeof(child_inserted));
			bloom_close(b);
			exit(0);
		}
	}

	close(pipefd[1]);

	inserted = 0;
	while ((ret = read(pipefd[0], &child_inserted, sizeof(child_inserted))) == sizeof(child_inserted)) {
		inserted += child_inserted;
	}

	missing = check_all(b, n, key_len);

	printf("inserted %d\n", inserted);
	printf("missing %d\n", missing);

	bloom_close(b);
	return 0;
}
