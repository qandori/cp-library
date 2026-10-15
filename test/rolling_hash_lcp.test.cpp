#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"
#include "../string/rolling_hash.hpp"
#include <iostream>
#include <string>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::string s;
	std::cin >> s;
	RollingHash hash(s);
	for (int i = 0; i < hash.size(); ++i) std::cout << hash.lcp(0, hash, i) << (i + 1 == hash.size() ? '\n' : ' ');
}
