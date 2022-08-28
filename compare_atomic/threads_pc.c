#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>

struct tparam {
	uint64_t nb_iter;
	uint32_t nb_prod;
	uint32_t nb_work;
	int atomic;
};

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

struct shared_cnt *shared_cnt;
static atomic_long acnt;

struct tparam tparam = {
	.nb_iter = 1,
	.nb_prod = 1,
	.nb_work = 100
};

void *cons(void *unused)
{
	uint64_t i;
	uint32_t p;

	for (i = 0; i < tparam.nb_iter; i++) {
		for (p = 0; p < tparam.nb_prod; p++) {
			struct shared_cnt *sc = &shared_cnt[p];
			uint64_t cons;

			cons = sc->cons;
			while (cons != sc->prod)
				cons++;
			sc->cons = cons;
		}
	}
}

void *atomic_cons(void *unused)
{
	uint64_t i;
	uint64_t p, prod;

	for (i = 0; i < tparam.nb_iter * tparam.nb_prod; i++) {
		struct shared_cnt *sc = &shared_cnt[0];

		while (sc->cons == sc->prod);
		sc->cons++;
	}
}

static void *atomic_prod(void *unused)
{
	uint64_t i;
	volatile int cnt;

	for (i = 0; i < tparam.nb_iter; i++) {
		struct shared_cnt *sc = &shared_cnt[0];

		for (cnt = 0; cnt < tparam.nb_work; cnt++);
		atomic_fetch_add(&sc->prod, 1);
	}
}

void *prod(void *arg)
{
	uint64_t i;
	struct shared_cnt *sc = arg;
	volatile int cnt;

	if (tparam.atomic)
		return atomic_prod(arg);

	sc->prod++;
	while (sc->prod != sc->cons);

	for (i = 1; i < tparam.nb_iter; i++) {
		for (cnt = 0; cnt < tparam.nb_work; cnt++);
		sc->prod++;
	}
}


int main(int argc, char **argv)
{
	pthread_t *prod_thread, cons_thread;
	uint32_t i;

	if (argc > 1)
		tparam.nb_iter = strtoul(argv[1], NULL, 10);
	if (argc > 2)
		tparam.nb_prod = strtoul(argv[2], NULL, 10);
	if (argc > 3)
		tparam.nb_work = strtoul(argv[3], NULL, 10);
	if (argc > 4)
		tparam.atomic = strtoul(argv[4], NULL, 10);

	tparam.nb_iter *= 1000000;

	prod_thread = malloc(tparam.nb_prod * sizeof(*prod_thread));
	shared_cnt = calloc(1, tparam.nb_prod * sizeof(*shared_cnt));

	for (i = 0; i < tparam.nb_prod; i++)
		pthread_create(&prod_thread[i], NULL, prod, &shared_cnt[i]);

	pthread_create(&cons_thread, NULL, cons, NULL);

	for (i = 0; i < tparam.nb_prod; i++)
		pthread_join(prod_thread[i], NULL);

	pthread_join(cons_thread, NULL);

	return 0;
}
