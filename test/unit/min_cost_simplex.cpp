#include "flow/min_cost_flow.hpp"
#include <random>

template<class Cap, class Cost> void check() {
	std::mt19937_64 rng(8896303);
	for (int mode = 0; mode < 5; ++mode) for (int trial = 0; trial < 400; ++trial) {
		int n = 2 + rng() % 14, m = rng() % 100;
		MinCostFlow<Cap, Cost> oracle(n);
		flow_detail::Graph<Cap> graph(n);
		std::vector<Cost> costs, potential(n);
		for (auto& p : potential) p = int(rng() % 2001) - 1000;
		for (int i = 0; i < m; ++i) {
			int from = rng() % n, to = rng() % n;
			Cap cap = rng() % 1000000;
			Cost cost = mode == 0 ? 0 : mode == 1 ? rng() % 4 : mode == 2 ? (rng() % 10 ? 0 : rng() % 1000000) : rng() % 1000000;
			if (mode == 4) cost += potential[to] - potential[from];
			oracle.add_edge(from, to, cap, cost);
			graph.add_edge(from, to, cap);
			costs.push_back(cost), costs.push_back(-cost);
		}
		graph.build();
		for (int part = 0; part < 2; ++part) {
			Cap limit = rng() % 10000000;
			auto expected = oracle.slope(0, n - 1, limit).back();
			auto previous = graph.arcs;
			auto result = flow_detail::simplex_flow<Cap, Cost, __int128_t>(graph, costs, 0, n - 1, limit);
			assert(result && *result == expected);
			std::vector<__int128_t> balance(n);
			__int128_t total = 0;
			for (int i = 0; i < m; ++i) {
				auto delta = __int128_t(graph.arcs[2 * i + 1].cap) - previous[2 * i + 1].cap;
				balance[graph.arcs[2 * i + 1].to] += delta;
				balance[graph.arcs[2 * i].to] -= delta;
				total += delta * costs[2 * i];
				assert(__int128_t(graph.arcs[2 * i].cap) + graph.arcs[2 * i + 1].cap == __int128_t(previous[2 * i].cap) + previous[2 * i + 1].cap);
			}
			assert(total == result->second);
			assert(balance[0] == result->first && balance[n - 1] == -__int128_t(result->first));
			for (int v = 1; v < n - 1; ++v) assert(balance[v] == 0);
		}
	}
}

void check_prices() {
	using Cap = std::uint64_t;
	using Cost = std::int64_t;
	using Total = __int128_t;
	const Cost maximum = std::numeric_limits<Cost>::max();
	const Cost boundary = (maximum / 5 - 1) / 3;
	const Cap capacity = std::numeric_limits<Cap>::max();
	for (Cost cost : {Cost(0), Cost(1), boundary - 1, boundary, boundary + 1, maximum / 2}) {
		flow_detail::Graph<Cap> graph(3);
		graph.add_edge(0, 1, capacity);
		graph.add_edge(1, 2, capacity);
		graph.build();
		std::vector<Cost> costs{cost, -cost, cost, -cost};
		auto reference = graph, stopped = graph;
		auto expected = flow_detail::NetworkSimplex<Cap>(reference, costs, 0, 2, capacity, 1000000).template run<Cost, Total>(reference, costs);
		auto actual = flow_detail::simplex_flow<Cap, Cost, Total>(graph, costs, 0, 2, capacity);
		assert(actual && actual == expected && actual->first == capacity && actual->second == Total(capacity) * cost * 2);
		for (std::size_t i = 0; i < graph.arcs.size(); ++i) assert(graph.arcs[i].cap == reference.arcs[i].cap);
		auto before = stopped.arcs;
		auto failure = flow_detail::simplex_flow<Cap, Cost, Total>(stopped, costs, 0, 2, capacity, 1);
		assert(!failure);
		for (std::size_t i = 0; i < before.size(); ++i) assert(stopped.arcs[i].cap == before[i].cap);
	}
	const Cost diamond_boundary = (maximum / 5 - 1) / 4;
	for (Cost cost : {diamond_boundary - 1, diamond_boundary, diamond_boundary + 1}) {
		flow_detail::Graph<Cap> graph(4);
		graph.add_edge(0, 1, 1);
		graph.add_edge(1, 3, 1);
		graph.add_edge(0, 2, 1);
		graph.add_edge(2, 3, 1);
		graph.add_edge(1, 2, 1);
		graph.build();
		std::vector<Cost> costs{cost, -cost, cost, -cost, cost, -cost, cost, -cost, -cost, cost};
		auto actual = flow_detail::simplex_flow<Cap, Cost, Total>(graph, costs, 0, 3, 10);
		assert(actual && actual->first == 2 && actual->second == Total(cost) * 4);
	}
}

void check_budget_bound() {
	using Cap = std::uint64_t;
	using Cost = std::int64_t;
	for (int negative : {0, 1, 10}) {
		flow_detail::Graph<Cap> graph(64);
		std::vector<Cost> costs;
		for (int v = 0; v < 63; ++v) {
			graph.add_edge(v, v + 1, Cap(1));
			Cost cost = v < negative ? -1 : 1;
			costs.push_back(cost), costs.push_back(-cost);
		}
		graph.build();
		for (std::uint64_t budget : {1ULL, 2ULL, 100ULL}) for (Cap limit : {Cap(0), Cap(1)}) {
			auto copy = graph;
			auto result = flow_detail::simplex_flow<Cap, Cost, __int128_t>(copy, costs, 0, 63, limit, budget);
			if (limit) assert(!result);
			else assert(result && result->first == 0 && result->second == 0);
			for (std::size_t i = 0; i < graph.arcs.size(); ++i) assert(copy.arcs[i].cap == graph.arcs[i].cap);
		}
		auto result = flow_detail::simplex_flow<Cap, Cost, __int128_t>(graph, costs, 0, 63, Cap(1), 1000000);
		assert(result && result->first == 1 && result->second == 63 - 2 * negative);
		auto exhausted = flow_detail::simplex_flow<Cap, Cost, __int128_t>(graph, costs, 0, 63, Cap(1), 1);
		assert(exhausted && exhausted->first == 0 && exhausted->second == 0);
	}
}

int main() {
	check_budget_bound();
	check_prices();
	check<int, int>();
	check<std::int64_t, std::int64_t>();
	check<std::uint64_t, std::int64_t>();
	flow_detail::Graph<std::int64_t> graph(3);
	graph.add_edge(0, 1, 10);
	graph.add_edge(1, 2, 10);
	graph.build();
	std::vector<std::int64_t> costs{3, -3, -2, 2};
	auto previous = graph.arcs;
	auto stopped = flow_detail::simplex_flow<std::int64_t, std::int64_t, __int128_t>(graph, costs, 0, 2, 10, 1);
	assert(!stopped);
	for (std::size_t i = 0; i < previous.size(); ++i) assert(graph.arcs[i].cap == previous[i].cap);
	auto resumed = flow_detail::scaling_flow<std::int64_t, std::int64_t, __int128_t>(graph, costs, 0, 2, 10);
	assert(resumed.first == 10 && resumed.second == 10);

	flow_detail::Graph<std::uint64_t> wide(2);
	auto capacity = std::numeric_limits<std::uint64_t>::max();
	auto cost = std::numeric_limits<std::int64_t>::max() / 16;
	wide.add_edge(0, 1, capacity);
	wide.build();
	std::vector<std::int64_t> wide_cost{cost, -cost};
	auto result = flow_detail::simplex_flow<std::uint64_t, std::int64_t, __int128_t>(wide, wide_cost, 0, 1, capacity);
	assert(result && result->first == capacity && result->second == __int128_t(capacity) * cost);

	flow_detail::Graph<std::uint64_t> narrow(2);
	narrow.add_edge(0, 1, capacity);
	narrow.build();
	narrow.arcs[0].cap -= 10;
	narrow.arcs[1].cap = 10;
	std::vector<std::int64_t> zero_cost{0, 0};
	auto delta = std::uint64_t(1) << 63;
	auto saved = narrow;
	auto crossing = flow_detail::simplex_flow<std::uint64_t, std::int64_t, std::int64_t>(narrow, zero_cost, 0, 1, delta);
	assert(crossing && crossing->first == delta && crossing->second == 0);
	auto scaled = flow_detail::scaling_flow<std::uint64_t, std::int64_t, std::int64_t>(saved, zero_cost, 0, 1, delta);
	assert(scaled.first == delta && scaled.second == 0);

	MinCostFlow<std::uint64_t, std::int64_t, std::int64_t> incremental(3);
	incremental.add_edge(1, 2, capacity, 0);
	auto initial = incremental.flow(1, 2, std::numeric_limits<std::int64_t>::max());
	assert(initial.first == std::uint64_t(std::numeric_limits<std::int64_t>::max()) && initial.second == 0);
	for (int i = 0; i < 8; ++i) incremental.add_edge(0, 2, 1, 0);
	incremental.add_edge(0, 1, 10, 0);
	auto extra = incremental.flow(0, 2, 10);
	assert(extra.first == 10 && extra.second == 0);

	MinCostFlow<> paths(24);
	for (int i = 1; i < 23; ++i) {
		paths.add_edge(0, i, 1, i);
		paths.add_edge(i, 23, 1, 0);
	}
	auto all = paths.flow(0, 23);
	assert(all.first == 22 && all.second == 253);
	const auto& potential = paths.potentials();
	for (auto e : paths.edges()) {
		auto reduced = e.cost + potential[e.from] - potential[e.to];
		if (e.flow < e.cap) assert(reduced >= 0);
		if (e.flow) assert(reduced <= 0);
	}
	paths.reset_flow();
	auto first = paths.flow(0, 23, 12);
	auto second = paths.flow(0, 23);
	assert(first.first + second.first == all.first && first.second + second.second == all.second);
}
