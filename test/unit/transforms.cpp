#include "convolution/bitwise_zeta.hpp"
#include "convolution/divisor_zeta.hpp"
#include "convolution/walsh_hadamard.hpp"
#include <bit>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

int main() {
	std::mt19937 rng(41);
	for (int n = 1; n <= 256; n *= 2) {
		std::vector<std::int64_t> a(n), sub(n), sup(n), walsh(n);
		for (auto &x : a) x = int(rng() % 100) - 50;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) {
				if ((i & j) == j) sub[i] += a[j];
				if ((i & j) == i) sup[i] += a[j];
				walsh[i] += (std::popcount(unsigned(i & j)) & 1) ? -a[j] : a[j];
			}
		auto b = a;
		bitwise_zeta(b);
		assert(b == sub);
		bitwise_zeta<true>(b);
		assert(b == a);
		bitwise_zeta<false, true>(b);
		assert(b == sup);
		bitwise_zeta<true, true>(b);
		assert(b == a);
		walsh_hadamard(b);
		assert(b == walsh);
		walsh_hadamard(b);
		for (int i = 0; i < n; ++i) assert(b[i] == n * a[i]);
	}
	for (int n = 0; n <= 200; ++n) {
		std::vector<std::int64_t> a(n), div(n), mul(n);
		for (auto &x : a) x = int(rng() % 100) - 50;
		if (n) a[0] = div[0] = mul[0] = 123;
		for (int i = 1; i < n; ++i)
			for (int j = i; j < n; j += i) div[j] += a[i], mul[i] += a[j];
		auto b = a;
		divisor_zeta(b);
		assert(b == div);
		divisor_zeta<true>(b);
		assert(b == a);
		divisor_zeta<false, true>(b, prime_sieve(n));
		assert(b == mul);
		divisor_zeta<true, true>(b);
		assert(b == a);
	}
	std::vector<int> empty;
	bitwise_zeta(empty);
	walsh_hadamard(empty);
	divisor_zeta(empty);
}
