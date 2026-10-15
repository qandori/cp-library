#define PROBLEM "https://judge.yosupo.jp/problem/lcm_convolution"
#include "../convolution/gcd_lcm_convolution.hpp"
#include "../math/mint.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	std::cin >> n;
	std::vector<mint> a(n + 1), b(n + 1);
	for (int i = 1; i <= n; ++i) std::cin >> a[i];
	for (int i = 1; i <= n; ++i) std::cin >> b[i];
	auto c = lcm_convolution(a, b);
	for (int i = 1; i <= n; ++i) std::cout << c[i] << ' ';
	std::cout << '\n';
}
