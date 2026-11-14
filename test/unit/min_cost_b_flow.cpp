#include "graph/min_cost_b_flow.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <map>
#include <numeric>
#include <random>

struct InputEdge {
	int from, to, lower, upper, cost;
};

std::map<int, long long> enumerate(int n, const std::vector<InputEdge>& edges,
	const std::vector<int>& supply, int s = -1, int t = -1) {
	std::vector<int> balance(n);
	std::map<int, long long> result;
	std::function<void(int, long long)> visit = [&](int index, long long cost) {
		if (index == int(edges.size())) {
			int amount = s < 0 ? 0 : balance[s] - supply[s];
			for (int v = 0; v < n; ++v) {
				int expected = supply[v];
				if (v == s) expected += amount;
				if (v == t) expected -= amount;
				if (balance[v] != expected) return;
			}
			auto [it, inserted] = result.emplace(amount, cost);
			if (!inserted && cost < it->second) it->second = cost;
			return;
		}
		auto edge = edges[index];
		for (int flow = edge.lower; flow <= edge.upper; ++flow) {
			balance[edge.from] += flow, balance[edge.to] -= flow;
			visit(index + 1, cost + static_cast<long long>(flow) * edge.cost);
			balance[edge.from] -= flow, balance[edge.to] += flow;
		}
	};
	visit(0, 0);
	return result;
}

template<class Graph> void validate(const Graph& graph, const typename Graph::Result& result,
	int s = -1, int t = -1) {
	assert(result.flow.size() == std::size_t(graph.edge_count()));
	assert(result.potential.size() == std::size_t(graph.size()));
	if (s < 0) assert(result.value == 0);
	std::vector<__int128_t> balance(graph.size());
	__int128_t total = 0;
	for (int i = 0; i < graph.edge_count(); ++i) {
		auto edge = graph.get_edge(i);
		auto flow = result.flow[i];
		assert(edge.lower <= flow && flow <= edge.upper);
		balance[edge.from] += flow, balance[edge.to] -= flow;
		total += __int128_t(flow) * edge.cost;
		auto reduced = __int128_t(edge.cost) + result.potential[edge.from] - result.potential[edge.to];
		if (flow < edge.upper) assert(reduced >= 0);
		if (flow > edge.lower) assert(reduced <= 0);
	}
	assert(total == result.cost);
	for (int v = 0; v < graph.size(); ++v) {
		__int128_t expected = graph.supply(v);
		if (v == s) expected += result.value;
		if (v == t) expected -= result.value;
		assert(balance[v] == expected);
	}
}

template<class Cap, class Cost> void random_check(int trials) {
	using Graph = MinCostBFlow<Cap, Cost>;
	std::mt19937_64 rng(941753);
	for (int trial = 0; trial < trials; ++trial) {
		int n = 1 + rng() % 5, m = rng() % 8;
		Graph graph(n);
		graph.reserve(m);
		std::vector<InputEdge> edges;
		std::vector<int> supply(n);
		for (int i = 0; i < m; ++i) {
			int from = rng() % n, to = rng() % n;
			int lower = int(rng() % 5) - 2, upper = lower + rng() % 3;
			int cost = int(rng() % 11) - 5;
			if (trial % 2 && lower == 0)
				assert(graph.add_edge(from, to, Cap(upper), Cost(cost)) == i);
			else assert(graph.add_edge(from, to, Cap(lower), Cap(upper), Cost(cost)) == i);
			edges.push_back({from, to, lower, upper, cost});
			int flow = lower + rng() % (upper - lower + 1);
			supply[from] += flow, supply[to] -= flow;
		}
		if (trial % 3) {
			for (auto& value : supply) value = int(rng() % 9) - 4;
			if (trial % 3 == 1) supply.back() -= std::accumulate(supply.begin(), supply.end(), 0);
		}
		for (int v = 0; v < n; ++v) {
			graph.set_supply(v, Cap(supply[v]));
			assert(graph.supply(v) == supply[v]);
			assert(graph.supplies()[v] == supply[v]);
		}
		assert(graph.size() == n && graph.edge_count() == m);
		auto listed = graph.edges();
		assert(int(listed.size()) == m);
		for (int i = 0; i < m; ++i) {
			assert(listed[i].from == edges[i].from && listed[i].to == edges[i].to);
			assert(listed[i].lower == edges[i].lower && listed[i].upper == edges[i].upper);
			assert(listed[i].cost == edges[i].cost);
		}
		const Graph& input = graph;
		auto expected = enumerate(n, edges, supply);
		auto actual = input.solve();
		assert(bool(actual) == !expected.empty());
		if (actual) {
			assert(actual->cost == expected.at(0));
			validate(input, *actual);
		}
		if (n < 2) continue;
		int s = rng() % n, t = (s + 1 + rng() % (n - 1)) % n;
		expected = enumerate(n, edges, supply, s, t);
		int amount = int(rng() % 13) - 6;
		auto fixed = input.flow(s, t, Cap(amount));
		assert(bool(fixed) == expected.contains(amount));
		if (fixed) {
			assert(fixed->value == amount && fixed->cost == expected.at(amount));
			validate(input, *fixed, s, t);
		}
		int lower = int(rng() % 11) - 5, upper = lower + rng() % 7;
		auto first = expected.lower_bound(lower), last = expected.upper_bound(upper);
		bool feasible = first != last;
		auto range = input.flow_range(s, t, Cap(lower), Cap(upper));
		auto optimum = input.flow(s, t, Cap(lower), Cap(upper));
		auto minimum = input.min_flow(s, t, Cap(lower), Cap(upper));
		auto maximum = input.max_flow(s, t, Cap(lower), Cap(upper));
		assert(bool(range) == feasible && bool(optimum) == feasible);
		assert(bool(minimum) == feasible && bool(maximum) == feasible);
		if (feasible) {
			auto end = std::prev(last);
			long long best = first->second;
			for (auto it = first; it != last; ++it) best = std::min(best, it->second);
			assert(range->first == first->first && range->second == end->first);
			for (int f = first->first; f <= end->first; ++f) assert(expected.contains(f));
			assert(lower <= optimum->value && optimum->value <= upper && optimum->cost == best);
			assert(optimum->cost == expected.at(optimum->value));
			assert(minimum->value == first->first && minimum->cost == first->second);
			assert(maximum->value == end->first && maximum->cost == end->second);
			validate(input, *optimum, s, t);
			validate(input, *minimum, s, t);
			validate(input, *maximum, s, t);
		}
		auto repeated = input.solve();
		assert(bool(repeated) == bool(actual));
		if (repeated) assert(repeated->cost == actual->cost && repeated->flow == actual->flow);
	}
}

template<class Cap> void one_sided_check() {
	using Graph = MinCostBFlow<Cap>;
	std::mt19937_64 rng(480591);
	for (int trial = 0; trial < 80; ++trial) {
		int lower = int(rng() % 5) - 2, upper = rng() % 5;
		int cost_a = int(rng() % 9) - 4, cost_b = int(rng() % 9) - 4;
		int cost_c = int(rng() % 9) - 4;
		std::vector<InputEdge> edges{{0, 1, lower, 6, cost_a},
			{1, 2, 0, upper, cost_b}, {0, 2, -2, 3, cost_c}};
		std::vector<int> supply{int(rng() % 5) - 2, int(rng() % 5) - 2, 0};
		supply[2] = -supply[0] - supply[1];
		Graph graph(3);
		assert(graph.add_edge_lower_bound(0, 1, Cap(lower), cost_a) == 0);
		assert(graph.add_edge(1, 2, Cap(upper), cost_b) == 1);
		graph.add_edge(0, 2, Cap(-2), Cap(3), cost_c);
		assert(graph.get_edge(0).lower == lower);
		assert(graph.get_edge(0).upper == std::numeric_limits<Cap>::max());
		assert(graph.get_edge(1).lower == 0 && graph.get_edge(1).upper == upper);
		for (int v = 0; v < 3; ++v) graph.set_supply(v, Cap(supply[v]));
		const Graph& input = graph;
		auto expected = enumerate(3, edges, supply);
		auto actual = input.solve();
		assert(bool(actual) == !expected.empty());
		if (actual) {
			assert(actual->cost == expected.at(0));
			validate(input, *actual);
		}
		expected = enumerate(3, edges, supply, 0, 2);
		for (int f = -6; f <= 6; ++f) {
			auto fixed = input.flow(0, 2, Cap(f));
			assert(bool(fixed) == expected.contains(f));
			if (fixed) {
				assert(fixed->value == f && fixed->cost == expected.at(f));
				validate(input, *fixed, 0, 2);
			}
		}
		auto first = expected.lower_bound(-6), last = expected.upper_bound(6);
		auto range = input.flow_range(0, 2, Cap(-6), Cap(6));
		auto optimum = input.flow(0, 2, Cap(-6), Cap(6));
		auto minimum = input.min_flow(0, 2, Cap(-6), Cap(6));
		auto maximum = input.max_flow(0, 2, Cap(-6), Cap(6));
		assert(bool(range) == (first != last) && bool(optimum) == (first != last));
		assert(bool(minimum) == (first != last) && bool(maximum) == (first != last));
		if (first != last) {
			auto end = std::prev(last);
			long long best = first->second;
			for (auto it = first; it != last; ++it) best = std::min(best, it->second);
			assert(range->first == first->first && range->second == end->first);
			assert(optimum->cost == best && expected.at(optimum->value) == best);
			assert(minimum->value == first->first && minimum->cost == first->second);
			assert(maximum->value == end->first && maximum->cost == end->second);
			validate(input, *optimum, 0, 2);
			validate(input, *minimum, 0, 2);
			validate(input, *maximum, 0, 2);
		}
	}
	constexpr Cap smallest = std::numeric_limits<Cap>::min();
	constexpr Cap largest = std::numeric_limits<Cap>::max();
	Graph span(2);
	span.add_edge_lower_bound(0, 1, smallest, 1);
	span.set_supply(0, largest), span.set_supply(1, Cap(-largest));
	auto full_width = span.solve();
	assert(full_width && full_width->flow[0] == largest && full_width->cost == largest);
	validate(span, *full_width);
	span.reset_supply();
	auto range = span.flow_range(0, 1, smallest, largest);
	assert(range && range->first == smallest && range->second == largest);
	auto minimum = span.min_flow(0, 1, smallest, largest);
	auto maximum = span.max_flow(0, 1, smallest, largest);
	assert(minimum && minimum->value == smallest && minimum->cost == smallest);
	assert(maximum && maximum->value == largest && maximum->cost == largest);
	validate(span, *minimum, 0, 1);
	validate(span, *maximum, 0, 1);
	auto fixed = span.flow(0, 1, smallest);
	assert(fixed && fixed->flow[0] == smallest && fixed->cost == smallest);
	validate(span, *fixed, 0, 1);
	Graph loops(1);
	loops.add_edge(0, 0, smallest, largest, 2);
	loops.add_edge_lower_bound(0, 0, smallest, -3);
	loops.add_edge(0, 0, largest, -4);
	auto endpoints = loops.solve();
	assert(endpoints && endpoints->flow[0] == smallest);
	assert(endpoints->flow[1] == largest && endpoints->flow[2] == largest);
	assert(endpoints->cost == __int128_t(smallest) * 2 - __int128_t(largest) * 7);
	validate(loops, *endpoints);
	Graph star(4);
	for (int v = 1; v < 4; ++v) star.add_edge_lower_bound(0, v, Cap(0), -1);
	auto no_flow = star.solve();
	assert(no_flow && no_flow->cost == 0 && no_flow->flow == std::vector<Cap>(3));
	validate(star, *no_flow);
	auto star_best = star.flow(0, 1, Cap(-2), Cap(2));
	assert(star_best && star_best->value == 2 && star_best->cost == -2);
	validate(star, *star_best, 0, 1);
	Graph costly_star(4);
	constexpr auto cost_limit = std::numeric_limits<std::int64_t>::max();
	for (int v = 1; v < 4; ++v) costly_star.add_edge_lower_bound(0, v, Cap(0), -cost_limit);
	auto no_cost = costly_star.solve();
	assert(no_cost && no_cost->cost == 0 && no_cost->flow == std::vector<Cap>(3));
	validate(costly_star, *no_cost);
	auto costly_fixed = costly_star.flow(0, 1, Cap(2));
	assert(costly_fixed && costly_fixed->value == 2 && costly_fixed->cost == -__int128_t(cost_limit) * 2);
	validate(costly_star, *costly_fixed, 0, 1);
	Graph negative_loops(1);
	for (int i = 0; i < 3; ++i) negative_loops.add_edge_lower_bound(0, 0, Cap(0), -1);
	auto finite_loops = negative_loops.solve();
	assert(finite_loops && finite_loops->flow == std::vector<Cap>(3, largest));
	assert(finite_loops->cost == -__int128_t(largest) * 3);
	validate(negative_loops, *finite_loops);
	for (int trial = 0; trial < 12; ++trial) {
		Graph cycles(4);
		std::vector<InputEdge> edges;
		std::vector<int> supply(4);
		for (int v = 1; v < 4; ++v) {
			int cost = -1 - int(rng() % 5), upper = rng() % 3;
			cycles.add_edge_lower_bound(0, v, Cap(0), cost);
			cycles.add_edge(v, 0, Cap(upper), 0);
			edges.push_back({0, v, 0, 5, cost});
			edges.push_back({v, 0, 0, upper, 0});
			supply[v] = int(rng() % 3) - 1;
			supply[0] -= supply[v];
		}
		for (int v = 0; v < 4; ++v) cycles.set_supply(v, Cap(supply[v]));
		auto expected = enumerate(4, edges, supply);
		auto actual = cycles.solve();
		assert(bool(actual) == !expected.empty());
		if (actual) {
			assert(actual->cost == expected.at(0));
			validate(cycles, *actual);
		}
		expected = enumerate(4, edges, supply, 1, 2);
		auto first = expected.lower_bound(-2), last = expected.upper_bound(2);
		auto best = cycles.flow(1, 2, Cap(-2), Cap(2));
		assert(bool(best) == (first != last));
		if (best) {
			long long cost = first->second;
			for (auto it = first; it != last; ++it) cost = std::min(cost, it->second);
			assert(best->cost == cost && expected.at(best->value) == cost);
			validate(cycles, *best, 1, 2);
		}
	}
}

void boundary_check() {
	MinCostBFlow<> empty;
	auto zero = empty.solve();
	assert(zero && zero->cost == 0 && zero->value == 0);
	assert(zero->flow.empty() && zero->potential.empty());
	MinCostBFlow<> isolated(2);
	isolated.set_supply(0, 1);
	assert(!isolated.solve());
	isolated.set_supply(1, -1);
	assert(!isolated.solve());
	isolated.add_edge(0, 1, 1, 1, 7);
	auto connected = isolated.solve();
	assert(connected && connected->flow[0] == 1 && connected->cost == 7);
	validate(isolated, *connected);
	isolated.reset_supply();
	assert(!isolated.solve());
	assert(connected->flow[0] == 1 && connected->cost == 7);
	isolated.change_edge(0, -1, 1, -4);
	auto circulation = isolated.solve();
	assert(circulation && circulation->flow[0] == 0 && circulation->cost == 0);
	isolated.add_supply(0, -1), isolated.add_supply(1, 1);
	auto reversed = isolated.solve();
	assert(reversed && reversed->flow[0] == -1 && reversed->cost == 4);
	validate(isolated, *reversed);
	isolated.clear();
	assert(isolated.size() == 2 && isolated.edge_count() == 0);
	assert(isolated.supplies() == std::vector<std::int64_t>(2));
	assert(isolated.solve()->cost == 0);
	assert(isolated.add_edge(1, 0, 0, 2, 3) == 0);
	MinCostBFlow<> cycles(3);
	cycles.add_edge(0, 0, -2, 3, -7);
	cycles.add_edge(1, 2, 1, 3, -5);
	cycles.add_edge(2, 1, 0, 3, 1);
	auto negative = cycles.solve();
	assert(negative && negative->cost == -33);
	assert((negative->flow == std::vector<std::int64_t>{3, 3, 3}));
	validate(cycles, *negative);
	MinCostBFlow<> interval(2);
	interval.add_edge(0, 1, 0, 1, -3);
	interval.add_edge(0, 1, 0, 1, 4);
	assert(interval.flow(0, 1, 0, 2)->value == 1);
	assert(interval.flow(0, 1, 0, 2)->cost == -3);
	assert(interval.max_flow(0, 1)->value == 2);
	assert(interval.max_flow(0, 1)->cost == 1);
	assert(interval.min_flow(0, 1)->value == 0);
	assert(interval.min_flow(0, 1)->cost == 0);
	assert((interval.flow_range(0, 1) == std::pair<std::int64_t, std::int64_t>{0, 2}));
	MinCostBFlow<> lower_only(2);
	lower_only.add_edge(0, 1, 3, 5, 2);
	assert(!lower_only.solve());
	assert(lower_only.min_flow(0, 1)->value == 3);
	assert(lower_only.max_flow(0, 1)->value == 5);
	assert((lower_only.flow_range(0, 1) == std::pair<std::int64_t, std::int64_t>{3, 5}));
	assert((lower_only.flow_range(1, 0, -9, -1) == std::pair<std::int64_t, std::int64_t>{-5, -3}));
	MinCostBFlow<> independent(4);
	independent.add_edge(0, 1, 0, 1, 2);
	independent.add_edge(2, 3, 0, 1, 7);
	independent.set_supply(2, 1), independent.set_supply(3, -1);
	auto maximum = independent.max_flow(0, 1);
	assert(maximum && maximum->value == 1 && maximum->cost == 9);
	validate(independent, *maximum, 0, 1);
	MinCostBFlow<> signed_interval(2);
	signed_interval.add_edge(0, 1, -2, 3, 2);
	constexpr auto smallest = std::numeric_limits<std::int64_t>::min();
	constexpr auto largest = std::numeric_limits<std::int64_t>::max();
	auto signed_best = signed_interval.flow(0, 1, smallest, largest);
	assert(signed_best && signed_best->value == -2 && signed_best->cost == -4);
	validate(signed_interval, *signed_best, 0, 1);
	assert((signed_interval.flow_range(0, 1, smallest, largest) == std::pair<std::int64_t, std::int64_t>{-2, 3}));
	MinCostBFlow<> fixed_minimum(3);
	constexpr std::int64_t half = std::int64_t(1) << 62;
	fixed_minimum.add_edge(0, 1, smallest, smallest, 1);
	fixed_minimum.add_edge(2, 1, half, half, 1);
	fixed_minimum.set_supply(0, smallest);
	fixed_minimum.set_supply(1, half), fixed_minimum.set_supply(2, half);
	auto minimum_bound = fixed_minimum.solve();
	assert(minimum_bound && minimum_bound->cost == -__int128_t(half));
	validate(fixed_minimum, *minimum_bound);
	MinCostBFlow<> large(2);
	constexpr std::int64_t unit = 1000000000000LL;
	large.add_edge(0, 1, unit, 2 * unit, unit);
	large.set_supply(0, unit + unit / 2), large.set_supply(1, -unit - unit / 2);
	auto wide = large.solve();
	assert(wide && wide->cost == __int128_t(unit + unit / 2) * unit);
	validate(large, *wide);
	MinCostBFlow<> many(2);
	for (int i = 0; i < 40; ++i) many.add_edge(0, 1, 0, 1, i);
	auto full = many.flow(0, 1, 40);
	assert(full && full->cost == 780);
	validate(many, *full, 0, 1);
	auto copy = many;
	copy.change_edge(0, 0, 1, -100);
	assert(copy.flow(0, 1, 40)->cost == 680);
	assert(many.flow(0, 1, 40)->cost == 780);
	MinCostBFlow<std::int64_t, std::int64_t, std::int64_t> custom_total(2);
	custom_total.add_edge(0, 1, 0, 5, -7);
	auto custom = custom_total.max_flow(0, 1, 0, 5);
	assert(custom && custom->value == 5 && custom->cost == -35);
	validate(custom_total, *custom, 0, 1);
}

int main() {
	random_check<int, int>(650);
	random_check<std::int64_t, std::int64_t>(650);
	random_check<signed char, short>(100);
	one_sided_check<std::int8_t>();
	one_sided_check<std::int16_t>();
	one_sided_check<std::int32_t>();
	one_sided_check<std::int64_t>();
	boundary_check();
}
