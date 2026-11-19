#include "graph/bipartite_matching.hpp"
#include "flow/max_flow.hpp"
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using Edge = std::pair<int, int>;
void require(bool condition) { if (!condition) std::abort(); }

int oracle(int l, int r, const std::vector<Edge>& edges) {
	MaxFlow<int> graph(l + r + 2);
	graph.reserve(edges.size() + l + r);
	for (int u = 0; u < l; ++u) graph.add_edge(l + r, u, 1);
	for (int v = 0; v < r; ++v) graph.add_edge(l + v, l + r + 1, 1);
	for (auto [u, v] : edges) graph.add_edge(u, l + v, 1);
	return graph.flow(l + r, l + r + 1);
}

void matching_check(int l, int r, const std::vector<Edge>& edges, const std::vector<Edge>& matching,
	const BipartiteMatching::VertexSet& required = {}) {
	std::vector<int> left(l), right(r);
	std::vector<std::vector<int>> adj(l);
	for (auto [u, v] : edges) adj[u].push_back(v);
	for (auto [u, v] : matching) {
		require(0 <= u && u < l && 0 <= v && v < r && !left[u]++ && !right[v]++);
		require(std::find(adj[u].begin(), adj[u].end(), v) != adj[u].end());
	}
	for (int u : required.left) require(left[u]);
	for (int v : required.right) require(right[v]);
}

void validate(BipartiteMatching& graph, const std::vector<Edge>& edges, int expected) {
	int l = graph.left_size(), r = graph.right_size();
	require(graph.build() == expected && graph.build() == expected);
	auto matching = graph.maximum_matching();
	require(int(matching.size()) == expected);
	matching_check(l, r, edges, matching);
	const auto& left = graph.left_mates();
	const auto& right = graph.right_mates();
	for (auto [u, v] : matching) require(left[u] == v && right[v] == u);
	auto ids = graph.matching_edge_ids();
	require(ids.size() == matching.size());
	for (int i = 0; i < expected; ++i) require(edges[ids[i]] == matching[i]);
	auto cover = graph.minimum_vertex_cover();
	require(int(cover.left.size() + cover.right.size()) == expected);
	std::vector<int> chosen_left(l), chosen_right(r);
	for (int u : cover.left) require(!chosen_left[u]++);
	for (int v : cover.right) require(!chosen_right[v]++);
	for (auto [u, v] : edges) require(chosen_left[u] || chosen_right[v]);
	require(graph.edge_classification().size() == edges.size());
}

void check(int l, int r, const std::vector<Edge>& edges, int expected) {
	BipartiteMatching graph(l, r);
	graph.reserve(edges.size());
	for (auto [u, v] : edges) graph.add_edge(u, v);
	BipartiteMatching::VertexSet required;
	if (expected == l) {
		required.left.resize(l);
		std::iota(required.left.begin(), required.left.end(), 0);
	}
	if (expected == r) {
		required.right.resize(r);
		std::iota(required.right.begin(), required.right.end(), 0);
	}
	auto result = std::as_const(graph).matching_covering(required);
	require(bool(result));
	matching_check(l, r, edges, *result, required);
	validate(graph, edges, expected);
}

void relabel_check(int l, int r, std::vector<Edge> edges, int expected, std::mt19937& random) {
	check(l, r, edges, expected);
	std::reverse(edges.begin(), edges.end());
	check(l, r, edges, expected);
	std::vector<int> left(l), right(r);
	std::iota(left.begin(), left.end(), 0);
	std::iota(right.begin(), right.end(), 0);
	std::shuffle(left.begin(), left.end(), random);
	std::shuffle(right.begin(), right.end(), random);
	for (auto& [u, v] : edges) u = left[u], v = right[v];
	std::shuffle(edges.begin(), edges.end(), random);
	for (int i = int(edges.size()) - 1; i >= 0; --i) if (i % 5 == 0) edges.push_back(edges[i]);
	check(l, r, edges, expected);
	for (auto& [u, v] : edges) std::swap(u, v);
	check(r, l, edges, expected);
}

void grids(std::mt19937& random) {
	for (auto [h, w] : std::vector<Edge>{ {1, 73}, {2, 41}, {7, 9}, {20, 31}, {41, 27}, {100, 100} }) {
		for (unsigned walls : {0u, 10u, 35u, 70u, 100u}) {
			std::vector<int> id(h * w, -1);
			int l = 0, r = 0;
			for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j)
				if (random() % 100 >= walls) id[i * w + j] = ((i + j) % 2 ? r++ : l++);
			std::vector<Edge> edges;
			for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) {
				if ((i + j) % 2 || id[i * w + j] < 0) continue;
				for (auto [di, dj] : std::vector<Edge>{ {1, 0}, {-1, 0}, {0, 1}, {0, -1} }) {
					int x = i + di, y = j + dj;
					if (0 <= x && x < h && 0 <= y && y < w && id[x * w + y] >= 0)
						edges.emplace_back(id[i * w + j], id[x * w + y]);
				}
			}
			relabel_check(l, r, edges, oracle(l, r, edges), random);
		}
	}
}

void alternating_paths(std::mt19937& random) {
	for (int length : {2, 3, 8, 31, 257}) {
		constexpr int components = 129;
		int n = length * components;
		std::vector<Edge> edges;
		for (int c = 0; c < components; ++c) for (int i = 0; i + 1 < length; ++i)
			edges.emplace_back(c * length + i, c * length + i + 1);
		for (int i = 0; i < n; ++i) edges.emplace_back(i, i);
		relabel_check(n, n, edges, n, random);
		bipartite_matching_detail::CSR csr(n, edges, false);
		std::vector<int> left(n, -1), right(n, -1);
		for (int c = 0; c < components; ++c) for (int i = 0; i + 1 < length; ++i) {
			int u = c * length + i;
			left[u] = u + 1, right[u + 1] = u;
		}
		auto phase = bipartite_matching_detail::hopcroft_karp(csr, left, right, 1, 2);
		int added = phase.added + bipartite_matching_detail::augment(csr, left, right);
		require(added == components);
		for (int u = 0; u < n; ++u) require(left[u] == u && right[u] == u);
	}
}

void incremental(std::mt19937& random) {
	for (bool transpose : {false, true}) {
		int l = transpose ? 193 : 257, r = transpose ? 257 : 193;
		BipartiteMatching graph(l, r);
		std::vector<Edge> edges;
		for (int batch = 0; batch < 24; ++batch) {
			for (int i = 0; i < 47; ++i) {
				Edge edge{int(random() % l), int(random() % r)};
				if (i % 7 == 0 && !edges.empty()) edge = edges.back();
				edges.push_back(edge);
				graph.add_edge(edge.first, edge.second);
			}
			validate(graph, edges, oracle(l, r, edges));
		}
		auto copy = graph;
		graph.clear();
		require(graph.build() == 0 && graph.edge_classification().empty());
		validate(copy, edges, oracle(l, r, edges));
	}
}

void csr_check(const bipartite_matching_detail::CSR& csr, const std::vector<std::vector<int>>& rows) {
	std::vector<int> start{0}, adj;
	for (const auto& row : rows) {
		adj.insert(adj.end(), row.begin(), row.end());
		start.push_back(int(adj.size()));
	}
	require(csr.start == start && csr.adj == adj);
}

void csr_construction(std::mt19937& random) {
	for (int trial = 0; trial < 300; ++trial) {
		int l = random() % 70, r = random() % 70;
		std::vector<Edge> edges;
		if (l && r) for (int m = random() % 500; m--; ) {
			Edge edge{int(random() % l), int(random() % r)};
			if (m % 5 == 0 && !edges.empty()) edge = edges.back();
			edges.push_back(edge);
		}
		std::vector<std::vector<int>> left(l), right(r), reverse(r);
		for (auto [u, v] : edges) left[u].push_back(v), right[v].push_back(u);
		bipartite_matching_detail::CSR graph(l, edges, false), transposed(r, edges, true);
		csr_check(graph, left);
		csr_check(transposed, right);
		for (int u = 0; u < l; ++u) for (int v : left[u]) reverse[v].push_back(u);
		csr_check(bipartite_matching_detail::CSR(r, graph), reverse);
	}
}

void isolated_unmatched() {
	for (int l : {0, 1, 63, 64, 65, 129}) {
		int r = l + 3, matched = l / 3;
		std::vector<Edge> edges;
		std::vector<int> left(l, -1), right(r, -1);
		for (int u = 0; u < matched; ++u) {
			edges.emplace_back(u, u);
			left[u] = right[u] = u;
		}
		auto before_left = left, before_right = right;
		bipartite_matching_detail::CSR csr(l, edges, false);
		require(bipartite_matching_detail::augment(csr, left, right) == 0);
		require(left == before_left && right == before_right);
		check(l, r, edges, matched);
		for (auto& [u, v] : edges) std::swap(u, v);
		check(r, l, edges, matched);
	}
}

int main() {
	std::mt19937 random(20261117);
	csr_construction(random);
	isolated_unmatched();
	grids(random);
	alternating_paths(random);
	incremental(random);
}
