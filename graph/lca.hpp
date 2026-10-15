#ifndef LCA_HPP
#define LCA_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <utility>
#include <vector>

class LCA {
	template <class T>
	struct Alloc : std::allocator<T> {
		template <class U>
		struct rebind {
			using other = Alloc<U>;
		};
		Alloc() = default;
		template <class U>
		Alloc(const Alloc<U> &) noexcept {}
		template <class U>
		void construct(U *p) { ::new (static_cast<void *>(p)) U; }
	};
	std::vector<int> in, out, order;
	std::vector<std::vector<int, Alloc<int>>> table;
	void check(int v) const {
		assert(0 <= v && v < n && static_cast<int>(in.size()) == n);
		(void)v;
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
		table.clear();
		if (!n) return;
		std::vector<int> start(std::size_t(n) + 1);
		for (auto [u, v] : edges) ++start[u + 1], ++start[v + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		std::vector<int> adj(2 * edges.size());
		std::copy(start.begin(), start.end() - 1, out.begin());
		for (auto [u, v] : edges) adj[out[u]++] = v, adj[out[v]++] = u;
		std::fill(out.begin(), out.end(), 1);
		int timer = 0, top = n - 1;
		order[top] = root;
		parent[root] = root;
		while (top < n) {
			int v = order[top++];
			in[v] = timer;
			order[timer++] = v;
			for (int i = start[v]; i < (start[v + 1]); ++i) {
				int to = adj[i];
				if (to == parent[v]) continue;
				assert(parent[to] == -1);
				parent[to] = v;
				depth[to] = depth[v] + 1;
				order[--top] = to;
			}
		}
		assert(timer == n);
		for (int i = (n)-1; i >= 1; --i) out[parent[order[i]]] += out[order[i]];
		for (int v = 0; v < n; ++v) out[v] += in[v];
		table.emplace_back(n);
		table[0][0] = 0;
		for (int i = 1; i < n; ++i) table[0][i] = in[parent[order[i]]];
		parent[root] = -1;
		for (int k = 1; (1 << k) <= n; ++k) {
			table.emplace_back(n - (1 << k) + 1);
			for (int i = 0; i < (static_cast<int>(table[k].size())); ++i) table[k][i] = std::min(table[k - 1][i], table[k - 1][i + (1 << (k - 1))]);
		}
	}
	int lca(int u, int v) const {
		check(u);
		check(v);
		int l = in[u], r = in[v];
		if (l == r) return u;
		if (l > r) std::swap(l, r);
		++l;
		int k = std::bit_width(unsigned(r - l + 1)) - 1;
		return order[std::min(table[k][l], table[k][r - (1 << k) + 1])];
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	bool is_ancestor(int u, int v) const {
		check(u);
		check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
};

#endif
