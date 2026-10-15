#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_14_B"
#include "../string/rolling_hash.hpp"
#include <iostream>
#include <string>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::string text, pattern;
	std::cin >> text >> pattern;
	RollingHash hash(text), target(pattern);
	for (int i = 0; i + target.size() <= hash.size(); ++i)
		if (hash.get(i, i + target.size()) == target.get()) std::cout << i << '\n';
}
