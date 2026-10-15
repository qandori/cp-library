#include "data_structure/wavelet_matrix.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace std;

void check(const vector<int64_t> &a) {
	WaveletMatrix wm(a);
	assert(wm.size() == static_cast<int>(a.size()) && wm.empty() == a.empty());
	for (int i = 0; i < (static_cast<int>(a.size())); ++i) assert(wm[i] == a[i] && wm.access(i) == a[i]);
	map<int64_t, vector<int>> positions;
	for (int i = 0; i < (static_cast<int>(a.size())); ++i) positions[a[i]].push_back(i);
	for (const auto &[x, indices] : positions) {
		for (int k = 0; k < (static_cast<int>(indices.size())); ++k) assert(wm.select(x, k) == indices[k]);
		assert(wm.select(x, -1) == -1 && wm.select(x, static_cast<int>(indices.size())) == -1);
	}
	mt19937_64 random(123);
	for (int repeat_index = 0; repeat_index < 200; ++repeat_index) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) swap(l, r);
		int64_t x = int(random() % 41) - 20, y = int(random() % 41) - 20;
		if (random() % 10 == 0) x = numeric_limits<int64_t>::min();
		if (random() % 10 == 0) x = numeric_limits<int64_t>::max();
		vector<int64_t> sorted(a.begin() + l, a.begin() + r);
		sort(sorted.begin(), sorted.end());
		int below = lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin();
		int exact = std::count(sorted.begin(), sorted.end(), x);
		assert(wm.count(l, r, x) == exact);
		assert(wm.rank(x, r) == std::count(a.begin(), a.begin() + r, x));
		assert(wm.range_freq(l, r, x) == below);
		int between = 0;
		for (int64_t v : sorted) between += x <= v && v < y;
		assert(wm.range_freq(l, r, x, y) == between);
		optional<int64_t> previous, next;
		if (below) previous = sorted[below - 1];
		if (below < static_cast<int>(sorted.size())) next = sorted[below];
		assert(wm.prev_value(l, r, x) == previous && wm.next_value(l, r, x) == next);
		vector<__int128_t> prefix(sorted.size() + 1);
		for (int i = 0; i < (static_cast<int>(sorted.size())); ++i) prefix[i + 1] = prefix[i] + sorted[i];
		for (int k = 0; k < (static_cast<int>(sorted.size()) + 1); ++k) {
			assert(wm.sum_smallest(l, r, k) == prefix[k]);
			assert(wm.sum_largest(l, r, k) == prefix.back() - prefix[sorted.size() - k]);
		}
		assert(wm.range_sum(l, r) == prefix.back());
		if (!sorted.empty()) {
			int k = random() % sorted.size();
			assert(wm.kth_smallest(l, r, k) == sorted[k]);
			assert(wm.kth_largest(l, r, k) == sorted[sorted.size() - 1 - k]);
		}
	}
	wm.build({});
	assert(wm.empty() && wm.count(0, 0, 1) == 0);
	wm.build(a);
	for (int i = 0; i < (static_cast<int>(a.size())); ++i) assert(wm[i] == a[i]);
	__int128_t total = 0;
	for (int64_t v : a) total += v;
	assert(wm.sum_smallest(0, static_cast<int>(a.size()), static_cast<int>(a.size())) == total);
	wm.build(vector<int64_t>(a.size(), -5));
	assert(wm.sum_largest(0, static_cast<int>(a.size()), static_cast<int>(a.size())) == -__int128_t(5) * static_cast<int>(a.size()));
}

template <class T>
void check_large(const vector<T> &a) {
	WaveletMatrix<T> lazy(a), eager(a, true);
	mt19937_64 random(98765);
	for (int repeat_index = 0; repeat_index < 100; ++repeat_index) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) swap(l, r);
		int k = random() % (r - l + 1);
		vector<T> sorted(a.begin() + l, a.begin() + r);
		sort(sorted.begin(), sorted.end());
		vector<__int128_t> prefix(sorted.size() + 1);
		for (int i = 0; i < (static_cast<int>(sorted.size())); ++i) prefix[i + 1] = prefix[i] + sorted[i];
		assert(lazy.sum_smallest(l, r, k) == prefix[k] && eager.sum_smallest(l, r, k) == prefix[k]);
		assert(lazy.sum_largest(l, r, k) == prefix.back() - prefix[sorted.size() - k]);
		assert(eager.sum_largest(l, r, k) == prefix.back() - prefix[sorted.size() - k]);
		assert(lazy.range_sum(l, r) == prefix.back() && eager.range_sum(l, r) == prefix.back());
		if (k < r - l) assert(lazy.kth_smallest(l, r, k) == sorted[k] && eager.kth_smallest(l, r, k) == sorted[k]);
		T x = a[random() % a.size()];
		assert(lazy.range_freq(l, r, x) == lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin());
		assert(lazy.count(l, r, x) == std::count(sorted.begin(), sorted.end(), x));
		int pos = random() % a.size();
		assert(lazy.access(pos) == a[pos] && eager.access(pos) == a[pos]);
		vector<int> occurrences;
		for (int i = 0; i < (static_cast<int>(a.size())); ++i)
			if (a[i] == x) occurrences.push_back(i);
		int occurrence = random() % occurrences.size();
		assert(lazy.select(x, occurrence) == occurrences[occurrence]);
		assert(lazy.select(x, static_cast<int>(occurrences.size())) == -1 && lazy.select(x, -1) == -1);
	}
	lazy.build({});
	assert(lazy.sum_smallest(0, 0, 0) == 0 && lazy.select(T(0), 0) == -1);
	lazy.build(a);
	assert(lazy.range_sum(0, static_cast<int>(a.size())) == eager.range_sum(0, static_cast<int>(a.size())));
}
template <class T>
void check_radix() {
	mt19937_64 random(87654);
	for (int n : {1023, 1024, 1025, 4097}) {
		vector<T> a(n);
		for (auto &v : a) v = T(random());
		a[0] = numeric_limits<T>::min();
		a[1] = numeric_limits<T>::max();
		check_large(a);
	}
}

int main() {
	WaveletMatrix<> empty;
	assert(empty.empty() && empty.select(1, 0) == -1);
	assert(!empty.prev_value(0, 0, 0) && !empty.next_value(0, 0, 0));
	check({});
	check({7});
	check(vector<int64_t>(129, -3));
	check({numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), -1, 0, 1});
	check(vector<int64_t>(65, numeric_limits<int64_t>::max()));
	check(vector<int64_t>(65, numeric_limits<int64_t>::min()));
	mt19937_64 random(456);
	for (int n : {2, 3, 63, 64, 65, 127, 128, 129, 1000}) {
		vector<int64_t> a(n);
		for (int i = 0; i < n; ++i) a[i] = int(random() % 21) - 10;
		check(a);
		iota(a.begin(), a.end(), -500);
		shuffle(a.begin(), a.end(), random);
		check(a);
	}
	for (int repeat_index = 0; repeat_index < 200; ++repeat_index) {
		vector<int64_t> a(random() % 80);
		for (auto &v : a) v = int64_t(random());
		check(a);
	}
	vector<uint64_t> unsigned_values{0, numeric_limits<uint64_t>::max(), 1};
	WaveletMatrix u(unsigned_values);
	assert(u.kth_smallest(0, 3, 2) == numeric_limits<uint64_t>::max());
	assert(u.range_freq(0, 3, numeric_limits<uint64_t>::max()) == 2);
	assert(u.sum_largest(0, 3, 1) == __int128_t(numeric_limits<uint64_t>::max()));
	assert(u.sum_smallest(0, 3, 3) == __int128_t(numeric_limits<uint64_t>::max()) + 1);
	WaveletMatrix precomputed(vector<int>{3, -2, 3, 1}, true);
	static_assert(same_as<decltype(precomputed)::sum_type, int64_t>);
	assert(precomputed.sum_smallest(0, 4, 2) == -1 && precomputed.sum_largest(0, 4, 2) == 6);
	const auto &constant = precomputed;
	assert(constant.range_sum(1, 4) == 2);
	WaveletMatrix<int64_t, int64_t> narrow(vector<int64_t>{1, 2, 3}, true);
	assert(narrow.sum_smallest(0, 3, 2) == 3);
	static_assert(same_as<WaveletMatrix<>::sum_type, __int128_t>);
	assert(empty.sum_smallest(0, 0, 0) == 0 && empty.sum_largest(0, 0, 0) == 0);
	check_large(vector<int64_t>(4097, -5));
	check_large(vector<uint64_t>(4097, numeric_limits<uint64_t>::max()));
	check_radix<int8_t>();
	check_radix<uint8_t>();
	check_radix<int16_t>();
	check_radix<uint16_t>();
	check_radix<int32_t>();
	check_radix<uint32_t>();
	check_radix<int64_t>();
	check_radix<uint64_t>();
	for (int sigma : {2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 63, 64, 65, 255, 256, 257, 1023, 1024, 1025}) {
		vector<int64_t> a(4097);
		for (int i = 0; i < (static_cast<int>(a.size())); ++i) a[i] = i % sigma - sigma / 2;
		shuffle(a.begin(), a.end(), random);
		check_large(a);
	}
	check_large(vector<int64_t>{numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), 0});
	check_large(vector<int64_t>{numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), -1, 1});
	vector<int64_t> shift(2049);
	for (int i = 0; i < (static_cast<int>(shift.size())); ++i) shift[i] = numeric_limits<int64_t>::min() + i % 5;
	check_large(shift);
	for (int i = 0; i < (static_cast<int>(shift.size())); ++i) shift[i] = numeric_limits<int64_t>::max() - i % 5;
	check_large(shift);
	vector<uint64_t> upper(2049);
	for (int i = 0; i < (static_cast<int>(upper.size())); ++i) upper[i] = numeric_limits<uint64_t>::max() - i % 7;
	check_large(upper);
	WaveletMatrix<pair<int, int>> pairs(vector<pair<int, int>>{{1, 2}, {0, 3}, {1, 2}});
	assert(pairs.count(0, 3, {1, 2}) == 2 && pairs.select({1, 2}, 1) == 2);
	assert(pairs.kth_smallest(0, 3, 0) == make_pair(0, 3));
	WaveletMatrix<bool> booleans(vector<bool>{true, false, true});
	assert(booleans.sum_largest(0, 3, 2) == 2 && booleans.count(0, 3, true) == 2);
	WaveletMatrix<double> real(vector<double>{-0.5, 2.5, 1});
	assert(real.sum_smallest(0, 3, 2) == 0.5 && real.sum_largest(0, 3, 2) == 3.5);
	WaveletMatrix<string> words(vector<string>{"b", "a", "b", "z"});
	assert(words.kth_smallest(0, 4, 1) == "b" && words.select("b", 1) == 2);
	assert(words.prev_value(0, 4, "b") == optional<string>("a"));
}
