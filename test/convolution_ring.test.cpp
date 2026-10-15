#define PROBLEM "https://yukicoder.me/problems/no/1307"
#include "../convolution/convolution_ring.hpp"
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
	auto c = convolution_ring(a, b);
	for (int i = 0; i < n; ++i)
		std::cout << c[i] + (i + n < int(c.size()) ? c[i + n] : 0) << (i + 1 == n ? '\n' : ' ');
}
