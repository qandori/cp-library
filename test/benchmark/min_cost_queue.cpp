#define flow_detail baseline_flow_detail
#define MinCostFlow BaselineMinCostFlow
#define RadixHeap BaselineRadixHeap
#define DaryHeap BaselineDaryHeap
#include BASELINE_MIN_COST_FLOW_HEADER
#undef flow_detail
#undef MinCostFlow
#undef RadixHeap
#undef DaryHeap
#undef FLOW_GRAPH_HPP
#undef MIN_COST_CIRCULATION_HPP
#undef MIN_COST_FLOW_HPP
#undef RADIX_HEAP_HPP
#undef D_ARY_HEAP_HPP
#include "graph/min_cost_flow.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <cstdlib>

struct Sample {
	unsigned long long hash;
	double wall, cpu;
	std::pair<long long, __int128_t> result;
};

template<bool Baseline> Sample run_min(int mode, int repeats, bool public_api) {
	std::mt19937_64 rng(36683669);
	int n = mode == 5 ? 2 : 100, m = 1000;
	using Graph = std::conditional_t<Baseline, baseline_flow_detail::Graph<long long>, flow_detail::Graph<long long>>;
	using Public = std::conditional_t<Baseline, BaselineMinCostFlow<long long, long long>, MinCostFlow<long long, long long>>;
	Graph base(n);
	std::vector<long long> costs;
	for (int i = 0; i < m; ++i) {
		int from = rng() % n, to = rng() % n;
		long long cap = rng() % 1000000000 + 1, cost = rng() % 1000000000;
		if (mode == 1 && rng() % 10) cost = 0;
		if (mode == 2) cost = 0;
		if (mode == 3) cost %= 4;
		if (mode == 4) from = rng() % (n - 1), to = from + 1 + rng() % (n - 1 - from);
		if (mode == 5) from = 0, to = 1, cost = i;
		base.add_edge(from, to, cap);
		costs.push_back(cost), costs.push_back(-cost);
	}
	base.build();
	Public public_base(n);
	for (int i = 0; i < m; ++i) public_base.add_edge(base.arcs[2 * i + 1].to, base.arcs[2 * i].to, base.arcs[2 * i].cap, costs[2 * i]);
	unsigned long long hash = 0;
	auto start = std::chrono::steady_clock::now();
	auto cpu = std::clock();
	std::pair<long long, __int128_t> result;
	for (int rep = 0; rep < repeats; ++rep) {
		if (public_api) {
			auto graph = public_base;
			result = graph.flow(0, n - 1, 1000000000000LL);
		} else {
			auto graph = base;
			if constexpr (Baseline) result = baseline_flow_detail::scaling_flow<long long, long long, __int128_t>(graph, costs, 0, n - 1, 1000000000000LL);
			else result = flow_detail::scaling_flow<long long, long long, __int128_t>(graph, costs, 0, n - 1, 1000000000000LL);
		}
		hash += static_cast<unsigned long long>(result.first) + static_cast<unsigned long long>(result.second);
	}
	auto end = std::chrono::steady_clock::now();
	return {hash, std::chrono::duration<double, std::milli>(end - start).count(), 1000.0 * (std::clock() - cpu) / CLOCKS_PER_SEC, result};
}

int main(int argc, char** argv) {
	if (argc != 5) return 1;
	int mode = std::atoi(argv[1]), api = std::atoi(argv[2]), trials = std::atoi(argv[3]), repeats = 1;
	double minimum = std::atof(argv[4]);
	auto run = [&](bool baseline) { return baseline ? run_min<true>(mode, repeats, api) : run_min<false>(mode, repeats, api); };
	auto baseline = run(true), candidate = run(false);
	if (baseline.result != candidate.result || baseline.hash != candidate.hash) return 2;
	int chunk[2] = {std::max(1, int(25 / std::max(baseline.cpu, .01))), std::max(1, int(25 / std::max(candidate.cpu, .01)))};
	std::cout << std::setprecision(17);
	for (int trial = 0; trial < trials; ++trial) for (int j = 0; j < 2; ++j) {
		int variant = (j + trial + mode + api) % 2;
		repeats = chunk[variant];
		double wall = 0, cpu = 0;
		unsigned long long total = 0;
		do {
			auto sample = run(variant == 0);
			if (sample.result != baseline.result || sample.hash != baseline.hash * repeats) return 3;
			wall += sample.wall, cpu += sample.cpu, total += repeats;
		} while (cpu < minimum);
		std::cout << trial << ' ' << variant << ' ' << total << ' ' << baseline.hash << ' ' << wall << ' ' << cpu << '\n';
	}
}
