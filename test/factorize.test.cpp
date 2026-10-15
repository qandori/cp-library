#define PROBLEM "https://judge.yosupo.jp/problem/factorize"
#include "../math/factorize.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	cin >> q;
	rep(q) {
		uint64_t n;
		cin >> n;
		auto factors = factorize(n);
		cout << sz(factors);
		for (auto p : factors) cout << ' ' << p;
		cout << '\n';
	}
}
