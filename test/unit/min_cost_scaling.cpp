#include "graph/min_cost_flow.hpp"
#include <random>

int main() {
	std::mt19937_64 rng(36683669);
	for (int mode = 0; mode < 5; ++mode) for (int trial = 0; trial < 1000; ++trial) {
		int n = 2 + rng() % 15, m = rng() % 120;
		MinCostFlow<> oracle(n);
		flow_detail::Graph<std::int64_t> graph(n);
		std::vector<std::int64_t> costs, potential(n);
		for (auto& p : potential) p = int(rng() % 2001) - 1000;
		for (int i = 0; i < m; ++i) {
			int from = rng() % n, to = rng() % n;
			std::int64_t cap = rng() % 1000000000;
			std::int64_t cost = mode == 0 ? 0 : mode == 1 ? rng() % 4 : mode == 2 ? (rng() % 10 ? 0 : rng() % 1000000000) : rng() % 1000000000;
			if (mode == 4) cost += potential[to] - potential[from];
			oracle.add_edge(from, to, cap, cost);
			graph.add_edge(from, to, cap);
			costs.push_back(cost), costs.push_back(-cost);
		}
		graph.build();
		std::int64_t limit = rng() % 10000000000LL;
		auto expected = oracle.slope(0, n - 1, limit).back();
		auto actual = flow_detail::scaling_flow<std::int64_t, std::int64_t, __int128_t>(graph, costs, 0, n - 1, limit);
		assert(actual == expected);
		std::vector<std::int64_t> balance(n);
		auto edges = oracle.edges();
		__int128_t total = 0;
		for (int i = 0; i < m; ++i) {
			auto e = edges[i];
			auto flow = graph.arcs[2 * i + 1].cap;
			assert(0 <= flow && flow <= e.cap);
			balance[e.from] += flow, balance[e.to] -= flow;
			total += __int128_t(flow) * e.cost;
		}
		assert(total == actual.second);
		assert(balance[0] == actual.first && balance[n - 1] == -actual.first);
		for (int v = 1; v < n - 1; ++v) assert(balance[v] == 0);
	}
}
