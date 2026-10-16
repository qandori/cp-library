#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "../components/ntt.hpp"
#include <bit>
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	std::cin >> n >> m;
	int length = n + m - 1, size = std::bit_ceil(unsigned(length));
	std::vector<fps_mint> a(size), b(size);
	for (int i = 0; i < n; ++i) std::cin >> a[i];
	for (int i = 0; i < m; ++i) std::cin >> b[i];
	ntt(a);
	ntt(b);
	for (int i = 0; i < size; ++i) a[i] *= b[i];
	ntt(a, true);
	for (int i = 0; i < length; ++i) std::cout << a[i] << ' ';
	std::cout << '\n';
}
