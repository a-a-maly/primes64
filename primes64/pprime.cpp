#define __STDC_FORMAT_MACROS
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include "primes.h"

uint_t mode;
uint64_t primes_cnt;
uint64_t primes_sum0, primes_sum1;
static uint64_t prime_prev;


static const uint_t D = 10;
static const uint_t DK = 4;
static const uint_t DP = D * D * D * D;
static char udump_base[DP][DK];

static void udump_init()
{
	char a[DK];
	
	for (uint_t i = 0; i < DK; i++)
		a[i] = '0';

	for (uint_t v = 0; ; v++) {
		char *b = udump_base[v];
		for (uint_t i = 0; i < DK; i++)
			b[i] = a[i];
		for (uint_t i = 0; i < DK; i++) {
			a[i]++;
			if ((uint_t) (a[i] - '0') < D)
				goto cont1;
			a[i] = '0';
		}
		break;

cont1:;		
	}
}


static uint_t udump32(char * __restrict__ buf, uint32_t x)
{
	uint_t l = 0;
	for (uint_t i = 0; x >= DP; i++) {
		uint32_t q = x / DP;
		uint32_t r = x % DP;
		x = q;
		for (uint_t j = 0; j < DK; j++)
			buf[l + j] = udump_base[r][j];
		l += DK;
	}

	{
		for (uint_t j = 0; j < DK; j++)
			buf[l + j] = udump_base[x][j];
		l += DK;
	}

	for (; l > 1 && buf[l - 1] == '0'; l--)
		;

	for (uint_t i = 0, j = l - 1; i < j; i++, j--) {
		char t = buf[i];
		buf[i] = buf[j];
		buf[j] = t;
	}

	buf[l++] = '\n';
	return l;
}


static uint_t udump64(char * __restrict__ buf, uint64_t x)
{
	uint_t l = 0;
	for (uint_t i = 0; x >= DP * DP; i++) {
		uint64_t q = x / (DP * DP);
		uint32_t r = x - q * (DP * DP);
		uint32_t r1 = r / DP, r0 = r % DP;
		x = q;

		for (uint_t j = 0; j < DK; j++)
			buf[l + j] = udump_base[r0][j];
		l += DK;

		for (uint_t j = 0; j < DK; j++)
			buf[l + j] = udump_base[r1][j];
		l += DK;
	}

	{
		uint32_t r = x;
		uint32_t r1 = r / DP, r0 = r % DP;

		for (uint_t j = 0; j < DK; j++)
			buf[l + j] = udump_base[r0][j];
		l += DK;

		for (uint_t j = 0; j < DK; j++)
			buf[l + j] = udump_base[r1][j];
		l += DK;
	}

	for (; l > 1 && buf[l - 1] == '0'; l--)
		;

	for (uint_t i = 0, j = l - 1; i < j; i++, j--) {
		char t = buf[i];
		buf[i] = buf[j];
		buf[j] = t;
	}

	buf[l++] = '\n';
	return l;
}


void pprime(uint64_t p)
{
	char buf[24];
	uint_t l;
	uint64_t v;
	uint16_t dv;

	uint64_t dp = p - prime_prev;

	if (dp >> 16) {
		fprintf(stderr, "p_prev=%" PRIu64 ", p=%" PRIu64 ", dp=%" PRIu64 " \n",
			prime_prev, p, dp);
		abort();
	}

	switch (mode & 3) {
	case 0: // simple text
		l = udump64(buf, p);
		fwrite(buf, 1, l, stdout);
		break;

	case 1: // simple binary
		v = htobe64(p);
		fwrite(&v, sizeof(v), 1, stdout);
		break;

	case 2: // difference text
		l = udump32(buf, dp);
		fwrite(buf, 1, l, stdout);
		break;

	case 3: // difference binary
		dv = htobe16((uint16_t) dp);
		fwrite(&dv, sizeof(dv), 1, stdout);
		break;
	}

	prime_prev = p;
	primes_cnt++;
	primes_sum0 += p;
	primes_sum1 += (primes_sum0 < p);
}


void pprime_init(uint64_t po)
{
	prime_prev = po;
	udump_init();
}

