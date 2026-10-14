#define PROBLEM "https://judge.yosupo.jp/problem/queue_operate_all_composite"
#include "../data_structure/disjoint_sparse_table.hpp"
#include "../math/mint.hpp"

using S = pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }
int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q; cin >> q;
	struct Query { int l, r; mint x; };
	vec<S> a; vec<Query> queries;
	a.reserve(q); queries.reserve(q);
	int l = 0, r = 0;
	rep(q) {
		int type; cin >> type;
		if (type == 0) { S f; cin >> f; a.push_back(f); ++r; }
		else if (type == 1) ++l;
		else { mint x; cin >> x; queries.push_back({l, r, x}); }
	}
	DisjointSparseTable<S, op, e> table(a);
	for (auto query : queries) {
		auto [p, q] = table.prod(query.l, query.r);
		cout << p * query.x + q << '\n';
	}
}
