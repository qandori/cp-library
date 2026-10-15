#include "graph/lca.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

int naive(const vector<int> &parent, const vector<int> &depth, int u, int v) {
	while (depth[u] > depth[v]) u = parent[u];
	while (depth[v] > depth[u]) v = parent[v];
	while (u != v) u = parent[u], v = parent[v];
	return u;
}

void check(LCA &tree, int root) {
	int n = tree.n;
	vector<vector<int>> adj(n);
	for (auto [u, v] : tree.edges) adj[u].push_back(v), adj[v].push_back(u);
	vector<int> parent(n, -1), depth(n), queue{root};
	for (int i = 0; i < (static_cast<int>(queue.size())); ++i) {
		int v = queue[i];
		for (int to : adj[v])
			if (to != parent[v]) {
				parent[to] = v;
				depth[to] = depth[v] + 1;
				queue.push_back(to);
			}
	}
	tree.build(root);
	assert(tree.parent == parent && tree.depth == depth);
	auto query = [&](int u, int v) {
		int w = naive(parent, depth, u, v);
		assert(tree.lca(u, v) == w);
		assert(tree.dist(u, v) == depth[u] + depth[v] - 2 * depth[w]);
		assert(tree.is_ancestor(u, v) == (w == u));
	};
	if (n <= 1024) {
		for (int u = 0; u < n; ++u)
			for (int v = 0; v < n; ++v) query(u, v);
	} else {
		mt19937 random(456);
		for (int repeat_index = 0; repeat_index < 200000; ++repeat_index) query(random() % n, random() % n);
	}
}

int main() {
	LCA empty;
	empty.build();
	empty.build();
	mt19937 random(123);
	for (int n : {1, 2, 3, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257}) {
		for (int kind = 0; kind < 5; ++kind) {
			LCA tree(n);
			vector<int> labels(n);
			iota(labels.begin(), labels.end(), 0);
			shuffle(labels.begin(), labels.end(), random);
			for (int v = 1; v < n; ++v) {
				int p = kind == 0 ? v - 1 : kind == 1 ? 0
										: kind == 2	  ? (v - 1) / 2
													  : random() % v;
				int id = tree.add_edge(labels[p], labels[v]);
				assert(id == v - 1);
			}
			shuffle((tree.edges).begin(), (tree.edges).end(), random);
			check(tree, labels[0]);
			check(tree, random() % n);
		}
	}
	for (int repeat_index = 0; repeat_index < 100; ++repeat_index) {
		int n = 1 + random() % 80;
		LCA tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(random() % v, v);
		check(tree, random() % n);
	}
	{
		LCA large(20000);
		vector<int> labels(large.n);
		iota(labels.begin(), labels.end(), 0);
		shuffle(labels.begin(), labels.end(), random);
		for (int v = 1; v < (large.n); ++v) large.add_edge(labels[random() % v], labels[v]);
		shuffle((large.edges).begin(), (large.edges).end(), random);
		check(large, labels[0]);
		check(large, random() % large.n);
	}
	const int n = 500000;
	LCA path(n);
	for (int v = 1; v < n; ++v) path.add_edge(v - 1, v);
	path.build(n / 2);
	for (int i = 0; i < 200000; ++i) {
		int u = random() % n, v = random() % n;
		int expected = min(u, v) > n / 2 ? min(u, v) : max(u, v) < n / 2 ? max(u, v)
																		 : n / 2;
		assert(path.lca(u, v) == expected);
		assert(path.dist(u, v) == abs(u - v));
	}
	LCA copy = path;
	copy.build(0);
	assert(copy.lca(n / 2 - 1, n / 2 + 1) == n / 2 - 1);
	assert(path.lca(n / 2 - 1, n / 2 + 1) == n / 2);
}
