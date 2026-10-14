#define PROBLEM "https://judge.yosupo.jp/problem/unionfind_with_potential"
#include "../data_structure/weighted_union_find.hpp"
#include "../math/mint.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	WeightedUnionFind<mint> uf(n);
	rep(q) {
		int t, u, v;
		cin >> t >> u >> v;
		if (t == 0) {
			mint x;
			cin >> x;
			cout << uf.merge(v, u, x) << '\n';
		} else if (uf.same(u, v)) cout << uf.diff(v, u) << '\n';
		else cout << -1 << '\n';
	}
}
