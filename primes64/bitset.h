#ifndef _BITSET_H
#define _BITSET_H

#include <vector>

template <typename I, typename D> class Bitset
{
	I size, size_d;
	std::vector<D> data;
//	static const unsigned int S = 8 * sizeof(D);
	static const unsigned int S = 32;

 public:
	Bitset() : size(0), size_d(0), data() {}

	void init(I _size) {
		size = _size;
		size_d = size / S + 1;
		data.resize(size_d);
		clear();
	}

	void clear() {
		for (I i = 0; i < size_d; i++)
			data[i] = 0;
	}

	unsigned int is_set(I i) const {
//		assert (i < size);
		return (data[i / S] >> (i % S)) & 1U;
	}

	void set(I i) {
//		assert (i < size);
		D one = 1;
		data[i / S] |= (one  << (i % S));
	}

	void free() {
		size = 0;
		size_d = 0;
		data.resize(0);
		data.reserve(0);
	}
};

#endif
