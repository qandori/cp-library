#define PROBLEM "https://yukicoder.me/problems/no/1307"
#include "../convolution/cyclic_convolution.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<long long> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	while (q--) {
		int r;
		std::cin >> r;
		++b[(n - r) % n];
	}
	auto c = cyclic_convolution(a, b, n);
	for (int i = 0; i < n; ++i) std::cout << c[i] << (i + 1 == n ? '\n' : ' ');
}
