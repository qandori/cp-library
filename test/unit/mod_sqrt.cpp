#include "math/mod_sqrt.hpp"
#include "math/is_prime.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;

U power(U a, U e, U p) {
	U result = 1;
	for (; e; e >>= 1, a = W(a) * a % p) if (e & 1) result = W(result) * a % p;
	return result;
}

void check(U a, U p) {
	auto root = mod_sqrt(a, p);
	U value = a % p;
	bool exists = p == 2 || value == 0 || power(value, (p - 1) / 2, p) == 1;
	assert(bool(root) == exists);
	if (root) {
		assert(*root < p && *root <= p - *root);
		assert(W(*root) * *root % p == value);
	}
}

int main() {
	for (U p = 2; p <= 1500; ++p) if (is_prime(p)) {
		std::vector<int> expected(p, -1);
		for (U x = 0; x <= p / 2; ++x) expected[x * x % p] = x;
		for (U a = 0; a < p; ++a) {
			auto root = mod_sqrt(a, p);
			assert(bool(root) == (expected[a] >= 0));
			if (root) assert(*root == U(expected[a]));
		}
	}
	std::mt19937_64 random(32023064);
	for (U p : {2ULL, 3ULL, 5ULL, 7ULL, 17ULL, 41ULL, 97ULL, 193ULL, 257ULL, 65537ULL,
				998244353ULL, 1000000007ULL, 2013265921ULL, 3221225473ULL, 4294967291ULL,
				4294967311ULL, 9223372036854775783ULL, 18446744069414584321ULL, 18446744073709551557ULL}) {
		assert(is_prime(p));
		for (U a : {U(0), U(1), U(2), p - 1, p, std::numeric_limits<U>::max()}) check(a, p);
		for (int i = 0; i < 1000; ++i) {
			U x = random() % p;
			U a = W(x) * x % p;
			auto root = mod_sqrt(a, p);
			assert(root && *root == std::min(x, p - x));
			check(random(), p);
		}
	}
	for (int s : {32, 40, 48, 56, 60}) {
		U p = 0;
		for (U q = 1; q < (std::numeric_limits<U>::max() >> s); q += 2) {
			U candidate = (q << s) + 1;
			if (is_prime(candidate)) { p = candidate; break; }
		}
		if (p) for (int i = 0; i < 200; ++i) check(random(), p);
	}
}
