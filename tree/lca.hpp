#ifndef LCA_HPP
#define LCA_HPP

#include "../components/static_range_min.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

class LCA {
	std::vector<int> in, out, order;
	bool ordered_ = false;
	StaticRangeMin<int, true> table;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < n && static_cast<int>(in.size()) == n); }

	template<bool Ordered = false> void finish(int root) {
		in[root] = n;
		ordered_ = root == 0;
		for (int i = n - 2; i >= 0; --i) {
			int v = Ordered ? n - 1 - i : order[i], p = parent[v];
			depth[v] = depth[p] + 1;
			in[v] = in[p];
			in[p] -= out[v];
		}
		std::vector<int> values(n);
		for (int v = 0; v < n; ++v) {
			--in[v];
			out[v] += in[v];
			order[in[v]] = v;
			if constexpr (Ordered) values[in[v]] = v ? parent[v] : 0;
			else ordered_ &= parent[v] < v;
		}
		if constexpr (!Ordered)
			for (int i = 1; i < n; ++i) values[i] = ordered_ ? parent[order[i]] : in[parent[order[i]]];
		table.build(std::move(values));
		parent[root] = -1;
	}

  public:
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> parent, depth;
	explicit LCA(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		edges.emplace_back(u, v);
		return static_cast<int>(edges.size()) - 1;
	}
	void build(int root = 0) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		assert(!n || (0 <= root && root < n));
		in.resize(n);
		out.resize(n);
		order.resize(n);
		parent.assign(n, -1);
		depth.assign(n, 0);
		table = StaticRangeMin<int, true>();
		if (!n) return;
		std::vector<int> degree(n), neighbors(n);
		for (auto [u, v] : edges) {
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		degree[root] += 2;
		std::fill(out.begin(), out.end(), 1);
		int count = 0;
		for (int v = 0; v < n; ++v) if (degree[v] == 1) order[count++] = v;
		for (int i = 0; i < count; ++i) {
			int v = order[i], p = neighbors[v];
			parent[v] = p;
			out[p] += out[v];
			neighbors[p] ^= v;
			if (--degree[p] == 1) order[count++] = p;
		}
		assert(count == n - 1);
		finish(root);
	}

	void build_from_parents(std::vector<int> parents) {
		assert(parents.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		n = static_cast<int>(parents.size());
		parent = std::move(parents);
		in.resize(n);
		out.assign(n, 1);
		order.resize(n);
		depth.assign(n, 0);
		table = StaticRangeMin<int, true>();
		edges.clear();
		edges.reserve(std::max(0, n - 1));
		if (!n) return;
		int root = -1;
		bool ordered = true;
		for (int v = 0; v < n; ++v) {
			int p = parent[v];
			assert(-1 <= p && p < n && p != v);
			if (p == -1) {
				assert(root == -1);
				root = v;
			} else {
				edges.emplace_back(p, v);
				ordered &= p < v;
			}
		}
		assert(root != -1);
		if (ordered) {
			for (int v = n - 1; v > 0; --v) {
				out[parent[v]] += out[v];
			}
			finish<true>(root);
			return;
		} else {
			std::fill(in.begin(), in.end(), 0);
			for (int v = 0; v < n; ++v) if (v != root) ++in[parent[v]];
			int count = 0;
			for (int v = 0; v < n; ++v) if (!in[v] && v != root) order[count++] = v;
			for (int i = 0; i < count; ++i) {
				int v = order[i], p = parent[v];
				out[p] += out[v];
				if (!--in[p] && p != root) order[count++] = p;
			}
			assert(count == n - 1);
		}
		finish(root);
	}

	int lca(int u, int v) const {
		check(u);
		check(v);
		int l = in[u], r = in[v];
		if (l == r) return u;
		if (l > r) std::swap(l, r);
		++l;
		int w = table.prod(l, r + 1);
		return ordered_ ? w : order[w];
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	int distance(int u, int v) const { return dist(u, v); }
	bool is_ancestor(int u, int v) const {
		check(u);
		check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
	std::pair<int, int> subtree(int v) const {
		check(v);
		return {in[v], out[v]};
	}
};

#endif
