#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#include "../data_structure/link_cut_tree.hpp"
#include "../math/mint.hpp"
#include <array>
#include <iostream>
#include <set>
#include <tuple>
#include <vector>

using namespace std;

using S = array<mint, 2>;
S op(const S &a, const S &b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
S e() { return {1, 0}; }

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vector<S> values(n);
	for (auto &v : values) cin >> v[0] >> v[1];
	LinkCutTree<S, op, e> tree(values);
	for (int repeat_index = 0; repeat_index < (n - 1); ++repeat_index) {
		int u, v;
		cin >> u >> v;
		tree.link(u, v);
	}
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int type, u;
		cin >> type >> u;
		if (type == 0) {
			int v, w, x;
			cin >> v >> w >> x;
			tree.cut(u, v);
			tree.link(w, x);
		} else if (type == 1) {
			S f;
			cin >> f[0] >> f[1];
			tree.set(u, f);
		} else {
			int v;
			mint x;
			cin >> v >> x;
			auto f = tree.prod(u, v);
			cout << f[0] * x + f[1] << '\n';
		}
	}
}
