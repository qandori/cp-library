#ifndef MIN_COST_CIRCULATION_HPP
#define MIN_COST_CIRCULATION_HPP

#include "flow_graph.hpp"
#include "d_ary_heap.hpp"
#include <bit>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace flow_detail {
template<class Cap, class Cost, class Total>
std::pair<Cap, Total> scaling_flow(Graph<Cap>& graph, const std::vector<Cost>& costs, int s, int t, Cap limit) {
	using Wide = __int128_t;
	using State = std::pair<Wide, int>;
	using Unsigned = std::make_unsigned_t<Cap>;
	Cap upper = 0;
	for (int j = graph.start[s]; j < graph.start[s + 1]; ++j) upper += std::min<Cap>(limit - upper, graph.arcs[graph.order[j]].cap);
	limit = upper;
	if (!limit) return {Cap(0), Total(0)};
	Graph<Cap> work = graph;
	std::vector<Cap> capacity;
	capacity.reserve(work.arcs.size() + 2);
	Unsigned maximum = 0;
	for (auto& e : work.arcs) capacity.push_back(e.cap), maximum = std::max(maximum, Unsigned(e.cap)), e.cap = 0;
	std::vector<Wide> cost(costs.begin(), costs.end());
	Wide largest = 0;
	for (Cost c : costs) largest = std::max(largest, c < 0 ? -Wide(c) : Wide(c));
	work.add_edge(t, s, 0);
	capacity.push_back(limit), capacity.push_back(0);
	cost.push_back(-Wide(graph.n) * largest - 1), cost.push_back(-cost.back());
	maximum = std::max(maximum, Unsigned(limit));
	work.build();
	int n = graph.n;
	std::vector<Wide> potential(n), distance(n);
	std::vector<std::int64_t> excess(n);
	std::vector<int> parent(n), settled, zero;
	std::vector<unsigned char> visited(n);
	DaryHeap<State, std::greater<State>, 4> heap;
	heap.reserve(work.arcs.size());
	settled.reserve(n), zero.reserve(n);
	const Wide inf = std::numeric_limits<Wide>::max();
	for (int bit = int(std::bit_width(maximum)); bit--;) {
		for (std::size_t i = 0; i < capacity.size(); ++i) work.arcs[i].cap = work.arcs[i].cap * 2 + ((Unsigned(capacity[i]) >> bit) & 1);
		std::fill(excess.begin(), excess.end(), 0);
		for (int i = 0; i < int(work.arcs.size()); ++i) {
			auto& e = work.arcs[i];
			int from = work.arcs[i ^ 1].to;
			if (e.cap && cost[i] + potential[from] - potential[e.to] < 0) {
				auto amount = e.cap;
				assert(amount == 1);
				e.cap = 0, work.arcs[i ^ 1].cap += amount;
				--excess[from], ++excess[e.to];
			}
		}
		for (int source = 0; source < n; ++source) while (excess[source] > 0) {
			std::fill(distance.begin(), distance.end(), inf);
			std::fill(visited.begin(), visited.end(), 0);
			heap.clear(), settled.clear(), zero.clear();
			distance[source] = 0;
			zero.push_back(source);
			int target = -1;
			while (!zero.empty() || !heap.empty()) {
				int v;
				if (!zero.empty()) v = zero.back(), zero.pop_back();
				else {
					auto [d, u] = heap.pop_value();
					if (distance[u] != d) continue;
					v = u;
				}
				if (visited[v]) continue;
				visited[v] = 1, settled.push_back(v);
				if (excess[v] < 0) { target = v; break; }
				for (int j = work.start[v]; j < work.start[v + 1]; ++j) {
					int i = work.order[j];
					const auto& e = work.arcs[i];
					if (!e.cap || visited[e.to]) continue;
					Wide reduced = cost[i] + potential[v] - potential[e.to];
					assert(reduced >= 0);
					Wide next = distance[v] + reduced;
					if (next >= distance[e.to]) continue;
					distance[e.to] = next, parent[e.to] = i;
					if (!reduced) zero.push_back(e.to);
					else heap.push({next, e.to});
				}
			}
			assert(target >= 0);
			for (int v : settled) potential[v] -= distance[target] - distance[v];
			std::int64_t amount = std::min(excess[source], -excess[target]);
			for (int v = target; v != source; v = work.arcs[parent[v] ^ 1].to) {
				auto cap = work.arcs[parent[v]].cap;
				if (Unsigned(cap) < std::uint64_t(amount)) amount = std::int64_t(cap);
			}
			for (int v = target; v != source; v = work.arcs[parent[v] ^ 1].to) {
				int i = parent[v];
				work.arcs[i].cap -= Cap(amount), work.arcs[i ^ 1].cap += Cap(amount);
			}
			excess[source] -= amount, excess[target] += amount;
		}
	}
	Total total = 0;
	for (std::size_t i = 0; i < graph.arcs.size(); ++i) {
		if (!(i & 1)) total += (Total(graph.arcs[i].cap) - Total(work.arcs[i].cap)) * Total(costs[i]);
		graph.arcs[i].cap = work.arcs[i].cap;
	}
	return {work.arcs.back().cap, total};
}
}

#endif
