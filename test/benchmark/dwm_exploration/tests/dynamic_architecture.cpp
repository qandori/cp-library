#include "components/dynamic_bit_vector.hpp"
#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

struct ScalarBits {
	std::vector<unsigned char> a;
	void reserve(int n) { a.reserve(n); }
	void clear() { a.clear(); }
	std::size_t memory_usage() const { return a.capacity(); }
	void build(int n, const std::vector<std::uint64_t> &packed) {
		a.resize(n);
		for (int i = 0; i < n; ++i) a[i] = (packed[i >> 6] >> (i & 63)) & 1;
	}
	int rank1(int p) const { return std::accumulate(a.begin(), a.begin() + p, 0); }
	std::pair<int, int> rank1(int l, int r) const { return {rank1(l), rank1(r)}; }
	std::pair<int, bool> access_rank(int p) const { return {rank1(p), bool(a[p])}; }
	int insert(int p, bool value) {
		int rank = rank1(p);
		a.insert(a.begin() + p, value);
		return rank;
	}
	std::pair<int, bool> erase_rank(int p) {
		auto result = access_rank(p);
		a.erase(a.begin() + p);
		return result;
	}
	std::pair<int, bool> set_rank(int p, bool value) {
		auto result = access_rank(p);
		a[p] = value;
		return result;
	}
	int select(bool value, int k) const {
		if (k < 0) return -1;
		for (int i = 0; i < int(a.size()); ++i)
			if (a[i] == value && !k--) return i;
		return -1;
	}
};

template <class BV>
void check_bits(const BV &bits, const std::vector<bool> &a) {
	assert(bits.size() == int(a.size()) && bits.empty() == a.empty());
	std::vector<int> prefix(a.size() + 1);
	for (int i = 0; i < int(a.size()); ++i) {
		assert(bits.access(i) == a[i] && bits[i] == a[i]);
		assert(bits.access_rank(i) == std::pair(prefix[i], bool(a[i])));
		prefix[i + 1] = prefix[i] + a[i];
	}
	assert(bits.count() == prefix.back());
	for (int i = 0; i <= int(a.size()); ++i) {
		assert(bits.rank1(i) == prefix[i] && bits.rank0(i) == i - prefix[i]);
		assert(bits.rank1(i, i) == std::pair(prefix[i], prefix[i]));
	}
	std::mt19937 random(37513);
	for (int rep = 0; rep < 400; ++rep) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) std::swap(l, r);
		assert(bits.rank1(l, r) == std::pair(prefix[l], prefix[r]));
		if (l < int(a.size())) {
			int k = a[l] ? prefix[l] : l - prefix[l];
			assert(bits.select(a[l], k) == l);
			assert((a[l] ? bits.select1(k) : bits.select0(k)) == l);
		}
	}
	assert(bits.select0(-1) == -1 && bits.select1(-1) == -1);
	assert(bits.select1(prefix.back()) == -1);
	assert(bits.select0(int(a.size()) - prefix.back()) == -1);
}

template <int Block, int Branch>
void test_bit_layout() {
	using BV = DynamicBitVector<Block, Branch>;
	std::mt19937 random(Block * 713 + Branch);
	BV bits;
	bits.reserve(Block * Branch + 300);
	for (int n : {0, 1, 63, 64, 65, Block - 1, Block, Block + 1, Block * Branch + 1}) {
		std::vector<bool> a(n);
		std::vector<std::uint64_t> packed((n + 63) / 64 + 1, ~std::uint64_t(0));
		for (int i = 0; i < n; ++i) {
			a[i] = random() & 1;
			if (!a[i]) packed[i >> 6] &= ~(std::uint64_t(1) << (i & 63));
		}
		bits.build(n, packed);
		check_bits(bits, a);
		for (int rep = 0; rep < 128; ++rep) {
			bool value = random() & 1;
			int p = random() % (a.size() + 1);
			if (rep % 4 == 0 || a.empty()) {
				assert(bits.insert(p, value) == std::count(a.begin(), a.begin() + p, true));
				a.insert(a.begin() + p, value);
			} else {
				if (p == int(a.size())) --p;
				int before = std::count(a.begin(), a.begin() + p, true);
				bool old = a[p];
				if (rep % 4 == 1) {
					assert(bits.erase_rank(p) == std::pair(before, old));
					a.erase(a.begin() + p);
				} else if (rep % 4 == 2) {
					assert(bits.set_rank(p, value) == std::pair(before, old));
					a[p] = value;
				} else {
					int to = random() % a.size();
					a.erase(a.begin() + p);
					int next = std::count(a.begin(), a.begin() + to, true);
					a.insert(a.begin() + to, value);
					assert(bits.move_rank(p, to, value) == std::tuple(before, old, next));
				}
			}
		}
		check_bits(bits, a);
		BV copy;
		copy = bits;
		copy = copy;
		copy = std::move(copy);
		check_bits(copy, a);
		BV empty;
		empty.swap(copy);
		assert(copy.empty());
		check_bits(empty, a);
		auto capacity = empty.memory_usage();
		empty.clear();
		assert(empty.empty() && empty.memory_usage() == capacity);
		empty.push_back(true);
		assert(empty.pop_back() && empty.empty());
	}
	std::vector<bool> ones(Block, true);
	bits.build(ones);
	check_bits(bits, ones);
}

void test_word_select() {
	std::mt19937_64 random(21371);
	for (int length : {1, 2, 7, 8, 15, 31, 32, 63, 64, 65, 127, 128, 129}) {
		for (int rep = 0; rep < 90; ++rep) {
			std::vector<std::uint64_t> packed((length + 63) / 64);
			for (auto &word : packed) word = rep < 64 ? std::uint64_t(1) << rep : rep == 64 ? ~std::uint64_t(0) : rep == 65 ? 0 : random();
			DynamicBitVector<128, 4> bits;
			bits.build(length, packed);
			for (bool value : {false, true}) {
				int k = 0;
				for (int i = 0; i < length; ++i)
					if (bool((packed[i >> 6] >> (i & 63)) & 1) == value) assert(bits.select(value, k++) == i);
				assert(bits.select(value, k) == -1);
			}
		}
	}
}

template <class WM, class T>
void check_wavelet(const WM &wm, const std::vector<T> &a) {
	assert(wm.size() == int(a.size()) && wm.empty() == a.empty());
	for (int i = 0; i < int(a.size()); ++i) assert(wm.access(i) == a[i] && wm[i] == a[i]);
	std::mt19937_64 random(1397);
	for (int rep = 0; rep < 80; ++rep) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) std::swap(l, r);
		T x = rep == 0 ? std::numeric_limits<T>::min() : rep == 1 ? std::numeric_limits<T>::max() : a.empty() ? T(0) : a[random() % a.size()];
		T y = T(random());
		std::vector<T> sorted(a.begin() + l, a.begin() + r);
		std::sort(sorted.begin(), sorted.end());
		int below = std::lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin();
		assert(wm.range_freq(l, r, x) == below);
		assert(wm.range_freq(l, r, x, y) == std::count_if(sorted.begin(), sorted.end(), [&](T value) { return x <= value && value < y; }));
		assert(wm.count(l, r, x) == std::count(sorted.begin(), sorted.end(), x));
		assert(wm.rank(x, r) == std::count(a.begin(), a.begin() + r, x));
		assert(wm.prev_value(l, r, x) == (below ? std::optional<T>(sorted[below - 1]) : std::nullopt));
		assert(wm.next_value(l, r, x) == (below < int(sorted.size()) ? std::optional<T>(sorted[below]) : std::nullopt));
		if (!sorted.empty()) {
			int k = random() % sorted.size();
			assert(wm.kth_smallest(l, r, k) == sorted[k]);
			assert(wm.kth_largest(l, r, k) == sorted[sorted.size() - 1 - k]);
		}
		int k = 0;
		for (int i = 0; i < int(a.size()); ++i)
			if (a[i] == x) assert(wm.select(x, k++) == i);
		assert(wm.select(x, k) == -1 && wm.select(x, -1) == -1);
	}
}

template <class T, int Bits = sizeof(T) * 8, class BV = DynamicBitVector<>>
void test_build() {
	using WM = DynamicWaveletMatrix<T, Bits, BV>;
	using U = std::make_unsigned_t<T>;
	constexpr U mask = std::numeric_limits<U>::max() >> (sizeof(T) * 8 - Bits);
	constexpr U sign = std::is_signed_v<T> ? U(1) << (Bits - 1) : 0;
	std::mt19937_64 random(7171 + sizeof(T) * 13 + Bits);
	auto value = [&]() {
		U x = U(random()) & mask;
		if constexpr (std::is_signed_v<T> && Bits < int(sizeof(T) * 8)) if (x & sign) x |= U(~mask);
		return T(x);
	};
	WM wm;
	wm.reserve(4100);
	for (int n : {0, 1, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 65, 127, 128, 1023, 1024, 1025, 4097}) {
		if constexpr (std::is_same_v<BV, ScalarBits>) if (n > 65) continue;
		std::vector<T> a(n);
		for (auto &x : a) x = value();
		wm.build(a);
		check_wavelet(wm, a);
		for (int rep = 0; rep < 20; ++rep) {
			int p = random() % (a.size() + 1);
			T x = value();
			if (rep % 3 == 0 || a.empty()) wm.insert(p, x), a.insert(a.begin() + p, x);
			else {
				if (p == int(a.size())) --p;
				if (rep % 3 == 1) wm.set(p, x), a[p] = x;
				else {
					assert(wm.erase(p) == a[p]);
					a.erase(a.begin() + p);
				}
			}
		}
		check_wavelet(wm, a);
		auto copy = wm;
		copy = copy;
		copy = std::move(copy);
		WM moved = std::move(copy);
		assert(copy.empty());
		copy.push_back(value());
		copy.pop_back();
		assert(copy.empty());
		wm.swap(moved);
		check_wavelet(wm, a);
		auto capacity = wm.memory_usage();
		wm.clear();
		assert(wm.empty() && wm.memory_usage() == capacity);
		std::fill(a.begin(), a.end(), T(0));
		wm.build(a);
		check_wavelet(wm, a);
	}
}

int main() {
	test_word_select();
	test_bit_layout<128, 4>();
	test_bit_layout<1024, 32>();
	test_bit_layout<65536, 4>();
	test_build<std::int8_t>();
	test_build<std::uint16_t>();
	test_build<std::int32_t>();
	test_build<std::uint32_t, 20>();
	test_build<std::int64_t>();
	test_build<std::uint64_t>();
	test_build<std::int64_t, 1>();
	test_build<std::uint64_t, 1>();
	test_build<int, 12, ScalarBits>();
}
