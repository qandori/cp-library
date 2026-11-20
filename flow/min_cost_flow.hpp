#ifndef MIN_COST_FLOW_HPP
#define MIN_COST_FLOW_HPP

#include "../components/flow_graph.hpp"
#include "../components/min_cost_circulation.hpp"
#include "../components/min_cost_simplex.hpp"
#include "../components/RadixHeap.hpp"
#include <cstdint>
#include <type_traits>
#include <utility>

template<class Cap = std::int64_t, class Cost = std::int64_t,
	class Total = std::conditional_t<std::is_floating_point_v<Cost>, Cost, __int128_t>>
class MinCostFlow {
	static constexpr bool fractional = std::is_floating_point_v<Cost>;
	static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool> && sizeof(Cap) <= 8);
	static_assert(fractional || (std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8));
	static_assert(fractional ? std::is_floating_point_v<Total> : std::numeric_limits<Total>::is_integer && std::numeric_limits<Total>::is_signed);
	using Distance = std::conditional_t<fractional, long double, Cost>;
	using Accumulator = std::conditional_t<fractional, long double, Total>;
	struct Empty {};
	flow_detail::Graph<Cap> graph_;
	std::vector<Cost> cost_;
	std::vector<Distance> potential_;
	[[no_unique_address]] std::conditional_t<std::is_same_v<Cost, Distance>, Empty, std::vector<Cost>> public_potential_;
	bool valid_ = false, zero_flow_ = true;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < graph_.n); }
	int index(int i) const { assert(0 <= i && std::size_t(i) < graph_.arcs.size() / 2); return 2 * i; }
	void initialize() {
		if (valid_) return;
		potential_.assign(graph_.n, Distance(0));
		bool changed = false;
		int step = zero_flow_ ? 2 : 1;
		for (int round = 0; round < graph_.n; ++round) {
			changed = false;
			for (int i = 0; i < int(cost_.size()); i += step) {
				if (!graph_.arcs[i].cap) continue;
				int from = graph_.arcs[i ^ 1].to, to = graph_.arcs[i].to;
				Distance next = potential_[from] + cost_[i];
				bool improve;
				if constexpr (fractional) improve = flow_detail::reduced_cost(Distance(cost_[i]), potential_[from], potential_[to]) < 0;
				else improve = next < potential_[to];
				if (improve) potential_[to] = next, changed = true;
			}
			if (!changed) break;
		}
		assert(!changed);
		valid_ = true;
	}
	static Distance mark(Distance d) {
		if constexpr (fractional) return -d;
		else return Distance(~d);
	}
	static bool marked(Distance d) {
		if constexpr (fractional) return std::signbit(d);
		else return d < 0;
	}
	template<class Report> std::pair<Cap, Accumulator> run(int s, int t, Cap limit, Report report, int rounds = -1) {
		vertex(s), vertex(t);
		assert(s != t && limit >= 0);
		if (!limit) return {0, 0};
		graph_.build();
		initialize();
		auto& g = graph_;
		const Distance inf = std::numeric_limits<Distance>::max();
		std::vector<Distance> distance(g.n);
		std::vector<int> parent(g.n), zero, settled;
		zero.reserve(g.n), settled.reserve(g.n);
		using State = std::pair<Distance, int>;
		std::conditional_t<fractional, DaryHeap<State, std::greater<State>, 4>, RadixHeap<Cost>> heap;
		Cap sent = 0;
		Accumulator total = 0;
		while (sent < limit && rounds != 0) {
			std::fill(distance.begin(), distance.end(), inf);
			zero.clear(), settled.clear(), heap.clear();
			distance[s] = 0;
			zero.push_back(s);
			while (!zero.empty() || !heap.empty()) {
				int v;
				if (!zero.empty()) v = zero.back(), zero.pop_back();
				else {
					auto [d, u] = heap.pop_value();
					if (d != distance[u]) continue;
					v = u;
				}
				Distance d = distance[v];
				distance[v] = mark(d);
				settled.push_back(v);
				if (v == t) break;
				for (int j = g.start[v]; j < g.start[v + 1]; ++j) {
					int i = g.order[j];
					const auto& e = g.arcs[i];
					if (!e.cap || marked(distance[e.to])) continue;
					Distance reduced = flow_detail::reduced_cost(Distance(cost_[i]), potential_[v], potential_[e.to]);
					if constexpr (fractional) reduced = std::max(Distance(0), reduced);
					else assert(reduced >= 0);
					if (reduced >= inf - d) continue;
					Distance next = d + reduced;
					if (next >= distance[e.to]) continue;
					distance[e.to] = next, parent[e.to] = i;
					if (!reduced) zero.push_back(e.to);
					else heap.push({next, e.to});
				}
			}
			if (!marked(distance[t])) break;
			Distance best = mark(distance[t]);
			for (int v : settled) potential_[v] -= best - mark(distance[v]);
			Cap amount = limit - sent;
			for (int v = t; v != s; v = g.arcs[parent[v] ^ 1].to) amount = std::min(amount, g.arcs[parent[v]].cap);
			zero_flow_ = false;
			Distance price = 0;
			for (int v = t; v != s; v = g.arcs[parent[v] ^ 1].to) {
				int i = parent[v];
				g.arcs[i].cap -= amount, g.arcs[i ^ 1].cap += amount;
				if constexpr (fractional) price += cost_[i];
			}
			if constexpr (!fractional) price = potential_[t] - potential_[s];
			sent += amount, total += Accumulator(amount) * Accumulator(price);
			report(sent, total, price);
			if (rounds > 0) --rounds;
		}
		return {sent, total};
	}

  public:
	using capacity_type = Cap;
	using cost_type = Cost;
	using total_type = Total;
	struct Edge { int from, to; Cap cap, flow; Cost cost; };
	explicit MinCostFlow(int n = 0) : graph_(n) {}
	int size() const { return graph_.n; }
	int edge_count() const { return int(cost_.size() / 2); }
	void reserve(std::size_t m) { graph_.reserve(m); cost_.reserve(2 * m); }
	int add_edge(int from, int to, Cap cap, Cost cost) {
		if constexpr (fractional) assert(std::isfinite(cost));
		else assert(cost > std::numeric_limits<Cost>::min());
		int id = graph_.add_edge(from, to, cap);
		cost_.push_back(cost), cost_.push_back(-cost);
		valid_ = false;
		return id;
	}
	Edge get_edge(int i) const {
		int j = index(i);
		const auto& a = graph_.arcs[j];
		const auto& b = graph_.arcs[j ^ 1];
		return {b.to, a.to, Cap(a.cap + b.cap), b.cap, cost_[j]};
	}
	std::vector<Edge> edges() const {
		std::vector<Edge> result;
		result.reserve(edge_count());
		for (int i = 0; i < edge_count(); ++i) result.push_back(get_edge(i));
		return result;
	}
	void reset_flow() { graph_.reset_flow(); valid_ = false; zero_flow_ = true; }
	const std::vector<Cost>& potentials() {
		initialize();
		if constexpr (std::is_same_v<Cost, Distance>) return potential_;
		else {
			public_potential_.assign(potential_.begin(), potential_.end());
			return public_potential_;
		}
	}
	std::pair<Cap, Total> flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
		int count = 0, budget = 8;
		auto result = run(s, t, limit, [&](Cap, Accumulator, Distance) { ++count; }, budget);
		if (count == budget && result.first < limit) {
			Cap remaining = limit - result.first;
			std::pair<Cap, Accumulator> extra;
			if constexpr (fractional) extra = flow_detail::scaling_flow<Cap, Cost, Accumulator>(graph_, cost_, s, t, remaining);
			else {
				auto simplex = flow_detail::simplex_flow<Cap, Cost, Total>(graph_, cost_, s, t, remaining);
				extra = simplex ? *simplex : flow_detail::scaling_flow<Cap, Cost, Total>(graph_, cost_, s, t, remaining);
			}
			result.first += extra.first, result.second += extra.second;
			valid_ = false;
		}
		return result;
	}
	std::vector<std::pair<Cap, Total>> slope(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
		std::vector<std::pair<Cap, Total>> result{{Cap(0), Total(0)}};
		Distance previous = 0;
		bool first = true;
		run(s, t, limit, [&](Cap sent, Accumulator total, Distance price) {
			if (!first && previous == price) result.pop_back();
			result.emplace_back(sent, total);
			previous = price, first = false;
		});
		return result;
	}
};

#endif
