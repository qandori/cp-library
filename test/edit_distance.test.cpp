#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=DPL_1_E"
#include "../string/edit_distance.hpp"
#include <iostream>
#include <string>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::string s, t;
	std::cin >> s >> t;
	std::cout << edit_distance(s, t) << '\n';
}
