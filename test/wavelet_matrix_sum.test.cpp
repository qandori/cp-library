#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound"
#include "../data_structure/wavelet_matrix.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vec<int64_t> a(n);
	cin >> a;
	WaveletMatrix wm(a, true);
	rep(q) {
		int l, r;
		int64_t x;
		cin >> l >> r >> x;
		int k = wm.range_freq(l, r, x) + wm.count(l, r, x);
		cout << k << ' ' << wm.sum_smallest(l, r, k) << '\n';
	}
}
