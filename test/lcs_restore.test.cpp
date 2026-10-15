#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_10_C"
#include "../string/lcs.hpp"
#include <iostream>
#include <string>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	while (q--) {
		std::string s, t;
		std::cin >> s >> t;
		std::cout << lcs(s, t).size() << '\n';
	}
}
