#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum"
#include "../data_structure/link_cut_tree.hpp"
#include <cstdint>
#include <iostream>
#include <set>
#include <tuple>
#include <vector>

using namespace std;

int64_t op(int64_t a, int64_t b) { return a + b; }
int64_t e() { return 0; }

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vector<int64_t> values(n);
	for (auto &v : values) cin >> v;
	LinkCutTree<int64_t, op, e, true> tree(values);
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
			int64_t x;
			cin >> x;
			tree.set(u, tree.get(u) + x);
		} else {
			int v;
			cin >> v;
			cout << tree.prod(u, v) << '\n';
		}
	}
}
