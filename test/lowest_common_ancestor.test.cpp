#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include "../graph/lca.hpp"
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	LCA tree(n);
	for (int v = 1; v < n; ++v) {
		int p;
		cin >> p;
		tree.add_edge(p, v);
	}
	tree.build();
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int u, v;
		cin >> u >> v;
		cout << tree.lca(u, v) << '\n';
	}
}
