#define PROBLEM "https://judge.yosupo.jp/problem/subset_convolution"
#include "../convolution/subset_convolution.hpp"
#include "../math/mint.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int exponent;
	std::cin >> exponent;
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : subset_convolution(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
