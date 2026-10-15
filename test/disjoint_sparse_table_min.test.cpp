#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "../data_structure/disjoint_sparse_table.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>

using namespace std;

int op(int a, int b) { return min(a, b); }
int e() { return numeric_limits<int>::max(); }
int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vector<int> a(n);
	for (auto &value : a) cin >> value;
	DisjointSparseTable<int, op, e> table(a);
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int l, r;
		cin >> l >> r;
		cout << table.prod(l, r) << '\n';
	}
}
