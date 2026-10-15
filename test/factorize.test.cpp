#define PROBLEM "https://judge.yosupo.jp/problem/factorize"
#include "../math/factorize.hpp"
#include <cstdint>
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	cin >> q;
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		uint64_t n;
		cin >> n;
		auto factors = factorize(n);
		cout << static_cast<int>(factors.size());
		for (auto p : factors) cout << ' ' << p;
		cout << '\n';
	}
}
