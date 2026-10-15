#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_3_B"
#include "../graph/low_link.hpp"
#include <algorithm>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	cin >> n >> m;
	LowLink g(n);
	for (int repeat_index = 0; repeat_index < m; ++repeat_index) {
		int u, v;
		cin >> u >> v;
		g.add_edge(u, v);
	}
	g.build();
	vector<pair<int, int>> answer;
	for (int id : g.bridges) {
		auto [u, v] = g.edges[id];
		answer.emplace_back(min(u, v), max(u, v));
	}
	sort(answer.begin(), answer.end());
	for (auto [u, v] : answer) cout << u << ' ' << v << '\n';
}
