#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"
#include "../graph/hld.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	HLD tree(n);
	for (int i = 1, u, v; i < n; ++i) {
		std::cin >> u >> v;
		tree.add_edge(u, v);
	}
	tree.build();
	while (q--) {
		int u, v, k;
		std::cin >> u >> v >> k;
		std::cout << tree.jump(u, v, k) << '\n';
	}
}
