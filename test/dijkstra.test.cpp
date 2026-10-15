#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "../graph/dijkstra.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, m, s, t;
	cin >> n >> m >> s >> t;
	vector<vector<pair<int, int64_t>>> g(n);
	for (int repeat_index = 0; repeat_index < m; ++repeat_index) {
		int u, v;
		int64_t c;
		cin >> u >> v >> c;
		g[u].emplace_back(v, c);
	}
	auto d = dijkstra(g, s);
	if (d[t] == -1) {
		cout << -1 << '\n';
		return 0;
	}
	vector<int> parent(n, -1), todo{s};
	parent[s] = s;
	for (int i = 0; i < (static_cast<int>(todo.size())); ++i) {
		int v = todo[i];
		for (auto [to, c] : g[v])
			if (parent[to] == -1 && d[to] == d[v] + c) {
				parent[to] = v;
				todo.push_back(to);
			}
	}
	vector<pair<int, int>> path;
	for (int v = t; v != s; v = parent[v]) path.emplace_back(parent[v], v);
	reverse(path.begin(), path.end());
	cout << d[t] << ' ' << static_cast<int>(path.size()) << '\n';
	for (auto [u, v] : path) cout << u << ' ' << v << '\n';
}
