#define PROBLEM "https://judge.yosupo.jp/problem/primality_test"
#include "../math/is_prime.hpp"
#include <cstdint>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	while (q--) {
		std::uint64_t n;
		std::cin >> n;
		std::cout << (is_prime(n) ? "Yes" : "No") << '\n';
	}
}
