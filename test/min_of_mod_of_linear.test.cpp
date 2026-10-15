#define PROBLEM "https://judge.yosupo.jp/problem/min_of_mod_of_linear"
#include "../math/min_of_mod_of_linear.hpp"
#include <cstdint>
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int t;
	std::cin >> t;
	while (t--) {
		std::uint64_t n, m, a, b;
		std::cin >> n >> m >> a >> b;
		std::cout << min_of_mod_of_linear(n, m, a, b) << '\n';
	}
}
