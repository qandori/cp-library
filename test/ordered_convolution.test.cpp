#define PROBLEM "https://yukicoder.me/problems/no/1307"
#include "../convolution/ordered_convolution.hpp"
#include "../math/mint.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	while (q--) {
		int r;
		std::cin >> r;
		b[(n - r) % n] += 1;
	}
	auto c = ordered_convolution(a, b), d = ordered_convolution(b, a);
	for (int i = 0; i < int(c.size()); ++i) c[i] += d[i];
	for (int i = 0; i < n; ++i) c[2 * i] += a[i] * b[i];
	for (int i = 0; i < n; ++i)
		std::cout << c[i] + (i + n < int(c.size()) ? c[i + n] : mint(0)) << (i + 1 == n ? '\n' : ' ');
}
