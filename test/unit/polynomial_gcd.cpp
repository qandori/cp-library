#include "fps/polynomial_gcd.hpp"
#include <cassert>
#include <random>

using H = fps_detail::PolynomialHalfGCD;
using O = fps_detail::Operations<mint>;

struct PlainMint {
	unsigned v = 0;
	static constexpr int mod() { return 998244353; }
	PlainMint() = default;
	PlainMint(long long x) { x %= mod(); v = unsigned(x < 0 ? x + mod() : x); }
	static PlainMint raw(unsigned x) { PlainMint r; r.v = x; return r; }
	unsigned val() const { return v; }
	PlainMint& operator+=(PlainMint x) { v += x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint& operator-=(PlainMint x) { v += mod() - x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint& operator*=(PlainMint x) { v = std::uint64_t(v) * x.v % mod(); return *this; }
	PlainMint& operator/=(PlainMint x) { return *this *= x.inv(); }
	PlainMint operator-() const { return raw(v ? mod() - v : 0); }
	PlainMint inv() const {
		PlainMint a = *this, r = 1;
		for (unsigned k = mod() - 2; k; k >>= 1, a *= a) if (k & 1) r *= a;
		return r;
	}
	friend PlainMint operator+(PlainMint a, PlainMint b) { return a += b; }
	friend PlainMint operator-(PlainMint a, PlainMint b) { return a -= b; }
	friend PlainMint operator*(PlainMint a, PlainMint b) { return a *= b; }
	friend PlainMint operator/(PlainMint a, PlainMint b) { return a /= b; }
	friend bool operator==(PlainMint, PlainMint) = default;
};

fps naive_remainder(fps a, fps b) {
	O::trim(a), O::trim(b);
	assert(!b.empty());
	if (a.size() < b.size()) return a;
	mint inverse = b.back().inv();
	for (int k = int(a.size() - b.size()); k >= 0; --k) {
		mint q = a[k + b.size() - 1] * inverse;
		for (int j = 0; j < int(b.size()); ++j) a[k + j] -= q * b[j];
	}
	a.resize(b.size() - 1), O::trim(a);
	return a;
}

fps naive_gcd(fps a, fps b) {
	O::trim(a), O::trim(b);
	while (!b.empty()) { auto r = naive_remainder(a, b); a = std::move(b), b = std::move(r); }
	if (!a.empty()) a /= a.back();
	return a;
}

void check(const fps& a, const fps& b, bool compare_naive = true) {
	fps g = polynomial_gcd(a, b);
	if (compare_naive) assert(g == naive_gcd(a, b));
	assert(g.empty() || g.back() == mint(1));
	assert(g.empty() || (naive_remainder(a, g).empty() && naive_remainder(b, g).empty()));
	auto [d, x, y] = polynomial_extended_gcd(a, b);
	assert(d == g);
	fps identity = a * x + b * y;
	O::trim(identity);
	assert(identity == g);
	assert(polynomial_gcd(b, a) == g);
	fps modulus = b; O::trim(modulus);
	if (!modulus.empty()) {
		auto inverse = polynomial_inv_mod(a, b);
		assert(bool(inverse) == (modulus.size() == 1 || g.size() == 1));
		if (inverse) {
			assert(inverse->size() < modulus.size());
			fps expected = modulus.size() == 1 ? fps{} : fps{1};
			assert(naive_remainder(a * *inverse, modulus) == expected);
		}
	}
}

void check_base(const fps& input_a, const fps& input_b) {
	using F = H::F;
	F a = fps_detail::Operations<mint>::import_work(input_a);
	F b = fps_detail::Operations<mint>::import_work(input_b);
	auto actual = H::half(a, b);
	H::Matrix expected;
	const int half = int(a.size()) / 2;
	while (int(b.size()) > half) {
		auto [q, r] = a.divmod(b);
		H::step(expected, q);
		a = std::move(b), b = std::move(r);
	}
	assert(actual.a == expected.a);
	assert(actual.shift == expected.shift);
	assert(actual.identity == expected.identity);
}

int main() {
	std::mt19937 random(294384);
	for (int n = 2; n <= 128; ++n) {
		for (int trial = 0; trial < 12; ++trial) {
			int m = trial == 0 ? n - 1 : 1 + random() % (n - 1);
			fps a(n), b(m);
			for (auto* f : {&a, &b}) {
				for (auto& x : *f) x = trial % 3 ? random() : random() % 2;
				f->back() = 1 + random() % 998244352;
			}
			check_base(a, b);
		}
	}
	for (int trial = 0; trial < 12; ++trial) {
		using P = FormalPowerSeries<PlainMint>;
		P a(140), b(129);
		for (auto* f : {&a, &b}) for (auto& x : *f) x = random() % 13;
		a.back() = b.back() = 1;
		auto g = polynomial_gcd(a, b);
		auto [d, x, y] = polynomial_extended_gcd(a, b);
		assert(g == d);
		auto identity = a * x + b * y;
		fps_detail::Operations<PlainMint>::trim(identity);
		assert(identity == g);
		auto inverse = polynomial_inv_mod(a, b);
		assert(bool(inverse) == (g.size() == 1));
		if (inverse) assert((a * *inverse).divmod(b).second == P{1});
	}
	for (fps a : {fps{}, fps{0}, fps{0, 0}, fps{1}, fps{2}, fps{0, 3}, fps{1, 2, 3, 0}})
		for (fps b : {fps{}, fps{0}, fps{0, 0}, fps{1}, fps{3}, fps{0, 0, 4}, fps{2, 4, 6}}) check(a, b);
	for (int n : {1, 2, 3, 8, 31, 32, 63, 64, 65, 127, 128, 129, 130, 255, 256, 257, 511, 512, 513, 1023}) {
		for (int trial = 0; trial < 9; ++trial) {
			int m = trial == 0 ? n : trial == 1 ? n / 2 : trial == 2 ? 1 : 1 + random() % n;
			fps a(n), b(m), common(trial % 4 ? 0 : n / 3);
			for (auto* f : {&a, &b, &common}) {
				for (auto& x : *f) x = random() % 13;
				if (!f->empty()) f->back() = 1 + random() % 12;
			}
			if (!common.empty()) a *= common, b *= common;
			if (trial == 3) b = a;
			if (trial == 4) a = a * b;
			check(a, b);
			O::trim(a), O::trim(b);
			if (a.size() < b.size()) a.swap(b);
			if (a.size() == b.size()) a = a.divmod(b).second, a.swap(b);
			if (a.size() > b.size()) {
				auto matrix = H::half(a, b);
				for (const auto& left : {H::Matrix{}, matrix}) {
					for (const auto& right : {H::Matrix{}, matrix}) {
						auto full = H::multiply(left, right);
						auto column = H::multiply_column(left, right);
						assert(column.a[0] == full.a[0] && column.a[2] == full.a[2]);
						assert(column.shift == full.shift && column.identity == full.identity);
					}
				}
				for (H::F quotient : {H::F{}, H::F{3}, H::F{1, 2, 3}}) {
					auto full = matrix, column = matrix;
					H::step(full, quotient), H::step_column(column, quotient);
					assert(column.a[0] == full.a[0] && column.a[2] == full.a[2]);
					assert(column.shift == full.shift && column.identity == full.identity);
				}
				fps x = a, y = b;
				H::apply(matrix, x, y);
				fps u = matrix.a[0] * a + matrix.a[1] * b;
				fps v = matrix.a[2] * a + matrix.a[3] * b;
				O::trim(u), O::trim(v);
				assert(x == u && y == v);
				assert(y.size() <= a.size() / 2 && x.size() > a.size() / 2);
				assert(int(a.size() - x.size()) == matrix.shift);
				fps determinant = matrix.a[0] * matrix.a[3] - matrix.a[1] * matrix.a[2];
				O::trim(determinant);
				assert(determinant == fps{1} || determinant == fps{mint(-1)});
			}
		}
	}
	for (int n : {129, 257, 1025, 8193}) {
		fps common(n), a(n / 2), b(n / 2 - 1);
		for (auto* f : {&common, &a, &b}) {
			for (auto& x : *f) x = random();
			f->back() = 1;
		}
		a = a * common, b = b * common;
		assert(polynomial_gcd(a, b) == common);
		check(a, b, false);
		fps sparse(n + 1); sparse.front() = -1, sparse.back() = 1;
		fps divisor{mint(-1), 1};
		assert(polynomial_gcd(sparse, divisor) == divisor);
	}
}
