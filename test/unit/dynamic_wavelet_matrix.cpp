#include "data_structure/dynamic_wavelet_matrix.hpp"
#include "data_structure/wavelet_matrix.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <utility>
#include <vector>

template <class WM, class T>
void check(const WM &wm, const std::vector<T> &a) {
	assert(wm.size() == int(a.size()) && wm.empty() == a.empty());
	for (int i = 0; i < int(a.size()); ++i) assert(wm[i] == a[i]);
	std::mt19937_64 random(987);
	for (int i = 0; i < 70; ++i) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) std::swap(l, r);
		T x = T(random()), y = T(random());
		if (!a.empty() && (i & 1)) x = a[random() % a.size()];
		if (i == 0) x = std::numeric_limits<T>::min();
		if (i == 1) x = std::numeric_limits<T>::max();
		std::vector<T> sorted(a.begin() + l, a.begin() + r);
		std::sort(sorted.begin(), sorted.end());
		int below = std::lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin();
		assert(wm.count(l, r, x) == std::count(sorted.begin(), sorted.end(), x));
		assert(wm.rank(x, r) == std::count(a.begin(), a.begin() + r, x));
		assert(wm.range_freq(l, r, x) == below);
		assert(wm.range_freq(l, r, x, y) == std::count_if(sorted.begin(), sorted.end(), [&](T v) { return x <= v && v < y; }));
		assert(wm.prev_value(l, r, x) == (below ? std::optional<T>(sorted[below - 1]) : std::nullopt));
		assert(wm.next_value(l, r, x) == (below < int(sorted.size()) ? std::optional<T>(sorted[below]) : std::nullopt));
		if (!sorted.empty()) {
			int k = random() % sorted.size();
			assert(wm.kth_smallest(l, r, k) == sorted[k]);
			assert(wm.kth_largest(l, r, k) == sorted[sorted.size() - 1 - k]);
		}
		std::vector<int> positions;
		for (int j = 0; j < int(a.size()); ++j)
			if (a[j] == x) positions.push_back(j);
		assert(wm.select(x, -1) == -1);
		assert(wm.select(x, int(positions.size())) == -1);
		for (int k = 0; k < int(positions.size()); k += 7) assert(wm.select(x, k) == positions[k]);
	}
}

template <class T, int Bits>
void random_test() {
	std::mt19937_64 random(sizeof(T) * 713 + Bits);
	auto value = [&]() -> T {
		if constexpr (Bits < int(sizeof(T) * 8)) {
			T v = T(random() & ((std::uint64_t(1) << Bits) - 1));
			if constexpr (std::is_signed_v<T>) v -= T(std::uint64_t(1) << (Bits - 1));
			return v;
		} else return T(random());
	};
	std::vector<T> a;
	DynamicWaveletMatrix<T, Bits> wm;
	check(wm, a);
	wm.reserve(6000);
	for (int i = 0; i < 5000; ++i) {
		int op = random() % 6, p = random() % (a.size() + 1);
		if (!a.empty() && p == int(a.size()) && op > 1) --p;
		T x = value();
		if (op <= 1 || a.empty()) {
			wm.insert(p, x);
			a.insert(a.begin() + p, x);
		} else if (op == 2) {
			assert(wm.erase(p) == a[p]);
			a.erase(a.begin() + p);
		} else {
			wm.set(p, op == 5 ? a[p] : x);
			if (op != 5) a[p] = x;
		}
		if (i % 199 == 0) check(wm, a);
	}
	check(wm, a);
	auto copy = wm;
	check(copy, a);
	auto moved = std::move(copy);
	assert(copy.empty());
	copy.push_back(value());
	copy.pop_back();
	assert(copy.empty());
	check(moved, a);
	copy = std::move(moved);
	assert(moved.empty());
	check(copy, a);
	wm.build(a);
	check(wm, a);
	wm.clear();
	a.clear();
	check(wm, a);
	wm.build(std::vector<T>(1100, value()));
	while (!wm.empty()) wm.pop_back();
	check(wm, a);
}

int main() {
	random_test<std::int64_t, 64>();
	random_test<std::uint64_t, 64>();
	random_test<int, 32>();
	random_test<int, 12>();
	random_test<unsigned, 10>();
	random_test<signed char, 8>();
	random_test<int, 1>();
	random_test<unsigned, 1>();
	std::vector<std::int64_t> boundaries{0, -1, 1, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()};
	DynamicWaveletMatrix wm(boundaries);
	check(wm, boundaries);
	for (int i = 0; i < 5; ++i) {
		wm.set(i, boundaries[4 - i]);
		boundaries[i] = boundaries[4 - i];
		check(wm, boundaries);
	}
	std::mt19937 random(713);
	std::vector<int> a(40000);
	for (auto &v : a) v = random() & 65535;
	DynamicWaveletMatrix<int, 17> dynamic(a);
	for (int i = 0; i < 20000; ++i) {
		int p = random() % a.size();
		if (i & 1) {
			a[p] = int(random() & 65535);
			dynamic.set(p, a[p]);
		} else {
			assert(dynamic.erase(p) == a[p]);
			a.erase(a.begin() + p);
			p = random() % (a.size() + 1);
			int value = random() & 65535;
			dynamic.insert(p, value);
			a.insert(a.begin() + p, value);
		}
	}
	WaveletMatrix reference(a);
	for (int i = 0; i < 2000; ++i) {
		int l = random() % a.size(), r = random() % a.size();
		if (l > r) std::swap(l, r);
		++r;
		int k = random() % (r - l), x = random() & 65535;
		assert(dynamic.kth_smallest(l, r, k) == reference.kth_smallest(l, r, k));
		assert(dynamic.range_freq(l, r, x) == reference.range_freq(l, r, x));
		assert(dynamic.count(l, r, x) == reference.count(l, r, x));
	}
	std::mt19937_64 random64(1741);
	std::vector<std::int64_t> wide(10000);
	for (auto &x : wide) x = std::int64_t(random64());
	wide[0] = std::numeric_limits<std::int64_t>::min();
	wide[1] = std::numeric_limits<std::int64_t>::max();
	DynamicWaveletMatrix full(wide);
	for (int i = 0; i < 2000; ++i) {
		int p = random64() % wide.size();
		if (i & 1) {
			wide[p] = std::int64_t(random64());
			full.set(p, wide[p]);
		} else {
			assert(full.erase(p) == wide[p]);
			wide.erase(wide.begin() + p);
			p = random64() % (wide.size() + 1);
			auto value = std::int64_t(random64());
			wide.insert(wide.begin() + p, value);
			full.insert(p, value);
		}
	}
	for (int i = 0; i < int(wide.size()); ++i) assert(full[i] == wide[i]);
	WaveletMatrix wide_reference(wide);
	for (int i = 0; i < 200; ++i) {
		int l = random64() % wide.size(), r = random64() % wide.size();
		if (l > r) std::swap(l, r);
		++r;
		int k = random64() % (r - l);
		auto value = wide[random64() % wide.size()];
		assert(full.kth_smallest(l, r, k) == wide_reference.kth_smallest(l, r, k));
		assert(full.range_freq(l, r, value) == wide_reference.range_freq(l, r, value));
		assert(full.select(value, 0) == wide_reference.select(value, 0));
	}
}
