#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_concave_arbitrary"
#include "../convolution/min_plus_convolution.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	std::cin >> n >> m;
	std::vector<std::int64_t> a(n), b(m);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : min_plus_convolution_concave(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
