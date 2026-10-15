#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_floor_of_linear"
#include "../math/monoid_floor_sum.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>

using namespace std;

using S = array<__uint128_t, 3>;
S op(const S &a, const S &b) { return {a[0] + b[0], a[1] + b[1], a[2] + b[2] + a[1] * b[0]}; }
S e() { return {}; }

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
	int q;
	cin >> q;
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		uint64_t n, m, a, b;
		cin >> n >> m >> a >> b;
		auto result = monoid_floor_sum<S, op, e>(n, m, a, b, {1, 0, 0}, {0, 1, 0});
		print_integer(__int128_t(result[2]));
	}
}
