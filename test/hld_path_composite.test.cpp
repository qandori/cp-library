#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"
#include "../data_structure/dynamic_segtree.hpp"
#include "../graph/hld.hpp"
#include "../math/mint.hpp"
#include <iostream>
#include <vector>

struct S {
	mint a, b, reverse_b;
};
S op(S x, S y) { return {x.a * y.a, x.b * y.a + y.b, y.reverse_b * x.a + x.reverse_b}; }
S e() { return {1, 0, 0}; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<S> values(n);
	for (auto &f : values) {
		std::cin >> f.a >> f.b;
		f.reverse_b = f.b;
	}
	HLD tree(n);
	for (int i = 1, u, v; i < n; ++i) {
		std::cin >> u >> v;
		tree.add_edge(u, v);
	}
	tree.build();
	DynamicSegtree<S, op, e> seg(n);
	seg.reserve(n);
	for (int v = 0; v < n; ++v) seg.set(tree.in[v], values[v]);
	while (q--) {
		int type, u, v;
		std::cin >> type >> u;
		if (type == 0) {
			mint a, b;
			std::cin >> a >> b;
			seg.set(tree.in[u], {a, b, b});
		} else {
			mint x;
			std::cin >> v >> x;
			tree.for_each_path(u, v, [&](int l, int r, bool reverse) {
				auto f = seg.prod(l, r);
				x = f.a * x + (reverse ? f.reverse_b : f.b);
			});
			std::cout << x << '\n';
		}
	}
}
