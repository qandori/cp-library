#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include "../data_structure/dynamic_segtree.hpp"
#include <cstdint>
#include <iostream>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	constexpr std::int64_t step = std::int64_t(1) << 40;
	DynamicSegtree<long long, op, e> tree(std::int64_t(n) * step);
	tree.reserve(n);
	for (int i = 0; i < n; ++i) {
		long long x;
		std::cin >> x;
		tree.set(std::int64_t(i) * step, x);
	}
	while (q--) {
		int type, l;
		long long r;
		std::cin >> type >> l >> r;
		if (type == 0) tree.multiply(std::int64_t(l) * step, r);
		else std::cout << tree.prod(std::int64_t(l) * step, r * step) << '\n';
	}
}
