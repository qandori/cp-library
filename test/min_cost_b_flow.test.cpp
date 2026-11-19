#define PROBLEM "https://judge.yosupo.jp/problem/min_cost_b_flow"
#include "flow/min_cost_b_flow.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	MinCostBFlow<> graph(n);
	graph.reserve(m);
	for (int v = 0; v < n; ++v) {
		std::int64_t supply = 0;
		input.read(supply);
		graph.set_supply(v, supply);
	}
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t lower = 0, upper = 0, cost = 0;
		input.read(u), input.read(v), input.read(lower), input.read(upper), input.read(cost);
		graph.add_edge(u, v, lower, upper, cost);
	}
	auto result = graph.solve();
	if (!result) { output.println("infeasible"); return 0; }
	output.println(result->cost);
	for (auto potential : result->potential) output.println(potential);
	for (auto flow : result->flow) output.println(flow);
}
