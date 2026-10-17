#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <optional>
#include <random>
#include <utility>
#include <vector>

void require(bool ok) {
	if (!ok) std::abort();
}

class ReferenceBitVector {
	std::vector<bool> values;

  public:
	struct RankPair { int first, second; };
	void clear() { values.clear(); }
	void reserve(int n) { values.reserve(n); }
	std::size_t memory_usage() const { return values.capacity() / 8; }
	void build(int n, const std::vector<std::uint64_t> &packed) {
		values.resize(n);
		for (int i = 0; i < n; ++i) values[i] = (packed[i >> 6] >> (i & 63)) & 1;
	}
	int rank1(int p) const { return std::count(values.begin(), values.begin() + p, true); }
	RankPair rank1(int l, int r) const { return {rank1(l), rank1(r)}; }
	std::pair<int, bool> access_rank(int p) const { return {rank1(p), values[p]}; }
	int insert(int p, bool value) {
		int rank = rank1(p);
		values.insert(values.begin() + p, value);
		return rank;
	}
	std::pair<int, bool> erase_rank(int p) {
		auto result = access_rank(p);
		values.erase(values.begin() + p);
		return result;
	}
	std::pair<int, bool> set_rank(int p, bool value) {
		auto result = access_rank(p);
		values[p] = value;
		return result;
	}
	int select(bool value, int k) const {
		for (int i = 0; i < int(values.size()); ++i)
			if (values[i] == value && !k--) return i;
		return -1;
	}
};

class SelectOnlyBitVector {
	ReferenceBitVector bits;

  public:
	void build(int n, const std::vector<std::uint64_t> &packed) { bits.build(n, packed); }
	auto rank1(int l, int r) const { return bits.rank1(l, r); }
	int select(bool value, int k) const { return bits.select(value, k); }
};

template <class WM, class T>
void check(const WM &wm, const std::vector<T> &a, int l, int r, T bound) {
	std::optional<T> previous, next;
	for (int i = l; i < r; ++i) {
		if (a[i] < bound && (!previous || *previous < a[i])) previous = a[i];
		if (bound <= a[i] && (!next || a[i] < *next)) next = a[i];
	}
	require(wm.prev_value(l, r, bound) == previous);
	require(wm.next_value(l, r, bound) == next);
}

template <class WM, class T>
void check_select(const WM &wm, const std::vector<T> &a, T value) {
	require(wm.select(value, -1) == -1);
	int k = 0;
	for (int i = 0; i < int(a.size()); ++i)
		if (a[i] == value) require(wm.select(value, k++) == i);
	require(wm.select(value, k) == -1);
	require(wm.select(value, std::numeric_limits<int>::max()) == -1);
}

template <class T, int Bits, class BV = DynamicBitVector<>>
void all_intervals(const std::vector<T> &a, const std::vector<T> &bounds) {
	DynamicWaveletMatrix<T, Bits, BV> wm(a);
	for (T bound : bounds) check_select(wm, a, bound);
	for (int l = 0; l <= int(a.size()); ++l)
		for (int r = l; r <= int(a.size()); ++r)
			for (T bound : bounds) check(wm, a, l, r, bound);
}

void exhaustive() {
	for (int n = 0, limit = 1; n <= 5; ++n, limit *= 4) {
		for (int code = 0; code < limit; ++code) {
			std::vector<int> a(n);
			for (int i = 0, digits = code; i < n; ++i, digits /= 4)
				a[i] = std::array<int, 4>{-4, -1, 0, 3}[digits % 4];
			all_intervals<int, 3>(a, {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4});
		}
	}
	all_intervals<int, 1>({-1, 0, -1, 0, 0, -1}, {-2, -1, 0, 1});
	all_intervals<unsigned, 1>({1, 0, 1, 1, 0, 0}, {0, 1, 2, 3});
	all_intervals<unsigned, 3>({0, 7, 1, 6, 2, 5, 3, 4}, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
	all_intervals<signed char, 8>({-128, 127, -1, 0, 1, -127, 126}, {-128, -127, -2, -1, 0, 1, 2, 126, 127});
	all_intervals<unsigned char, 8>({0, 255, 1, 254, 127, 128}, {0, 1, 2, 126, 127, 128, 129, 254, 255});
	all_intervals<int, 3, ReferenceBitVector>({-4, 3, 0, -1, 2, -3, 0, 3}, {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4});
}

template <class T>
void boundaries() {
	using U = std::make_unsigned_t<T>;
	constexpr T lo = std::numeric_limits<T>::min(), hi = std::numeric_limits<T>::max();
	std::vector<T> a{lo, hi, T(0), T(1), T(lo + 1), T(hi - 1), T(7), T(0), T(hi - 1)};
	std::vector<T> bounds{lo, hi, T(lo + 1), T(hi - 1)};
	for (int bit = 0; bit < 64; ++bit) {
		U key = U(1) << bit;
		for (U value : {key, U(key - 1), U(key + 1), U(~key)}) bounds.push_back(T(value));
	}
	all_intervals<T, 64>(a, bounds);
	for (int bit = 0; bit < 64; ++bit) {
		T x = T((U(1) << bit) - 1), y = T(U(x) ^ (U(1) << bit));
		all_intervals<T, 64>({x, y, x, y, x}, {lo, hi, x, y, T(U(x) + 1), T(U(y) + 1)});
	}
	for (T x : {lo, hi, T(0), T(1), T(7)})
		all_intervals<T, 64>(std::vector<T>(9, x), bounds);
}

template <class T, int Bits, class BV = DynamicBitVector<128, 4>>
void mutations() {
	using U = std::make_unsigned_t<T>;
	constexpr int width = std::numeric_limits<U>::digits;
	constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	std::mt19937_64 rng(998173 + Bits);
	auto value = [&]() {
		U x = U(rng()) & mask;
		if constexpr (std::is_signed_v<T> && Bits < width)
			if (x & (U(1) << (Bits - 1))) x |= U(~mask);
		return T(x);
	};
	std::vector<T> a(193);
	for (auto &x : a) x = value();
	DynamicWaveletMatrix<T, Bits, BV> wm(a);
	for (int i = 0; i < 1800; ++i) {
		int op = rng() % 6, p = rng() % (a.size() + 1);
		T x = value();
		if (a.empty() || (op == 0 && a.size() < 300)) {
			wm.insert(p, x), a.insert(a.begin() + p, x);
		} else {
			p %= a.size();
			if (op == 1) {
				require(wm.erase(p) == a[p]);
				a.erase(a.begin() + p);
			} else wm.set(p, x), a[p] = x;
		}
		for (int j = 0; j < 6; ++j) {
			int l = rng() % (a.size() + 1), r = rng() % (a.size() + 1);
			if (l > r) std::swap(l, r);
			T bound = T(rng());
			if (j == 0 && !a.empty()) bound = a[rng() % a.size()];
			if (j == 1) bound = std::numeric_limits<T>::min();
			if (j == 2) bound = std::numeric_limits<T>::max();
			check(wm, a, l, r, bound);
		}
		if (i % 41 == 0) {
			check_select(wm, a, x);
			check_select(wm, a, T(rng()));
		}
		if (i % 151 == 0) {
			auto copy = wm;
			auto moved = std::move(copy);
			require(copy.empty());
			check(moved, a, 0, int(a.size()), x);
			wm.swap(moved);
			wm.reserve(400);
			wm.build(a);
		}
	}
	wm.clear();
	check(wm, std::vector<T>{}, 0, 0, T(0));
	wm.build(a);
	for (int p = 0; p < int(a.size()); ++p) {
		check(wm, a, p, p, a[p]);
		check(wm, a, p, p + 1, a[p]);
	}
}

int main() {
	std::vector<int> values{-4, 0, 2, 7, 0};
	DynamicWaveletMatrix<int, 8, SelectOnlyBitVector> select_only(values);
	for (int x = -5; x <= 8; ++x) check_select(select_only, values, x);
	exhaustive();
	boundaries<std::int64_t>();
	boundaries<std::uint64_t>();
	mutations<int, 1>();
	mutations<unsigned, 1>();
	mutations<int, 9>();
	mutations<unsigned, 9>();
	mutations<std::int64_t, 64>();
	mutations<std::uint64_t, 64>();
	mutations<int, 4, ReferenceBitVector>();
}
