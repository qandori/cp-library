#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include "../data_structure/dynamic_segtree.hpp"
#include "../math/mint.hpp"
#include <cstdint>
#include <iostream>
#include <utility>

using S = std::pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	constexpr std::int64_t step = std::int64_t(1) << 40;
	DynamicSegtree<S, op, e> tree(std::int64_t(n) * step);
	tree.reserve(n);
	for (int i = 0; i < n; ++i) {
		S f;
		std::cin >> f.first >> f.second;
		tree.set(std::int64_t(i) * step, f);
	}
	while (q--) {
		int type, l;
		std::cin >> type >> l;
		if (type == 0) {
			S f;
			std::cin >> f.first >> f.second;
			tree.set(std::int64_t(l) * step, f);
		} else {
			int r;
			mint x;
			std::cin >> r >> x;
			auto [a, b] = tree.prod(std::int64_t(l) * step, std::int64_t(r) * step);
			std::cout << a * x + b << '\n';
		}
	}
}
