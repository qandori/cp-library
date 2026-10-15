#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound"
#include "../data_structure/wavelet_matrix.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

void print_integer(__int128_t value) {
	__uint128_t x = value;
	if (value < 0) cout << '-', x = -x;
	string digits;
	do {
		digits.push_back('0' + x % 10);
		x /= 10;
	} while (x);
	reverse(digits.begin(), digits.end());
	cout << digits << '\n';
}

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vector<int64_t> a(n);
	for (auto &value : a) cin >> value;
	WaveletMatrix wm(a, true);
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int l, r;
		int64_t x;
		cin >> l >> r >> x;
		int k = wm.range_freq(l, r, x) + wm.count(l, r, x);
		cout << k << ' ';
		print_integer(wm.sum_smallest(l, r, k));
	}
}
