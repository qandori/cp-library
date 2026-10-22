#ifndef BIPARTITE_GRAPH_HPP
#define BIPARTITE_GRAPH_HPP

#include "bipartite_matching.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

class BipartiteGraph {
	bool bipartite_ = true;
	std::vector<int> color_, left_, right_;
	BipartiteMatching matching_{0, 0};
	std::vector<int> restore(BipartiteMatching::VertexSet vertices) const {
		std::vector<int> result;
		result.reserve(vertices.left.size() + vertices.right.size());
		for (int v : vertices.left) result.push_back(left_[v]);
		for (int v : vertices.right) result.push_back(right_[v]);
		return result;
	}

  public:
	BipartiteGraph() = default;
	explicit BipartiteGraph(const std::vector<std::vector<int>>& graph) { build(graph); }
	bool build(const std::vector<std::vector<int>>& graph) {
		assert(graph.size() < std::size_t(std::numeric_limits<int>::max()));
		int n = static_cast<int>(graph.size());
		bipartite_ = true;
		color_.assign(n, -1);
		left_.clear(), right_.clear();
		matching_ = BipartiteMatching(0, 0);
		std::vector<int> queue;
		queue.reserve(n);
		for (int root = 0; root < n; ++root) if (color_[root] < 0) {
			color_[root] = 0;
			queue.clear();
			queue.push_back(root);
			for (std::size_t head = 0; head < queue.size(); ++head) {
				int v = queue[head];
				for (int to : graph[v]) {
					assert(0 <= to && to < n);
					if (color_[to] < 0) color_[to] = color_[v] ^ 1, queue.push_back(to);
					else if (color_[to] == color_[v]) return bipartite_ = false;
				}
			}
		}
		std::vector<int> index(n);
		std::size_t m = 0;
		for (int v = 0; v < n; ++v) {
			auto& side = color_[v] ? right_ : left_;
			index[v] = static_cast<int>(side.size());
			side.push_back(v);
			if (!color_[v]) {
				assert(graph[v].size() <= std::size_t(std::numeric_limits<int>::max()) - m);
				m += graph[v].size();
			}
		}
		matching_ = BipartiteMatching(static_cast<int>(left_.size()), static_cast<int>(right_.size()));
		matching_.reserve(m);
		for (int v : left_) for (int to : graph[v]) matching_.add_edge(index[v], index[to]);
		return true;
	}
	int size() const { return static_cast<int>(color_.size()); }
	bool empty() const { return color_.empty(); }
	bool is_bipartite() const { return bipartite_; }
	const std::vector<int>& coloring() const { assert(bipartite_); return color_; }
	int matching_size() { assert(bipartite_); return matching_.build(); }
	std::vector<std::pair<int, int>> maximum_matching() {
		assert(bipartite_);
		auto result = matching_.edges();
		for (auto& [u, v] : result) u = left_[u], v = right_[v];
		return result;
	}
	std::vector<int> mates() {
		assert(bipartite_);
		std::vector<int> result(size(), -1);
		for (auto [u, v] : maximum_matching()) result[u] = v, result[v] = u;
		return result;
	}
	std::vector<int> minimum_vertex_cover() {
		assert(bipartite_);
		return restore(matching_.minimum_vertex_cover());
	}
	std::vector<int> maximum_independent_set() {
		assert(bipartite_);
		return restore(matching_.maximum_independent_set());
	}
	std::optional<std::vector<std::pair<int, int>>> minimum_edge_cover() {
		assert(bipartite_);
		auto ids = matching_.minimum_edge_cover();
		if (!ids) return std::nullopt;
		std::vector<std::pair<int, int>> result;
		result.reserve(ids->size());
		for (int id : *ids) {
			auto [u, v] = matching_.get_edge(id);
			result.emplace_back(left_[u], right_[v]);
		}
		return result;
	}
};

#endif
