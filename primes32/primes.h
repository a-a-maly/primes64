#ifndef _PRIMES_H
#define _PRIMES_H

#include <inttypes.h>
typedef unsigned int uint_t;
extern uint_t mode;
extern uint32_t primes_cnt;
extern uint64_t primes_sum;

void primes(uint32_t start, uint32_t stop);
void pprime(uint32_t p);
void pprime_init(uint32_t po);
#endif
