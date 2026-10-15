#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_path_sum"
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
	for (int i = 1, u, v; i < n; ++i) {
		std::cin >> u >> v;
		tree.add_edge(u, v);
	}
	tree.build();
	DynamicSegtree<long long, op, e> seg(n);
	seg.reserve(n);
	for (int v = 0; v < n; ++v) seg.set(tree.in[v], a[v]);
	while (q--) {
		int type, u;
		long long v;
		std::cin >> type >> u >> v;
		if (type == 0) seg.multiply(tree.in[u], v);
		else {
			long long result = 0;
			tree.for_each_path(u, int(v), [&](int l, int r, bool) { result += seg.prod(l, r); });
			std::cout << result << '\n';
		}
	}
}
