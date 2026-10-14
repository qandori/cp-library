#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "../data_structure/disjoint_sparse_table.hpp"

int op(int a, int b) { return min(a, b); }
int e() { return numeric_limits<int>::max(); }
int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q; cin >> n >> q;
	vec<int> a(n); cin >> a;
	DisjointSparseTable<int, op, e> table(a);
	rep(q) {
		int l, r; cin >> l >> r;
		cout << table.prod(l, r) << '\n';
	}
}
