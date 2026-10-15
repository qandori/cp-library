#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "../data_structure/sparse_table.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

int op(int a, int b) { return std::min(a, b); }
int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<int> a(n);
	for (auto &x : a) std::cin >> x;
	SparseTable<int, op> table(a);
	while (q--) {
		int l, r;
		std::cin >> l >> r;
		std::cout << table.prod(l, r) << '\n';
	}
}
