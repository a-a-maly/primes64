#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "primes.h"


void usage()
{
	fprintf(stderr, "usage: primes64 [-t] [-b] [-a] [-d] [start [stop]] \n");
	exit(1);
}


static uint_t ctod(char c)
{
	uint_t res = c - '0';
	if (res < 10)
		return res;

	res = c - 'A';
	if (res < 26)
		return 10 + res;

	res = c - 'a';
	if (res < 26)
		return 10 + res;

	return ~0;
}


static uint64_t atou64(const char *os)
{
	const char *s = os;
	uint_t state = 0, base = 10;
	uint64_t res = 0;

	for (; *s; s++) {
		char c = *s;
		uint_t v = ctod(c);
		switch (c) {

		case '+':
			if (state != 0)
				goto err;
			state = 1;
			break;

		case 'x':
		case 'X':
			if (state != 2)
				goto err;
			state = 3;
			base = 16;
			break;

		case '0':
			if (state < 2) {
				state = 2;
				base = 8;
				break;
			}

		default:
			if (v >= base)
				goto err;
			state = 4;
			uint64_t nres = res * base;
			if (nres / base != res)
				goto err;
			nres += v;
			if (nres < v)
				goto err;
			res = nres;
		}
	}

	if ((0x14 >> state) & 1)
		return res;

err:
	fprintf(stderr, "argument '%s' incorrect at position %u \n",
		os, (uint_t) (s - os));
	exit(1);
}


int main(int ac, char **av)
{
	uint64_t start = 0, stop = 0;
	mode = 0;

	for (;;) {
		int opt = getopt(ac, av, "btad");
		if (opt == -1)
			break;
		switch (opt) {
		case 'b': mode |= 1; break;
		case 't': mode &= ~1; break;
		case 'd': mode |= 2; break;
		case 'a': mode &= ~2; break;
		default: usage();
		}
	}

	if ((mode & 1) && isatty(1)) {
		fprintf(stderr, "Cowardly refusing to dump binary data to a terminal. \n");
		exit(1);
	}

	switch (ac - optind) {
	case 0:
	break;

	case 1:
		stop  = atou64(av[optind + 0]);
	break;

	case 2:
		start = atou64(av[optind + 0]);
		stop  = atou64(av[optind + 1]);
	break;

	default:
		usage();
	}

	if (stop < start) {
		fprintf(stderr, "start should be less than or equal to stop \n");
		exit(1);
	}

	fprintf(stderr, "start=%" PRIu64 ", stop=%" PRIu64 ", mode=0x%x \n",
		start, stop, mode);
	primes(start, stop);

	return 0;
}

