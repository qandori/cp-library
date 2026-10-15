#define PROBLEM "https://judge.yosupo.jp/problem/multivariate_convolution"
#include "../convolution/multivariate_convolution.hpp"
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int k;
	std::cin >> k;
	std::vector<int> dimensions(k);
	for (auto &d : dimensions) std::cin >> d;
	int n = std::accumulate(dimensions.begin(), dimensions.end(), 1, std::multiplies<int>{});
	std::vector<mint> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : multivariate_convolution(a, b, dimensions)) std::cout << x << ' ';
	std::cout << '\n';
}
