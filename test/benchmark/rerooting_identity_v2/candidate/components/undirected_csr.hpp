#ifndef UNDIRECTED_CSR_HPP
#define UNDIRECTED_CSR_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <utility>
#include <vector>

struct UndirectedCSR {
	struct Arc { int to, id; };
	int n;
	std::vector<int> start;
	std::unique_ptr<Arc[]> adj;
	UndirectedCSR(int n, std::span<const std::pair<int, int>> edges) : n(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		assert(edges.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		start.assign(std::size_t(n) + 1, 0);
		for (auto [u, v] : edges) {
			assert(0 <= u && u < n && 0 <= v && v < n);
			++start[u + 1], ++start[v + 1];
		}
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		adj.reset(new Arc[2 * edges.size()]);
		std::vector<int> cursor(start.begin(), start.end() - 1);
		for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
			auto [u, v] = edges[i];
			adj[cursor[u]++] = {v, i}, adj[cursor[v]++] = {u, i};
		}
	}
	std::span<const Arc> operator[](int v) const {
		assert(0 <= v && v < n);
		return {adj.get() + start[v], std::size_t(start[v + 1] - start[v])};
	}
};

#endif
