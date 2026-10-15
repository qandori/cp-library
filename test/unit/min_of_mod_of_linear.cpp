#include "math/min_of_mod_of_linear.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;

U naive(U n, U m, U a, U b) {
	U result = m;
	for (U i = 0; i < n; ++i) result = std::min(result, U((W(a) * i + b) % m));
	return result;
}

W floor_sum(U n, U m, U a, W b) {
	W result = 0;
	for (;;) {
		result += W(n) * (b / m);
		b %= m;
		if (a >= m) {
			result += W(n) * (n - 1) / 2 * (a / m);
			a %= m;
		}
		W z = W(a) * n + b;
		if (z < m) return result;
		n = z / m;
		b = z % m;
		std::swap(a, m);
	}
}

void check(U n, U m, U a, U b) {
	U value = min_of_mod_of_linear(n, m, a, b);
	assert(value < m);
	a %= m, b %= m;
	W base = floor_sum(n, m, a, b);
	auto less = [&](U bound) { return W(n) - (floor_sum(n, m, a, W(b) + m - bound) - base); };
	assert(less(value) == 0);
	assert(less(value + 1) > 0);
	if (n <= 128) assert(value == naive(n, m, a, b));
	if (n >= m) assert(value == b % std::gcd(a, m));
}

int main() {
	for (U m = 1; m <= 40; ++m)
		for (U a = 0; a < m; ++a)
			for (U b = 0; b < m; ++b) {
				U expected = m;
				for (U n = 1; n <= 2 * m + 2; ++n) {
					expected = std::min(expected, (a * (n - 1) + b) % m);
					assert(min_of_mod_of_linear(n, m, a, b) == expected);
					assert(min_of_mod_of_linear(n, m, a + 3 * m, b + 5 * m) == expected);
					if (m <= 20) {
						constexpr U scale = (U(1) << 32) + 15;
						assert(min_of_mod_of_linear(n, m * scale, a * scale, b * scale) == expected * scale);
					}
				}
			}
	std::mt19937_64 random(713);
	for (int i = 0; i < 20000; ++i) {
		U n = random() % 128 + 1, m = random() | 1, a = random(), b = random();
		assert(min_of_mod_of_linear(n, m, a, b) == naive(n, m, a, b));
	}
	constexpr U maximum = std::numeric_limits<U>::max();
	for (U m : std::array<U, 10>{1, 2, 3, (U(1) << 32) - 1, U(1) << 32, (U(1) << 32) + 1, (U(1) << 63) - 1, U(1) << 63, maximum - 1, maximum})
		for (U n : std::array<U, 7>{1, 2, 3, m, m > 1 ? m - 1 : 1, maximum / 2, maximum})
			for (U a : std::array<U, 7>{0, 1, m / 2, m - 1, m, maximum - 1, maximum})
				for (U b : std::array<U, 6>{0, 1, m / 2, m - 1, m, maximum}) check(n, m, a, b);
	for (int i = 0; i < 10000; ++i) {
		U m = random() | 1;
		check(random() | 1, m, random(), random());
		check(random() % m + 1, m, random(), random());
	}
	std::vector<U> fibonacci{1, 1};
	while (W(fibonacci.back()) + fibonacci[fibonacci.size() - 2] <= maximum)
		fibonacci.push_back(fibonacci.back() + fibonacci[fibonacci.size() - 2]);
	for (std::size_t i = 2; i < fibonacci.size(); ++i) {
		U m = fibonacci[i], a = fibonacci[i - 1];
		for (U n : std::array<U, 6>{1, 2, m / 2, m - 1, m, maximum})
			for (U b : std::array<U, 5>{0, 1, m / 3, m - 1, maximum}) check(n, m, a, b);
	}
}
