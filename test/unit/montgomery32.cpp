#include "components/montgomery32.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>

int main() {
	using U = std::uint32_t;
	using W = std::uint64_t;
	std::mt19937_64 random(9832724);
	for (int trial = 0; trial < 30000; ++trial) {
		U p = U(random()) | 1;
		if (p == 1) p = 3;
		if (trial % 4 == 0) p = std::numeric_limits<U>::max() - 2 * (trial % 100);
		Montgomery32 m(p);
		U a = random() % p, b = random() % p;
		auto x = m.in(a), y = m.in(b);
		assert(m.reduce(x) == a && m.reduce(y) == b);
		assert(m.reduce(m.mul(x, y)) == W(a) * b % p);
		assert(m.reduce(m.add(x, y)) == (W(a) + b) % p);
		U exponent = trial % 6 == 0 ? 0 : trial % 6 == 1 ? std::numeric_limits<U>::max() : U(random());
		U expected = 1;
		for (U value = a, e = exponent; e; e >>= 1, value = W(value) * value % p)
			if (e & 1) expected = W(expected) * value % p;
		assert(m.reduce(m.pow(x, exponent)) == expected);
		W value = (W(random() % p) << 32) | U(random());
		U reduced = m.reduce(value);
		assert(reduced < p && (W(reduced) << 32) % p == value % p);
	}
}
