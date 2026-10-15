#include "math/monoid_floor_sum.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <utility>

using namespace std;

using U = uint64_t;
using W = __uint128_t;
constexpr U mod = 1000000007;
using Stats = array<U, 3>;
Stats combine(Stats a, Stats b) {
	return {(a[0] + b[0]) % mod, (a[1] + b[1]) % mod, (a[2] + b[2] + a[1] * b[0]) % mod};
}
Stats stats_unit() { return {}; }

U floor_sum(U n, U m, U a, U b) {
	U result = 0;
	for (;;) {
		if (n) result = (result + W((W(n) * (n - 1) / 2) % mod) * (a / m)) % mod;
		result = (result + W(n % mod) * (b / m)) % mod;
		a %= m, b %= m;
		W z = W(a) * n + b;
		if (z < m) return result;
		n = z / m, b = z % m;
		swap(a, m);
	}
}

void check(U n, U m, U a, U b) {
	auto result = monoid_floor_sum<Stats, combine, stats_unit>(n, m, a, b, {1, 0, 0}, {0, 1, 0});
	assert(result[0] == n % mod);
	assert(result[1] == ((W(a) * n + b) / m) % mod);
	assert(result[2] == floor_sum(n, m, a, b));
	using Counts = array<W, 2>;
	constexpr auto add = [](Counts a, Counts b) { return Counts{a[0] + b[0], a[1] + b[1]}; };
	constexpr auto zero = [] { return Counts{}; };
	auto counts = monoid_floor_sum<Counts, add, zero>(n, m, a, b, {1, 0}, {0, 1});
	assert(counts[0] == n && counts[1] == (W(a) * n + b) / m);
}

using Affine = array<U, 2>;
Affine compose(Affine a, Affine b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
Affine unit() { return {1, 0}; }
Affine power(Affine a, U k) {
	auto r = unit();
	for (; k; k >>= 1, a = compose(a, a))
		if (k & 1) r = compose(r, a);
	return r;
}

struct Node {
	U value;
	explicit Node(U value) : value(value) {}
};

int main() {
	constexpr auto concat = [](const string &a, const string &b) { return a + b; };
	constexpr auto empty = [] { return string{}; };
	for (int n = 0; n < 9; ++n)
		for (int m = 1; m < 10; ++m)
			for (int a = 0; a < 10; ++a)
				for (int b = 0; b < 10; ++b) {
					string expected(b / m, 'U');
					for (int i = 1; i < (n + 1); ++i) expected += 'R', expected.append((a * i + b) / m - (a * (i - 1) + b) / m, 'U');
					assert((monoid_floor_sum<string, concat, empty>(n, m, a, b, "R", "U") == expected));
					check(n, m, a, b);
				}
	mt19937_64 random(123);
	for (int repeat_index = 0; repeat_index < 2000; ++repeat_index) {
		U n = random() % 50, m = max<U>(1, random()), a = random(), b = random();
		Affine x{random(), random()}, y{random(), random()};
		auto expected = power(y, b / m);
		W previous = b / m;
		U sum = 0;
		for (U i = 0; i < n; ++i) sum = (sum + ((W(a) * i + b) / m) % mod) % mod;
		for (U i = 1; i <= n; ++i) {
			W current = (W(a) * i + b) / m;
			expected = compose(compose(expected, x), power(y, U(current - previous)));
			previous = current;
		}
		assert((monoid_floor_sum<Affine, compose, unit>(n, m, a, b, x, y) == expected));
		auto result = monoid_floor_sum<Stats, combine, stats_unit>(n, m, a, b, {1, 0, 0}, {0, 1, 0});
		assert(result[2] == sum);
	}
	const U maximum = numeric_limits<U>::max();
	for (U n : {U(0), U(1), U(2), maximum / 2, maximum})
		for (U m : {U(1), U(2), U(3), maximum / 2, maximum - 1, maximum})
			for (U a : {U(0), U(1), maximum - 2, maximum - 1, maximum})
				for (U b : {U(0), U(1), maximum - 1, maximum}) check(n, m, a, b);
	check(maximum, 12200160415121876738ULL, 7540113804746346429ULL, 0);
	for (int repeat_index = 0; repeat_index < 10000; ++repeat_index) {
		U m = random();
		check(random(), m ? m : 1, random(), random());
	}
	constexpr auto add = [](Node a, Node b) { return Node(a.value + b.value); };
	constexpr auto zero = [] { return Node(0); };
	assert((monoid_floor_sum<Node, add, zero>(10, 7, 3, 4, Node(1), Node(1)).value == 14));
	static int operations;
	constexpr auto count = [](U a, U b) { ++operations; return a + b; };
	constexpr auto e = [] { return U(0); };
	U result = monoid_floor_sum<U, count, e>(maximum, 12200160415121876738ULL, 7540113804746346429ULL, maximum, 1, 1);
	assert(result == U(W(maximum) + (W(7540113804746346429ULL) * maximum + maximum) / 12200160415121876738ULL));
	assert(operations < 1024);
}
