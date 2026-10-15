#include "math/is_prime.hpp"
#include "math/montgomery.hpp"
#include "math/prime_sieve.hpp"
#include "math/splitmix64.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;
int main() {
	const int limit = 100000;
	std::vector<bool> sieve(limit, true);
	sieve[0] = sieve[1] = false;
	for (int i = 2; i * i < limit; ++i)
		if (sieve[i])
			for (int j = i * i; j < limit; j += i) sieve[j] = false;
	auto primes = prime_sieve(limit);
	std::vector<std::size_t> expected;
	for (int i = 0; i < limit; ++i) {
		assert(is_prime(i) == sieve[i]);
		if (sieve[i]) expected.push_back(i);
	}
	assert(primes == expected);
	for (int n = 0; n < 100; ++n) {
		auto p = prime_sieve(n);
		std::vector<std::size_t> e;
		for (int i = 0; i < n; ++i)
			if (sieve[i]) e.push_back(i);
		assert(p == e);
	}
	for (U p : {4294967291ULL, 9223372036854775783ULL, 18446744073709551557ULL}) assert(is_prime(p));
	for (U n : {341ULL, 561ULL, 3215031751ULL, 341550071728321ULL, 3825123056546413051ULL, 18446744073709551615ULL}) assert(!is_prime(n));
	std::mt19937_64 rng(78);
	for (int i = 0; i < 20000; ++i) {
		U mod = rng() | 1;
		if (mod == 1) mod = 3;
		Montgomery64 m(mod);
		U a = rng() % mod, b = rng() % mod, k = rng() % 128;
		assert(m.reduce(m.in(a)) == a);
		assert(m.reduce(m.mul(m.in(a), m.in(b))) == W(a) * b % mod);
		assert(m.reduce(m.add(m.in(a), m.in(b))) == (W(a) + b) % mod);
		U power = 1;
		for (U j = 0; j < k; ++j) power = W(power) * a % mod;
		assert(m.reduce(m.pow(m.in(a), k)) == power);
	}
	SplitMix64 split;
	assert(split() == 0xe220a8397b1dcdafULL);
	assert(split() == 0x6e789e6aa1b965f4ULL);
	assert(split() == 0x06c45d188009454fULL);
	for (U seed : {0ULL, 1ULL, 18446744073709551615ULL}) {
		SplitMix64 g(seed);
		U state = seed;
		for (int i = 0; i < 100; ++i) assert(g() == splitmix64(state += 0x9e3779b97f4a7c15ULL));
	}
}
