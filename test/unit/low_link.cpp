#include "graph/low_link.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

using namespace std;

int components(int n, const vector<pair<int, int>> &edges, int vertex = -1, int edge = -1) {
	vector<vector<int>> adj(n);
	for (int i = 0; i < (static_cast<int>(edges.size())); ++i) {
		auto [u, v] = edges[i];
		if (i == edge || u == vertex || v == vertex) continue;
		adj[u].push_back(v);
		adj[v].push_back(u);
	}
	vector<int> seen(n), queue;
	int count = 0;
	for (int v = 0; v < n; ++v) {
		if (v == vertex || seen[v]) continue;
		++count;
		queue.assign(1, v);
		seen[v] = 1;
		for (int i = 0; i < (static_cast<int>(queue.size())); ++i)
			for (int to : adj[queue[i]])
				if (!seen[to]) {
					seen[to] = 1;
					queue.push_back(to);
				}
	}
	return count;
}

void check(int n, const vector<pair<int, int>> &edges) {
	LowLink g(n);
	for (auto [u, v] : edges) {
		int id = g.add_edge(u, v);
		assert(id == static_cast<int>((g.edges).size()) - 1);
	}
	g.build();
	int base = components(n, edges);
	vector<int> cut, bridges;
	for (int v = 0; v < n; ++v) {
		if (components(n, edges, v) > base) cut.push_back(v);
		assert(0 <= g.low[v] && g.low[v] <= g.ord[v] && g.ord[v] < n);
	}
	for (int i = 0; i < (static_cast<int>(edges.size())); ++i) {
		bool bridge = components(n, edges, -1, i) > base;
		if (bridge) bridges.push_back(i);
		assert(g.is_bridge(i) == bridge);
	}
	assert(g.articulation == cut);
	sort((g.bridges).begin(), (g.bridges).end());
	assert(g.bridges == bridges);
	auto ord = g.ord;
	sort(ord.begin(), ord.end());
	for (int v = 0; v < n; ++v) assert(ord[v] == v);
	g.build();
	assert(g.articulation == cut);
	sort((g.bridges).begin(), (g.bridges).end());
	assert(g.bridges == bridges);
}

int main() {
	for (int n = 0; n < 6; ++n) {
		vector<pair<int, int>> all_edges;
		for (int u = 0; u < n; ++u)
			for (int v = u + 1; v < n; ++v) all_edges.emplace_back(u, v);
		for (int mask = 0; mask < (1 << static_cast<int>(all_edges.size())); ++mask) {
			vector<pair<int, int>> edges;
			for (int i = 0; i < (static_cast<int>(all_edges.size())); ++i)
				if (mask >> i & 1) edges.push_back(all_edges[i]);
			check(n, edges);
		}
	}
	check(1, {{0, 0}, {0, 0}});
	check(2, {{0, 1}, {0, 1}, {0, 0}, {1, 1}});
	check(7, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 2}, {5, 6}});
	mt19937 random(123);
	for (int repeat_index = 0; repeat_index < 1500; ++repeat_index) {
		int n = 1 + random() % 12, m = random() % 40;
		vector<pair<int, int>> edges;
		for (int repeat_index = 0; repeat_index < m; ++repeat_index) edges.emplace_back(random() % n, random() % n);
		check(n, edges);
	}
	LowLink g(200000);
	g.edges.reserve(g.n);
	for (int v = 1; v < (g.n); ++v) {
		int id = g.add_edge(v - 1, v);
		assert(id == v - 1);
	}
	g.build();
	assert(static_cast<int>((g.bridges).size()) == g.n - 1 && static_cast<int>((g.articulation).size()) == g.n - 2);
	for (int v = 1; v < (g.n - 1); ++v) assert(g.articulation[v - 1] == v);
	for (int i = 0; i < (static_cast<int>((g.edges).size())); ++i) assert(g.is_bridge(i));
	g.add_edge(0, g.n - 1);
	g.build();
	assert(g.bridges.empty() && g.articulation.empty());
	for (int i = 0; i < (static_cast<int>((g.edges).size())); ++i) assert(!g.is_bridge(i));
	LowLink parallel(2);
	int id = parallel.add_edge(0, 1);
	parallel.build();
	assert(parallel.is_bridge(id));
	parallel.add_edge(0, 1);
	parallel.build();
	assert(parallel.bridges.empty() && parallel.articulation.empty());
}
