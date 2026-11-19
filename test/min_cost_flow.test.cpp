#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_6_B"
#include "flow/min_cost_flow.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	std::int64_t limit = 0;
	input.read(n), input.read(m), input.read(limit);
	MinCostFlow<> graph(n);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t cap = 0, cost = 0;
		input.read(u), input.read(v), input.read(cap), input.read(cost);
		graph.add_edge(u, v, cap, cost);
	}
	auto [flow, cost] = graph.flow(0, n - 1, limit);
	output.println(flow == limit ? cost : -1);
}
