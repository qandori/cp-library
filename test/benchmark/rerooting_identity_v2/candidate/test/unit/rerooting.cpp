#include "tree/rerooting.hpp"
#include <cstdlib>
#include <memory>
#include <numeric>
#include <random>
#include <string>

void require(bool ok) {
	if (!ok) std::abort();
}

using Pair = std::pair<long long, int>;

struct Vertex {
	long long sum = 0;
	int count = 0;
};

struct NoDefault {
	int value;
	explicit NoDefault(int value) : value(value) {}
};

int main() {
	std::mt19937 rng(973401);
	Rerooting<int> empty;
	require(empty.empty() && empty.size() == 0 && empty.edge_count() == 0);
	empty.build(0, std::plus<int>{}, [](int x, int) { return x + 1; }, [](int x, int, int, int) { return x; });
	require(empty.values().empty());
	for (int parent = 0; parent < 3; ++parent) {
		Rerooting<std::string> tree(4);
		std::vector<int> neighbors{2, 3};
		neighbors.insert(neighbors.begin() + parent, 0);
		for (int v : neighbors) tree.add_edge(1, v);
		int merges = 0, vertices = 0, lifts = 0;
		tree.build(std::string{}, [&](const auto &x, const auto &y) { ++merges; return x + y; },
			[&](const auto &x, int v) { ++vertices; return '(' + std::to_string(v) + x + ')'; },
			[&](const auto &x, int, int, int) { ++lifts; return x; });
		require(tree[0] == "(0(1(2)(3)))" && tree.edge_dp(parent) == "(1(2)(3))");
		require(merges == (parent == 1 ? 4 : 5) && vertices == 10 && lifts == 6);
	}
	for (int n = 1; n <= 35; ++n) for (int trial = 0; trial < 12; ++trial) {
		Rerooting<Vertex, Pair> tree(n);
		std::vector<std::pair<int, int>> edges;
		for (int v = 1; v < n; ++v) if (trial % 3 || rng() % 3) {
			int p = rng() % v;
			if (rng() & 1) edges.emplace_back(p, v);
			else edges.emplace_back(v, p);
		}
		std::shuffle(edges.begin(), edges.end(), rng);
		std::vector<std::vector<std::pair<int, int>>> graph(n);
		std::vector<Pair> weights(edges.size());
		std::vector<long long> a(n);
		for (auto &x : a) x = int(rng() % 101) - 50;
		for (int id = 0; id < static_cast<int>(edges.size()); ++id) {
			auto [u, v] = edges[id];
			require(tree.add_edge(u, v) == id && tree.edge(id) == edges[id]);
			graph[u].emplace_back(v, id), graph[v].emplace_back(u, id);
			weights[id] = {int(rng() % 21) - 10, int(rng() % 21) - 10};
		}
		auto merge = [](Pair x, Pair y) { return Pair{x.first + y.first, x.second + y.second}; };
		auto vertex = [&](Pair x, int v) { return Vertex{x.first + a[v], x.second + 1}; };
		auto lift = [&](Vertex x, int from, int, int id) {
			long long w = from == edges[id].first ? weights[id].first : weights[id].second;
			return Pair{x.sum + w * x.count, x.count};
		};
		auto naive = [&](auto&& self, int v, int banned) -> Vertex {
			Pair x{0, 0};
			for (auto [to, id] : graph[v]) if (id != banned) x = merge(x, lift(self(self, to, id), to, v, id));
			return vertex(x, v);
		};
		for (int root : {0, n - 1, int(rng() % n)}) {
			const auto &result = tree.build(Pair{0, 0}, merge, vertex, lift, root);
			require(&result == &tree.values() && static_cast<int>(result.size()) == n);
			for (int v = 0; v < n; ++v) {
				auto expected = naive(naive, v, -1);
				require(tree[v].sum == expected.sum && tree[v].count == expected.count);
			}
			for (int id = 0; id < tree.edge_count(); ++id) {
				auto [u, v] = tree.edge(id);
				require(tree.edge_dp(id) == lift(naive(naive, u, id), u, v, id));
				require(tree.edge_dp(id, true) == lift(naive(naive, v, id), v, u, id));
			}
		}
		for (auto &x : a) x += 31;
		tree.build(Pair{0, 0}, merge, vertex, lift);
		for (int v = 0; v < n; ++v) require(tree[v].sum == naive(naive, v, -1).sum);
		if (n <= 9) {
			Rerooting<std::string> strings(n);
			for (auto [u, v] : edges) strings.add_edge(u, v);
			auto combine = [](const auto &x, const auto &y) { return x + y; };
			auto add = [](const auto &x, int v) { return '(' + std::to_string(v) + ':' + x + ')'; };
			auto pass = [](const auto &x, int from, int to, int id) {
				return '[' + std::to_string(from) + '>' + std::to_string(to) + ':' + std::to_string(id) + x + ']';
			};
			auto expected = [&](auto&& self, int v, int banned) -> std::string {
				std::string x;
				for (auto [to, id] : graph[v]) if (id != banned) x += pass(self(self, to, id), to, v, id);
				return add(x, v);
			};
			for (int root = 0; root < n; ++root) {
				strings.build(std::string{}, combine, add, pass, root);
				for (int v = 0; v < n; ++v) require(strings[v] == expected(expected, v, -1));
				for (int id = 0; id < strings.edge_count(); ++id) {
					auto [u, v] = strings.edge(id);
					require(strings.edge_dp(id) == pass(expected(expected, u, id), u, v, id));
					require(strings.edge_dp(id, true) == pass(expected(expected, v, id), v, u, id));
				}
			}
		}
	}
	for (int n = 1; n <= 9; ++n) for (int trial = 0; trial < 8; ++trial) {
		Rerooting<unsigned long long> tree(n);
		std::vector<unsigned> adjacent(n);
		for (int v = 1; v < n; ++v) {
			int p = rng() % v;
			tree.add_edge(p, v);
			adjacent[p] |= 1U << v, adjacent[v] |= 1U << p;
		}
		std::vector<int> counts(n);
		for (unsigned mask = 1; mask < (1U << n); ++mask) {
			unsigned reached = mask & -mask, last = 0;
			while (last != reached) {
				last = reached;
				for (int v = 0; v < n; ++v) if ((reached >> v) & 1) reached |= adjacent[v] & mask;
			}
			if (reached == mask) for (int v = 0; v < n; ++v) counts[v] += (mask >> v) & 1;
		}
		for (unsigned long long mod : {1, 2, 6, 12, 1000000007}) {
			tree.build(1 % mod, [=](auto x, auto y) { return x * y % mod; },
				[](auto x, int) { return x; }, [=](auto x, int, int, int) { return (x + 1) % mod; });
			for (int v = 0; v < n; ++v) require(tree[v] == counts[v] % mod);
		}
	}
	{
		Rerooting<NoDefault> tree(5);
		tree.add_edge(2, 0), tree.add_edge(3, 1);
		int vertex_calls = 0, edge_calls = 0;
		auto vertex = [counter = std::make_unique<int>(0), &vertex_calls](NoDefault x, int) {
			++*counter, ++vertex_calls;
			return NoDefault(x.value + 1);
		};
		auto lift = [&](NoDefault x, int, int, int) { ++edge_calls; return x; };
		auto merge = [](NoDefault x, NoDefault y) { return NoDefault(x.value + y.value); };
		tree.build(NoDefault(0), merge, std::move(vertex), lift, 4);
		require(vertex_calls == 9 && edge_calls == 4 && tree[4].value == 1 && tree[0].value == 2);
		tree.add_edge(0, 4);
		tree.build(NoDefault(0), merge, vertex, lift, 2);
		require(tree[0].value == 3 && tree[2].value == 3 && tree[1].value == 2);
		auto copy = tree;
		require(copy[4].value == 3 && copy.edge_dp(2).value == 2);
	}
	for (bool star : {false, true}) {
		const int n = 300000;
		Rerooting<Pair> tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(star ? 0 : v - 1, v);
		tree.build(Pair{0, 0}, [](Pair x, Pair y) { return Pair{x.first + y.first, x.second + y.second}; },
			[](Pair x, int) { return Pair{x.first, x.second + 1}; },
			[](Pair x, int, int, int) { return Pair{x.first + x.second, x.second}; }, n / 2);
		for (int v = 0; v < n; ++v) {
			long long expected = star ? (v ? 2LL * n - 3 : n - 1) :
				1LL * v * (v + 1) / 2 + 1LL * (n - 1 - v) * (n - v) / 2;
			require(tree[v].first == expected && tree[v].second == n);
		}
	}
}
