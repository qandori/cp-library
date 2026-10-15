#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_frequency"
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
		int type;
		std::cin >> type;
		if (type == 0) {
			int p;
			unsigned value;
			std::cin >> p >> value;
			wm.set(p, value);
		} else {
			int l, r;
			unsigned value;
			std::cin >> l >> r >> value;
			std::cout << wm.count(l, r, value) << '\n';
		}
	}
}
