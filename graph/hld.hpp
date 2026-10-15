#ifndef HLD_HPP
#define HLD_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

class HLD {
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < n && int(in.size()) == n); }

  public:
	struct Segment {
		int l, r;
		bool reversed;
	};
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> parent, depth, size, heavy, head, in, out, order;
	explicit HLD(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		edges.emplace_back(u, v);
		return int(edges.size()) - 1;
	}
	void build(int root = 0) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		assert(!n || (0 <= root && root < n));
		parent.assign(n, -1);
		depth.assign(n, 0);
		size.assign(n, 1);
		heavy.assign(n, -1);
		head.resize(n), in.resize(n), out.resize(n), order.resize(n);
		if (!n) return;
		std::vector<int> start(std::size_t(n) + 1);
		for (auto [u, v] : edges) ++start[u + 1], ++start[v + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		std::vector<int> adj(2 * edges.size());
		std::copy(start.begin(), start.end() - 1, out.begin());
		for (auto [u, v] : edges) adj[out[u]++] = v, adj[out[v]++] = u;
		int count = 1;
		order[0] = root;
		parent[root] = root;
		for (int i = 0; i < count; ++i) {
			int v = order[i];
			for (int j = start[v]; j < start[v + 1]; ++j) {
				int to = adj[j];
				if (to == parent[v]) continue;
				assert(parent[to] == -1);
				parent[to] = v;
				depth[to] = depth[v] + 1;
				order[count++] = to;
			}
		}
		assert(count == n);
		for (int i = n - 1; i > 0; --i) {
			int v = order[i], p = parent[v];
			size[p] += size[v];
			if (heavy[p] == -1 || size[heavy[p]] < size[v]) heavy[p] = v;
		}
		int timer = 0, top = n - 1;
		order[top] = root;
		head[root] = root;
		while (top < n) {
			int v = order[top++];
			in[v] = timer;
			order[timer++] = v;
			for (int j = start[v]; j < start[v + 1]; ++j) {
				int to = adj[j];
				if (to == parent[v] || to == heavy[v]) continue;
				head[to] = to;
				order[--top] = to;
			}
			if (heavy[v] != -1) {
				head[heavy[v]] = head[v];
				order[--top] = heavy[v];
			}
		}
		for (int v = 0; v < n; ++v) out[v] = in[v] + size[v];
		parent[root] = -1;
	}
	int lca(int u, int v) const {
		check(u), check(v);
		while (head[u] != head[v]) {
			if (depth[head[u]] > depth[head[v]]) u = parent[head[u]];
			else v = parent[head[v]];
		}
		return depth[u] < depth[v] ? u : v;
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	bool is_ancestor(int u, int v) const {
		check(u), check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
	int ancestor(int v, int k) const {
		check(v);
		if (k < 0 || k > depth[v]) return -1;
		while (k > depth[v] - depth[head[v]]) {
			k -= depth[v] - depth[head[v]] + 1;
			v = parent[head[v]];
		}
		return order[in[v] - k];
	}
	int jump(int u, int v, int k) const {
		int w = lca(u, v), a = depth[u] - depth[w], b = depth[v] - depth[w];
		if (k < 0 || k > a + b) return -1;
		return k <= a ? ancestor(u, k) : ancestor(v, a + b - k);
	}
	std::pair<int, int> subtree(int v, bool edge = false) const {
		check(v);
		return {in[v] + edge, out[v]};
	}
	int edge_vertex(int id) const {
		assert(0 <= id && id < int(edges.size()));
		auto [u, v] = edges[id];
		check(u);
		return parent[u] == v ? u : v;
	}
	template <class F>
	void for_each_path(int u, int v, F &&f, bool edge = false) const {
		check(u), check(v);
		std::array<std::array<int, 2>, 32> right;
		int count = 0;
		while (head[u] != head[v]) {
			if (depth[head[u]] > depth[head[v]]) {
				f(in[head[u]], in[u] + 1, true);
				u = parent[head[u]];
			} else {
				assert(count < int(right.size()));
				right[count++] = {in[head[v]], in[v] + 1};
				v = parent[head[v]];
			}
		}
		bool reversed = depth[u] > depth[v];
		int l = std::min(in[u], in[v]) + edge, r = std::max(in[u], in[v]) + 1;
		if (l < r) f(l, r, reversed);
		while (count) {
			auto [a, b] = right[--count];
			f(a, b, false);
		}
	}
	std::vector<Segment> path(int u, int v, bool edge = false) const {
		std::vector<Segment> result;
		for_each_path(u, v, [&](int l, int r, bool reversed) { result.push_back({l, r, reversed}); }, edge);
		return result;
	}
};

#endif
