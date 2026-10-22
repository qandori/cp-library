#define PROBLEM "https://judge.yosupo.jp/problem/bipartitematching"
#include "../graph/matching/bipartite_graph.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int l = 0, r = 0, m = 0;
	in.read(l), in.read(r), in.read(m);
	std::vector<std::vector<int>> adj(l + r);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		in.read(u), in.read(v);
		v += l;
		adj[u].push_back(v), adj[v].push_back(u);
	}
	BipartiteGraph graph(adj);
	auto matching = graph.maximum_matching();
	auto independent = graph.maximum_independent_set();
	auto cover = graph.minimum_vertex_cover();
	assert(independent.size() + matching.size() == adj.size() && cover.size() == matching.size());
	std::vector<unsigned char> selected(adj.size());
	for (int v : independent) {
		assert(0 <= v && v < l + r && !selected[v]);
		selected[v] = 1;
	}
	for (int u = 0; u < l + r; ++u) for (int v : adj[u]) assert(!selected[u] || !selected[v]);
	for (int v : cover) {
		assert(0 <= v && v < l + r && !selected[v]);
		selected[v] = 1;
	}
	for ([[maybe_unused]] int used : selected) assert(used);
	out.println(matching.size());
	for (auto [u, v] : matching) {
		if (u >= l) std::swap(u, v);
		out.write(u), out.write(' '), out.println(v - l);
	}
}
