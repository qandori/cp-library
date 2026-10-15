#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#include "../data_structure/link_cut_tree.hpp"
#include "../math/mint.hpp"

using S = array<mint, 2>;
S op(const S &a, const S &b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
S e() { return {1, 0}; }

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vec<S> values(n);
	for (auto &v : values) cin >> v[0] >> v[1];
	LinkCutTree<S, op, e> tree(values);
	rep(n - 1) {
		int u, v;
		cin >> u >> v;
		tree.link(u, v);
	}
	rep(q) {
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
