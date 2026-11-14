#ifndef MIN_COST_FLOW_HPP
#define MIN_COST_FLOW_HPP

#include "../components/flow_graph.hpp"
#include "../components/min_cost_circulation.hpp"
#include "../components/min_cost_simplex.hpp"
#include "../components/RadixHeap.hpp"
#include <cstdint>
#include <type_traits>
#include <utility>

template<class Cap = std::int64_t, class Cost = std::int64_t, class Total = __int128_t>
class MinCostFlow {
	static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool> && sizeof(Cap) <= 8);
	static_assert(std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8);
	static_assert(std::numeric_limits<Total>::is_integer && std::numeric_limits<Total>::is_signed);
	flow_detail::Graph<Cap> graph_;
	std::vector<Cost> cost_, potential_;
	bool valid_ = false;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < graph_.n); }
	int index(int i) const { assert(0 <= i && std::size_t(i) < graph_.arcs.size() / 2); return 2 * i; }
	void initialize() {
		if (valid_) return;
		potential_.assign(graph_.n, Cost(0));
		bool changed = false;
		for (int round = 0; round < graph_.n; ++round) {
			changed = false;
			for (int i = 0; i < int(cost_.size()); ++i) {
				if (!graph_.arcs[i].cap) continue;
				int from = graph_.arcs[i ^ 1].to, to = graph_.arcs[i].to;
				Cost next = potential_[from] + cost_[i];
				if (next < potential_[to]) potential_[to] = next, changed = true;
			}
			if (!changed) break;
		}
		assert(!changed);
		valid_ = true;
	}
	template<class Report> std::pair<Cap, Total> run(int s, int t, Cap limit, Report report, int rounds = -1) {
		vertex(s), vertex(t);
		assert(s != t && limit >= 0);
		graph_.build();
		initialize();
		auto& g = graph_;
		const Cost inf = std::numeric_limits<Cost>::max();
		std::vector<Cost> distance(g.n);
		std::vector<int> parent(g.n), zero, settled;
		std::vector<unsigned char> visited(g.n);
		zero.reserve(g.n), settled.reserve(g.n);
		RadixHeap<Cost> heap;
		Cap sent = 0;
		Total total = 0;
		while (sent < limit && rounds != 0) {
			std::fill(distance.begin(), distance.end(), inf);
			std::fill(visited.begin(), visited.end(), 0);
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
				if (visited[v]) continue;
				visited[v] = 1;
				settled.push_back(v);
				if (v == t) break;
				for (int j = g.start[v]; j < g.start[v + 1]; ++j) {
					int i = g.order[j];
					const auto& e = g.arcs[i];
					if (!e.cap || visited[e.to]) continue;
					Cost reduced = cost_[i] + potential_[v] - potential_[e.to];
					assert(reduced >= 0);
					if (reduced >= inf - distance[v]) continue;
					Cost next = distance[v] + reduced;
					if (next >= distance[e.to]) continue;
					distance[e.to] = next, parent[e.to] = i;
					if (!reduced) zero.push_back(e.to);
					else heap.push({next, e.to});
				}
			}
			if (!visited[t]) break;
			for (int v : settled) potential_[v] -= distance[t] - distance[v];
			Cap amount = limit - sent;
			for (int v = t; v != s; v = g.arcs[parent[v] ^ 1].to) amount = std::min(amount, g.arcs[parent[v]].cap);
			for (int v = t; v != s; v = g.arcs[parent[v] ^ 1].to) {
				int i = parent[v];
				g.arcs[i].cap -= amount, g.arcs[i ^ 1].cap += amount;
			}
			Cost price = potential_[t] - potential_[s];
			sent += amount, total += Total(amount) * Total(price);
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
		assert(cost > std::numeric_limits<Cost>::min());
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
	void reset_flow() { graph_.reset_flow(); valid_ = false; }
	const std::vector<Cost>& potentials() { initialize(); return potential_; }
	std::pair<Cap, Total> flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
		int count = 0, budget = 8;
		auto result = run(s, t, limit, [&](Cap, Total, Cost) { ++count; }, budget);
		if (count == budget && result.first < limit) {
			auto extra = flow_detail::simplex_flow<Cap, Cost, Total>(graph_, cost_, s, t, limit - result.first);
			if (!extra) extra = flow_detail::scaling_flow<Cap, Cost, Total>(graph_, cost_, s, t, limit - result.first);
			result.first += extra->first, result.second += extra->second;
			valid_ = false;
		}
		return result;
	}
	std::vector<std::pair<Cap, Total>> slope(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
		std::vector<std::pair<Cap, Total>> result{{Cap(0), Total(0)}};
		Cost previous = 0;
		bool first = true;
		run(s, t, limit, [&](Cap sent, Total total, Cost price) {
			if (!first && previous == price) result.pop_back();
			result.emplace_back(sent, total);
			previous = price, first = false;
		});
		return result;
	}
};

#endif
