#define PROBLEM "https://yukicoder.me/problems/no/2304"
#include "../data_structure/slope_trick.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	std::cin >> n;
	std::vector<std::int64_t> a(n);
	for (auto &x : a) std::cin >> x;
	std::sort(a.begin(), a.end());
	SlopeTrick<> slope;
	slope.reserve(n);
	for (auto x : a) {
		slope.shift(1);
		slope.clear_right();
		slope.add_abs(x);
	}
	std::cout << slope.min() << '\n';
}
