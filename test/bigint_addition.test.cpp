#define PROBLEM "https://judge.yosupo.jp/problem/addition_of_big_integers"
#include "../math/bigint.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int t;
	std::cin >> t;
	while (t--) {
		BigInt a, b;
		std::cin >> a >> b;
		std::cout << a + b << '\n';
	}
}
