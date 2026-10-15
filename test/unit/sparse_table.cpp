#include "data_structure/sparse_table.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

int minimum(int a, int b) { return std::min(a, b); }
int identity() { return std::numeric_limits<int>::max(); }
struct Box {
	int x;
	Box() = delete;
	explicit Box(int x) : x(x) {}
	bool operator==(const Box &) const = default;
};
Box min_box(const Box &a, const Box &b) { return a.x < b.x ? a : b; }
int main() {
	std::mt19937 rng(91);
	SparseTable<int, minimum, identity> table;
	assert(table.empty() && table.all_prod() == identity());
	for (int n = 0; n <= 150; ++n) {
		std::vector<int> a(n);
		for (auto &x : a) x = rng() % 1000;
		table.build(a);
		assert(table.size() == n);
		for (int l = 0; l <= n; ++l) {
			int value = identity();
			for (int r = l; r <= n; ++r) {
				assert(table.prod(l, r) == value);
				if (l < r) assert(table.prod_unchecked(l, r) == value);
				if (r < n) value = std::min(value, a[r]);
			}
		}
		auto copy = table;
		auto moved = std::move(copy);
		assert(copy.empty());
		assert(moved.all_prod() == table.all_prod());
		copy = std::move(moved);
		assert(moved.empty() && copy.size() == n);
		table.build(n, [&](int i) { return a[i]; });
		for (int i = 0; i < n; ++i) assert(table[i] == a[i]);
	}
	SparseTable<Box, min_box> box(std::vector<Box>{Box(8), Box(2), Box(5)});
	assert(box.prod(0, 3).x == 2);
	box.build(2, [](int i) { return Box(7 - i); });
	assert(box.all_prod().x == 6);
	SparseTable<bool, [](bool a, bool b) { return a || b; }, [] { return false; }> bits(std::vector<bool>{false, true, false});
	assert(!bits[0] && bits.prod(1, 3) && !bits.prod(2, 3) && !bits.prod(3, 3));
	SparseTable<int, [](int a, int b) { return std::gcd(a, b); }> gcd(std::vector<int>{12, 18, 24});
	assert(gcd.prod(0, 3) == 6);
}
