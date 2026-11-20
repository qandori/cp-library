#include "math/modint.hpp"
#include "math/crt.hpp"
#include "math/floor_sum.hpp"
#include "string/suffix_array.hpp"
#include "string/lcp_array.hpp"
#include "string/z_algorithm.hpp"
#include "math/modular_arithmetic.hpp"
#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>

static_assert(StaticModint<17>(-2).pow(3).val() == 9);
static_assert(inv_mod(3, 8) == 3);
static_assert(inv_mod(3, 8) == 3 && floor_sum(3, 5, -2, 1) == -2);
static_assert(StaticModint<1000000007>(-1).val() == 1000000006);

template <class M> void check(unsigned m) {
	std::mt19937_64 rng(m);
	for (int t = 0; t < 2000; ++t) {
		std::int64_t a = std::int64_t(rng()), b = std::int64_t(rng());
		M x(a), y(b);
		auto norm = [m](std::int64_t x) { auto r = x % std::int64_t(m); return std::uint64_t(r < 0 ? r + m : r); };
		auto u = norm(a), v = norm(b);
		assert(x.val() == u && y.val() == v);
		assert((x + y).val() == (u + v) % m);
		assert((x - y).val() == (u + m - v) % m);
		assert((x * y).val() == u * v % m);
		assert((2 + x).val() == (u + 2) % m);
		assert(x.pow(19).val() == pow_mod(a, 19, m));
		if (std::gcd(v, std::uint64_t(m)) == 1) assert((x / y * y) == x);
		auto z = x; assert(z++.val() == u); assert(z.val() == (u + 1) % m); --z; assert(z == x);
		std::stringstream stream; stream << x; M parsed; stream >> parsed; assert(parsed == x);
	}
}

int main() {
	assert(pow_mod(-2, 3, 5) == 2);
	const std::vector<long long> residues{2, 5}, moduli{6, 9};
	assert((crt(residues, moduli) == CRTSolution{14, 18}));
	const auto sa_example = suffix_array("aba");
	assert(sa_example == (std::vector{2, 0, 1}));
	assert(lcp_array("aba", sa_example) == (std::vector{1, 0}));
	assert(z_algorithm("aba") == (std::vector{3, 0, 1}));
	check<StaticModint<1>>(1); check<StaticModint<17>>(17); check<StaticModint<1000000007>>(1000000007); check<StaticModint<4294967295U>>(4294967295U);
	for (unsigned m : {1U, 2U, 12U, 998244353U, 4294967295U}) { DynamicModint<>::set_mod(m); check<DynamicModint<>>(m); }
	DynamicModint<1>::set_mod(7); assert(DynamicModint<1>::mod() == 7 && DynamicModint<>::mod() == 4294967295U);
	bool caught = false; try { inv_mod(2, 4); } catch (const std::domain_error &) { caught = true; } assert(caught);
	std::mt19937 rng(2383);
	for (int n = 0; n < 150; ++n) for (int trial = 0; trial < 20; ++trial) {
		std::vector<int> a(n), expected(n);
		std::string s(n, '\0');
		for (int i = 0; i < n; ++i) { s[i] = char(rng() % 256); a[i] = int(rng() % 7) - 3; }
		std::iota(expected.begin(), expected.end(), 0);
		std::sort(expected.begin(), expected.end(), [&](int i, int j) { return std::lexicographical_compare(a.begin() + i, a.end(), a.begin() + j, a.end()); });
		assert(suffix_array(a) == expected);
		auto lcp = lcp_array(a, expected);
		for (int i = 1; i < n; ++i) { int h = 0; while (expected[i] + h < n && expected[i-1] + h < n && a[expected[i]+h] == a[expected[i-1]+h]) ++h; assert(lcp[i-1] == h); }
		for (auto &x : a) x += 3;
		assert(suffix_array(a, 6) == expected && suffix_array(a, INT32_MAX) == expected);
		for (int i = 0; i < n; ++i) a[i] = static_cast<unsigned char>(s[i]);
		auto sa = suffix_array(s); assert(sa == suffix_array(a, 255)); assert(lcp_array(s, sa) == lcp_array(a, sa));
	}
	assert(StaticModint<17>(__uint128_t(-1)).val() == __uint128_t(-1) % 17);
}
