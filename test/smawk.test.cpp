#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary"
#include "../math/smawk.hpp"
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
	auto best = smawk(n + m - 1, m, [&](int row, int x, int y) {
		if (row < y) return false;
		if (row - x >= n) return true;
		return a[row - y] + b[y] < a[row - x] + b[x];
	});
	for (int i = 0; i < n + m - 1; ++i) std::cout << a[i - best[i]] + b[best[i]] << ' ';
	std::cout << '\n';
}
