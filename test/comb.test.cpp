#define PROBLEM "https://yukicoder.me/problems/no/3172"
#define COMB_MAX 200000
#include "../math/comb.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	std::cin >> n;
	init_comb();
	for (int k = 0; k <= n; ++k)
		std::cout << ((n - k) % 2 ? mint(0) : comb(n, (n - k) / 2) * (k ? 2 : 1)) << (k == n ? '\n' : ' ');
}
