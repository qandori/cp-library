#include "graph/low_link.hpp"

int components(int n, const vec<pair<int, int>> &edges, int vertex = -1, int edge = -1) {
	vec<vec<int>> adj(n);
	rep(i, sz(edges)) {
		auto [u, v] = edges[i];
		if (i == edge || u == vertex || v == vertex) continue;
		adj[u].push_back(v);
		adj[v].push_back(u);
	}
	vec<int> seen(n), queue;
	int count = 0;
	rep(v, n) {
		if (v == vertex || seen[v]) continue;
		++count;
		queue.assign(1, v);
		seen[v] = 1;
		rep(i, sz(queue)) for (int to : adj[queue[i]]) if (!seen[to]) {
			seen[to] = 1;
			queue.push_back(to);
		}
	}
	return count;
}

void check(int n, const vec<pair<int, int>> &edges) {
	LowLink g(n);
	for (auto [u, v] : edges) {
		int id = g.add_edge(u, v);
		assert(id == sz(g.edges) - 1);
	}
	g.build();
	int base = components(n, edges);
	vec<int> cut, bridges;
	rep(v, n) {
		if (components(n, edges, v) > base) cut.push_back(v);
		assert(0 <= g.low[v] && g.low[v] <= g.ord[v] && g.ord[v] < n);
	}
	rep(i, sz(edges)) {
		bool bridge = components(n, edges, -1, i) > base;
		if (bridge) bridges.push_back(i);
		assert(g.is_bridge(i) == bridge);
	}
	assert(g.articulation == cut);
	sort(all(g.bridges));
	assert(g.bridges == bridges);
	auto ord = g.ord;
	sort(all(ord));
	rep(v, n) assert(ord[v] == v);
	g.build();
	assert(g.articulation == cut);
	sort(all(g.bridges));
	assert(g.bridges == bridges);
}

int main() {
	rep(n, 6) {
		vec<pair<int, int>> all_edges;
		rep(u, n) rep(v, u + 1, n) all_edges.emplace_back(u, v);
		rep(mask, 1 << sz(all_edges)) {
			vec<pair<int, int>> edges;
			rep(i, sz(all_edges)) if (mask >> i & 1) edges.push_back(all_edges[i]);
			check(n, edges);
		}
	}
	check(1, {{0, 0}, {0, 0}});
	check(2, {{0, 1}, {0, 1}, {0, 0}, {1, 1}});
	check(7, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 2}, {5, 6}});
	mt19937 random(123);
	rep(1500) {
		int n = 1 + random() % 12, m = random() % 40;
		vec<pair<int, int>> edges;
		rep(m) edges.emplace_back(random() % n, random() % n);
		check(n, edges);
	}
	LowLink g(200000);
	g.edges.reserve(g.n);
	rep(v, 1, g.n) {
		int id = g.add_edge(v - 1, v);
		assert(id == v - 1);
	}
	g.build();
	assert(sz(g.bridges) == g.n - 1 && sz(g.articulation) == g.n - 2);
	rep(v, 1, g.n - 1) assert(g.articulation[v - 1] == v);
	rep(i, sz(g.edges)) assert(g.is_bridge(i));
	g.add_edge(0, g.n - 1);
	g.build();
	assert(g.bridges.empty() && g.articulation.empty());
	rep(i, sz(g.edges)) assert(!g.is_bridge(i));
	LowLink parallel(2);
	int id = parallel.add_edge(0, 1);
	parallel.build();
	assert(parallel.is_bridge(id));
	parallel.add_edge(0, 1);
	parallel.build();
	assert(parallel.bridges.empty() && parallel.articulation.empty());
}
