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
	int shape_ = 0, center_ = 0;
	struct Node { int position, head, parent, offset; };
	std::vector<Node> nodes;
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
		nodes.resize(n);
		if (!n) return;
		std::vector<int> degree(n), neighbors(n);
		for (auto [u, v] : edges) {
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		center_ = int(std::max_element(degree.begin(), degree.end()) - degree.begin());
		shape_ = degree[center_] <= 2 ? 1 : degree[center_] == n - 1 ? 2 : 0;
		degree[root] += 2;
		int count = 0;
		for (int v = 0; v < n; ++v) if (degree[v] == 1) order[count++] = v;
		for (int i = 0; i < count; ++i) {
			int v = order[i], p = neighbors[v];
			parent[v] = p;
			size[p] += size[v];
			if (heavy[p] == -1 || size[heavy[p]] < size[v]) heavy[p] = v;
			neighbors[p] ^= v;
			if (--degree[p] == 1) order[count++] = p;
		}
		assert(count == n - 1);
		in[root] = 0, out[root] = n, head[root] = root;
		for (int i = n - 2; i >= 0; --i) {
			int v = order[i], p = parent[v];
			depth[v] = depth[p] + 1;
			if (heavy[p] == v) in[v] = in[p] + 1, head[v] = head[p];
			else in[v] = out[p] -= size[v], head[v] = v;
			out[v] = in[v] + size[v];
		}
		for (int v = 0; v < n; ++v) out[v] = in[v] + size[v], order[in[v]] = v;
		parent[root] = -1;
		for (int v = 0; v < n; ++v) nodes[v] = {in[v], in[head[v]], parent[head[v]], depth[v] - in[v]};
	}
	int lca(int u, int v) const {
		check(u), check(v);
		if (shape_ == 1) return head[u] != head[v] ? order[0] : in[u] < in[v] ? u : v;
		if (shape_ == 2) return u == v ? u : u == order[0] || v == order[0] ? order[0] : center_;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) u = nodes[u].parent;
			else v = nodes[v].parent;
		}
		return nodes[u].position < nodes[v].position ? u : v;
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
		if (shape_) {
			if (k < 0 || k > depth[v]) return -1;
			if (!k) return v;
			if (k == depth[v]) return order[0];
			return shape_ == 2 ? parent[v] : order[in[v] - k];
		}
		if (k < 0) return -1;
		int target = nodes[v].position + nodes[v].offset - k;
		if (target < 0) return -1;
		while (nodes[v].head + nodes[v].offset > target) v = nodes[v].parent;
		return order[target - nodes[v].offset];
	}
	int jump(int u, int v, int k) const {
		check(u), check(v);
		if (k < 0) return -1;
		if (!k) return u;
		if (shape_ == 1) {
			if (head[u] == head[v]) {
				int delta = in[v] - in[u];
				if (k > (delta < 0 ? -delta : delta)) return -1;
				return order[in[u] + (delta < 0 ? -k : k)];
			}
			int a = depth[u], b = depth[v];
			if (k > a + b) return -1;
			if (k == a) return order[0];
			return k < a ? order[in[u] - k] : order[in[v] - (a + b - k)];
		}
		if (shape_ == 2) {
			int d = (u != center_) + (v != center_);
			if (u == v || k > d) return -1;
			return k == d ? v : center_;
		}
		int from = u, to = v, target = nodes[u].position + nodes[u].offset - k;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) {
				if (nodes[u].head + nodes[u].offset <= target) return order[target - nodes[u].offset];
				u = nodes[u].parent;
			} else v = nodes[v].parent;
		}
		int w = nodes[u].position < nodes[v].position ? u : v;
		int d = nodes[w].position + nodes[w].offset;
		int length = nodes[from].position + nodes[from].offset + nodes[to].position + nodes[to].offset - 2 * d;
		if (k > length) return -1;
		if (target >= d) return order[target - nodes[u].offset];
		return ancestor(to, length - k);
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
		if (shape_ == 1) {
			int x = in[u], y = in[v];
			if (head[u] == head[v]) {
				int l = std::min(x, y) + edge, r = std::max(x, y) + 1;
				if (l < r) f(l, r, x > y);
			} else if (head[u] == order[0]) {
				if (edge < x + 1) f(int(edge), x + 1, true);
				f(in[head[v]], y + 1, false);
			} else {
				f(in[head[u]], x + 1, true);
				if (edge < y + 1) f(int(edge), y + 1, false);
			}
			return;
		}
		std::array<std::array<int, 2>, 32> right;
		int count = 0;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) {
				f(nodes[u].head, nodes[u].position + 1, true);
				u = nodes[u].parent;
			} else {
				assert(count < int(right.size()));
				right[count++] = {nodes[v].head, nodes[v].position + 1};
				v = nodes[v].parent;
			}
		}
		bool reversed = nodes[u].position > nodes[v].position;
		int l = std::min(nodes[u].position, nodes[v].position) + edge, r = std::max(nodes[u].position, nodes[v].position) + 1;
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
