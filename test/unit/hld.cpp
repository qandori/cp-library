#include "graph/hld.hpp"
#include "string/rolling_hash.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

std::vector<int> naive_path(const std::vector<int> &parent, const std::vector<int> &depth, int u, int v) {
	std::vector<int> a, b;
	while (u != v) {
		if (depth[u] >= depth[v]) a.push_back(u), u = parent[u];
		else b.push_back(v), v = parent[v];
	}
	a.push_back(u);
	a.insert(a.end(), b.rbegin(), b.rend());
	return a;
}

void check(HLD &tree, int root, std::mt19937 &random) {
	int n = tree.n;
	std::vector<std::vector<int>> adj(n);
	for (auto [u, v] : tree.edges) adj[u].push_back(v), adj[v].push_back(u);
	std::vector<int> parent(n, -1), depth(n), sizes(n, 1), queue{root};
	for (int i = 0; i < n; ++i) {
		int v = queue[i];
		for (int to : adj[v]) {
			if (to == parent[v]) continue;
			parent[to] = v, depth[to] = depth[v] + 1;
			queue.push_back(to);
		}
	}
	for (int i = n - 1; i > 0; --i) sizes[parent[queue[i]]] += sizes[queue[i]];
	tree.build(root);
	assert(tree.parent == parent && tree.depth == depth && tree.size == sizes);
	std::vector<int> sorted = tree.order;
	std::sort(sorted.begin(), sorted.end());
	for (int v = 0; v < n; ++v) {
		assert(sorted[v] == v && tree.order[tree.in[v]] == v);
		auto [l, r] = tree.subtree(v);
		assert(l == tree.in[v] && r == tree.out[v] && r - l == sizes[v]);
		assert(tree.subtree(v, true) == std::pair(l + 1, r));
		for (int i = l; i < r; ++i) {
			int u = tree.order[i];
			while (depth[u] > depth[v]) u = parent[u];
			assert(u == v);
		}
		int heavy = tree.heavy[v];
		if (heavy != -1) {
			assert(parent[heavy] == v && tree.head[heavy] == tree.head[v] && tree.in[heavy] == tree.in[v] + 1);
			for (int to : adj[v])
				if (to != parent[v]) assert(sizes[to] <= sizes[heavy]);
		}
		int u = v;
		for (int k = 0; k <= depth[v]; ++k) {
			assert(tree.ancestor(v, k) == u);
			u = parent[u];
		}
		assert(tree.ancestor(v, -1) == -1 && tree.ancestor(v, depth[v] + 1) == -1);
	}
	for (int id = 0; id < int(tree.edges.size()); ++id) {
		auto [u, v] = tree.edges[id];
		assert(tree.edge_vertex(id) == (parent[u] == v ? u : v));
	}
	RollingHash forward(tree.order);
	auto reverse_order = tree.order;
	std::reverse(reverse_order.begin(), reverse_order.end());
	RollingHash backward(reverse_order);
	auto query = [&](int u, int v) {
		auto expected = naive_path(parent, depth, u, v);
		int lca = *std::min_element(expected.begin(), expected.end(), [&](int a, int b) { return depth[a] < depth[b]; });
		assert(tree.lca(u, v) == lca && tree.dist(u, v) + 1 == int(expected.size()));
		assert(tree.is_ancestor(u, v) == (lca == u));
		assert(tree.jump(u, v, -1) == -1 && tree.jump(u, v, int(expected.size())) == -1);
		for (int k = 0; k < int(expected.size()); ++k) assert(tree.jump(u, v, k) == expected[k]);
		for (bool edge : {false, true}) {
			std::vector<int> actual, callback;
			RollingHash::Value product = RollingHash::e();
			for (auto [l, r, reversed] : tree.path(u, v, edge)) {
				assert(0 <= l && l < r && r <= n);
				if (reversed)
					for (int i = r - 1; i >= l; --i) actual.push_back(tree.order[i]);
				else
					for (int i = l; i < r; ++i) actual.push_back(tree.order[i]);
			}
			tree.for_each_path(u, v, [&](int l, int r, bool reversed) {
				product = RollingHash::op(product, reversed ? backward.value(n - r, n - l) : forward.value(l, r));
				if (reversed)
					for (int i = r - 1; i >= l; --i) callback.push_back(tree.order[i]);
				else
					for (int i = l; i < r; ++i) callback.push_back(tree.order[i]); }, edge);
			std::vector<int> target = expected;
			if (edge) {
				target.clear();
				for (int i = 1; i < int(expected.size()); ++i) {
					int a = expected[i - 1], b = expected[i];
					target.push_back(parent[a] == b ? a : b);
				}
			}
			assert(actual == target && callback == target);
			assert(product == RollingHash(target).value());
		}
	};
	if (n <= 32) {
		for (int u = 0; u < n; ++u)
			for (int v = 0; v < n; ++v) query(u, v);
	} else {
		for (int i = 0; i < 1000; ++i) query(random() % n, random() % n);
	}
}

int main() {
	HLD empty;
	empty.build();
	empty.build();
	assert(empty.order.empty() && empty.parent.empty());
	std::mt19937 random(713);
	for (int n : {1, 2, 3, 7, 31, 64, 127, 257}) {
		for (int kind = 0; kind < 5; ++kind) {
			HLD tree(n);
			std::vector<int> label(n);
			std::iota(label.begin(), label.end(), 0);
			std::shuffle(label.begin(), label.end(), random);
			for (int v = 1; v < n; ++v) {
				int p = kind == 0 ? v - 1 : kind == 1			 ? 0
										: kind == 2				 ? (v - 1) / 2
										: kind == 3 && v < n / 2 ? v - 1
																 : random() % v;
				int u = label[p], w = label[v];
				if (random() & 1) std::swap(u, w);
				assert(tree.add_edge(u, w) == v - 1);
			}
			std::shuffle(tree.edges.begin(), tree.edges.end(), random);
			check(tree, label[0], random);
			check(tree, random() % n, random);
			auto copy = tree;
			check(copy, random() % n, random);
		}
	}
	const int n = 500000;
	HLD line(n);
	for (int v = 1; v < n; ++v) line.add_edge(v - 1, v);
	line.build(n / 2);
	for (int i = 0; i < 20000; ++i) {
		int u = random() % n, v = random() % n, k = random() % (std::abs(u - v) + 1);
		assert(line.jump(u, v, k) == (u <= v ? u + k : u - k));
		int count = 0;
		line.for_each_path(u, v, [&](int l, int r, bool) { count += r - l; });
		assert(count == std::abs(u - v) + 1);
		count = 0;
		line.for_each_path(u, v, [&](int l, int r, bool) { count += r - l; }, true);
		assert(count == std::abs(u - v));
	}
	HLD balanced((1 << 18) - 1);
	for (int v = 1; v < balanced.n; ++v) balanced.add_edge((v - 1) / 2, v);
	balanced.build();
	for (int i = 0; i < 10000; ++i) {
		int u = random() % balanced.n, v = random() % balanced.n;
		auto expected = naive_path(balanced.parent, balanced.depth, u, v);
		std::vector<int> actual;
		for (auto [l, r, reverse] : balanced.path(u, v)) {
			if (reverse)
				for (int j = r - 1; j >= l; --j) actual.push_back(balanced.order[j]);
			else
				for (int j = l; j < r; ++j) actual.push_back(balanced.order[j]);
		}
		assert(actual == expected);
	}
}
