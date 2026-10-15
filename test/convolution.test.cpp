#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "../math/fps.hpp"
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	cin >> n >> m;
	fps a(n), b(m);
	for (auto &v : a) cin >> v;
	for (auto &v : b) cin >> v;
	for (const auto &value : a *b) cout << value << ' ';
	cout << '\n';
}
