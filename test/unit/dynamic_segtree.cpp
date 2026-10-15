#include "data_structure/dynamic_segtree.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace std;
long long add(long long a, long long b) { return a + b; }
long long zero() { return 0; }
using Tree = DynamicSegtree<long long, add, zero>;
string concat(const string &a, const string &b) { return a + b; }
string empty_string() { return {}; }
struct Box {
	long long x;
	Box() = delete;
	explicit Box(long long x) : x(x) {}
};
Box box_add(const Box &a, const Box &b) { return Box(a.x + b.x); }
Box box_zero() { return Box(0); }
int calls = 0;
long long counted_add(long long a, long long b) {
	++calls;
	return a + b;
}

int main() {
	mt19937_64 rng(592367);
	Tree empty;
	assert(empty.size() == 0 && empty.node_count() == 0 && empty.prod(0, 0) == 0 && empty.all_prod() == 0);
	assert(empty.max_right(0, [](auto x) { return x == 0; }) == 0);
	assert(empty.min_left(0, [](auto x) { return x == 0; }) == 0);
	for (int n : {1, 2, 3, 7, 8, 9, 31, 64, 129}) {
		Tree tree(-n / 2, n - n / 2);
		vector<long long> a(n);
		vector<bool> used(n);
		for (int t = 0; t < 3000; ++t) {
			int i = rng() % n;
			long long value = rng() % 30;
			if (rng() & 1) tree.set(i - n / 2, a[i] = value);
			else tree.multiply(i - n / 2, value), a[i] += value;
			used[i] = true;
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) swap(l, r);
			assert(tree.get(i - n / 2) == a[i]);
			assert(tree.prod(l - n / 2, r - n / 2) == accumulate(a.begin() + l, a.begin() + r, 0LL));
			assert(tree.all_prod() == accumulate(a.begin(), a.end(), 0LL));
			assert(tree.node_count() == size_t(count(used.begin(), used.end(), true)));
			long long limit = rng() % 200;
			int right = l, left = r;
			long long sum = 0;
			while (right < n && sum + a[right] <= limit) sum += a[right++];
			sum = 0;
			while (left > 0 && a[left - 1] + sum <= limit) sum += a[--left];
			assert(tree.max_right(l - n / 2, [&](auto x) { return x <= limit; }) == right - n / 2);
			assert(tree.min_left(r - n / 2, [&](auto x) { return x <= limit; }) == left - n / 2);
		}
		auto copy = tree;
		copy.set(-n / 2, 0);
		assert(tree.get(-n / 2) == a[0]);
		auto moved = std::move(copy);
		assert(moved.get(-n / 2) == 0);
		assert(copy.node_count() == 0 && copy.all_prod() == 0);
		copy.multiply(-n / 2, 9);
		assert(copy.get(-n / 2) == 9);
		copy = std::move(moved);
		assert(moved.node_count() == 0 && copy.get(-n / 2) == 0);
		tree.clear();
		assert(tree.node_count() == 0 && tree.all_prod() == 0);
		tree.reserve(n);
		tree.multiply(-n / 2, 7);
		assert(tree.get(-n / 2) == 7 && tree.node_count() == 1);
	}
	array<int, 7> order{0, 1, 2, 3, 4, 5, 6};
	do {
		Tree tree(16);
		for (int x : order) tree.set(2 * x + 1, x + 1);
		for (int l = 0; l <= 16; ++l) {
			long long sum = 0;
			for (int r = l; r <= 16; ++r) {
				assert(tree.prod(l, r) == sum);
				if (r < 14 && r % 2) sum += r / 2 + 1;
			}
		}
	} while (next_permutation(order.begin(), order.end()));
	DynamicSegtree<string, concat, empty_string> text(256);
	vector<string> words(256);
	for (int t = 0; t < 3000; ++t) {
		int i = rng() % words.size();
		string value(1, char('a' + rng() % 26));
		if (rng() & 1) text.set(i, words[i] = value);
		else text.multiply(i, value), words[i] += value;
		int l = rng() % 257, r = rng() % 257;
		if (l > r) swap(l, r);
		string expected;
		for (int j = l; j < r; ++j) expected += words[j];
		assert(text.prod(l, r) == expected);
		int cut = l + (r - l) / 2;
		string prefix;
		for (int j = l; j < cut; ++j) prefix += words[j];
		int right = l;
		string current;
		while (right < 256 && prefix.starts_with(current + words[right])) current += words[right++];
		assert(text.max_right(l, [&](const auto &s) { return prefix.starts_with(s); }) == right);
		string suffix;
		for (int j = cut; j < r; ++j) suffix += words[j];
		int left = r;
		current.clear();
		while (left > 0 && suffix.ends_with(words[left - 1] + current)) current = words[--left] + current;
		assert(text.min_left(r, [&](const auto &s) { return suffix.ends_with(s); }) == left);
	}
	const auto lo = numeric_limits<int64_t>::min(), hi = numeric_limits<int64_t>::max();
	Tree wide(lo, hi);
	map<int64_t, long long> reference;
	vector<int64_t> positions{lo, lo + 1, lo + 2, -1, 0, 1, hi - 1, hi - 2};
	for (int i = 0; i < 64; ++i) {
		positions.push_back(int64_t(uint64_t(1) << i));
		positions.push_back(~(uint64_t(1) << i));
	}
	for (int i = 0; i < 500; ++i) positions.push_back(int64_t(rng()));
	for (int t = 0; t < 4000; ++t) {
		auto p = positions[rng() % positions.size()];
		if (p == hi) continue;
		long long v = rng() % 100;
		wide.multiply(p, v);
		reference[p] += v;
		auto l = positions[rng() % positions.size()], r = positions[rng() % positions.size()];
		if (l > r) swap(l, r);
		long long sum = 0;
		for (auto it = reference.lower_bound(l); it != reference.end() && it->first < r; ++it) sum += it->second;
		assert(wide.get(p) == reference[p] && wide.prod(l, r) == sum);
		assert(wide.node_count() == reference.size());
		long long limit = rng() % 1000;
		int64_t right = hi, left = lo;
		sum = 0;
		for (auto it = reference.lower_bound(l); it != reference.end(); ++it) {
			if (sum + it->second > limit) {
				right = it->first;
				break;
			}
			sum += it->second;
		}
		sum = 0;
		for (auto it = reference.lower_bound(r); it != reference.begin();) {
			--it;
			if (it->second + sum > limit) {
				left = it->first + 1;
				break;
			}
			sum += it->second;
		}
		assert(wide.max_right(l, [&](auto x) { return x <= limit; }) == right);
		assert(wide.min_left(r, [&](auto x) { return x <= limit; }) == left);
	}
	DynamicSegtree<long long, add, zero, uint64_t> unsigned_tree(0, UINT64_MAX);
	unsigned_tree.set(UINT64_MAX - 1, 9);
	unsigned_tree.set(0, 7);
	assert(unsigned_tree.size() == UINT64_MAX && unsigned_tree.prod(0, UINT64_MAX) == 16);
	assert(unsigned_tree.max_right(1, [](auto x) { return x <= 8; }) == UINT64_MAX - 1);
	assert(unsigned_tree.min_left(UINT64_MAX, [](auto x) { return x <= 8; }) == UINT64_MAX);
	for (int reverse = 0; reverse < 2; ++reverse) {
		DynamicSegtree<long long, add, zero, uint64_t> chain(0, UINT64_MAX);
		vector<uint64_t> points{0};
		for (int i = 0; i < 64; ++i) points.push_back(uint64_t(1) << i);
		if (reverse) std::reverse(points.begin(), points.end());
		for (auto p : points) chain.set(p, 1);
		for (auto p : points) {
			chain.multiply(p, 2);
			assert(chain.get(p) == 3);
		}
		assert(chain.all_prod() == 195 && chain.node_count() == 65);
	}
	DynamicSegtree<Box, box_add, box_zero> boxes(10);
	boxes.set(3, Box(7));
	boxes.multiply(3, Box(5));
	assert(boxes.prod(0, 5).x == 12);
	boxes.clear();
	assert(boxes.all_prod().x == 0);
	DynamicSegtree<int, [](int a, int b) { return a | b; }, [] { return 0; }, uint8_t> small(255);
	small.set(0, 1);
	small.set(254, 2);
	assert(small.prod(0, 255) == 3);
	DynamicSegtree<int, [](int a, int b) { return min(a, b); }, [] { return 999; }, int8_t> minimum(-128, 127);
	for (int i = -128; i < 127; ++i) minimum.set(i, i + 128);
	for (int l = -128; l < 127; ++l)
		for (int r = l; r <= 127; ++r) assert(minimum.prod(l, r) == (l == r ? 999 : l + 128));
	DynamicSegtree<bool, [](bool a, bool b) { return a || b; }, [] { return false; }> flags(100);
	flags.set(40, true);
	assert(!flags.prod(0, 40) && flags.prod(40, 100));
	DynamicSegtree<long long, counted_add, zero> clustered(hi);
	for (int i = 0; i < 10000; ++i) clustered.set((int64_t(1) << 60) + i, 1);
	calls = 0;
	assert(clustered.prod((int64_t(1) << 60) + 3000, (int64_t(1) << 60) + 7000) == 4000 && calls < 128);
	calls = 0;
	assert(clustered.max_right(int64_t(1) << 60, [](auto x) { return x <= 7000; }) == (int64_t(1) << 60) + 7000 && calls < 256);
	assert(clustered.node_count() == 10000);
}
