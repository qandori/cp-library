#include "tree/dsu_on_tree.hpp"
#include <bit>
#include <cstdint>
#include <memory>
#include <numeric>
#include <random>

void verify(int n, const std::vector<std::pair<int, int>>& edges, int root) {
	DSUOnTree tree(n);
	for (int id = 0; id < int(edges.size()); ++id) {
		assert(tree.add_edge(edges[id].first, edges[id].second) == id);
		assert(tree.edge(id) == edges[id]);
	}
	tree.build(root);
	assert(tree.size() == n && tree.empty() == (n == 0));
	assert(tree.root() == (n ? root : -1));
	std::vector<std::vector<int>> graph(n), members(n);
	for (auto [u, v] : edges) graph[u].push_back(v), graph[v].push_back(u);
	std::vector<int> parent(n, -2), queue;
	if (n) parent[root] = -1, queue.push_back(root);
	for (int i = 0; i < int(queue.size()); ++i) {
		int v = queue[i];
		for (int to : graph[v]) if (to != parent[v]) parent[to] = v, queue.push_back(to);
	}
	for (int v = 0; v < n; ++v) for (int p = v; p != -1; p = parent[p]) members[p].push_back(v);
	std::vector<int> used(n);
	for (int i = 0; i < n; ++i) {
		int v = tree.vertex(i);
		assert(tree.order()[i] == v && used[v]++ == 0);
		assert(tree.parent(v) == parent[v]);
		auto [l, r] = tree.subtree(v);
		assert(l == i && r - l == int(members[v].size()));
		assert(tree.subtree_size(v) == int(members[v].size()));
		auto actual = std::vector<int>(tree.vertices(v).begin(), tree.vertices(v).end());
		std::sort(actual.begin(), actual.end());
		assert(actual == members[v]);
		int h = tree.heavy_child(v), maximum = 0;
		for (int to : graph[v]) if (parent[to] == v) maximum = std::max(maximum, int(members[to].size()));
		if (h == -1) assert(maximum == 0);
		else assert(parent[h] == v && int(members[h].size()) == maximum && tree.subtree(h).first == i + 1);
	}
	std::vector<int> active(n), frequency(7), answered(n);
	int sum = 0, added = 0, removed = 0;
	auto add = [&](int v) {
		assert(active[v]++ == 0);
		++frequency[v % 7];
		sum += v + 1;
		++added;
	};
	auto remove = [&](int v) {
		assert(--active[v] == 0);
		--frequency[v % 7];
		sum -= v + 1;
		++removed;
	};
	auto answer = [&](int v) {
		assert(answered[v]++ == 0);
		std::vector<int> expected(n), counts(7);
		int total = 0;
		for (int x : members[v]) expected[x] = 1, ++counts[x % 7], total += x + 1;
		assert(active == expected && sum == total && frequency == counts);
	};
	for (bool keep : {false, true}) {
		tree.run(add, remove, answer, keep);
		assert(added <= n * int(std::bit_width(unsigned(n))));
		assert(added - removed == (keep ? n : 0));
		if (keep) for (int v = 0; v < n; ++v) remove(v);
		assert(sum == 0 && std::all_of(frequency.begin(), frequency.end(), [](int x) { return x == 0; }));
		std::fill(answered.begin(), answered.end(), 0);
		added = removed = 0;
	}
	int resets = 0;
	tree.run_clear(add, answer, [&] {
		++resets;
		std::fill(active.begin(), active.end(), 0);
		std::fill(frequency.begin(), frequency.end(), 0);
		sum = 0;
	});
	assert(sum == 0 && resets <= n && (n == 0 || resets > 0));
	if (n > 1) {
		tree.build((root + 1) % n);
		int count = 0;
		tree.run([&](int) { ++count; }, [&](int) { --count; }, [&](int v) { assert(count == tree.subtree_size(v)); });
		assert(count == 0);
	}
	auto copy = tree;
	int calls = 0;
	copy.run([](int) {}, [](int) {}, [state = std::make_unique<int>(0), &calls](int) mutable { ++*state; ++calls; });
	assert(calls == n);
}

void large(int n, int shape) {
	DSUOnTree tree(n);
	for (int v = 1; v < n; ++v) tree.add_edge(v, shape == 0 ? v - 1 : shape == 1 ? 0 : (v - 1) / 2);
	tree.build();
	int count = 0, answers = 0;
	std::int64_t additions = 0, removals = 0;
	tree.run([&](int) { ++count; ++additions; }, [&](int) { --count; ++removals; },
		[&](int v) { assert(count == tree.subtree_size(v)); ++answers; });
	assert(count == 0 && answers == n && additions == removals);
	if (shape == 0) assert(additions == n);
	if (shape == 1) assert(additions == 2 * n - 2);
}

int main() {
	verify(0, {}, 0);
	verify(1, {}, 0);
	std::mt19937 random(20261021);
	for (int trial = 0; trial < 600; ++trial) {
		int n = 2 + int(random() % 79);
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), random);
		std::vector<std::pair<int, int>> edges;
		for (int v = 1; v < n; ++v) {
			int p = trial % 5 == 0 ? 0 : trial % 5 == 1 ? v - 1 : int(random() % v);
			edges.emplace_back(labels[v], labels[p]);
		}
		std::shuffle(edges.begin(), edges.end(), random);
		verify(n, edges, int(random() % n));
	}
	for (int shape = 0; shape < 3; ++shape) large(200000, shape);
}
