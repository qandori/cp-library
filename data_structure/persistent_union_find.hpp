#ifndef PERSISTENT_UNION_FIND_HPP
#define PERSISTENT_UNION_FIND_HPP

#include "persistent_array.hpp"
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class PersistentUnionFind {
	PersistentArray<int> parent;

  public:
	struct root_type {
		PersistentArray<int>::root_type data = 0;
		int components = 0;
		bool operator==(const root_type &) const = default;
	};
  private:
	std::pair<int, int> leader_size(root_type root, int x) const {
		for (;;) {
			int p = parent.get(root.data, x);
			if (p < 0) return {x, p};
			x = p;
		}
	}
  public:
	explicit PersistentUnionFind(int n = 0) : parent(n, -1) {}
	int vertex_count() const { return parent.size(); }
	int size() const { return vertex_count(); }
	root_type initial_root() const { return {parent.initial_root(), vertex_count()}; }
	int leader(root_type root, int x) const { return leader_size(root, x).first; }
	bool same(root_type root, int a, int b) const { return leader(root, a) == leader(root, b); }
	int size(root_type root, int x) const { return -leader_size(root, x).second; }
	int component_count(root_type root) const { return root.components; }
	int count(root_type root) const { return component_count(root); }
	root_type merge(root_type root, int a, int b) {
		auto [u, x] = leader_size(root, a);
		auto [v, y] = leader_size(root, b);
		if (u == v) return root;
		if (x > y) { std::swap(u, v); std::swap(x, y); }
		root.data = parent.set_pair(root.data, u, x + y, v, u);
		--root.components;
		return root;
	}
	std::vector<std::vector<int>> groups(root_type root) const {
		std::vector<std::vector<int>> result(vertex_count());
		for (int i = 0; i < vertex_count(); ++i) result[leader(root, i)].push_back(i);
		std::vector<std::vector<int>> groups;
		for (auto &g : result) if (!g.empty()) groups.push_back(std::move(g));
		return groups;
	}
	void reserve(std::size_t count) { parent.reserve(count); }
	std::size_t node_count() const { return parent.node_count(); }
	std::size_t memory_usage() const { return parent.memory_usage(); }
};

#endif
