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
	struct Edge { int from, to; Cap cap, flow, lower, upper; };
	struct Result { std::vector<Cap> flow; Cap value; };

  private:
	struct RawEdge { int from, to; Cap lower, upper; };
	std::vector<Cap> supply_;
	std::vector<RawEdge> edges_;
	std::optional<Result> state_;
	std::vector<Wide> delta_;
	bool state_active_ = false;
	struct State { MaxFlow<Unsigned> graph; Cap value; };
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void index([[maybe_unused]] int i) const { assert(0 <= i && i < edge_count()); }
	template<bool Continuing = false>
	std::optional<State> feasible(int s, int t, Cap lower, Cap upper,
		const std::vector<Wide>* values = nullptr, const std::vector<Cap>* initial = nullptr) const {
		int n = size(), m = edge_count();
		if constexpr (Continuing) assert(initial && initial->size() == edges_.size());
		std::vector<Wide> balance = values ? *values : std::vector<Wide>(supply_.begin(), supply_.end());
		Wide sum = 0;
		for (Wide b : balance) sum += b;
		if (sum) return std::nullopt;
		MaxFlow<Unsigned> graph(n + 2);
		graph.reserve(std::size_t(m) + n + 2);
		auto insert = [&](const RawEdge& e, Cap base) {
			balance[e.from] -= Wide(base), balance[e.to] += Wide(base);
			Unsigned cap = Bounds::capacity(e.lower, e.upper);
			int id = graph.add_edge(e.from, e.to, cap);
			if constexpr (Continuing)
				if (base != e.lower) graph.change_edge(id, cap, Bounds::amount(Wide(base) - Wide(e.lower)));
		};
		for (int i = 0; i < m; ++i) {
			const auto& e = edges_[i];
			if constexpr (Continuing) insert(e, std::clamp((*initial)[i], e.lower, e.upper));
			else insert(e, e.lower);
		}
		if (s >= 0) insert(RawEdge{t, s, lower, upper}, lower);
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
	MaxFlowLowerBound(const MaxFlowLowerBound& other)
		: supply_(other.supply_), edges_(other.edges_), state_active_(other.state_active_) {
		if (state_active_) state_ = other.state_, delta_ = other.delta_;
	}
	MaxFlowLowerBound& operator=(const MaxFlowLowerBound& other) {
		if (this == &other) return *this;
		supply_ = other.supply_, edges_ = other.edges_;
		if (other.state_active_) state_ = other.state_, delta_ = other.delta_;
		state_active_ = other.state_active_;
		return *this;
	}
	MaxFlowLowerBound(MaxFlowLowerBound&&) = default;
	MaxFlowLowerBound& operator=(MaxFlowLowerBound&&) = default;
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
		if (state_active_) state_->flow.push_back(Cap(0));
		return id;
	}
	int add_edge(int from, int to, Cap upper) { return add_edge(from, to, Cap(0), upper); }
	int add_edge_lower_bound(int from, int to, Cap lower) {
		return add_edge(from, to, lower, std::numeric_limits<Cap>::max());
	}
	Edge get_edge(int i) const {
		index(i);
		const auto& e = edges_[i];
		return {e.from, e.to, e.upper, state_active_ ? state_->flow[i] : Cap(0), e.lower, e.upper};
	}
	std::vector<Edge> edges() const {
		std::vector<Edge> result;
		result.reserve(edge_count());
		for (int i = 0; i < edge_count(); ++i) result.push_back(get_edge(i));
		return result;
	}
	void change_edge(int i, Cap lower, Cap upper) {
		index(i);
		Bounds::capacity(lower, upper);
		edges_[i].lower = lower, edges_[i].upper = upper;
		reset_flow();
	}
	Cap supply(int v) const { vertex(v); return supply_[v]; }
	const std::vector<Cap>& supplies() const { return supply_; }
	void set_supply(int v, Cap amount) { vertex(v); supply_[v] = amount; reset_flow(); }
	void add_supply(int v, Cap amount) {
		vertex(v);
		supply_[v] = Bounds::value(Wide(supply_[v]) + Wide(amount));
		reset_flow();
	}
	void reset_supply() { std::fill(supply_.begin(), supply_.end(), Cap(0)); reset_flow(); }
	void reset_flow() { state_active_ = false; }
	void clear() { edges_.clear(); reset_supply(); }
	std::optional<Result> solve() const {
		auto state = feasible(-1, -1, 0, 0);
		if (!state) return std::nullopt;
		return result(*state);
	}
	std::optional<Result> solve(int s, int t, Cap amount) const { return solve(s, t, amount, amount); }
	std::optional<Result> solve(int s, int t, Cap lower, Cap upper) const {
		validate(s, t, lower, upper);
		auto state = feasible(s, t, lower, upper);
		if (!state) return std::nullopt;
		return result(*state);
	}
	std::optional<Cap> flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
		validate(s, t, Cap(0), limit);
		std::vector<Wide> values;
		if (state_active_) {
			values.assign(supply_.begin(), supply_.end());
			for (int v = 0; v < size(); ++v) values[v] += delta_[v];
		}
		auto state = state_active_ ? feasible<true>(s, t, 0, limit, &values, &state_->flow) : feasible(s, t, 0, limit);
		if (!state) return std::nullopt;
		Unsigned sent = augment(state->graph, s, t, Bounds::amount(Wide(limit) - Wide(state->value)));
		state->value = Bounds::value(Wide(state->value) + Wide(sent));
		Result answer = result(*state);
		if (!state_active_) delta_.assign(size(), Wide(0));
		delta_[s] += Wide(answer.value), delta_[t] -= Wide(answer.value);
		state_ = std::move(answer);
		state_active_ = true;
		return state_->value;
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
