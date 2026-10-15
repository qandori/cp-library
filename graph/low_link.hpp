#ifndef LOW_LINK_HPP
#define LOW_LINK_HPP

#include "../template.hpp"

struct LowLink {
	struct Arc {
		int to, id;
	};
	struct Frame {
		int v, parent, next;
	};
	int n;
	vec<pair<int, int>> edges;
	vec<int> ord, low, articulation, bridges;
	explicit LowLink(int n) : n(n) { assert(n >= 0); }
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		assert(edges.size() < size_t(numeric_limits<int>::max() / 2));
		int id = sz(edges);
		edges.emplace_back(u, v);
		return id;
	}
	void build() {
		assert(n >= 0 && edges.size() <= size_t(numeric_limits<int>::max() / 2));
		vec<int> start(size_t(n) + 1);
		for (auto [u, v] : edges) {
			assert(0 <= u && u < n && 0 <= v && v < n);
			++start[u + 1], ++start[v + 1];
		}
		rep(v, n) start[v + 1] += start[v];
		unique_ptr<Arc[]> adj(new Arc[2 * edges.size()]);
		low.assign(start.begin(), start.end() - 1);
		rep(i, sz(edges)) {
			auto [u, v] = edges[i];
			adj[low[u]++] = {v, i};
			adj[low[v]++] = {u, i};
		}
		ord.assign(n, -1);
		low.assign(n, -1);
		articulation.clear();
		bridges.clear();
		vec<uint8_t> cut(n);
		vec<Frame> stack;
		stack.reserve(n);
		int timer = 0;
		rep(root, n) {
			if (ord[root] != -1) continue;
			int children = 0;
			ord[root] = low[root] = timer++;
			stack.push_back({root, -1, start[root]});
			while (!stack.empty()) {
				auto &f = stack.back();
				int v = f.v;
				if (f.next == start[v + 1]) {
					int id = f.parent;
					stack.pop_back();
					if (stack.empty()) cut[v] = children > 1;
					else {
						int p = stack.back().v;
						chmin(low[p], low[v]);
						if (ord[p] < low[v]) bridges.push_back(id);
						if (p != root && ord[p] <= low[v]) cut[p] = 1;
					}
					continue;
				}
				auto [to, id] = adj[f.next++];
				if (id == f.parent) continue;
				if (ord[to] != -1) {
					chmin(low[v], ord[to]);
					continue;
				}
				children += v == root;
				ord[to] = low[to] = timer++;
				stack.push_back({to, id, start[to]});
			}
		}
		rep(v, n) if (cut[v]) articulation.push_back(v);
	}
	bool is_bridge(int id) const {
		assert(0 <= id && id < sz(edges) && sz(ord) == n);
		auto [u, v] = edges[id];
		if (ord[u] > ord[v]) swap(u, v);
		return ord[u] < low[v];
	}
};

#endif
