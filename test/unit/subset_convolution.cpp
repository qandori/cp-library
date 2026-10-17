#include "convolution/subset_convolution.hpp"
#include "convolution/boolean_convolution.hpp"
#include "math/mint.hpp"
#include "test/benchmark/subset_convolution_baseline.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <type_traits>

template <class T>
std::vector<T> reference(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> c(std::bit_ceil(std::max(a.size(), b.size())), T(0));
	for (std::size_t mask = 0; mask < c.size(); ++mask)
		for (std::size_t sub = mask;; sub = (sub - 1) & mask) {
			if (sub < a.size() && (mask ^ sub) < b.size()) c[mask] += a[sub] * b[mask ^ sub];
			if (!sub) break;
		}
	return c;
}

struct Matrix {
	std::array<std::uint32_t, 4> v;
	explicit Matrix(int n) : v{std::uint32_t(n), 0, 0, std::uint32_t(n)} {}
	Matrix &operator+=(const Matrix &b) {
		for (int i = 0; i < 4; ++i) v[i] += b.v[i];
		return *this;
	}
	Matrix &operator-=(const Matrix &b) {
		for (int i = 0; i < 4; ++i) v[i] -= b.v[i];
		return *this;
	}
	friend Matrix operator*(const Matrix &a, const Matrix &b) {
		Matrix c(0);
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k) c.v[2 * i + j] += a.v[2 * i + k] * b.v[2 * k + j];
		return c;
	}
	friend bool operator==(const Matrix &, const Matrix &) = default;
};

std::mt19937_64 rng(20261017);
template <class T>
T random_value() {
	if constexpr (std::is_same_v<T, Matrix>) {
		Matrix x(0);
		for (auto &y : x.v) y = rng();
		return x;
	} else if constexpr (std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>) return T(rng());
	else return T(int(rng() % 15) - 7);
}

template <class T>
void check() {
	for (int trial = 0; trial < 180; ++trial) {
		std::vector<T> a(rng() % 90, T(0)), b(rng() % 90, T(0));
		for (auto &x : a) x = random_value<T>();
		for (auto &x : b) x = random_value<T>();
		auto copy_a = a, copy_b = b;
		assert(subset_convolution(a, b) == reference(a, b));
		assert(a == copy_a && b == copy_b);
	}
	for (int e = 0; e <= 12; ++e)
		for (int delta : {-1, 0, 1}) {
			std::size_t n = (std::size_t(1) << e) + delta;
			std::vector<T> a(n, T(0)), b(n / 2 + 1, T(0));
			for (auto &x : a) x = random_value<T>();
			for (auto &x : b) x = random_value<T>();
			assert(subset_convolution(a, b) == reference(a, b));
			assert(subset_convolution(a, a) == subset_convolution_baseline::subset_convolution(a, a));
		}
}

void check_boolean() {
	for (int trial = 0; trial < 30; ++trial) {
		std::vector<std::uint32_t> a(1 + rng() % 40), b(1 + rng() % 40);
		for (auto &x : a) x = rng();
		for (auto &x : b) x = rng();
		std::vector<std::uint32_t> expected_or(std::bit_ceil(std::max(a.size(), b.size())));
		std::vector<std::uint32_t> expected_and(expected_or.size(), ~std::uint32_t(0));
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j)
				if (!(i & j)) {
					expected_or[i | j] |= a[i] & b[j];
					expected_and[i | j] &= a[i] | b[j];
				}
		assert(subset_or_convolution(a, b) == expected_or);
		assert(subset_and_convolution(a, b) == expected_and);
	}
	assert(subset_or_convolution(std::vector<bool>{false, true}, std::vector<bool>{true, false}) == std::vector<bool>({false, true}));
}

int main() {
	check<mint>();
	check<std::uint32_t>();
	check<std::uint64_t>();
	check<__int128_t>();
	check<double>();
	check<Matrix>();
	check_boolean();
	std::vector<mint> a(1 << 16, mint(1));
	auto c = subset_convolution(a, a);
	for (std::size_t mask = 0; mask < c.size(); ++mask) assert(c[mask] == mint(2).pow(std::popcount(mask)));
}
