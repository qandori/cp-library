#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "../data_structure/wavelet_matrix.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vec<int> a(n);
	cin >> a;
	WaveletMatrix wm(a);
	rep(q) {
		int l, r, k;
		cin >> l >> r >> k;
		cout << wm.kth_smallest(l, r, k) << '\n';
	}
}
