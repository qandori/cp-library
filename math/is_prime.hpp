#ifndef IS_PRIME_HPP
#define IS_PRIME_HPP

#include "../components/montgomery.hpp"
#include <bit>
#include <cstdint>
#include <initializer_list>

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr std::uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

inline bool prime(const Montgomery64 &m) {
	U n = m.mod, d = n - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto witness = [&](U a) {
		if (!(a %= n)) return true;
		U x = m.pow(m.in(a), d);
		if (x == m.one || x == n - m.one) return true;
		for (int i = 1; i < s; ++i) {
			x = m.mul(x, x);
			if (x == n - m.one) return true;
		}
		return false;
	};
	if (n < (U(1) << 32)) {
		for (U a : {2, 7, 61})
			if (!witness(a)) return false;
	} else {
		for (U a : {2, 325, 9375, 28178, 450775, 9780504, 1795265022})
			if (!witness(a)) return false;
	}
	return true;
}
}

inline bool is_prime(std::uint64_t n) {
	if (n < 2) return false;
	for (auto p : is_prime_detail::small_primes)
		if (n % p == 0) return n == p;
	return n < 1681 || is_prime_detail::prime(Montgomery64(n));
}

#endif
