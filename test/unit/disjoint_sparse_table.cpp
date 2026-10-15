#include "data_structure/disjoint_sparse_table.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace std;

int minimum(int a, int b) { return min(a, b); }
int infinity() { return numeric_limits<int>::max(); }
int64_t addition(int64_t a, int64_t b) { return a + b; }
int64_t zero() { return 0; }
int op_calls = 0;
int counted_add(int a, int b) {
	++op_calls;
	return a + b;
}
constexpr uint64_t mod = 998244353;
struct Affine {
	uint64_t a, b;
	bool operator==(const Affine &) const = default;
};
Affine compose(Affine f, Affine g) { return {g.a * f.a % mod, (g.a * f.b + g.b) % mod}; }
Affine affine_e() { return {1, 0}; }
struct Box {
	int64_t value;
	Box() = delete;
	explicit Box(int64_t x) : value(x) {}
};
Box box_add(const Box &a, const Box &b) { return Box(a.value + b.value); }

int main() {
	mt19937_64 random(5678);
	DisjointSparseTable<int, minimum, infinity> empty;
	assert(empty.empty() && empty.size() == 0 && empty.prod(0, 0) == infinity() && empty.all_prod() == infinity());
	for (int n : {0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 1023, 1024, 1025, 4097}) {
		vector<int> a(n);
		vector<int64_t> b(n);
		vector<Affine> c(n);
		for (int i = 0; i < n; ++i) {
			a[i] = int(random() % 2001) - 1000;
			b[i] = a[i];
			c[i] = {random() % mod, random() % mod};
		}
		DisjointSparseTable<int, minimum, infinity> rmq(a);
		DisjointSparseTable<int64_t, addition, zero> sums(b);
		DisjointSparseTable<Affine, compose, affine_e> affine(c);
		assert(rmq.size() == n && rmq.empty() == !n);
		for (int i = 0; i < n; ++i) assert(rmq[i] == a[i] && affine[i] == c[i]);
		auto check = [&](int l, int r) {
			int best = infinity();
			int64_t total = 0;
			uint64_t x = 123456789, y = x;
			for (int i = l; i < r; ++i) {
				best = min(best, a[i]);
				total += b[i];
				y = (c[i].a * y + c[i].b) % mod;
			}
			assert(rmq.prod(l, r) == best && sums.prod(l, r) == total);
			auto f = affine.prod(l, r);
			assert((f.a * x + f.b) % mod == y);
		};
		if (n <= 129)
			for (int l = 0; l < (n + 1); ++l)
				for (int r = l; r < (n + 1); ++r) check(l, r);
		else
			for (int repeat_index = 0; repeat_index < 1000; ++repeat_index) {
				int l = random() % (n + 1), r = random() % (n + 1);
				if (l > r) swap(l, r);
				check(l, r);
			}
		check(0, n);
		assert(rmq.all_prod() == rmq.prod(0, n));
		auto copy = rmq;
		auto moved = std::move(copy);
		assert(moved.all_prod() == rmq.all_prod());
		assert(copy.empty() && copy.all_prod() == infinity());
		copy = moved;
		assert(copy.all_prod() == moved.all_prod());
		auto destination = rmq;
		destination = std::move(copy);
		assert(copy.empty() && copy.all_prod() == infinity() && destination.all_prod() == rmq.all_prod());
		rmq.build({});
		assert(rmq.empty() && rmq.all_prod() == infinity());
		rmq.build(a);
		assert(rmq.all_prod() == moved.all_prod());
		rmq.build({5, 3, 7});
		assert(rmq.prod(0, 3) == 3 && rmq[2] == 7);
	}
	vector<string> words{"a", "bc", "d", "ef", "g", "hi", "j"};
	auto cat = [](const string &a, const string &b) { return a + b; };
	DisjointSparseTable<string, cat> text(words);
	for (int l = 0; l < (static_cast<int>(words.size())); ++l) {
		string expected;
		for (int r = l + 1; r < (static_cast<int>(words.size()) + 1); ++r) {
			expected += words[r - 1];
			assert(text.prod(l, r) == expected);
		}
	}
	auto text_copy = text;
	assert(text_copy.all_prod() == "abcdefghij");
	DisjointSparseTable<Box, box_add> boxes(vector<Box>{Box(2), Box(4), Box(-3), Box(7), Box(1)});
	assert(boxes.prod(0, 5).value == 11 && boxes.prod(1, 4).value == 8);
	auto box_copy = boxes;
	assert(box_copy.all_prod().value == 11);
	DisjointSparseTable<int, counted_add, [] { return 0; }> counted(vector<int>{1, 2, 3, 4});
	op_calls = 0;
	assert(counted.prod(0, 3) == 6 && op_calls == 1);
	op_calls = 0;
	assert(counted.prod(1, 2) == 2 && op_calls == 0);
	assert(counted.prod(2, 2) == 0 && op_calls == 0);
	DisjointSparseTable<bool, [](bool a, bool b) { return a || b; }, [] { return false; }> flags(vector<bool>{false, true, false});
	assert(flags.prod(0, 3) && !flags.prod(0, 1) && !flags.prod(3, 3));
	DisjointSparseTable<int, minimum> singleton(vector<int>{-8});
	assert(singleton.all_prod() == -8 && singleton[0] == -8);
}
