#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include "../math/fps.hpp"
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	cin >> n >> m;
	fps f(n);
	vector<mint> xs(m);
	for (auto &v : f) cin >> v;
	for (auto &value : xs) cin >> value;
	for (const auto &value : f.multipoint_eval(xs)) cout << value << ' ';
	cout << '\n';
}
