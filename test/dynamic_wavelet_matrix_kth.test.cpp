#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "../data_structure/dynamic_wavelet_matrix.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<unsigned> a(n);
	for (auto &value : a) std::cin >> value;
	DynamicWaveletMatrix<unsigned, 30> wm(a);
	while (q--) {
		int l, r, k;
		std::cin >> l >> r >> k;
		std::cout << wm.kth_smallest(l, r, k) << '\n';
	}
}
