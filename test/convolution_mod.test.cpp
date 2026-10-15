#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod_1000000007"
#include "../convolution/convolution_mod.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	std::cin >> n >> m;
	std::vector<std::uint32_t> a(n), b(m);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : convolution_mod(a, b, 1000000007)) std::cout << x << ' ';
	std::cout << '\n';
}
