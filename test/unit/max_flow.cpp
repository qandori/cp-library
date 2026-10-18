#include "graph/max_flow.hpp"
#include <random>

template<class Cap> void check() {
	std::mt19937_64 rng(3668);
	for (int trial = 0; trial < 1500; ++trial) {
		int n = 2 + rng() % 7, m = rng() % 40;
		MaxFlow<Cap> graph(n);
		graph.reserve(m);
		for (int i = 0; i < m; ++i) {
			int u = rng() % n, v = rng() % n;
			Cap c = rng() % 100;
			assert(graph.add_edge(u, v, c) == i);
		}
		auto edges = graph.edges();
		Cap expected = std::numeric_limits<Cap>::max();
		for (int mask = 1; mask < (1 << n); mask += 2) {
			if (mask >> (n - 1) & 1) continue;
			Cap cut = 0;
			for (auto e : edges) if ((mask >> e.from & 1) && !(mask >> e.to & 1)) cut += e.cap;
			expected = std::min(expected, cut);
		}
		Cap limit = rng() % 150;
		assert(graph.flow(0, n - 1, limit) == std::min(expected, limit));
		assert(graph.flow(0, n - 1) == expected - std::min(expected, limit));
		assert(graph.flow(0, n - 1) == 0);
		auto cut = graph.min_cut(0);
		assert(cut[0] && !cut[n - 1]);
		Cap cut_cap = 0;
		std::vector<long long> balance(n);
		for (auto e : graph.edges()) {
			assert(e.flow <= e.cap);
			balance[e.from] += e.flow, balance[e.to] -= e.flow;
			if (cut[e.from] && !cut[e.to]) cut_cap += e.cap;
		}
		assert(cut_cap == expected && balance[0] == static_cast<long long>(expected));
		assert(balance[n - 1] == -static_cast<long long>(expected));
		for (int v = 1; v < n - 1; ++v) assert(balance[v] == 0);
		graph.reset_flow();
		for (auto e : graph.edges()) assert(e.flow == 0);
		assert(graph.flow(0, n - 1) == expected);
		graph.add_edge(0, n - 1, 7);
		assert(graph.flow(0, n - 1) == 7);
		graph.reset_flow();
		graph.change_edge(graph.edge_count() - 1, 11, 0);
		assert(graph.flow(0, n - 1) == expected + 11);
	}
}

template<class Cap> void changed_flow() {
	MaxFlow<Cap> graph(4);
	graph.reserve(7);
	graph.add_edge(0, 1, 12);
	graph.add_edge(1, 3, 12);
	graph.add_edge(0, 2, 9);
	graph.add_edge(2, 3, 9);
	graph.add_edge(1, 2, 7);
	graph.add_edge(1, 1, 5);
	graph.add_edge(0, 1, 3);
	graph.change_edge(0, 12, 5);
	graph.change_edge(1, 12, 5);
	assert(graph.size() == 4 && graph.edge_count() == 7);
	assert(graph.get_edge(0).cap == 12 && graph.get_edge(0).flow == 5);
	assert(graph.get_edge(1).cap == 12 && graph.get_edge(1).flow == 5);
	assert(graph.flow(0, 3, 0) == 0);
	assert(graph.flow(0, 3, 4) == 4);
	assert(graph.flow(0, 3) == 12);
	assert(graph.flow(0, 3) == 0);
	std::vector<int> balance(4);
	for (auto e : graph.edges()) {
		assert(e.flow <= e.cap);
		balance[e.from] += int(e.flow), balance[e.to] -= int(e.flow);
	}
	assert((balance == std::vector<int>{21, 0, 0, -21}));
	auto cut = graph.min_cut(0);
	int capacity = 0;
	for (auto e : graph.edges()) if (cut[e.from] && !cut[e.to]) capacity += e.cap;
	assert(cut[0] && !cut[3] && capacity == 21);
	graph.reset_flow();
	for (auto e : graph.edges()) assert(e.flow == 0);
	assert(graph.flow(0, 3) == 21);
	graph.add_edge(0, 3, 3);
	assert(graph.flow(0, 3) == 3);
	graph.reset_flow();
	graph.change_edge(7, 8, 0);
	assert(graph.flow(0, 3) == 29);
}

int main() {
	changed_flow<signed char>();
	changed_flow<unsigned char>();
	changed_flow<short>();
	changed_flow<unsigned short>();
	changed_flow<int>();
	changed_flow<unsigned int>();
	changed_flow<std::int64_t>();
	changed_flow<std::uint64_t>();
	check<int>();
	check<std::int64_t>();
	check<std::uint64_t>();
	MaxFlow<> chain(300000);
	for (int i = 1; i < chain.size(); ++i) chain.add_edge(i - 1, i, 123);
	assert(chain.flow(0, chain.size() - 1) == 123);
	MaxFlow<> huge(3);
	auto cap = std::numeric_limits<std::int64_t>::max();
	huge.add_edge(0, 1, cap), huge.add_edge(1, 2, cap);
	assert(huge.flow(0, 2) == cap);
	MaxFlow<> overflow(3);
	overflow.add_edge(0, 1, cap), overflow.add_edge(0, 1, cap), overflow.add_edge(1, 2, 7);
	assert(overflow.flow(0, 2) == 7);
	MaxFlow<> empty;
	assert(empty.size() == 0 && empty.edge_count() == 0 && empty.edges().empty());
}
