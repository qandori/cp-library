#ifndef MONOID_FLOOR_SUM_HPP
#define MONOID_FLOOR_SUM_HPP

#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>

template <class S, auto op, auto e>
S monoid_floor_sum(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b, S x, S y) {
	assert(m > 0);
	auto append = [](S value, const S &base, std::uint64_t k) {
		if (!k) return value;
		S p = base;
		for (;;) {
			if (k & 1) value = op(std::move(value), p);
			k >>= 1;
			if (!k) return value;
			p = op(p, p);
		}
	};
	S prefix = e();
	if (b >= m) {
		prefix = append(std::move(prefix), y, b / m);
		b %= m;
	}
	if (!n) return prefix;
	S suffix = e();
	for (;;) {
		if (a >= m) {
			x = append(std::move(x), y, a / m);
			a %= m;
		}
		__uint128_t z = __uint128_t(a) * n + b;
		if (z < m) return op(append(std::move(prefix), x, n), std::move(suffix));
		std::uint64_t k, r;
		if (z >> 64) k = z / m, r = z % m;
		else {
			std::uint64_t v = z;
			k = v / m, r = v % m;
		}
		suffix = op(append(y, x, r / a), std::move(suffix));
		n = k - 1;
		std::uint64_t t = m - b - 1;
		prefix = append(std::move(prefix), x, t / a + 1);
		b = t % a;
		if (!n) return op(std::move(prefix), std::move(suffix));
		std::swap(a, m);
		std::ranges::swap(x, y);
	}
}

#endif
