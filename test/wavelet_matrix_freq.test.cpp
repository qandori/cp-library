#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
#include "../data_structure/wavelet_matrix.hpp"
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vector<int> a(n);
	for (auto &value : a) cin >> value;
	WaveletMatrix wm(a);
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int l, r, x;
		cin >> l >> r >> x;
		cout << wm.count(l, r, x) << '\n';
	}
}
