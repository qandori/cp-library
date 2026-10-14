#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
#include "../data_structure/wavelet_matrix.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vec<int> a(n);
	cin >> a;
	WaveletMatrix wm(a);
	rep(q) {
		int l, r, x;
		cin >> l >> r >> x;
		cout << wm.count(l, r, x) << '\n';
	}
}
