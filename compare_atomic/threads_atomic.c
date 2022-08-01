#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>

static uint64_t iter_cnt;
static atomic_long acnt __attribute__ ((aligned (64)));

void *cons(void *p)
{
	uint64_t i;

	for (i = 0; i < iter_cnt; i++)
		acnt--;
}

void *prod(void *p)
{
	uint64_t i;

	for (i = 0; i < iter_cnt; i++)
		acnt++;
}

int main(int argc, char **argv)
{
	pthread_t prod_thread, cons_thread;

	printf("%p\n", &acnt);

	iter_cnt = strtoul(argv[1], NULL, 10);
	iter_cnt *= 1000000;

	pthread_create(&prod_thread, NULL, prod, NULL);
	pthread_create(&cons_thread, NULL, cons, NULL);
	pthread_join(prod_thread, NULL);
	pthread_join(cons_thread, NULL);

	return 0;
}
