#ifndef REROOTING_HPP
#define REROOTING_HPP

#include "../components/undirected_csr.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template<class T, class E = T>
class Rerooting {
	int n_;
	bool built_ = false;
	std::vector<std::pair<int, int>> edges_;
	std::vector<T> values_;
	std::vector<E> messages_;

  public:
	using Vertex = T;
	using Edge = E;
	explicit Rerooting(int n = 0) : n_(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		edges_.reserve(std::max(0, n - 1));
	}
	int size() const { return n_; }
	bool empty() const { return n_ == 0; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	int add_edge(int u, int v) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		built_ = false;
		edges_.emplace_back(u, v);
		return edge_count() - 1;
	}
	const std::pair<int, int>& edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const T& operator[](int v) const {
		assert(built_ && 0 <= v && v < n_);
		return values_[v];
	}
	const std::vector<T>& values() const {
		assert(built_);
		return values_;
	}
	const E& edge_dp(int id, bool reverse = false) const {
		assert(built_ && 0 <= id && id < edge_count());
		return messages_[2 * id + reverse];
	}
	template<class Merge, class AddVertex, class AddEdge>
	const std::vector<T>& build(E unit, Merge&& merge, AddVertex&& add_vertex, AddEdge&& add_edge, int root = 0) {
		assert(n_ == 0 || (0 <= root && root < n_));
		built_ = false;
		values_.clear();
		messages_.assign(2 * edges_.size(), unit);
		if (n_ == 0) {
			built_ = true;
			return values_;
		}
		UndirectedCSR graph(n_, edges_);
		for (int v = 0; v < n_; ++v)
			for (int i = graph.start[v]; i < graph.start[v + 1]; ++i) {
				int id = graph.adj[i].id;
				graph.adj[i].id = 2 * id + (edges_[id].second == v);
			}
		std::vector<int> parent(n_, -2), parent_edge(n_, -1), order;
		order.reserve(n_);
		auto visit = [&](int s) {
			if (parent[s] != -2) return;
			parent[s] = -1;
			order.push_back(s);
			for (std::size_t i = order.size() - 1; i < order.size(); ++i) {
				int v = order[i];
				for (auto [to, id] : graph[v]) {
					if (id == parent_edge[v]) continue;
					assert(parent[to] == -2);
					if (parent[to] != -2) continue;
					parent[to] = v, parent_edge[to] = id ^ 1;
					order.push_back(to);
				}
			}
		};
		visit(root);
		for (int v = 0; v < n_; ++v) visit(v);
		for (auto it = order.rbegin(); it != order.rend(); ++it) {
			int v = *it, id = parent_edge[v];
			if (id == -1) continue;
			E value = unit;
			for (auto arc : graph[v])
				if (arc.id != id) value = merge(value, messages_[arc.id ^ 1]);
			messages_[id] = add_edge(add_vertex(value, v), v, parent[v], id >> 1);
		}
		if constexpr (std::is_same_v<T, E>) values_.assign(n_, unit);
		else values_.resize(n_);
		int maximum = 0;
		for (int v = 0; v < n_; ++v) maximum = std::max(maximum, graph.start[v + 1] - graph.start[v]);
		std::vector<E> prefix(maximum, unit);
		for (int v : order) {
			auto arcs = graph[v];
			int degree = static_cast<int>(arcs.size());
			if (degree == 0) {
				values_[v] = add_vertex(unit, v);
				continue;
			}
			auto incoming = [&](int i) -> const E& { return messages_[arcs[i].id ^ 1]; };
			if (degree > 1) prefix[1] = incoming(0);
			for (int i = 1; i + 1 < degree; ++i) prefix[i + 1] = merge(prefix[i], incoming(i));
			values_[v] = add_vertex(degree == 1 ? incoming(0) : merge(prefix[degree - 1], incoming(degree - 1)), v);
			E suffix = unit;
			for (int i = degree - 1; i >= 0; --i) {
				auto [to, id] = arcs[i];
				if (id != parent_edge[v]) {
					E value = i == degree - 1 ? prefix[i] : i == 0 ? suffix : merge(prefix[i], suffix);
					messages_[id] = add_edge(add_vertex(value, v), v, to, id >> 1);
				}
				if (i > 0) suffix = i == degree - 1 ? incoming(i) : merge(incoming(i), suffix);
			}
		}
		built_ = true;
		return values_;
	}
};

#endif
