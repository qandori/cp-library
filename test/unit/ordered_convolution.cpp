#include "convolution/ordered_convolution.hpp"
#include "../benchmark/ordered_convolution_baseline.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

struct OrderedRing {
	std::int64_t x;
	OrderedRing() = delete;
	OrderedRing(std::int64_t x) : x(x) {}
	OrderedRing &operator+=(OrderedRing b) { x += b.x; return *this; }
	OrderedRing &operator-=(OrderedRing b) { x -= b.x; return *this; }
	friend OrderedRing operator*(OrderedRing a, OrderedRing b) { return a.x * b.x; }
	friend bool operator==(OrderedRing, OrderedRing) = default;
};

template <class T>
void check() {
	std::mt19937 rng(20261017);
	for (int n : {0, 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257})
		for (int m : {0, 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257}) {
			std::vector<T> a(n, T(0)), b(m, T(0)), expected(n && m ? n + m - 1 : 0, T(0));
			for (auto &x : a) x = T(int(rng() % 19) - 9);
			for (auto &x : b) x = T(int(rng() % 19) - 9);
			for (int i = 0; i < n; ++i)
				for (int j = i + 1; j < m; ++j) expected[i + j] += a[i] * b[j];
			assert(ordered_convolution(a, b) == expected);
			assert(ordered_convolution(a, b) == ordered_convolution_baseline(a, b));
		}
}

int main() {
	check<mint>();
	check<std::int64_t>();
	check<OrderedRing>();
	std::mt19937 rng(20261018);
	for (auto [n, m] : std::vector<std::pair<int, int>>{{1024, 1024}, {4096, 173}, {257, 4096}}) {
		std::vector<mint> a(n), b(m);
		for (auto &x : a) x = mint::raw(rng() % mint::mod());
		for (auto &x : b) x = mint::raw(rng() % mint::mod());
		assert(ordered_convolution(a, b) == ordered_convolution_baseline(a, b));
	}
	std::vector<double> a{1.25, 2.5, -4, 7}, b{2, 0.5, 3};
	assert(ordered_convolution(a, b) == ordered_convolution_baseline(a, b));
}
