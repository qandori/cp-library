#ifndef MAX_FLOW_LOWER_BOUND_HPP
#define MAX_FLOW_LOWER_BOUND_HPP

#include "max_flow.hpp"
#include "../components/flow_bounds.hpp"
#include <optional>
#include <utility>

template<class Cap = std::int64_t> class MaxFlowLowerBound {
	using Bounds = flow_detail::CapacityBounds<Cap>;
	using Unsigned = typename Bounds::Residual;
	using Wide = typename Bounds::Wide;

  public:
	using capacity_type = Cap;
	struct Edge { int from, to; Cap lower, upper; };
	struct Result { std::vector<Cap> flow; Cap value; };

  private:
	std::vector<Cap> supply_;
	std::vector<Edge> edges_;
	struct State { MaxFlow<Unsigned> graph; Cap value; };
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void index([[maybe_unused]] int i) const { assert(0 <= i && i < edge_count()); }
	std::optional<State> feasible(int s, int t, Cap lower, Cap upper) const {
		int n = size(), m = edge_count();
		std::vector<Wide> balance(supply_.begin(), supply_.end());
		Wide sum = 0;
		for (Wide b : balance) sum += b;
		if (sum) return std::nullopt;
		MaxFlow<Unsigned> graph(n + 2);
		graph.reserve(std::size_t(m) + n + 2);
		auto insert = [&](const Edge& e) {
			balance[e.from] -= Wide(e.lower), balance[e.to] += Wide(e.lower);
			graph.add_edge(e.from, e.to, Bounds::capacity(e.lower, e.upper));
		};
		for (const auto& e : edges_) insert(e);
		if (s >= 0) insert(Edge{t, s, lower, upper});
		Wide need = 0;
		for (int v = 0; v < n; ++v) {
			if (balance[v] > 0) graph.add_edge(n, v, Bounds::amount(balance[v])), need += balance[v];
			else if (balance[v] < 0) graph.add_edge(v, n + 1, Bounds::amount(-balance[v]));
		}
		Unsigned required = Bounds::amount(need);
		if (required && graph.flow(n, n + 1) != required) return std::nullopt;
		Cap value = s >= 0 ? Bounds::value(Wide(lower) + Wide(graph.get_edge(m).flow)) : Cap(0);
		for (int i = m; i < graph.edge_count(); ++i) graph.change_edge(i, 0, 0);
		return State{std::move(graph), value};
	}
	Unsigned augment(MaxFlow<Unsigned>& graph, int s, int t, Unsigned limit) const {
		if (!limit) return 0;
		Wide outgoing = 0;
		for (int i = 0; i < edge_count(); ++i) {
			const auto e = graph.get_edge(i);
			if (e.from == e.to) continue;
			if (e.from == s) outgoing += Wide(e.cap) - Wide(e.flow);
			if (e.to == s) outgoing += Wide(e.flow);
			if (outgoing > Wide(limit)) break;
		}
		if (outgoing <= Wide(limit)) return outgoing ? graph.flow(s, t) : Unsigned(0);
		int id = graph.add_edge(size(), s, limit);
		Unsigned sent = graph.flow(size(), t);
		graph.change_edge(id, 0, 0);
		return sent;
	}
	Result result(const State& state) const {
		Result answer{{}, state.value};
		answer.flow.reserve(edges_.size());
		for (int i = 0; i < edge_count(); ++i)
			answer.flow.push_back(Bounds::value(Wide(edges_[i].lower) + Wide(state.graph.get_edge(i).flow)));
		return answer;
	}
	void validate(int s, int t, [[maybe_unused]] Cap lower, [[maybe_unused]] Cap upper) const {
		vertex(s), vertex(t);
		assert(s != t && lower <= upper);
	}
	std::optional<Result> extreme_flow(int s, int t, Cap lower, Cap upper, bool maximum) const {
		validate(s, t, lower, upper);
		auto state = feasible(s, t, lower, upper);
		if (!state) return std::nullopt;
		if (maximum) {
			Unsigned sent = augment(state->graph, s, t, Bounds::amount(Wide(upper) - Wide(state->value)));
			state->value = Bounds::value(Wide(state->value) + Wide(sent));
		} else {
			Unsigned sent = augment(state->graph, t, s, Bounds::amount(Wide(state->value) - Wide(lower)));
			state->value = Bounds::value(Wide(state->value) - Wide(sent));
		}
		return result(*state);
	}

  public:
	explicit MaxFlowLowerBound(int n = 0) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2 - 3);
		supply_.assign(n, Cap(0));
	}
	int size() const { return int(supply_.size()); }
	int edge_count() const { return int(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(m);
	}
	int add_edge(int from, int to, Cap lower, Cap upper) {
		vertex(from), vertex(to);
		Bounds::capacity(lower, upper);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		int id = edge_count();
		edges_.push_back({from, to, lower, upper});
		return id;
	}
	int add_edge(int from, int to, Cap upper) { return add_edge(from, to, Cap(0), upper); }
	int add_edge_lower_bound(int from, int to, Cap lower) {
		return add_edge(from, to, lower, std::numeric_limits<Cap>::max());
	}
	Edge get_edge(int i) const { index(i); return edges_[i]; }
	std::vector<Edge> edges() const { return edges_; }
	void change_edge(int i, Cap lower, Cap upper) {
		index(i);
		Bounds::capacity(lower, upper);
		edges_[i].lower = lower, edges_[i].upper = upper;
	}
	Cap supply(int v) const { vertex(v); return supply_[v]; }
	const std::vector<Cap>& supplies() const { return supply_; }
	void set_supply(int v, Cap amount) { vertex(v); supply_[v] = amount; }
	void add_supply(int v, Cap amount) { vertex(v); supply_[v] = Bounds::value(Wide(supply_[v]) + Wide(amount)); }
	void reset_supply() { std::fill(supply_.begin(), supply_.end(), Cap(0)); }
	void clear() { edges_.clear(); reset_supply(); }
	std::optional<Result> solve() const {
		auto state = feasible(-1, -1, 0, 0);
		if (!state) return std::nullopt;
		return result(*state);
	}
	std::optional<Result> flow(int s, int t, Cap amount) const { return flow(s, t, amount, amount); }
	std::optional<Result> flow(int s, int t, Cap lower, Cap upper) const {
		validate(s, t, lower, upper);
		auto state = feasible(s, t, lower, upper);
		if (!state) return std::nullopt;
		return result(*state);
	}
	std::optional<Result> min_flow(int s, int t, Cap lower = 0, Cap upper = std::numeric_limits<Cap>::max()) const {
		return extreme_flow(s, t, lower, upper, false);
	}
	std::optional<Result> max_flow(int s, int t, Cap lower = 0, Cap upper = std::numeric_limits<Cap>::max()) const {
		return extreme_flow(s, t, lower, upper, true);
	}
	std::optional<std::pair<Cap, Cap>> flow_range(int s, int t, Cap lower = 0, Cap upper = std::numeric_limits<Cap>::max()) const {
		validate(s, t, lower, upper);
		auto state = feasible(s, t, lower, upper);
		if (!state) return std::nullopt;
		auto graph = state->graph;
		Cap minimum = Bounds::value(Wide(state->value) - Wide(augment(graph, t, s, Bounds::amount(Wide(state->value) - Wide(lower)))));
		Cap maximum = Bounds::value(Wide(state->value) + Wide(augment(state->graph, s, t, Bounds::amount(Wide(upper) - Wide(state->value)))));
		return std::pair<Cap, Cap>{minimum, maximum};
	}
};

#endif
