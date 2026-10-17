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
	if (n % 2 == 0) return n == 2;
	if (n % 3 == 0) return n == 3;
	if (n % 5 == 0) return n == 5;
	if (n % 7 == 0) return n == 7;
	if (n % 11 == 0) return n == 11;
	if (n % 13 == 0) return n == 13;
	if (n % 17 == 0) return n == 17;
	if (n % 19 == 0) return n == 19;
	if (n % 23 == 0) return n == 23;
	if (n % 29 == 0) return n == 29;
	if (n % 31 == 0) return n == 31;
	if (n % 37 == 0) return n == 37;
	return n < 1681 || is_prime_detail::prime(Montgomery64(n));
}

#endif
