#ifndef MIN_COST_B_FLOW_HPP
#define MIN_COST_B_FLOW_HPP

#include "min_cost_flow.hpp"
#include "../components/flow_bounds.hpp"
#include <optional>

template<class Cap = std::int64_t, class Cost = std::int64_t, class Total = __int128_t>
class MinCostBFlow {
	static_assert(std::is_integral_v<Cap> && std::is_signed_v<Cap> && sizeof(Cap) <= 8);
	static_assert(std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8);
	static_assert(std::numeric_limits<Total>::is_integer && std::numeric_limits<Total>::is_signed);

  public:
	using capacity_type = Cap;
	using cost_type = Cost;
	using total_type = Total;
	struct Edge { int from, to; Cap lower, upper; Cost cost; };
	struct Result {
		Total cost;
		std::vector<Cap> flow;
		std::vector<Cost> potential;
		Cap value;
	};

  private:
	using Bounds = flow_detail::CapacityBounds<Cap>;
	using Wide = typename Bounds::Wide;
	using Residual = typename Bounds::Residual;
	std::vector<Cap> supply_;
	std::vector<Edge> edges_;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void index([[maybe_unused]] int i) const { assert(0 <= i && i < edge_count()); }
	static Cap narrow(Wide value) {
		return Bounds::value(value);
	}
	static Residual capacity(Cap lower, Cap upper) {
		return Bounds::capacity(lower, upper);
	}
	std::optional<Result> solve_fallback(int s, int t, Cap lower, Cap upper, int objective) const {
		int n = size(), m = edge_count();
		std::vector<Wide> balance(supply_.begin(), supply_.end());
		std::vector<Cost> costs;
		costs.reserve(2 * (std::size_t(m) + n + (s >= 0)));
		flow_detail::Graph<Residual> graph(n + 2);
		graph.reserve(std::size_t(m) + n + (s >= 0));
		auto insert = [&](int from, int to, Cap low, Cap high, Cost cost) {
			Cap value = std::clamp(Cap(0), low, high);
			balance[from] -= Wide(value), balance[to] += Wide(value);
			int id = graph.add_edge(from, to, Bounds::amount(Wide(high) - Wide(value)));
			graph.arcs[2 * id + 1].cap = Bounds::amount(Wide(value) - Wide(low));
			costs.push_back(cost), costs.push_back(-cost);
		};
		for (const auto& e : edges_) insert(e.from, e.to, e.lower, e.upper, objective ? Cost(0) : e.cost);
		if (s >= 0) insert(t, s, lower, upper, Cost(objective));
		Wide need = 0;
		for (int v = 0; v < n; ++v) {
			if (balance[v] > 0) {
				graph.add_edge(n, v, Bounds::amount(balance[v])), need += balance[v];
				costs.push_back(0), costs.push_back(0);
			} else if (balance[v] < 0) {
				graph.add_edge(v, n + 1, Bounds::amount(-balance[v]));
				costs.push_back(0), costs.push_back(0);
			}
		}
		Residual required = Bounds::amount(need);
		if (!required) {
			graph.add_edge(n, n + 1, Residual(1));
			costs.push_back(0), costs.push_back(0);
			required = 1;
		}
		graph.build();
		auto [sent, unused_cost] = flow_detail::scaling_flow<Residual, Cost, __uint128_t>(graph, costs, n, n + 1, required);
		(void) unused_cost;
		if (sent != required) return std::nullopt;
		Result result{Total(0), {}, {}, Cap(0)};
		result.flow.reserve(m);
		auto recovered = [&](int i, Cap low) {
			return narrow(Wide(low) + Wide(graph.arcs[2 * i + 1].cap));
		};
		for (int i = 0; i < m; ++i) {
			const auto& e = edges_[i];
			Cap value = recovered(i, e.lower);
			result.flow.push_back(value);
			if (!objective) result.cost += Total(value) * Total(e.cost);
		}
		if (s >= 0) result.value = recovered(m, lower);
		if (!objective) {
			std::vector<Wide> potential(n);
			bool changed = false;
			for (int round = 0; round < n; ++round) {
				changed = false;
				for (int i = 0; i < m; ++i) {
					const auto& e = edges_[i];
					if (graph.arcs[2 * i].cap && potential[e.to] > potential[e.from] + Wide(e.cost))
						potential[e.to] = potential[e.from] + Wide(e.cost), changed = true;
					if (graph.arcs[2 * i + 1].cap && potential[e.from] > potential[e.to] - Wide(e.cost))
						potential[e.from] = potential[e.to] - Wide(e.cost), changed = true;
				}
				if (!changed) break;
			}
			assert(!changed);
			result.potential.reserve(n);
			for (Wide value : potential) {
				assert(std::numeric_limits<Cost>::min() <= value && value <= std::numeric_limits<Cost>::max());
				result.potential.push_back(Cost(value));
			}
		}
		return result;
	}
	std::optional<Result> solve_impl(int s, int t, Cap lower, Cap upper, int objective) const {
		int n = size(), m = edge_count();
		if (s >= 0) {
			Wide left = lower, right = upper;
			for (int v : {s, t}) {
				Wide low = -Wide(supply_[v]), high = low;
				for (const auto& e : edges_) {
					if (e.from == e.to) continue;
					if (e.from == v) low += e.lower, high += e.upper;
					if (e.to == v) low -= e.upper, high -= e.lower;
				}
				if (v == t) { Wide previous = low; low = -high, high = -previous; }
				left = std::max(left, low), right = std::min(right, high);
			}
			if (left > right) return std::nullopt;
			lower = narrow(left), upper = narrow(right);
			capacity(lower, upper);
		}
		std::vector<Wide> balance(supply_.begin(), supply_.end());
		Wide sum = 0;
		for (Wide b : balance) sum += b;
		if (sum) return std::nullopt;
		MinCostFlow<Residual, Cost, Total> graph(n + 2);
		graph.reserve(std::size_t(m) + n + (s >= 0));
		using BaseTotal = std::conditional_t<std::is_integral_v<Total> || std::is_same_v<Total, Wide>, __uint128_t, Total>;
		BaseTotal base_cost = 0;
		auto insert = [&](const Edge& e, Cost cost) {
			bool reverse = cost < 0;
			Cap base = reverse ? e.upper : e.lower;
			balance[e.from] -= Wide(base), balance[e.to] += Wide(base);
			base_cost += BaseTotal(Wide(base) * Wide(cost));
			graph.add_edge(reverse ? e.to : e.from, reverse ? e.from : e.to,
				capacity(e.lower, e.upper), reverse ? -cost : cost);
		};
		for (const auto& e : edges_) insert(e, objective ? Cost(0) : e.cost);
		if (s >= 0) insert(Edge{t, s, lower, upper, Cost(objective)}, Cost(objective));
		Wide need = 0;
		const Wide maximum = std::numeric_limits<Residual>::max();
		for (int v = 0; v < n; ++v) {
			if (balance[v] < -maximum || maximum < balance[v]) return solve_fallback(s, t, lower, upper, objective);
			if (balance[v] > 0) {
				need += balance[v];
				if (need > maximum) return solve_fallback(s, t, lower, upper, objective);
				graph.add_edge(n, v, Bounds::amount(balance[v]), 0);
			} else if (balance[v] < 0) graph.add_edge(v, n + 1, Bounds::amount(-balance[v]), 0);
		}
		auto [sent, cost] = graph.flow(n, n + 1, Bounds::amount(need));
		if (Wide(sent) != need) return std::nullopt;
		Result result{Total(base_cost) + cost, {}, {}, Cap(0)};
		result.flow.reserve(m);
		for (int i = 0; i < m; ++i) {
			const auto& e = edges_[i];
			bool reverse = !objective && e.cost < 0;
			Cap base = reverse ? e.upper : e.lower;
			Wide delta = graph.get_edge(i).flow;
			result.flow.push_back(narrow(Wide(base) + (reverse ? -delta : delta)));
		}
		if (s >= 0) {
			Wide delta = graph.get_edge(m).flow;
			result.value = narrow(objective < 0 ? Wide(upper) - delta : Wide(lower) + delta);
		}
		if (!objective) {
			const auto& p = graph.potentials();
			result.potential.assign(p.begin(), p.begin() + n);
		}
		return result;
	}
	std::optional<Result> extreme_flow(int s, int t, Cap lower, Cap upper, int objective) const {
		vertex(s), vertex(t);
		assert(s != t);
		assert(lower <= upper);
		auto result = solve_impl(s, t, lower, upper, objective);
		if (!result) return std::nullopt;
		return flow(s, t, result->value);
	}

  public:
	explicit MinCostBFlow(int n = 0) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() - 2);
		supply_.assign(n, Cap(0));
	}
	int size() const { return int(supply_.size()); }
	int edge_count() const { return int(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(m);
	}
	int add_edge(int from, int to, Cap lower, Cap upper, Cost cost) {
		vertex(from), vertex(to);
		capacity(lower, upper);
		assert(cost > std::numeric_limits<Cost>::min());
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		int id = edge_count();
		edges_.push_back({from, to, lower, upper, cost});
		return id;
	}
	int add_edge(int from, int to, Cap upper, Cost cost) { return add_edge(from, to, Cap(0), upper, cost); }
	int add_edge_lower_bound(int from, int to, Cap lower, Cost cost) {
		return add_edge(from, to, lower, std::numeric_limits<Cap>::max(), cost);
	}
	Edge get_edge(int i) const { index(i); return edges_[i]; }
	std::vector<Edge> edges() const { return edges_; }
	void change_edge(int i, Cap lower, Cap upper, Cost cost) {
		index(i);
		capacity(lower, upper);
		assert(cost > std::numeric_limits<Cost>::min());
		edges_[i].lower = lower, edges_[i].upper = upper, edges_[i].cost = cost;
	}
	Cap supply(int v) const { vertex(v); return supply_[v]; }
	const std::vector<Cap>& supplies() const { return supply_; }
	void set_supply(int v, Cap amount) { vertex(v); supply_[v] = amount; }
	void add_supply(int v, Cap amount) { vertex(v); supply_[v] = narrow(Wide(supply_[v]) + amount); }
	void reset_supply() { std::fill(supply_.begin(), supply_.end(), Cap(0)); }
	void clear() { edges_.clear(); reset_supply(); }
	std::optional<Result> solve() const { return solve_impl(-1, -1, 0, 0, 0); }
	std::optional<Result> flow(int s, int t, Cap amount) const { return flow(s, t, amount, amount); }
	std::optional<Result> flow(int s, int t, Cap lower, Cap upper) const {
		vertex(s), vertex(t);
		assert(s != t);
		assert(lower <= upper);
		return solve_impl(s, t, lower, upper, 0);
	}
	std::optional<Result> min_flow(int s, int t, Cap lower = 0, Cap upper = std::numeric_limits<Cap>::max()) const {
		return extreme_flow(s, t, lower, upper, 1);
	}
	std::optional<Result> max_flow(int s, int t, Cap lower = 0, Cap upper = std::numeric_limits<Cap>::max()) const {
		return extreme_flow(s, t, lower, upper, -1);
	}
	std::optional<std::pair<Cap, Cap>> flow_range(int s, int t, Cap lower = 0, Cap upper = std::numeric_limits<Cap>::max()) const {
		vertex(s), vertex(t);
		assert(s != t);
		assert(lower <= upper);
		auto first = solve_impl(s, t, lower, upper, 1);
		if (!first) return std::nullopt;
		auto last = solve_impl(s, t, lower, upper, -1);
		assert(last);
		return std::pair<Cap, Cap>{first->value, last->value};
	}
};

#endif
