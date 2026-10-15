#ifndef FACTORIZE_HPP
#define FACTORIZE_HPP

#include "../template.hpp"

namespace factorize_detail {
using U = uint64_t;
using W = __uint128_t;
inline constexpr U small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

struct Montgomery {
	U mod, inverse = 1, r2, one;
	explicit Montgomery(U n) : mod(n), r2(-W(n) % n) {
		rep(6) inverse *= 2 - n * inverse;
		one = reduce(r2);
	}
	U reduce(W x) const {
		U a = x >> 64, b = (W(U(x) * inverse) * mod) >> 64;
		return a < b ? a - b + mod : a - b;
	}
	U in(U x) const { return reduce(W(x) * r2); }
	U mul(U a, U b) const { return reduce(W(a) * b); }
	U add(U a, U b) const {
		U c = a + b;
		return c < a || c >= mod ? c - mod : c;
	}
	U pow(U a, U e) const {
		U r = one;
		for (; e; e >>= 1, a = mul(a, a))
			if (e & 1) r = mul(r, a);
		return r;
	}
};

inline bool prime(const Montgomery &m) {
	U n = m.mod, d = n - 1;
	int s = countr_zero(d);
	d >>= s;
	auto witness = [&](U a) {
		if (!(a %= n)) return true;
		U x = m.pow(m.in(a), d);
		if (x == m.one || x == n - m.one) return true;
		rep(i, 1, s) {
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

struct Factorizer {
	U state;
	vec<U> result;
	explicit Factorizer(U n) : state(n ^ 0x9e3779b97f4a7c15ULL) { result.reserve(64); }
	U random() {
		U x = (state += 0x9e3779b97f4a7c15ULL);
		x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
		x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
		return x ^ (x >> 31);
	}
	U rho(const Montgomery &m) {
		U n = m.mod;
		for (;;) {
			U y = m.in(random() % (n - 1) + 1), c = m.in(random() % (n - 1) + 1);
			U x = 0, saved = 0, g = 1;
			auto step = [&](U v) { return m.add(m.mul(v, v), c); };
			auto diff = [](U a, U b) { return a < b ? b - a : a - b; };
			for (int length = 1; g == 1 && length <= (1 << 18); length <<= 1) {
				x = y;
				rep(length) y = step(y);
				for (int k = 0; k < length && g == 1; k += 128) {
					saved = y;
					U q = m.one;
					rep(min(128, length - k)) {
						y = step(y);
						q = m.mul(q, diff(x, y));
					}
					g = gcd(q, n);
				}
			}
			if (g == n)
				rep(128) {
					saved = step(saved);
					g = gcd(diff(x, saved), n);
					if (g != 1) break;
				}
			if (g > 1 && g < n) return g;
		}
	}
	void split(U n) {
		if (n < 1681) {
			result.push_back(n);
			return;
		}
		if ((n & 7) == 1) {
			U r = sqrtl(n);
			r -= W(r) * r > n;
			if (W(r) * r == n) {
				int begin = sz(result);
				split(r);
				int end = sz(result);
				rep(i, begin, end) result.push_back(result[i]);
				return;
			}
		}
		Montgomery m(n);
		if (prime(m)) result.push_back(n);
		else {
			U d = rho(m);
			split(d);
			split(n / d);
		}
	}
};
}

inline bool is_prime(uint64_t n) {
	if (n < 2) return false;
	for (auto p : factorize_detail::small_primes)
		if (n % p == 0) return n == p;
	return n < 1681 || factorize_detail::prime(factorize_detail::Montgomery(n));
}

inline vec<uint64_t> factorize(uint64_t n) {
	assert(n > 0);
	if (n < 2) return {};
	factorize_detail::Factorizer f(n);
	for (auto p : factorize_detail::small_primes)
		while (n % p == 0) f.result.push_back(p), n /= p;
	if (n > 1) f.split(n);
	sort(all(f.result));
	return std::move(f.result);
}

inline vec<pair<uint64_t, int>> factorize_count(uint64_t n) {
	vec<pair<uint64_t, int>> result;
	for (auto p : factorize(n)) {
		if (result.empty() || result.back().first != p) result.emplace_back(p, 1);
		else ++result.back().second;
	}
	return result;
}

#endif
