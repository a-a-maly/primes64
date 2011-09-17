#define __STDC_FORMAT_MACROS
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include "bitset.h"
#include "primes.h"


static const uint_t GS = 28;


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

	return a;
}


uint_t sqrtu64(uint64_t x)
{
	if (x <= 1)
		return x;

	uint32_t a = 1, b = 0xffffffffU;
	while (a < b) {
		uint32_t c = a + (b - a + 1) / 2;
		uint64_t cc = c * (uint64_t) c;
		if (cc <= x)
			a = c;
		else
			b = c - 1;
	}

#if 0
	fprintf(stderr, "sqrt(%" PRIu64 ")=%u \n", x, (uint_t) a);
#endif

	return a;
}


struct Event32 {
	uint32_t v;
	uint16_t p;
};

struct Event64 {
	uint64_t v;
	uint32_t p;
};


struct Primes32
{
	uint_t pmax, size;
	Bitset<uint32_t, uint32_t> bits;
	Primes32() : pmax(0), size(0), bits() {};

	void init(uint_t _pmax)
	{
		if (loglevel >= 4) {
			fprintf(stderr, "%s(%u) \n", __func__, _pmax);
		}

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


static const uint32_t tprimes[] = {2, 3, 5, 7, 11, 13, 17};
static const uint32_t tprimes_div = 3 * 5 * 7 * 11 * 13 * 17;
static const uint_t tprimes_n = sizeof(tprimes) / sizeof(tprimes[0]);
static uint32_t tprimes_max = tprimes[tprimes_n - 1];
static Bitset<uint32_t, uint32_t> tprimes_mask;


static void tprimes_init(uint64_t _start, uint_t tmul)
{
	if (loglevel >= 3) {
		fprintf(stderr, "%s(%" PRIu64 ", %u) \n", __func__, _start, tmul);
	}

	uint_t tdiv = tmul * tprimes_div;
	uint_t start = _start % (2 * tdiv);
	tprimes_mask.init(tdiv);
	for (uint_t i = 0; i < tdiv; i++) {
		uint_t pi = 2 * i + start;
		for (uint_t k = 1; k < tprimes_n; k++) {
			if (pi % tprimes[k] == 0) {
				tprimes_mask.set(i);
				break;
			}
		}
	}
}


void fill_sdiffs(std::vector<uint8_t> &sdiffs, uint_t maxsp)
{
	sdiffs.resize(0);

	uint32_t p_min = (tprimes_max + 1) / 2;
	uint32_t p_lim = maxsp / 2 + maxsp % 2;
	uint32_t p_len = p_lim - p_min;

	if (loglevel >= 3) {
		fprintf(
			stderr,
			"%s([], %u): %u prime indexes in [%u, %u) \n",
			__func__, maxsp, p_len, p_min, p_lim
		);
	}

	if (p_lim <= p_min)
		return;

	uint32_t maxssp = sqrtu32(maxsp);
	uint32_t si_min = (tprimes_max + 1) / 2;
	uint32_t si_lim = (maxssp + 1) / 2;

	if (loglevel >= 3) {
		fprintf(
			stderr,
			"%s: sieving very small primes in (%u, %u], "
			"indexes in [%u, %u) \n",
			__func__, tprimes_max, maxssp, si_min, si_lim
		);
	}

	Primes32 ssprimes;
	ssprimes.init(maxssp);

	std::vector<Event32> ssevs;

	for (uint_t i = si_min; i < si_lim; i++) {
		if (ssprimes.bits.is_set(i))
			continue;

		uint32_t pi = 2 * i + 1;
		uint32_t j = (2 * p_min) / pi;
		j = (j + 1) >> 1;
		if (j < i)
			j = i;

		uint32_t v = pi * j + i;
		if (v >= p_lim)
			continue;
#if 0
		fprintf(stderr, "pi=%u, j=%u, v=%u \n", pi, j, v);
#endif
		Event32 ei; ei.p = pi; ei.v = v;
		ssevs.push_back(ei);
	}

	uint_t nssevs = ssevs.size();
	if (loglevel >= 3) {
		fprintf(stderr, "%s: found %u very small events \n",
			__func__, nssevs);
	}

	uint32_t tdiv = tprimes_div;
	tprimes_init(p_min * 2 + 1, 1);

	uint32_t cnt_d = p_len / tdiv;
	uint32_t cnt_m = p_len % tdiv;

	if (loglevel >= 3) {
		fprintf(stderr, "%s: %u is %u full segments of size %u plus %u rest \n",
			__func__, p_len, cnt_d, tdiv, cnt_m);
	}

	Bitset<uint32_t, uint32_t> bits;
	uint_t i_c = 0;

	for (uint32_t j = 0; j <= cnt_d; j++) {
		uint32_t i_min = p_min + j * tdiv;
		uint32_t i_lim = i_min + tdiv;
		if (j == cnt_d) {
			if (cnt_m <= 0)
				break;
			i_lim = p_lim;
		}
		uint_t i_len = i_lim - i_min;

		if (loglevel >= 4) {
			fprintf(stderr, "%s: segment #%u is [%u, %u) \n",
				__func__, j, i_min, i_lim);
		}

		bits = tprimes_mask;
		for (uint_t k = 0; k < nssevs; k++) {
			Event32 &ek = ssevs[k];
			uint32_t pk = ek.p;
			uint32_t vk = ek.v;
			assert (vk >= i_min);
			for (; vk < i_lim; vk += pk) {
				bits.set(vk - i_min);
			}
			ek.v = vk;
		}

		for (uint_t di = 0; di < i_len; di++) {
			if (bits.is_set(di))
				continue;
			uint32_t i = i_min + di;
			assert ((i - i_c) < 256);
			sdiffs.push_back(i - i_c);
			i_c = i;
		}
	}

}


void primes(uint64_t start, uint64_t stop)
{
	pprime_init();
	summary_init(start);

	for (uint_t i = 0; i < tprimes_n; i++) {
		uint32_t pi = tprimes[i];
		if (stop <= pi)
			break;
		if (start <= pi)
			pprime(pi);
	}

	if (start < tprimes_max + 1)
		start = tprimes_max + 1;

	start |= 1;
	stop &= ~1;
	if (stop <= start) {
		summary_print();
		return;
	}

	uint64_t q_min = start / 2, q_lim = stop / 2, q_len = q_lim - q_min;

	if (loglevel >= 2) {
		fprintf(
			stderr,
			"%s(%" PRIu64 ", %" PRIu64 "): "
			"%" PRIu64 " indexes in [%" PRIu64 ", %" PRIu64 ") \n",
			__func__, start, stop, q_len, q_min, q_lim
		);
	}

	uint32_t maxsp = sqrtu64(stop - 1);
	double maxspl = log(maxsp);
	uint_t maxspl2 = maxspl * M_LOG2E;
	if (maxspl2 >= 32)
		maxspl2 = 31;

	uint_t maxspl2i = (maxspl2 <= 16 ? 0 : maxspl2 - 16);
	uint_t tmul = 1 + maxspl2i;
	uint_t tdiv = tprimes_div * tmul;
	uint_t sres = (maxsp <= 16 ? 0 : 1 + floor(maxsp / (maxspl - 1.1)));
	uint64_t cnt_dd = q_len / (GS * tdiv);

	if (loglevel >= 2) {
		fprintf(
			stderr,
			"%s: %" PRIu64 " full groups of %u intervals "
			"of %u * %u = %u size each \n",
			__func__, cnt_dd, GS, tmul, tprimes_div, tdiv
		);
		fprintf(
			stderr,
			"%s: small primes will be up to %u, appr. %u of them \n",
			__func__, maxsp, sres
		);
	}

	std::vector<uint8_t> sdiffs;
	sdiffs.reserve(sres);
	fill_sdiffs(sdiffs, maxsp);

	uint_t sdsize = sdiffs.size();

	if (loglevel >= 2) {
		fprintf(stderr, "%s: really found %u small primes \n",
			__func__, sdsize);
	}

	Bitset<uint32_t, uint32_t> bits;
	tprimes_init(start, tmul);

	std::vector<Event64> sevs;
	sevs.reserve(0x1a00000);

	for (uint64_t k = 0; k <= cnt_dd; k++) {
		uint64_t g_min = q_min + k * (GS * tdiv);
		uint64_t g_lim = g_min + (GS * tdiv);
		if (k == cnt_dd) {
			g_lim = q_lim;
			if (g_lim <= g_min)
				break;
		}
		uint64_t g_len = g_lim - g_min;

		sevs.resize(0);
		uint_t i_c = 0;
		for (uint_t i = 0; i < sdsize; i++) {
			i_c += sdiffs[i];

			uint32_t pi = 2 * i_c + 1;
			uint64_t j = (2 * g_min) / pi;
			j = (j + 1) >> 1;
			if (j < i_c)
				j = i_c;

			uint64_t v = pi * j + i_c;
			if (v >= g_lim)
				continue;
#if 0
			fprintf(stderr, "pi=%u, j=%" PRIu64 ", v=%" PRIu64 " \n",
				(uint_t) pi, j, v);
#endif
			Event64 ei; ei.p = pi; ei.v = v;
			sevs.push_back(ei);
		}
		uint_t nsevs = sevs.size();
		uint64_t cnt_d = g_len / tdiv;

		if (loglevel >= 4) {
			fprintf(
				stderr,
				"%s: group #%" PRIu64 "/%" PRIu64 " "
				"is [%" PRIu64 ", %" PRIu64 ") \n length is %" PRIu64 ", "
				"or %" PRIu64 " full intervals; %u events ready. \n",
				__func__, k, cnt_dd, g_min, g_lim, g_len, cnt_d, nsevs
			);
		}

		for (uint64_t j = 0; j <= cnt_d; j++) {
			uint64_t i_min = g_min + j * tdiv;
			uint64_t i_lim = i_min + tdiv;
			if (j == cnt_d) {
				i_lim = g_lim;
				if (i_lim <= i_min)
					break;
			}
			uint64_t i_len = i_lim - i_min;

			bits = tprimes_mask;
			for (uint_t l = 0; l < nsevs; l++) {
				Event64 &el = sevs[l];
				uint32_t pl = el.p;
				uint64_t vl = el.v;
				assert (vl >= i_min);
				for (; vl < i_lim; vl += pl)
					bits.set(vl - i_min);

				el.v = vl;
			}

			for (uint_t di = 0; di < i_len; di++) {
				if (bits.is_set(di))
					continue;
				uint64_t pi = 2 * (i_min + di) + 1;
				pprime(pi);
			}

		}

	}

	summary_print();
}

