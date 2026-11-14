#include "graph/max_flow_lower_bound.hpp"
#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <set>

struct InputEdge {
	int from, to, lower, upper;
};

std::set<int> enumerate(int n, const std::vector<InputEdge>& edges,
	const std::vector<int>& supply, int s = -1, int t = -1) {
	std::vector<int> balance(n);
	std::set<int> result;
	std::function<void(int)> visit = [&](int index) {
		if (index == int(edges.size())) {
			int amount = s < 0 ? 0 : balance[s] - supply[s];
			for (int v = 0; v < n; ++v) {
				int expected = supply[v];
				if (v == s) expected += amount;
				if (v == t) expected -= amount;
				if (balance[v] != expected) return;
			}
			result.insert(amount);
			return;
		}
		auto edge = edges[index];
		for (int flow = edge.lower; flow <= edge.upper; ++flow) {
			balance[edge.from] += flow, balance[edge.to] -= flow;
			visit(index + 1);
			balance[edge.from] -= flow, balance[edge.to] += flow;
		}
	};
	visit(0);
	return result;
}

template<class Graph> void validate(const Graph& graph, const typename Graph::Result& result,
	int s = -1, int t = -1) {
	assert(result.flow.size() == std::size_t(graph.edge_count()));
	if (s < 0) assert(result.value == 0);
	std::vector<__int128_t> balance(graph.size());
	for (int i = 0; i < graph.edge_count(); ++i) {
		auto edge = graph.get_edge(i);
		auto flow = result.flow[i];
		assert(edge.lower <= flow && flow <= edge.upper);
		balance[edge.from] += flow, balance[edge.to] -= flow;
	}
	for (int v = 0; v < graph.size(); ++v) {
		__int128_t expected = graph.supply(v);
		if (v == s) expected += result.value;
		if (v == t) expected -= result.value;
		assert(balance[v] == expected);
	}
}

template<class Graph> void check_interval(const Graph& graph, const std::set<int>& expected,
	int s, int t, int lower, int upper) {
	using Cap = typename Graph::capacity_type;
	auto first = expected.lower_bound(lower), last = expected.upper_bound(upper);
	bool feasible = first != last;
	auto range = graph.flow_range(s, t, Cap(lower), Cap(upper));
	auto any = graph.flow(s, t, Cap(lower), Cap(upper));
	auto minimum = graph.min_flow(s, t, Cap(lower), Cap(upper));
	auto maximum = graph.max_flow(s, t, Cap(lower), Cap(upper));
	assert(bool(range) == feasible && bool(any) == feasible);
	assert(bool(minimum) == feasible && bool(maximum) == feasible);
	if (feasible) {
		int end = *std::prev(last);
		assert(range->first == *first && range->second == end);
		for (int f = *first; f <= end; ++f) assert(expected.contains(f));
		assert(lower <= any->value && any->value <= upper && expected.contains(any->value));
		assert(minimum->value == *first && maximum->value == end);
		validate(graph, *any, s, t);
		validate(graph, *minimum, s, t);
		validate(graph, *maximum, s, t);
	}
}

template<class Cap> void random_check(int trials) {
	using Graph = MaxFlowLowerBound<Cap>;
	std::mt19937_64 rng(105803);
	for (int trial = 0; trial < trials; ++trial) {
		int n = 1 + rng() % 5, m = rng() % 8;
		Graph graph(n);
		graph.reserve(m);
		std::vector<InputEdge> edges;
		std::vector<int> supply(n);
		for (int i = 0; i < m; ++i) {
			int from = rng() % n, to = rng() % n;
			int lower = int(rng() % 5) - 2, upper = lower + rng() % 3;
			if (lower == 0 && trial % 2) assert(graph.add_edge(from, to, Cap(upper)) == i);
			else assert(graph.add_edge(from, to, Cap(lower), Cap(upper)) == i);
			edges.push_back({from, to, lower, upper});
			int flow = lower + rng() % (upper - lower + 1);
			supply[from] += flow, supply[to] -= flow;
		}
		if (trial % 3) {
			for (auto& value : supply) value = int(rng() % 9) - 4;
			if (trial % 3 == 1) supply.back() -= std::accumulate(supply.begin(), supply.end(), 0);
		}
		for (int v = 0; v < n; ++v) {
			graph.set_supply(v, Cap(supply[v]));
			assert(graph.supply(v) == supply[v] && graph.supplies()[v] == supply[v]);
		}
		assert(graph.size() == n && graph.edge_count() == m);
		auto listed = graph.edges();
		assert(int(listed.size()) == m);
		for (int i = 0; i < m; ++i) {
			assert(listed[i].from == edges[i].from && listed[i].to == edges[i].to);
			assert(listed[i].lower == edges[i].lower && listed[i].upper == edges[i].upper);
		}
		const Graph& input = graph;
		auto expected = enumerate(n, edges, supply);
		auto actual = input.solve();
		assert(bool(actual) == !expected.empty());
		if (actual) validate(input, *actual);
		if (n < 2) continue;
		int s = rng() % n, t = (s + 1 + rng() % (n - 1)) % n;
		expected = enumerate(n, edges, supply, s, t);
		for (int f : std::array<int, 6>{-6, -1, 0, 1, 6, int(rng() % 13) - 6}) {
			auto fixed = input.flow(s, t, Cap(f));
			assert(bool(fixed) == expected.contains(f));
			if (fixed) {
				assert(fixed->value == f);
				validate(input, *fixed, s, t);
			}
		}
		for (auto [lower, upper] : std::array<std::pair<int, int>, 5>{
			{{-6, 6}, {-6, -1}, {0, 0}, {0, 6}, {-3, 2}}})
			check_interval(input, expected, s, t, lower, upper);
		int lower = int(rng() % 11) - 5, upper = lower + rng() % 7;
		check_interval(input, expected, s, t, lower, upper);
		auto first = expected.lower_bound(0);
		auto minimum = input.min_flow(s, t), maximum = input.max_flow(s, t);
		auto range = input.flow_range(s, t);
		assert(bool(minimum) == (first != expected.end()));
		assert(bool(maximum) == bool(minimum) && bool(range) == bool(minimum));
		if (minimum) {
			assert(minimum->value == *first && maximum->value == *expected.rbegin());
			assert(range->first == minimum->value && range->second == maximum->value);
			validate(input, *minimum, s, t);
			validate(input, *maximum, s, t);
		}
		auto repeated = input.solve();
		assert(bool(repeated) == bool(actual));
		if (repeated) {
			assert(repeated->flow == actual->flow && repeated->value == actual->value);
			validate(input, *repeated);
		}
	}
}

template<class Cap> void one_sided_check() {
	using Graph = MaxFlowLowerBound<Cap>;
	std::mt19937_64 rng(490613);
	for (int trial = 0; trial < 80; ++trial) {
		int lower = int(rng() % 5) - 2, upper = rng() % 5;
		std::vector<InputEdge> edges{{0, 1, lower, 6}, {1, 2, 0, upper}, {0, 2, -2, 3}};
		std::vector<int> supply{int(rng() % 5) - 2, int(rng() % 5) - 2, 0};
		supply[2] = -supply[0] - supply[1];
		Graph graph(3);
		assert(graph.add_edge_lower_bound(0, 1, Cap(lower)) == 0);
		assert(graph.add_edge(1, 2, Cap(upper)) == 1);
		graph.add_edge(0, 2, Cap(-2), Cap(3));
		assert(graph.get_edge(0).lower == lower);
		assert(graph.get_edge(0).upper == std::numeric_limits<Cap>::max());
		assert(graph.get_edge(1).lower == 0 && graph.get_edge(1).upper == upper);
		for (int v = 0; v < 3; ++v) graph.set_supply(v, Cap(supply[v]));
		const Graph& input = graph;
		auto expected = enumerate(3, edges, supply);
		auto actual = input.solve();
		assert(bool(actual) == !expected.empty());
		if (actual) validate(input, *actual);
		expected = enumerate(3, edges, supply, 0, 2);
		for (int f = -6; f <= 6; ++f) {
			auto fixed = input.flow(0, 2, Cap(f));
			assert(bool(fixed) == expected.contains(f));
			if (fixed) {
				assert(fixed->value == f);
				validate(input, *fixed, 0, 2);
			}
		}
		check_interval(input, expected, 0, 2, -6, 6);
	}
	constexpr Cap smallest = std::numeric_limits<Cap>::min();
	constexpr Cap largest = std::numeric_limits<Cap>::max();
	Graph span(2);
	span.add_edge_lower_bound(0, 1, smallest);
	auto range = span.flow_range(0, 1, smallest, largest);
	assert(range && range->first == smallest && range->second == largest);
	auto minimum = span.min_flow(0, 1, smallest, largest);
	auto maximum = span.max_flow(0, 1, smallest, largest);
	assert(minimum && minimum->value == smallest && minimum->flow[0] == smallest);
	assert(maximum && maximum->value == largest && maximum->flow[0] == largest);
	validate(span, *minimum, 0, 1);
	validate(span, *maximum, 0, 1);
	for (Cap value : {smallest, Cap(0), largest}) {
		auto fixed = span.flow(0, 1, value);
		assert(fixed && fixed->value == value && fixed->flow[0] == value);
		validate(span, *fixed, 0, 1);
	}
	span.set_supply(0, largest), span.set_supply(1, Cap(-largest));
	auto full_width = span.solve();
	assert(full_width && full_width->flow[0] == largest);
	validate(span, *full_width);
	span.reset_supply();
	span.change_edge(0, Cap(0), largest);
	auto finite = span.max_flow(0, 1);
	assert(finite && finite->value == largest);
	validate(span, *finite, 0, 1);
	span.add_edge(0, 1, largest);
	auto clipped = span.max_flow(0, 1);
	assert(clipped && clipped->value == largest);
	validate(span, *clipped, 0, 1);
	Graph reversed(2);
	reversed.add_edge(1, 0, largest);
	reversed.add_edge(1, 0, largest);
	auto negative = reversed.flow(0, 1, smallest);
	assert(negative && negative->value == smallest);
	validate(reversed, *negative, 0, 1);
	auto negative_range = reversed.flow_range(0, 1, smallest, largest);
	assert(negative_range && negative_range->first == smallest && negative_range->second == 0);
	Graph loops(1);
	loops.add_edge_lower_bound(0, 0, smallest);
	loops.add_edge(0, 0, largest);
	auto circulation = loops.solve();
	assert(circulation && circulation->value == 0);
	validate(loops, *circulation);
}

void boundary_check() {
	MaxFlowLowerBound<> empty;
	auto zero = empty.solve();
	assert(zero && zero->flow.empty() && zero->value == 0);
	MaxFlowLowerBound<> graph(3);
	graph.set_supply(0, 1);
	assert(!graph.solve());
	graph.add_supply(2, -1);
	assert(!graph.solve());
	assert(graph.add_edge(0, 1, 1, 4) == 0);
	assert(graph.add_edge(1, 2, 4) == 1);
	auto connected = graph.solve();
	assert(connected && connected->flow[0] == 1 && connected->flow[1] == 1);
	validate(graph, *connected);
	graph.reset_supply();
	assert(!graph.solve());
	auto fixed = graph.flow(0, 2, 3);
	assert(fixed && fixed->value == 3 && fixed->flow[0] == 3 && fixed->flow[1] == 3);
	validate(graph, *fixed, 0, 2);
	assert(graph.min_flow(0, 2)->value == 1);
	assert(graph.max_flow(0, 2)->value == 4);
	assert(!graph.flow(0, 2, 0));
	auto copy = graph;
	copy.change_edge(0, -4, -2);
	assert(!copy.max_flow(0, 2));
	copy.change_edge(1, -3, -1);
	auto backward = copy.min_flow(0, 2, -9, -1);
	assert(backward && backward->value == -3);
	validate(copy, *backward, 0, 2);
	assert(graph.max_flow(0, 2)->value == 4);
	assert(fixed->value == 3 && fixed->flow[0] == 3);
	copy.clear();
	assert(copy.size() == 3 && copy.edge_count() == 0);
	assert(copy.supplies() == std::vector<std::int64_t>(3));
	assert(copy.solve()->flow.empty());
	assert(copy.add_edge_lower_bound(2, 0, 2) == 0);
	assert(copy.max_flow(2, 0)->value == std::numeric_limits<std::int64_t>::max());
	MaxFlowLowerBound<> independent(4);
	independent.add_edge(0, 1, 2);
	independent.add_edge(2, 3, 1);
	independent.set_supply(2, 1), independent.set_supply(3, -1);
	auto maximum = independent.max_flow(0, 1);
	assert(maximum && maximum->value == 2 && maximum->flow[1] == 1);
	validate(independent, *maximum, 0, 1);
}

int main() {
	random_check<std::int8_t>(200);
	random_check<std::int16_t>(200);
	random_check<std::int32_t>(400);
	random_check<std::int64_t>(400);
	one_sided_check<std::int8_t>();
	one_sided_check<std::int16_t>();
	one_sided_check<std::int32_t>();
	one_sided_check<std::int64_t>();
	boundary_check();
}
