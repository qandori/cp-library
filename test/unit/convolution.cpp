#include "convolution/convolution.hpp"
#include "convolution/bitwise_convolution.hpp"
#include "convolution/boolean_convolution.hpp"
#include "convolution/cyclic_convolution.hpp"
#include "convolution/gcd_lcm_convolution.hpp"
#include "convolution/max_min_convolution.hpp"
#include "convolution/min_plus_convolution.hpp"
#include "convolution/multiplicative_convolution.hpp"
#include "convolution/multivariate_convolution.hpp"
#include "convolution/ordered_convolution.hpp"
#include "convolution/subset_convolution.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace std;
using I = int64_t;
mt19937_64 random_engine(857123);

template <class T>
vector<T> naive(const vector<T> &a, const vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	vector<T> c(a.size() + b.size() - 1, T(0));
	for (size_t i = 0; i < a.size(); ++i)
		for (size_t j = 0; j < b.size(); ++j) c[i + j] += a[i] * b[j];
	return c;
}

template <class T, class Reduce, class Term>
vector<T> extrema(const vector<T> &a, const vector<T> &b, Reduce reduce, Term term) {
	if (a.empty() || b.empty()) return {};
	vector<T> c(a.size() + b.size() - 1, term(a[0], b[0]));
	vector<bool> seen(c.size());
	for (size_t i = 0; i < a.size(); ++i)
		for (size_t j = 0; j < b.size(); ++j) {
			auto value = term(a[i], b[j]);
			c[i + j] = seen[i + j] ? reduce(c[i + j], value) : value;
			seen[i + j] = true;
		}
	return c;
}

void check_bitwise() {
	for (int trial = 0; trial < 150; ++trial) {
		int n = random_engine() % 60, m = random_engine() % 60;
		vector<I> a(n), b(m);
		for (auto &x : a) x = I(random_engine() % 15) - 7;
		for (auto &x : b) x = I(random_engine() % 15) - 7;
		int length = n && m ? bit_ceil(unsigned(max(n, m))) : 0;
		vector<I> expected_and(length), expected_or(length), expected_xor(length), expected_subset(length);
		for (int i = 0; i < n && m; ++i)
			for (int j = 0; j < m; ++j) {
				expected_and[i & j] += a[i] * b[j];
				expected_or[i | j] += a[i] * b[j];
				expected_xor[i ^ j] += a[i] * b[j];
				if (!(i & j)) expected_subset[i | j] += a[i] * b[j];
			}
		assert(and_convolution(a, b) == expected_and);
		assert(or_convolution(a, b) == expected_or);
		assert(xor_convolution(a, b) == expected_xor);
		assert(subset_convolution(a, b) == expected_subset);
		vector<mint> x(a.begin(), a.end()), y(b.begin(), b.end());
		auto actual = xor_convolution(x, y);
		for (int i = 0; i < length; ++i) assert(actual[i] == mint(expected_xor[i]));
		x.resize(length);
		auto copy = x;
		bitwise_zeta(x);
		bitwise_zeta<true>(x);
		assert(x == copy);
		bitwise_zeta<false, true>(x);
		bitwise_zeta<true, true>(x);
		assert(x == copy);
	}
	vector<mint> a(1 << 10, 1);
	auto c = subset_convolution(a, a);
	for (size_t i = 0; i < c.size(); ++i) assert(c[i] == mint(2).pow(popcount(i)));
	assert(xor_convolution(vector<int8_t>(256), vector<int8_t>(256)) == vector<int8_t>(256));
}

void check_divisors() {
	for (int trial = 0; trial < 100; ++trial) {
		int n = 1 + random_engine() % 90, m = 1 + random_engine() % 90;
		vector<I> a(n), b(m), x(max(n, m)), y(max(n, m));
		for (int i = 1; i < n; ++i) a[i] = I(random_engine() % 21) - 10;
		for (int i = 1; i < m; ++i) b[i] = I(random_engine() % 21) - 10;
		for (int i = 1; i < n; ++i)
			for (int j = 1; j < m; ++j) {
				x[gcd(i, j)] += a[i] * b[j];
				int k = lcm(i, j);
				if (k < int(y.size())) y[k] += a[i] * b[j];
			}
		assert(gcd_convolution(a, b) == x);
		assert(lcm_convolution(a, b) == y);
	}
}

template <int index, bool dual>
void check_boolean_case(const vector<uint64_t> &a, const vector<uint64_t> &b) {
	size_t n = max(a.size(), b.size());
	if constexpr (index < 3 || index == 5) n = bit_ceil(n);
	vector<uint64_t> expected(n, dual ? ~uint64_t(0) : 0);
	for (size_t i = index == 3 || index == 4; i < a.size(); ++i)
		for (size_t j = index == 3 || index == 4; j < b.size(); ++j) {
			size_t k;
			if constexpr (index == 0) k = i & j;
			if constexpr (index == 1) k = i | j;
			if constexpr (index == 2) k = i ^ j;
			if constexpr (index == 3) k = gcd(i, j);
			if constexpr (index == 4) k = lcm(i, j);
			if constexpr (index == 5) {
				if (i & j) continue;
				k = i | j;
			}
			if (k >= n) continue;
			if constexpr (dual) expected[k] &= a[i] | b[j];
			else expected[k] |= a[i] & b[j];
		}
	if constexpr (index == 3 || index == 4) expected[0] = 0;
	assert((boolean_convolution_detail::multiply<index, dual>(a, b) == expected));
}

void check_boolean() {
	for (int trial = 0; trial < 20; ++trial) {
		vector<uint64_t> a(1 + random_engine() % 20), b(1 + random_engine() % 20);
		for (auto &x : a) x = random_engine();
		for (auto &x : b) x = random_engine();
		check_boolean_case<0, false>(a, b);
		check_boolean_case<0, true>(a, b);
		check_boolean_case<1, false>(a, b);
		check_boolean_case<1, true>(a, b);
		check_boolean_case<2, false>(a, b);
		check_boolean_case<2, true>(a, b);
		check_boolean_case<3, false>(a, b);
		check_boolean_case<3, true>(a, b);
		check_boolean_case<4, false>(a, b);
		check_boolean_case<4, true>(a, b);
		check_boolean_case<5, false>(a, b);
		check_boolean_case<5, true>(a, b);
	}
	assert(and_or_convolution(vector<bool>{false, true}, vector<bool>{true, false}) == vector<bool>({true, false}));
	assert(or_and_convolution(vector<bool>{false, true}, vector<bool>{true, false}) == vector<bool>({true, false}));
	assert(xor_or_convolution(vector<unsigned>{}, vector<unsigned>{1}).empty());
	assert(and_or_convolution(vector<int32_t>{-1, 0}, vector<int32_t>{numeric_limits<int32_t>::min(), numeric_limits<int32_t>::max()}) == vector<int32_t>({-1, 0}));
}

struct Ring {
	unsigned value;
	Ring(unsigned x = 0) : value(x % 8) {}
	Ring &operator+=(Ring b) {
		value = (value + b.value) % 8;
		return *this;
	}
	Ring &operator-=(Ring b) {
		value = (value + 8 - b.value) % 8;
		return *this;
	}
	friend Ring operator*(Ring a, Ring b) { return Ring(a.value * b.value); }
	friend bool operator==(Ring a, Ring b) = default;
};

template <uint32_t P>
struct OtherMint {
	uint64_t value;
	OtherMint(I x = 0) : value((x % I(P) + P) % P) {}
	static constexpr uint32_t mod() { return P; }
	static OtherMint raw(uint32_t x) {
		OtherMint result;
		result.value = x;
		return result;
	}
	uint32_t val() const { return value; }
	OtherMint &operator+=(OtherMint b) {
		value = (value + b.value) % P;
		return *this;
	}
	OtherMint &operator-=(OtherMint b) {
		value = (value + P - b.value) % P;
		return *this;
	}
	OtherMint &operator*=(OtherMint b) {
		value = value * b.value % P;
		return *this;
	}
	friend OtherMint operator*(OtherMint a, OtherMint b) { return a *= b; }
	friend bool operator==(OtherMint, OtherMint) = default;
};

void check_polynomial() {
	for (auto [n, m] : vector<pair<int, int>>{{0, 3}, {1, 1}, {3, 9}, {33, 33}, {127, 82}, {1025, 33}, {33, 1025}, {600, 601}}) {
		vector<I> a(n), b(m);
		for (auto &x : a) x = I(random_engine() % 2001) - 1000;
		for (auto &x : b) x = I(random_engine() % 2001) - 1000;
		auto expected = naive(a, b);
		assert(convolution(a, b) == expected);
		assert(convolution_ring(a, b) == expected);
		assert(convolution_integer(a, b) == expected);
		for (uint32_t mod : {1u, 8u, 998244353u, 1000000007u, 4294967295u}) {
			auto actual = convolution_mod(a, b, mod);
			for (size_t i = 0; i < actual.size(); ++i) assert(actual[i] == uint32_t((expected[i] % I(mod) + mod) % mod));
		}
		vector<mint> x(a.begin(), a.end()), y(b.begin(), b.end());
		auto result = convolution(x, y);
		for (size_t i = 0; i < result.size(); ++i) assert(result[i] == mint(expected[i]));
		assert(convolution(x, x) == naive(x, x));
		vector<OtherMint<998244353>> wide_a(a.begin(), a.end()), wide_b(b.begin(), b.end());
		assert(convolution(wide_a, wide_b) == naive(wide_a, wide_b));
		vector<OtherMint<8>> other_a(a.begin(), a.end()), other_b(b.begin(), b.end());
		assert(convolution(other_a, other_b) == naive(other_a, other_b));
		vector<Ring> ra(a.begin(), a.end()), rb(b.begin(), b.end());
		assert(convolution(ra, rb) == naive(ra, rb));
		vector<double> da(a.begin(), a.end()), db(b.begin(), b.end());
		auto real = convolution(da, db);
		for (size_t i = 0; i < real.size(); ++i) assert(abs(real[i] - expected[i]) < 1e-5);
		vector<complex<double>> ca(n), cb(m);
		for (int i = 0; i < n; ++i) ca[i] = {double(a[i]), double(i % 7)};
		for (int j = 0; j < m; ++j) cb[j] = {double(b[j]), double(j % 11)};
		auto complex_result = convolution(ca, cb), complex_expected = naive(ca, cb);
		for (size_t i = 0; i < complex_result.size(); ++i) assert(abs(complex_result[i] - complex_expected[i]) < 1e-5);
	}
	I low = numeric_limits<I>::min(), high = numeric_limits<I>::max();
	assert(convolution_integer(vector<I>{low, high}, vector<I>{1}) == vector<I>({low, high}));
	uint64_t maximum = numeric_limits<uint64_t>::max();
	assert(convolution_integer(vector<uint64_t>{maximum}, vector<uint64_t>{1}) == vector<uint64_t>{maximum});
	assert(convolution_mod(vector<uint64_t>{maximum}, vector<uint64_t>{maximum}, 4294967295u) == vector<uint32_t>{0});
}

void check_minplus() {
	auto minimum = [](I a, I b) { return min(a, b); };
	auto maximum = [](I a, I b) { return max(a, b); };
	auto plus = [](I a, I b) { return a + b; };
	for (int trial = 0; trial < 3000; ++trial) {
		int n = 1 + random_engine() % 70, m = 1 + random_engine() % 70;
		vector<I> a(n), b(m);
		I slope = -30;
		a[0] = I(random_engine() % 100) - 50;
		for (int i = 1; i < n; ++i) slope += random_engine() % 5, a[i] = a[i - 1] + slope;
		for (auto &x : b) x = I(random_engine() % 1000) - 500;
		assert(min_plus_convolution_convex(a, b) == extrema(a, b, minimum, plus));
		assert(max_plus_convolution_convex(a, b) == extrema(a, b, maximum, plus));
		for (auto &x : a) x = -x;
		assert(max_plus_convolution_concave(a, b) == extrema(a, b, maximum, plus));
		assert(min_plus_convolution_concave(a, b) == extrema(a, b, minimum, plus));
	}
	assert(min_plus_convolution_convex(vector<I>{}, vector<I>{1}).empty());
	vector<I> zeros(257), ones(100, 1);
	assert(min_plus_convolution_convex(zeros, ones) == vector<I>(356, 1));
	assert(min_plus_convolution_concave(zeros, ones) == vector<I>(356, 1));
}

void check_extrema() {
	auto minimum = [](const auto &a, const auto &b) { return min(a, b); };
	auto maximum = [](const auto &a, const auto &b) { return max(a, b); };
	for (auto [n, m] : vector<pair<int, int>>{{0, 1}, {1, 5}, {32, 70}, {33, 33}, {70, 91}, {128, 257}}) {
		for (int trial = 0; trial < 6; ++trial) {
			vector<I> a(n), b(m);
			for (auto &x : a) x = I(random_engine() % (trial ? 300 : 1)) - 100;
			for (auto &x : b) x = I(random_engine() % (trial ? 300 : 1)) - 100;
			assert(max_min_convolution(a, b) == extrema(a, b, maximum, minimum));
			assert(min_max_convolution(a, b) == extrema(a, b, minimum, maximum));
		}
	}
	vector<string> a(40), b(39);
	for (auto &x : a) x = string(1, 'a' + random_engine() % 26);
	for (auto &x : b) x = string(1, 'a' + random_engine() % 26);
	assert(max_min_convolution(a, b) == extrema(a, b, maximum, minimum));
	assert(min_max_convolution(a, b) == extrema(a, b, minimum, maximum));
}

void check_indexed() {
	for (int trial = 0; trial < 30; ++trial) {
		int n = random_engine() % 100, m = random_engine() % 100;
		vector<mint> a(n), b(m);
		for (auto &x : a) x = random_engine() % 10;
		for (auto &x : b) x = random_engine() % 10;
		vector<mint> expected(n && m ? n + m - 1 : 0);
		for (int i = 0; i < n; ++i)
			for (int j = i + 1; j < m; ++j) expected[i + j] += a[i] * b[j];
		assert(ordered_convolution(a, b) == expected);
		for (int size : {1, 2, 7, 63}) {
			vector<mint> positive(size), negative(size);
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < m; ++j) {
					positive[(i + j) % size] += a[i] * b[j];
					if (((i + j) / size) & 1) negative[(i + j) % size] -= a[i] * b[j];
					else negative[(i + j) % size] += a[i] * b[j];
				}
			assert(cyclic_convolution(a, b, size) == positive);
			assert(cyclic_convolution<true>(a, b, size) == negative);
		}
	}
	for (int p : {2, 3, 7, 17, 101, 257}) {
		vector<mint> a(p), b(p), expected(p);
		for (auto &x : a) x = random_engine() % 100;
		for (auto &x : b) x = random_engine() % 100;
		for (int i = 0; i < p; ++i)
			for (int j = 0; j < p; ++j) expected[i * j % p] += a[i] * b[j];
		assert(multiplicative_convolution(a, b) == expected);
		vector<Ring> x(p), y(p), ring_expected(p);
		for (int i = 0; i < p; ++i) x[i] = a[i].val(), y[i] = b[i].val();
		for (int i = 0; i < p; ++i)
			for (int j = 0; j < p; ++j) ring_expected[i * j % p] += x[i] * y[j];
		assert(multiplicative_convolution(x, y) == ring_expected);
	}
	for (auto dims : vector<vector<int>>{{}, {1}, {2}, {3, 4}, {2, 2, 2, 2, 2}, {1, 3, 1, 5}}) {
		int n = accumulate(dims.begin(), dims.end(), 1, multiplies<int>{});
		vector<mint> a(n), b(n), expected(n);
		vector<Ring> x(n), y(n), ring_expected(n);
		for (int i = 0; i < n; ++i) {
			a[i] = random_engine() % 100;
			b[i] = random_engine() % 100;
			x[i] = a[i].val();
			y[i] = b[i].val();
		}
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) {
				int u = i, v = j;
				bool valid = true;
				for (int d : dims) {
					valid &= u % d + v % d < d;
					u /= d;
					v /= d;
				}
				if (valid) expected[i + j] += a[i] * b[j], ring_expected[i + j] += x[i] * y[j];
			}
		assert(multivariate_convolution(a, b, dims) == expected);
		assert(multivariate_convolution(x, y, dims) == ring_expected);
		vector<complex<double>> ca(n), cb(n);
		for (int i = 0; i < n; ++i) ca[i] = double(a[i].val()), cb[i] = double(b[i].val());
		auto complex_result = multivariate_convolution(ca, cb, dims);
		for (int i = 0; i < n; ++i) assert(abs(complex_result[i] - double(expected[i].val())) < 1e-6);
	}
}

int main() {
	check_polynomial();
	check_bitwise();
	check_divisors();
	check_boolean();
	check_minplus();
	check_extrema();
	check_indexed();
}
