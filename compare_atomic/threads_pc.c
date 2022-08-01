#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static uint64_t iter_cnt;
struct shared_cnt {
	union {
		uint64_t prod;
		char ppad[64];
	};

	union {
		uint64_t cons;
		char cpad[64];
	};
};

struct shared_cnt shared_cnt;

void *cons(void *p)
{
	uint64_t i;
	struct shared_cnt *sc = p;

	for (i = 0; i < iter_cnt; i++) {
		while (sc->cons != sc->prod)
			sc->cons++;
	}

}

void *prod(void *p)
{
	uint64_t i;
	struct shared_cnt *sc = p;

	sc->prod++;
	while (sc->prod != sc->cons);

	for (i = 1; i < iter_cnt; i++)
		sc->prod++;
}

int main(int argc, char **argv)
{
	pthread_t prod_thread, cons_thread;

	iter_cnt = strtoul(argv[1], NULL, 10);
	iter_cnt *= 1000000;

	printf("ppad %p cpad %p\n", &shared_cnt.prod, &shared_cnt.cons);

	pthread_create(&prod_thread, NULL, prod, &shared_cnt);
	pthread_create(&cons_thread, NULL, cons, &shared_cnt);
	pthread_join(prod_thread, NULL);
	pthread_join(cons_thread, NULL);

	printf("cnt %lu\n", shared_cnt.prod);

	return 0;
}
