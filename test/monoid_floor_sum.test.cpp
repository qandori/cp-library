#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_floor_of_linear"
#include "../math/monoid_floor_sum.hpp"

using S = array<__uint128_t, 3>;
S op(const S &a, const S &b) { return {a[0] + b[0], a[1] + b[1], a[2] + b[2] + a[1] * b[0]}; }
S e() { return {}; }

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	cin >> q;
	rep(q) {
		uint64_t n, m, a, b;
		cin >> n >> m >> a >> b;
		auto result = monoid_floor_sum<S, op, e>(n, m, a, b, {1, 0, 0}, {0, 1, 0});
		cout << __int128_t(result[2]) << '\n';
	}
}
