#define PROBLEM "https://judge.yosupo.jp/problem/division_of_big_integers"
#include "../math/bigint.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int t;
	std::cin >> t;
	while (t--) {
		BigInt a, b;
		std::cin >> a >> b;
		auto [q, r] = divmod(a, b);
		std::cout << q << ' ' << r << '\n';
	}
}
