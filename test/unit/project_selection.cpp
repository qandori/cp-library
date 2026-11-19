#include "flow/project_selection.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <utility>
#include <vector>

using Wide = __int128_t;

struct Model {
	struct Pair { int u, v; std::array<Wide, 4> cost; };
	struct Group { std::vector<int> vertices; bool value; Wide cost; };
	struct Forbidden { int u, v; bool u_value, v_value; };
	struct Any { std::vector<int> vertices; bool value; Wide cost; };
	int n;
	Wide constant = 0;
	std::vector<std::array<Wide, 2>> unary;
	std::vector<Pair> pairs;
	std::vector<Group> groups;
	std::vector<Forbidden> forbidden;
	std::vector<Any> any;
	std::vector<std::pair<int, bool>> forced;
	std::vector<std::pair<int, int>> implications;
	explicit Model(int n) : n(n), unary(n) {}
	std::optional<Wide> evaluate(unsigned mask) const {
		for (auto [v, value] : forced) if (bool(mask >> v & 1) != value) return std::nullopt;
		for (auto [u, v] : implications) if ((mask >> u & 1) && !(mask >> v & 1)) return std::nullopt;
		for (auto [u, v, a, b] : forbidden)
			if (bool(mask >> u & 1) == a && bool(mask >> v & 1) == b) return std::nullopt;
		Wide cost = constant;
		for (int v = 0; v < n; ++v) cost += unary[v][mask >> v & 1];
		for (const auto &pair : pairs) cost += pair.cost[2 * (mask >> pair.u & 1) + (mask >> pair.v & 1)];
		for (const auto &group : groups) {
			bool all = true;
			for (int v : group.vertices) all &= bool(mask >> v & 1) == group.value;
			if (all) cost += group.cost;
		}
		for (const auto &group : any) {
			bool found = false;
			for (int v : group.vertices) found |= bool(mask >> v & 1) == group.value;
			if (found) cost += group.cost;
		}
		return cost;
	}
};

template<class Cost>
void check(const ProjectSelection<Cost> &problem, const Model &model) {
	assert(problem.size() == model.n);
	std::optional<Wide> expected;
	for (unsigned mask = 0; mask < (1U << model.n); ++mask) {
		auto cost = model.evaluate(mask);
		if (cost && (!expected || *cost < *expected)) expected = cost;
	}
	auto result = problem.solve();
	assert(bool(result) == bool(expected));
	if (!expected) return;
	assert(result->cost == *expected);
	assert(result->selected.size() == std::size_t(model.n));
	unsigned mask = 0;
	for (int v = 0; v < model.n; ++v) mask |= unsigned(result->selected[v]) << v;
	assert(model.evaluate(mask) == expected);
}

template<class Cost>
void random_cases() {
	std::mt19937 random(20261021);
	auto number = [&] { return Cost(int(random() % 41) - 20); };
	for (int trial = 0; trial < 1200; ++trial) {
		int n = random() % 9;
		ProjectSelection<Cost> problem(n);
		Model model(n);
		for (int step = 0; step < 24; ++step) {
			int op = n ? random() % 12 : random() % 2 ? 6 : 10;
			int u = n ? random() % n : 0, v = n ? random() % n : 0;
			Cost a = number(), b = number(), c = Cost(random() % 21);
			switch (op) {
			case 0:
				problem.add_cost(u, a, b);
				model.unary[u][0] += a, model.unary[u][1] += b;
				break;
			case 1:
				problem.add_cost(u, -Wide(a));
				model.unary[u][1] -= a;
				break;
			case 2:
				problem.add_cost(u, -Wide(a), -Wide(b));
				model.unary[u][0] -= a, model.unary[u][1] -= b;
				break;
			case 3:
				problem.add_cost_if(u, v, true, false, c);
				model.pairs.push_back({u, v, {0, 0, c, 0}});
				break;
			case 4:
				problem.add_equal_cost(u, v, false, c);
				model.pairs.push_back({u, v, {0, c, c, 0}});
				break;
			case 5: {
				Cost d = number();
				Cost c00 = a, c01 = a + b, c10 = a + d + c, c11 = a + b + d;
				problem.add_pair_cost(u, v, c00, c01, c10, c11);
				model.pairs.push_back({u, v, {c00, c01, c10, c11}});
				break;
			}
			case 6: {
				std::vector<int> vertices;
				if (n) for (int i = random() % 6; i; --i) vertices.push_back(random() % n);
				bool value = random() & 1;
				problem.add_all_cost(vertices, value, -Wide(c));
				model.groups.push_back({vertices, value, -Wide(c)});
				break;
			}
			case 7:
				problem.force(u, bool(c & 1));
				model.forced.emplace_back(u, bool(c & 1));
				break;
			case 8:
				problem.forbid(u, v, true, false);
				model.implications.emplace_back(u, v);
				break;
			case 9:
				problem.force_equal(u, v);
				model.implications.emplace_back(u, v);
				model.implications.emplace_back(v, u);
				break;
			case 10:
				problem.add_constant(a);
				model.constant += a;
				break;
			default:
				problem.add_cost(u, a);
				model.unary[u][1] += a;
			}
			if (step % 8 == 7) check(problem, model);
		}
		check(problem, model);
		auto copy = problem;
		copy.add_constant(Cost(7));
		model.constant += 7;
		check(copy, model);
	}
}

template<class Cost>
void flipped_cases() {
	std::mt19937 random(20261022);
	auto number = [&] { return Cost(int(random() % 41) - 20); };
	for (int trial = 0; trial < 1200; ++trial) {
		int n = random() % 9;
		std::vector<bool> flipped(n);
		for (int v = 0; v < n; ++v) flipped[v] = random() & 1;
		ProjectSelection<Cost> problem(n, flipped);
		Model model(n);
		for (int v = 0; v < n; ++v) assert(problem.is_flipped(v) == flipped[v]);
		for (int step = 0; step < 28; ++step) {
			int op = n ? random() % (step < 14 ? 14 : 26) : 12 + int(random() % 2);
			int u = n ? random() % n : 0, v = n ? random() % n : 0;
			bool value = random() & 1;
			Cost a = number(), b = number(), c = Cost(random() % 21);
			if (op == 0) {
				problem.add_cost_if(u, value, a);
				model.unary[u][value] += a;
			} else if (op == 1) {
				problem.add_cost_if(u, value, -Wide(a));
				model.unary[u][value] -= a;
			} else if (op <= 5) {
				bool x = value != flipped[u], y = (value != (op == 2 || op == 5)) != flipped[v];
				Wide cost = op == 2 || op == 5 ? Wide(c) : -Wide(c);
				problem.add_cost_if(u, v, x, y, cost);
				std::array<Wide, 4> table{};
				table[2 * int(x) + int(y)] = cost;
				model.pairs.push_back({u, v, table});
			} else if (op <= 7) {
				Cost d = number();
				std::array<Cost, 4> encoded{a, Cost(a + b), Cost(a + d + c), Cost(a + b + d)};
				std::array<Wide, 4> table;
				for (int x = 0; x < 2; ++x) for (int y = 0; y < 2; ++y)
					table[2 * x + y] = encoded[2 * (x != flipped[u]) + (y != flipped[v])];
				problem.add_pair_cost(u, v, table[0], table[1], table[2], table[3]);
				model.pairs.push_back({u, v, table});
			} else if (op <= 11) {
				bool same = op == 8 || op == 10;
				bool profit = op >= 10;
				bool opposite = same != profit;
				if (u != v && ((flipped[u] != flipped[v]) != opposite)) v = u;
				Wide cost = profit ? -Wide(c) : Wide(c);
				problem.add_equal_cost(u, v, same, cost);
				model.pairs.push_back({u, v, same ? std::array<Wide, 4>{cost, 0, 0, cost} : std::array<Wide, 4>{0, cost, cost, 0}});
			} else if (op <= 13) {
				std::vector<int> vertices, candidates;
				for (int w = 0; w < n; ++w) if (flipped[w] == flipped[u]) candidates.push_back(w);
				if (n) for (int k = random() % 6; k; --k) vertices.push_back(candidates[random() % candidates.size()]);
				if (op == 12) {
					problem.add_all_cost(vertices, value, -Wide(c));
					model.groups.push_back({vertices, value, -Wide(c)});
				} else {
					problem.add_any_cost(vertices, value, c);
					model.any.push_back({vertices, value, c});
				}
			} else if (op <= 15) {
				bool x = value != flipped[u], y = (value != (op == 15)) != flipped[v];
				if (u == v) x = random() & 1, y = random() & 1;
				if (op == 14) {
					problem.forbid(u, v, x, !y);
					model.forbidden.push_back({u, v, x, !y});
				} else {
					problem.forbid(u, v, x, y);
					model.forbidden.push_back({u, v, x, y});
				}
			} else if (op <= 19) {
				bool opposite = op != 19;
				if (u != v && ((flipped[u] != flipped[v]) != opposite)) v = u;
				if (op == 16) problem.forbid(u, v, true, true);
				if (op == 17) problem.forbid(u, v, false, false);
				if (op == 18) problem.force_equal(u, v, false);
				if (op == 19) problem.force_equal(u, v);
				if (op == 16 || op == 18) model.forbidden.push_back({u, v, true, true});
				if (op == 17 || op == 18) model.forbidden.push_back({u, v, false, false});
				if (op == 19) {
					model.forbidden.push_back({u, v, true, false});
					model.forbidden.push_back({u, v, false, true});
				}
			} else if (op == 20) {
				problem.force(u, value);
				model.forced.emplace_back(u, value);
			} else if (op == 21) {
				problem.add_constant(a);
				model.constant += a;
			} else if (op == 22) {
				problem.add_cost(u, -Wide(a), -Wide(b));
				model.unary[u][0] -= a, model.unary[u][1] -= b;
			} else if (op == 23) {
				problem.add_cost(u, a, b);
				model.unary[u][0] += a, model.unary[u][1] += b;
			} else {
				if (flipped[u] != flipped[v]) v = u;
				if (op == 24) {
					problem.forbid(u, v, true, false);
					model.implications.emplace_back(u, v);
				} else {
					problem.add_cost_if(u, v, true, false, c);
					model.pairs.push_back({u, v, {0, 0, c, 0}});
				}
			}
			if (step % 7 == 6) check(problem, model);
		}
		check(problem, model);
		auto copy = problem;
		copy.add_constant(Cost(7));
		model.constant += 7;
		check(copy, model);
	}
}

void exhaustive_flipped_pairs() {
	for (int flip = 0; flip < 4; ++flip) {
		std::vector<bool> flipped{bool(flip & 2), bool(flip & 1)};
		for (int c00 = -2; c00 <= 2; ++c00) for (int c01 = -2; c01 <= 2; ++c01)
			for (int c10 = -2; c10 <= 2; ++c10) for (int c11 = -2; c11 <= 2; ++c11) {
				std::array<int, 4> table{c00, c01, c10, c11};
				if (table[flip] + table[3 ^ flip] > table[1 ^ flip] + table[2 ^ flip]) continue;
				for (int state = 0; state < 4; ++state) {
					ProjectSelection<> problem(2, flipped);
					problem.add_pair_cost(0, 1, c00, c01, c10, c11);
					problem.force(0, state & 2), problem.force(1, state & 1);
					assert(problem.solve()->cost == table[state]);
				}
			}
		for (int pattern = 0; pattern < 4; ++pattern) {
			if (bool((pattern ^ flip) & 2) == bool((pattern ^ flip) & 1)) continue;
			for (int state = 0; state < 4; ++state) {
				ProjectSelection<> problem(2, flipped);
				problem.forbid(0, 1, pattern & 2, pattern & 1);
				problem.force(0, state & 2), problem.force(1, state & 1);
				assert(bool(problem.solve()) == (state != pattern));
			}
		}
	}
}

void equal_and_group_cases() {
	const auto low = std::numeric_limits<std::int64_t>::min();
	const auto high = std::numeric_limits<std::int64_t>::max();
	for (int flip = 0; flip < 4; ++flip) {
		std::vector<bool> flipped{bool(flip & 2), bool(flip & 1)};
		for (bool equal : {false, true}) for (Wide cost : {-Wide(high), Wide(-7), Wide(0), Wide(5), Wide(high)})
			for (int v : {0, 1}) {
				if (v && cost && ((equal != (flipped[0] != flipped[1])) != (cost < 0))) continue;
				for (int state = 0; state < 4; ++state) {
					ProjectSelection<> problem(2, flipped);
					problem.add_equal_cost(0, v, equal, cost);
					problem.force(0, bool(state & 2));
					problem.force(1, bool(state & 1));
					bool same = bool(state & 2) == (v ? bool(state & 1) : bool(state & 2));
					auto answer = problem.solve();
					assert(answer && answer->cost == (same == equal ? cost : 0));
				}
			}
	}
	for (bool flip : {false, true}) for (bool value : {false, true})
		for (Wide cost : {Wide(low), Wide(-3), Wide(0), Wide(7), -Wide(low)})
			for (bool state : {false, true}) {
				ProjectSelection<> all(1, {flip}), any(1, {flip});
				all.add_all_cost({0}, value, cost);
				any.add_any_cost({0}, value, cost);
				all.force(0, state), any.force(0, state);
				Wide expected = state == value ? cost : 0;
				assert(all.solve()->cost == expected && any.solve()->cost == expected);
			}
	ProjectSelection<> empty;
	empty.add_all_cost({}, true, 7);
	empty.add_all_cost({}, false, -11);
	empty.add_any_cost({}, true, low);
	empty.add_any_cost({}, false, -Wide(low));
	assert(empty.solve()->cost == -4);
	ProjectSelection<std::int8_t> narrow(1);
	narrow.add_cost(0, -Wide(std::numeric_limits<std::int8_t>::min()));
	narrow.force(0, true);
	assert(narrow.solve()->cost == 128);
	const Wide minimum = std::numeric_limits<Wide>::min();
	ProjectSelection<> self_equal(1), self_different(1), empty_all, empty_any;
	self_equal.add_equal_cost(0, 0, true, minimum);
	self_different.add_equal_cost(0, 0, false, minimum);
	self_different.add_cost_if(0, 0, false, true, minimum);
	empty_all.add_all_cost({}, true, minimum);
	empty_any.add_any_cost({}, false, minimum);
	assert(self_equal.solve()->cost == minimum && self_different.solve()->cost == 0);
	assert(empty_all.solve()->cost == minimum && empty_any.solve()->cost == 0);
	ProjectSelection<> zero(2, {false, true});
	zero.add_equal_cost(0, 1, true, 0);
	zero.add_equal_cost(0, 1, false, 0);
	zero.add_all_cost({0, 1, 0}, true, 0);
	zero.add_any_cost({0, 1}, false, 0);
	assert(zero.solve()->cost == 0);
}

int main() {
	random_cases<int>();
	random_cases<std::int64_t>();
	flipped_cases<std::int8_t>();
	flipped_cases<int>();
	flipped_cases<std::int64_t>();
	exhaustive_flipped_pairs();
	equal_and_group_cases();
	for (int c00 = -2; c00 <= 2; ++c00) for (int c01 = -2; c01 <= 2; ++c01)
		for (int c10 = -2; c10 <= 2; ++c10) for (int c11 = -2; c11 <= 2; ++c11) {
			if (c00 + c11 > c01 + c10) continue;
			for (int state = 0; state < 4; ++state) {
				ProjectSelection<> problem(2);
				problem.add_pair_cost(0, 1, c00, c01, c10, c11);
				problem.force(0, state >> 1), problem.force(1, state & 1);
				assert(problem.solve()->cost == (std::array<int, 4>{c00, c01, c10, c11}[state]));
			}
		}
	ProjectSelection<> empty;
	assert(empty.solve()->cost == 0 && empty.solve()->selected.empty());
	ProjectSelection<> example(2);
	example.add_cost(0, -10);
	example.add_cost(1, 4);
	example.forbid(0, 1, true, false);
	assert(-example.solve()->cost == 6);
	assert((example.solve()->selected == std::vector<bool>{true, true}));
	example.force(1, false);
	assert(example.solve()->cost == 0);
	example.force(0, true);
	assert(!example.solve());
	ProjectSelection<> extreme(2);
	const auto low = std::numeric_limits<std::int64_t>::min();
	const auto high = std::numeric_limits<std::int64_t>::max();
	extreme.add_cost(0, low);
	extreme.add_cost(1, -Wide(low));
	extreme.add_cost(1, -Wide(high));
	auto result = extreme.solve();
	assert(result && result->cost == Wide(low));
	extreme.force(0, false);
	assert(extreme.solve()->cost == 0);
	ProjectSelection<> pair(2);
	pair.add_pair_cost(0, 1, high - 1, -1, high - 1, -1);
	assert(pair.solve()->cost == -1);
	ProjectSelection<> wide_constant;
	wide_constant.add_constant(high);
	wide_constant.add_constant(high);
	assert(wide_constant.solve()->cost == Wide(high) * 2);
	ProjectSelection<> wide_capacity(1);
	wide_capacity.add_cost(0, low, high - 1);
	wide_capacity.force(0, true);
	assert(wide_capacity.solve()->cost == Wide(high) - 1);
	ProjectSelection<> contradictions(10);
	contradictions.add_cost(0, -Wide(high));
	for (int v = 0; v < 10; ++v) {
		contradictions.force(v, true);
		contradictions.force(v, false);
	}
	assert(!contradictions.solve());
	ProjectSelection<> diagonal(1);
	diagonal.add_pair_cost(0, 0, 10, low, low, -7);
	assert(diagonal.solve()->cost == -7);
	ProjectSelection<> chain(200000);
	chain.add_cost(0, -17);
	chain.add_cost(199999, 3);
	for (int v = 1; v < chain.size(); ++v) chain.forbid(v - 1, v, true, false);
	assert(-chain.solve()->cost == 14);
	chain.force(199999, false);
	assert(chain.solve()->cost == 0);
	chain.force(0, true);
	assert(!chain.solve());
	ProjectSelection<> flipped_extreme(2, {false, true});
	flipped_extreme.add_cost_if(0, 1, true, true, -Wide(low));
	flipped_extreme.force(0, true), flipped_extreme.force(1, true);
	assert(flipped_extreme.solve()->cost == -Wide(low));
	ProjectSelection<> flipped_capacity(1, {true});
	flipped_capacity.add_cost(0, low, high - 1);
	flipped_capacity.force(0, true);
	assert(flipped_capacity.solve()->cost == Wide(high) - 1);
	ProjectSelection<> max_reward(2, {true, true});
	max_reward.add_equal_cost(0, 1, true, -Wide(high));
	assert(-max_reward.solve()->cost == Wide(high));
	ProjectSelection<> max_difference(2, {false, true});
	max_difference.add_equal_cost(0, 1, false, -Wide(high));
	assert(-max_difference.solve()->cost == Wide(high));
	max_difference.force(0, true), max_difference.force(1, true);
	assert(max_difference.solve()->cost == 0);
	std::vector<bool> flags{true, false};
	ProjectSelection<> copied_flags(2, flags);
	flags[0] = false;
	assert(copied_flags.is_flipped(0) && !copied_flags.is_flipped(1));
	ProjectSelection<> self(1, {true});
	self.add_cost_if(0, 0, false, true, low);
	self.add_equal_cost(0, 0, true, -7);
	self.add_equal_cost(0, 0, false, -9);
	self.add_any_cost({}, true, 17);
	assert(-self.solve()->cost == 7);
	self.force_equal(0, 0, false);
	assert(!self.solve());
	std::vector<bool> alternating(200000);
	for (int v = 0; v < 200000; ++v) alternating[v] = v & 1;
	ProjectSelection<> coloring(200000, alternating);
	coloring.add_cost(0, -17);
	coloring.add_cost_if(199999, false, 3);
	for (int v = 1; v < coloring.size(); ++v) coloring.force_equal(v - 1, v, false);
	assert(-coloring.solve()->cost == 14);
	coloring.force(199999, true);
	assert(coloring.solve()->cost == 0);
	coloring.force(0, true);
	assert(!coloring.solve());
}
