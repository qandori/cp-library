#define PROBLEM "https://judge.yosupo.jp/problem/mul_modp_convolution"
#include "../convolution/multiplicative_convolution.hpp"
#include "../math/mint.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	std::cin >> n;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : multiplicative_convolution(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
