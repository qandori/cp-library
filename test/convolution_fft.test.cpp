#define PROBLEM "https://yukicoder.me/problems/no/1307"
#include "../convolution/convolution_fft.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<double> a(n), b(n);
	for (auto &x : a) std::cin >> x;
	while (q--) {
		int r;
		std::cin >> r;
		++b[(n - r) % n];
	}
	auto c = convolution_fft(a, b);
	for (int i = 0; i < n; ++i)
		std::cout << std::llround(c[i] + (i + n < int(c.size()) ? c[i + n] : 0)) << (i + 1 == n ? '\n' : ' ');
}
