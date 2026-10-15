#include "math/factorize.hpp"

using U = uint64_t;
using W = __uint128_t;

void check(U n) {
	auto f = factorize(n);
	assert(is_sorted(all(f)));
	W product = 1;
	for (U p : f) {
		assert(is_prime(p));
		product *= p;
	}
	assert(product == n);
	auto counts = factorize_count(n);
	vec<U> expanded;
	for (auto [p, k] : counts) {
		assert(k > 0);
		rep(k) expanded.push_back(p);
	}
	assert(expanded == f);
}

int main() {
	const int limit = 100000;
	vec<int> smallest(limit + 1);
	rep(i, 2, limit + 1) {
		if (smallest[i]) continue;
		for (int j = i; j <= limit; j += i)
			if (!smallest[j]) smallest[j] = i;
	}
	rep(n, limit + 1) assert(is_prime(n) == (n >= 2 && smallest[n] == n));
	rep(n, 1, limit + 1) {
		vec<U> expected;
		for (int x = n; x > 1; x /= smallest[x]) expected.push_back(smallest[x]);
		assert(factorize(n) == expected);
	}
	for (U n : {341ULL, 561ULL, 1105ULL, 1729ULL, 2047ULL, 1373653ULL, 25326001ULL,
				3215031751ULL, 2152302898747ULL, 3474749660383ULL, 341550071728321ULL,
				3825123056546413051ULL, 18446744073709551615ULL}) {
		assert(!is_prime(n));
		check(n);
	}
	for (U p : {2ULL, 3ULL, 41ULL, 65537ULL, 1000000007ULL, 4294967291ULL,
				9223372036854775783ULL, 18446744073709551557ULL}) {
		assert(is_prime(p));
		check(p);
	}
	for (U n : {1ULL, 1ULL << 63, 12157665459056928801ULL, 7450580596923828125ULL,
				1000000014000000049ULL, 18446744030759878681ULL,
				18446743979220271189ULL, 9223372036854775807ULL}) check(n);
	mt19937_64 random(123);
	rep(10000) {
		U n = random() | 1;
		if (n == 1) n = 3;
		factorize_detail::Montgomery m(n);
		U a = random() % n, b = random() % n;
		assert(m.reduce(m.in(a)) == a);
		assert(m.reduce(m.mul(m.in(a), m.in(b))) == W(a) * b % n);
		assert(m.reduce(m.add(m.in(a), m.in(b))) == (W(a) + b) % n);
	}
	rep(1000) check(random() % 1000000000000ULL + 1);
	rep(50) {
		U p = (random() % 1000000000 + 3000000000) | 1;
		U q = (random() % 1000000000 + 3000000000) | 1;
		while (!is_prime(p)) p += 2;
		while (!is_prime(q)) q += 2;
		check(p * q);
		assert((factorize(p * q) == vec<U>{min(p, q), max(p, q)}));
	}
	assert((factorize(18446743979220271189ULL) == vec<U>{4294967279ULL, 4294967291ULL}));
	assert((factorize_count(360) == vec<pair<U, int>>{{2, 3}, {3, 2}, {5, 1}}));
}
