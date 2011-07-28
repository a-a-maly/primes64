#define _BSD_SOURCE
#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include "bitset.h"
#include "primes.h"


uint_t sqrtu32(uint32_t x)
{
	if (x <= 1)
		return x;

	uint_t a = 1, b = 0xffff;
	while (a < b) {
		uint_t c = a + (b - a + 1) / 2, cc = c * (uint32_t) c;
		if (cc <= x)
			a = c;
		else
			b = c - 1;
	}

	fprintf(stderr, "sqrt(%u)=%u \n", x, a);
	return a;
}


struct Event32 {
	uint32_t v;
	uint16_t p;
};


struct Primes32
{
	uint_t pmax, size;
	Bitset<uint32_t, uint32_t> bits;
	Primes32() : pmax(0), size(0), bits() {};

	void init(uint_t _pmax)
	{
		fprintf(stderr, "Primes::init _pmax=%u \n", _pmax);
		pmax = _pmax;
		size = pmax / 2 + pmax % 2;
		bits.init(size);
		for (uint_t i = 1; i < size; i++) {
			if (bits.is_set(i))
				continue;

			uint_t pi = i + i + 1;
			uint_t kpi = pmax / pi;
			if (pi > kpi)
				break;
			uint_t cnt = (kpi - pi) / 2 + 1;
			uint_t idx = i * (pi + 1);
#if 0
			fprintf(stderr, "i=%u, pi=%u, kpi=%u, cnt=%u \n", i, pi, kpi, cnt);
#endif
			for (; cnt > 0; cnt--, idx += pi)
				bits.set(idx);
		}
	}
};


static const uint32_t tprimes[] = {2, 3, 5, 7, 11, 13};
static const uint32_t tprimes_div = 3 * 5 * 7 * 11 * 13 * 8;
static const uint_t tprimes_n = sizeof(tprimes) / sizeof(tprimes[0]);
static uint32_t tprimes_max = tprimes[tprimes_n - 1];
static Bitset<uint32_t, uint32_t> tprimes_mask;

static void tprimes_init(uint_t start)
{
	start %= 2 * tprimes_div;
	tprimes_mask.init(tprimes_div);
	for (uint_t i = 0; i < tprimes_div; i++) {
		uint_t pi = 2 * i + start;
		for (uint_t k = 1; k < tprimes_n; k++) {
			if (pi % tprimes[k] == 0) {
				tprimes_mask.set(i);
				break;
			}
		}
	}
}


void primes(uint32_t start, uint32_t stop)
{
//	prime_prev = start;
	pprime_init(start);

	for (uint_t i = 0; i < tprimes_n; i++) {
		uint32_t pi = tprimes[i];
		if (stop <= pi)
			break;
		if (start <= pi)
			pprime(pi);
	}

	uint32_t maxtp = tprimes_max;
	if (start < maxtp + 1)
		start = maxtp + 1;
	start |= 1;
	stop &= ~1;
	if (stop <= start)
		return;
	uint_t cnt = (stop - start + 1) / 2;
	uint_t maxsp = sqrtu32(stop - 1);
	fprintf(stderr, "cnt=%u, maxsp=%u \n", cnt, maxsp);

	Primes32 sprimes;
	sprimes.init(maxsp);

	std::vector<Event32> sevs;
	uint_t i_min = (tprimes_max + 1) / 2;
	uint_t i_lim = (maxsp + 1) / 2;	
	for (uint_t i = i_min; i < i_lim; i++) {
		if (sprimes.bits.is_set(i))
			continue;
		uint_t pi = i + i + 1;
		uint_t j = (start - 1) / pi;
		j = (j + 1) | 1;
		if (j < pi)
			j = pi;
		uint_t v = pi * j;
#if 0
		fprintf(stderr, "pi=%u, j=%u, v=%u \n", pi, j, v);
#endif
		if (v / pi != j || v >= stop)
			continue;
#if 0
		fprintf(stderr, "pi=%u, v=%u \n", pi, v);
#endif
		Event32 ei; ei.p = pi; ei.v = v / 2;
		sevs.push_back(ei);
	}

	uint_t nsevs = sevs.size();
	fprintf(stderr, "sevs.size=%u \n", (uint_t) nsevs);

	sprimes.bits.init(0);
	tprimes_init(start);

	uint_t cnt_d = cnt / tprimes_div, cnt_m = cnt % tprimes_div;
	Bitset<uint32_t, uint32_t> bits;
	for (uint_t j = 0; j <= cnt_d; j++) {
		uint_t i_min = (start / 2) + j * tprimes_div;
		uint_t i_lim = i_min + tprimes_div;
		if (j == cnt_d) {
			if (cnt_m <= 0)
				break;
			i_lim = stop / 2;
		}
//		fprintf(stderr, "j=%u, i_min=%u, i_lim=%u \n", j, i_min, i_lim);
		bits = tprimes_mask;
		for (uint_t k = 0; k < nsevs; k++) {
			Event32 &ek = sevs[k];
			uint_t pk = ek.p, vk = ek.v;
			assert (vk >= i_min);
			for (; vk < i_lim; vk += pk) {
				bits.set(vk - i_min);
			}
			ek.v = vk;
		}

		for (uint_t i = i_min; i < i_lim; i++) {
			if (bits.is_set(i - i_min))
				continue;
			uint_t pi = 2 * i + 1;
			pprime(pi);
		}
	}

	fprintf(stderr, "totally %u primes with sum=%" PRIu64 " \n",
		primes_cnt, primes_sum);
}

