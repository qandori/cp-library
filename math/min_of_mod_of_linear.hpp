#ifndef MIN_OF_MOD_OF_LINEAR_HPP
#define MIN_OF_MOD_OF_LINEAR_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>

inline std::uint64_t min_of_mod_of_linear(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b) {
	assert(n > 0 && m > 0);
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	if (!a || !b || n == 1) return b;
	if (n >= m) return b % std::gcd(a, m);
	if (m <= std::numeric_limits<std::uint32_t>::max()) {
		std::uint64_t best = b;
		while (a) {
			std::uint64_t z = a * (n - 1) + b;
			if (z < m) break;
			n = z / m;
			b = z % m % a;
			if (!b) return 0;
			best = std::min(best, b);
			std::uint64_t next = m % a;
			m = a, a = next;
		}
		return best;
	}
	std::uint64_t p = m - a, q = a, sp = 1, sq = 1, remaining = n - 1;
	for (;;) {
		if (p <= b) {
			std::uint64_t k = b / p;
			if (__uint128_t(k) * sp > remaining) return b - remaining / sp * p;
			b %= p;
			remaining -= k * sp;
			if (!b) return 0;
		}
		if (sp > remaining || sq > remaining - sp) return b;
		if (p > q) {
			std::uint64_t k = (p - b - 1) / q + 1, d = k * q;
			if (d == p) return b;
			if (d > p) --k;
			sp += k * sq;
			if (sp > remaining) return b;
			p -= k * q;
		} else {
			std::uint64_t k = q / p;
			q %= p;
			if (!q) return b;
			sq += k * sp;
		}
	}
}

#endif
