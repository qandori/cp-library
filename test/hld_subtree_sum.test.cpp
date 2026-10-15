#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"
#include "../data_structure/dynamic_segtree.hpp"
#include "../graph/hld.hpp"
#include <iostream>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<long long> a(n);
	for (auto &x : a) std::cin >> x;
	HLD tree(n);
	for (int v = 1, p; v < n; ++v) {
		std::cin >> p;
		tree.add_edge(p, v);
	}
	tree.build();
	DynamicSegtree<long long, op, e> seg(n);
	seg.reserve(n);
	for (int v = 0; v < n; ++v) seg.set(tree.in[v], a[v]);
	while (q--) {
		int type, v;
		std::cin >> type >> v;
		if (type == 0) {
			long long x;
			std::cin >> x;
			seg.multiply(tree.in[v], x);
		} else {
			auto [l, r] = tree.subtree(v);
			std::cout << seg.prod(l, r) << '\n';
		}
	}
}
