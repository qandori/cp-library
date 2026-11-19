#include "../../data_structure/segtree.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <string>

long long add(long long a, long long b) { return a + b; }
long long zero() { return 0; }
bool at_most_six(long long x) { return x <= 6; }
std::string concatenate(std::string a, const std::string &b) { return a + b; }
std::string empty() { return {}; }

int main() {
	const cp_acl::segtree<long long, add, zero> acl_tree({1, 5, 3});
	assert(acl_tree.max_right<at_most_six>(0) == 2);
	assert(acl_tree.min_left<at_most_six>(3) == 2);
	assert(acl_tree.max_right<at_most_six>(3) == 3);
	assert(acl_tree.min_left<at_most_six>(0) == 0);
	std::mt19937 random(47);
	for (int n = 0; n <= 160; ++n) {
		std::vector<long long> values(n);
		for (auto &x : values) x = random() % 20;
		Segtree<long long, add, zero> tree(values);
		for (int repeat = 0; repeat < 100; ++repeat) {
			if (n && random() % 2) {
				int p = random() % n;
				long long value = random() % 20;
				if (random() % 2) values[p] = value, tree.set(p, value);
				else values[p] += value, tree.multiply(p, value);
				assert(tree.get(p) == values[p]);
			}
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			long long expected = 0, total = 0;
			for (int i = 0; i < n; ++i) {
				total += values[i];
				if (l <= i && i < r) expected += values[i];
			}
			const auto &constant = tree;
			assert(constant.prod(l, r) == expected && constant.all_prod() == total);
			long long limit = random() % 80, sum = 0;
			int right = l, left = r;
			while (right < n && sum + values[right] <= limit) sum += values[right++];
			sum = 0;
			while (left && sum + values[left - 1] <= limit) sum += values[--left];
			auto accepted = [&](long long value) { return value <= limit; };
			assert(constant.max_right(l, accepted) == right && constant.min_left(r, accepted) == left);
		}
		auto copied = tree;
		auto moved = std::move(tree);
		assert(copied.all_prod() == moved.all_prod() && tree.size() == 0 && tree.all_prod() == 0 && tree.prod(0, 0) == 0);
		tree = std::move(moved);
		assert(tree.all_prod() == copied.all_prod() && moved.size() == 0 && moved.all_prod() == 0);
	}
	std::vector<std::string> values{"ab", "c", "de", "f", "gh"};
	Segtree<std::string, concatenate, empty> tree(values);
	tree.multiply(2, "XY");
	values[2] += "XY";
	assert(tree.get(2) == "deXY");
	for (int l = 0; l <= 5; ++l) for (int r = l; r <= 5; ++r) {
		std::string expected;
		for (int i = l; i < r; ++i) expected += values[i];
		assert(tree.prod(l, r) == expected);
		auto prefix = [&](const std::string &value) { return expected.starts_with(value); };
		auto suffix = [&](const std::string &value) { return expected.ends_with(value); };
		assert(tree.max_right(l, prefix) == r && tree.min_left(r, suffix) == l);
	}
}
