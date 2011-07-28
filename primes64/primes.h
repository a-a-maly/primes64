#ifndef _PRIMES_H
#define _PRIMES_H

#include <inttypes.h>
typedef unsigned int uint_t;
extern uint_t mode;

extern uint64_t primes_cnt;
extern uint64_t primes_sum0, primes_sum1;

void primes(uint64_t start, uint64_t stop);
void pprime(uint64_t p);
void pprime_init(uint64_t po);
#endif
