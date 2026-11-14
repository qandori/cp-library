#line 1 "test/min_cost_b_flow.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/min_cost_b_flow"
#line 1 "graph/min_cost_b_flow.hpp"



#line 1 "graph/min_cost_flow.hpp"



#line 1 "components/flow_graph.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace flow_detail {
template<class Cap> struct Graph {
	struct Arc { int to; Cap cap; };
	int n;
	std::vector<Arc> arcs;
	std::vector<int> start, order;
	bool built = false;
	explicit Graph(int n = 0) : n(n) { assert(n >= 0); }
	void reserve(std::size_t m) { assert(m <= std::size_t(std::numeric_limits<int>::max() / 2)); arcs.reserve(2 * m); }
	int add_edge(int from, int to, Cap cap) {
		assert(0 <= from && from < n && 0 <= to && to < n && cap >= 0);
		assert(arcs.size() <= std::size_t(std::numeric_limits<int>::max() - 2));
		int id = int(arcs.size());
		arcs.push_back({to, cap});
		arcs.push_back({from, Cap(0)});
		built = false;
		return id / 2;
	}
	void build() {
		if (built) return;
		start.assign(std::size_t(n) + 1, 0);
		for (std::size_t i = 0; i < arcs.size(); ++i) ++start[arcs[i ^ 1].to + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		order.resize(arcs.size());
		auto next = start;
		for (int i = 0; i < int(arcs.size()); ++i) order[next[arcs[i ^ 1].to]++] = i;
		built = true;
	}
	void reset_flow() {
		for (std::size_t i = 0; i < arcs.size(); i += 2) arcs[i].cap += arcs[i + 1].cap, arcs[i + 1].cap = 0;
	}
};
}


#line 1 "components/min_cost_circulation.hpp"



#line 1 "components/d_ary_heap.hpp"



#line 7 "components/d_ary_heap.hpp"
#include <functional>
#include <utility>
#line 10 "components/d_ary_heap.hpp"

template <class T, class Compare = std::less<T>, std::size_t D = 2>
class DaryHeap {
	static_assert(D >= 2);
	std::vector<T> data;
	[[no_unique_address]] Compare compare{};
	void up(std::size_t i, T value) {
		while (i) {
			std::size_t p = (i - 1) / D;
			if (!compare(data[p], value)) break;
			data[i] = std::move(data[p]);
			i = p;
		}
		data[i] = std::move(value);
	}
	void down(std::size_t i, T value) {
		std::size_t n = data.size();
		while (i < n / D + (n % D > 1)) {
			std::size_t best = i * D + 1, end = n;
			if constexpr (D <= std::size_t(-1) / 2) end = std::min(best + D, n);
			for (std::size_t j = best + 1; j < end; ++j)
				if (compare(data[best], data[j])) best = j;
			if (!compare(value, data[best])) break;
			data[i] = std::move(data[best]);
			i = best;
		}
		data[i] = std::move(value);
	}

  public:
	DaryHeap() = default;
	explicit DaryHeap(Compare compare) : compare(std::move(compare)) {}
	explicit DaryHeap(std::vector<T> values, Compare compare = {}) : data(std::move(values)), compare(std::move(compare)) {
		for (std::size_t i = data.size() / D + (data.size() % D > 1); i--;) down(i, std::move(data[i]));
	}
	bool empty() const { return data.empty(); }
	std::size_t size() const { return data.size(); }
	void reserve(std::size_t n) { data.reserve(n); }
	void clear() { data.clear(); }
	const std::vector<T> &values() const { return data; }
	decltype(auto) top() const {
		assert(!empty());
		return data.front();
	}
	void push(T value) {
		data.push_back(std::move(value));
		up(data.size() - 1, std::move(data.back()));
	}
	T replace_top(T value) {
		assert(!empty());
		T result = std::move(data.front());
		down(0, std::move(value));
		return result;
	}
	T push_pop(T value) {
		if (empty() || !compare(value, data.front())) return value;
		return replace_top(std::move(value));
	}
	T pop_value() {
		assert(!empty());
		T result = std::move(data.front());
		if (data.size() == 1) data.pop_back();
		else {
			T value = std::move(data.back());
			data.pop_back();
			down(0, std::move(value));
		}
		return result;
	}
	void pop() { pop_value(); }
	void swap(DaryHeap &other) noexcept(noexcept(std::swap(compare, other.compare))) {
		data.swap(other.data);
		std::swap(compare, other.compare);
	}
};


#line 6 "components/min_cost_circulation.hpp"
#include <bit>
#include <cstdint>
#line 9 "components/min_cost_circulation.hpp"
#include <type_traits>
#line 11 "components/min_cost_circulation.hpp"

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
		if (!(i & 1)) total += Total((Wide(graph.arcs[i].cap) - Wide(work.arcs[i].cap)) * Wide(costs[i]));
		graph.arcs[i].cap = work.arcs[i].cap;
	}
	return {work.arcs.back().cap, total};
}
}


#line 1 "components/min_cost_simplex.hpp"



#line 5 "components/min_cost_simplex.hpp"
#include <cmath>
#line 7 "components/min_cost_simplex.hpp"
#include <optional>

namespace flow_detail {
template<class Cap> class NetworkSimplex {
	using Cost = __int128_t;
	struct Edge { int to; Cap cap; Cost cost; };
	struct Parent { int vertex, edge; Cap up, down; };
	int n;
	std::vector<Edge> edges;
	std::vector<Cost> potential;
	std::uint64_t work = 0, budget;
	bool solve() {
		const int original_edge_count = int(edges.size());
		potential.assign(n + 1, Cost(0));
		Cost artificial_cost = 1;
		for (int edge = 0; edge < original_edge_count; edge += 2)
			artificial_cost += edges[edge].cost < 0 ? -edges[edge].cost : edges[edge].cost;

		std::vector<Parent> parent(n);
		edges.reserve(original_edge_count + 2 * n);

		for (int vertex = 0; vertex < n; ++vertex) {
			edges.push_back(Edge{n, 0, artificial_cost});
			edges.push_back(Edge{vertex, 0, -artificial_cost});
			potential[vertex] = -artificial_cost;
			parent[vertex] = Parent{n, int(edges.size()) - 2, 0, 0};
		}

		std::vector<int> depth(n + 1, 1);
		depth[n] = 0;
		std::vector<int> next(2 * (n + 1));
		std::vector<int> previous(2 * (n + 1));
		auto connect = [&](int first, int second) {
			next[first] = second;
			previous[second] = first;
		};
		for (int vertex = 0; vertex <= n; ++vertex) {
			connect(2 * vertex, 2 * vertex + 1);
		}
		for (int vertex = 0; vertex < n; ++vertex) {
			connect(2 * vertex + 1, next[2 * n]);
			connect(2 * n, 2 * vertex);
		}

		auto push_flow = [&](int entering_edge) {
			const int first = edges[entering_edge ^ 1].to;
			const int second = edges[entering_edge].to;
			const Cost cycle_cost =
				edges[entering_edge].cost
				+ potential[first] - potential[second];

			Cap amount = edges[entering_edge].cap;
			bool leave_first_side = true;
			int leaving_vertex = second;

			int first_ancestor = first;
			int second_ancestor = second;
			auto move_first_up = [&] {
				if (++work > budget) return false;
				if (parent[first_ancestor].down < amount) {
					amount = parent[first_ancestor].down;
					leaving_vertex = first_ancestor;
					leave_first_side = true;
				}
				first_ancestor = parent[first_ancestor].vertex;
				return true;
			};
			auto move_second_up = [&] {
				if (++work > budget) return false;
				if (parent[second_ancestor].up <= amount) {
					amount = parent[second_ancestor].up;
					leaving_vertex = second_ancestor;
					leave_first_side = false;
				}
				second_ancestor = parent[second_ancestor].vertex;
				return true;
			};
			if (depth[first_ancestor] >= depth[second_ancestor]) {
				int difference = depth[first_ancestor] - depth[second_ancestor];
				for (int i = 0; i < difference; ++i) if (!move_first_up()) return false;
			} else {
				int difference = depth[second_ancestor] - depth[first_ancestor];
				for (int i = 0; i < difference; ++i) if (!move_second_up()) return false;
			}
			while (first_ancestor != second_ancestor) {
				if (!move_first_up() || !move_second_up()) return false;
			}
			const int ancestor = first_ancestor;

			if (amount != Cap(0)) {
				int vertex = first;
				while (vertex != ancestor) {
					if (++work > budget) return false;
					parent[vertex].up += amount;
					parent[vertex].down -= amount;
					vertex = parent[vertex].vertex;
				}
				vertex = second;
				while (vertex != ancestor) {
					if (++work > budget) return false;
					parent[vertex].up -= amount;
					parent[vertex].down += amount;
					vertex = parent[vertex].vertex;
				}
			}

			int vertex = first;
			int new_parent = second;
			std::pair<Cap, Cap> parent_capacities{
				edges[entering_edge].cap - amount,
				edges[entering_edge ^ 1].cap + amount
			};
			Cost potential_difference = -cycle_cost;
			if (!leave_first_side) {
				std::swap(vertex, new_parent);
				std::swap(parent_capacities.first, parent_capacities.second);
				potential_difference = -potential_difference;
			}
			int parent_edge = entering_edge ^ (leave_first_side ? 0 : 1);

			while (new_parent != leaving_vertex) {
				if (++work > budget) return false;
				int new_depth = depth[new_parent];
				int tour_index = 2 * vertex;
				while (tour_index != 2 * vertex + 1) {
					if (++work > budget) return false;
					if ((tour_index & 1) == 0) {
						new_depth++;
						potential[tour_index / 2] += potential_difference;
						depth[tour_index / 2] = new_depth;
					} else {
						new_depth--;
					}
					tour_index = next[tour_index];
				}

				connect(previous[2 * vertex], next[2 * vertex + 1]);
				connect(2 * vertex + 1, next[2 * new_parent]);
				connect(2 * new_parent, 2 * vertex);

				std::swap(parent[vertex].edge, parent_edge);
				parent_edge ^= 1;
				std::swap(parent[vertex].up, parent_capacities.first);
				std::swap(parent[vertex].down, parent_capacities.second);
				std::swap(parent_capacities.first, parent_capacities.second);

				int old_parent = parent[vertex].vertex;
				parent[vertex].vertex = new_parent;
				new_parent = vertex;
				vertex = old_parent;
			}
			edges[parent_edge].cap = parent_capacities.first;
			edges[parent_edge ^ 1].cap = parent_capacities.second;
			return true;
		};

		const int candidate_limit = std::max(
			int(0.2 * std::sqrt(double(original_edge_count))), 10
		);
		const int minor_limit = std::max(candidate_limit / 10, 3);
		std::vector<int> candidates;
		candidates.reserve(candidate_limit);

		auto minor_pivot = [&] {
			Cost best_cost = Cost(0);
			int best_edge = -1;
			int index = 0;
			while (index < int(candidates.size())) {
				if (++work > budget) return false;
				int edge = candidates[index];
				if (edges[edge].cap == Cap(0)) {
					candidates[index] = candidates.back();
					candidates.pop_back();
					continue;
				}
				Cost reduced_cost =
					edges[edge].cost
					+ potential[edges[edge ^ 1].to]
					- potential[edges[edge].to];
				if (reduced_cost >= Cost(0)) {
					candidates[index] = candidates.back();
					candidates.pop_back();
					continue;
				}
				if (reduced_cost < best_cost) {
					best_cost = reduced_cost;
					best_edge = edge;
				}
				++index;
			}
			if (best_edge == -1) return false;
			return push_flow(best_edge);
		};

		int edge = 0;
		while (true) {
			for (int iteration = 0; iteration < minor_limit; ++iteration) {
				if (++work > budget) return false;
				if (!minor_pivot()) break;
			}

			if (work > budget) return false;
			Cost best_cost = Cost(0);
			int best_edge = -1;
			candidates.clear();
			for (int scanned = 0; scanned < int(edges.size()); ++scanned) {
				if (++work > budget) return false;
				if (edges[edge].cap != Cap(0)) {
					Cost reduced_cost =
						edges[edge].cost
						+ potential[edges[edge ^ 1].to]
						- potential[edges[edge].to];
					if (reduced_cost < Cost(0)) {
						if (reduced_cost < best_cost) {
							best_cost = reduced_cost;
							best_edge = edge;
						}
						candidates.push_back(edge);
						if (int(candidates.size()) == candidate_limit) break;
					}
				}
				++edge;
				if (edge == int(edges.size())) edge = 0;
			}
			if (candidates.empty()) break;
			if (!push_flow(best_edge)) return false;
		}

		for (int vertex = 0; vertex < n; ++vertex) {
			edges[parent[vertex].edge].cap = parent[vertex].up;
			edges[parent[vertex].edge ^ 1].cap = parent[vertex].down;
		}

		return true;
	}

public:
	template<class Cost>
	NetworkSimplex(const Graph<Cap>& graph, const std::vector<Cost>& costs, int s, int t, Cap limit, std::uint64_t budget)
		: n(graph.n), budget(budget) {
		edges.reserve(graph.arcs.size() + std::size_t(n) * 2 + 2);
		__int128_t maximum = 0;
		for (std::size_t i = 0; i < graph.arcs.size(); ++i) {
			edges.push_back({graph.arcs[i].to, graph.arcs[i].cap, __int128_t(costs[i])});
			maximum = std::max(maximum, costs[i] < 0 ? -__int128_t(costs[i]) : __int128_t(costs[i]));
		}
		edges.push_back({s, limit, -__int128_t(n) * maximum - 1});
		edges.push_back({t, 0, __int128_t(n) * maximum + 1});
	}
	template<class Cost, class Total>
	std::optional<std::pair<Cap, Total>> run(Graph<Cap>& graph, const std::vector<Cost>& costs) {
		if (!solve()) return std::nullopt;
		Total total = 0;
		for (std::size_t i = 0; i < graph.arcs.size(); i += 2) {
			total += Total((__int128_t(edges[i ^ 1].cap) - __int128_t(graph.arcs[i ^ 1].cap)) * __int128_t(costs[i]));
			graph.arcs[i].cap = edges[i].cap;
			graph.arcs[i ^ 1].cap = edges[i ^ 1].cap;
		}
		return std::pair<Cap, Total>{edges[graph.arcs.size() + 1].cap, total};
	}
};

template<class Cap, class Cost, class Total>
std::optional<std::pair<Cap, Total>> simplex_flow(Graph<Cap>& graph, const std::vector<Cost>& costs,
	int s, int t, Cap limit, std::uint64_t budget = 0) {
	if (graph.n > (std::numeric_limits<int>::max() - 2) / 2) return std::nullopt;
	if (graph.arcs.size() > std::size_t(std::numeric_limits<int>::max()) - std::size_t(graph.n) * 2 - 2) return std::nullopt;
	Cap upper = 0;
	for (int j = graph.start[s]; j < graph.start[s + 1]; ++j)
		upper += std::min<Cap>(limit - upper, graph.arcs[graph.order[j]].cap);
	if (!upper) return std::pair<Cap, Total>{0, 0};
	if (!budget) budget = std::max<std::uint64_t>(10000, std::min<std::uint64_t>(25000000, std::uint64_t(graph.n) * graph.arcs.size()) * 4);
	return NetworkSimplex<Cap>(graph, costs, s, t, upper, budget).template run<Cost, Total>(graph, costs);
}
}


#line 1 "components/RadixHeap.hpp"



#line 13 "components/RadixHeap.hpp"

template <class Key = std::uint64_t, class T = int>
class RadixHeap {
	static_assert(std::is_integral_v<Key> && !std::is_same_v<Key, bool> && sizeof(Key) <= 8);
	using U = std::make_unsigned_t<Key>;
	using State = std::pair<Key, T>;
	std::vector<State> bucket[std::numeric_limits<U>::digits + 1];
	Key lower;
	std::size_t count = 0;
	int index(Key key) const { return std::bit_width(U(U(key) ^ U(lower))); }
	void refill() {
		assert(count);
		if (!bucket[0].empty()) return;
		int i = 1;
		while (bucket[i].empty()) ++i;
		lower = bucket[i][0].first;
		for (const auto &s : bucket[i]) lower = std::min(lower, s.first);
		for (auto &s : bucket[i]) bucket[index(s.first)].push_back(std::move(s));
		bucket[i].clear();
	}

  public:
	explicit RadixHeap(Key minimum = 0) : lower(minimum) {}
	RadixHeap(const RadixHeap &) = default;
	RadixHeap &operator=(const RadixHeap &) = default;
	RadixHeap(RadixHeap &&other) noexcept : lower(other.lower), count(std::exchange(other.count, 0)) {
		for (int i = 0; i <= std::numeric_limits<U>::digits; ++i) bucket[i] = std::move(other.bucket[i]);
	}
	RadixHeap &operator=(RadixHeap &&other) noexcept {
		if (this != &other) {
			lower = other.lower;
			count = std::exchange(other.count, 0);
			for (int i = 0; i <= std::numeric_limits<U>::digits; ++i) bucket[i] = std::move(other.bucket[i]);
		}
		return *this;
	}

	bool empty() const { return count == 0; }
	std::size_t size() const { return count; }
	Key last() const { return lower; }
	void clear(Key minimum = 0) {
		for (auto &b : bucket) b.clear();
		lower = minimum;
		count = 0;
	}
	void push(State s) {
		assert(s.first >= lower);
		bucket[index(s.first)].push_back(std::move(s));
		++count;
	}
	const State &top() {
		refill();
		return bucket[0].back();
	}
	void pop() {
		refill();
		bucket[0].pop_back();
		--count;
	}
	State pop_value() {
		refill();
		State s = std::move(bucket[0].back());
		bucket[0].pop_back();
		--count;
		return s;
	}
};


#line 11 "graph/min_cost_flow.hpp"

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


#line 1 "components/flow_bounds.hpp"



#line 8 "components/flow_bounds.hpp"

namespace flow_detail {

template <class Cap>
struct CapacityBounds {
	static_assert(std::is_integral_v<Cap> && std::is_signed_v<Cap> && sizeof(Cap) <= 8);
	using Wide = __int128_t;
	using Residual = std::make_unsigned_t<Cap>;
	static Cap value(Wide amount) {
		assert(std::numeric_limits<Cap>::min() <= amount && amount <= std::numeric_limits<Cap>::max());
		return Cap(amount);
	}
	static Residual amount(Wide value) {
		assert(0 <= value && value <= Wide(std::numeric_limits<Residual>::max()));
		return Residual(value);
	}
	static Residual capacity(Cap lower, Cap upper) {
		assert(lower <= upper);
		return amount(Wide(upper) - Wide(lower));
	}
};

}


#line 7 "graph/min_cost_b_flow.hpp"

template<class Cap = std::int64_t, class Cost = std::int64_t, class Total = __int128_t>
class MinCostBFlow {
	static_assert(std::is_integral_v<Cap> && std::is_signed_v<Cap> && sizeof(Cap) <= 8);
	static_assert(std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8);
	static_assert(std::numeric_limits<Total>::is_integer && std::numeric_limits<Total>::is_signed);

  public:
	using capacity_type = Cap;
	using cost_type = Cost;
	using total_type = Total;
	struct Edge { int from, to; Cap cap, flow; Cost cost; Cap lower, upper; };
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
	struct StoredEdge { int from, to; Cap lower, upper; Cost cost; };
	std::vector<Cap> supply_;
	std::vector<StoredEdge> edges_;
	std::optional<Result> state_;
	std::vector<Wide> delta_;
	bool active_ = false;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void index([[maybe_unused]] int i) const { assert(0 <= i && i < edge_count()); }
	static Cap narrow(Wide value) {
		return Bounds::value(value);
	}
	static Residual capacity(Cap lower, Cap upper) {
		return Bounds::capacity(lower, upper);
	}
	std::optional<Result> solve_fallback(int s, int t, Cap lower, Cap upper, int objective,
		const std::vector<Wide>* values, const std::vector<Cap>* initial) const {
		int n = size(), m = edge_count();
		std::vector<Wide> balance(supply_.begin(), supply_.end());
		if (values) balance = *values;
		std::vector<Cost> costs;
		costs.reserve(2 * (std::size_t(m) + n + (s >= 0)));
		flow_detail::Graph<Residual> graph(n + 2);
		graph.reserve(std::size_t(m) + n + (s >= 0));
		auto insert = [&](int from, int to, Cap low, Cap high, Cost cost, Cap base) {
			Cap value = std::clamp(base, low, high);
			balance[from] -= Wide(value), balance[to] += Wide(value);
			int id = graph.add_edge(from, to, Bounds::amount(Wide(high) - Wide(value)));
			graph.arcs[2 * id + 1].cap = Bounds::amount(Wide(value) - Wide(low));
			costs.push_back(cost), costs.push_back(-cost);
		};
		for (int i = 0; i < m; ++i) {
			const auto& e = edges_[i];
			insert(e.from, e.to, e.lower, e.upper, objective ? Cost(0) : e.cost, initial ? (*initial)[i] : Cap(0));
		}
		if (s >= 0) insert(t, s, lower, upper, Cost(objective), Cap(0));
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
	std::optional<Result> solve_impl(int s, int t, Cap lower, Cap upper, int objective,
		const std::vector<Wide>* values = nullptr, const std::vector<Cap>* initial = nullptr) const {
		int n = size(), m = edge_count();
		if (s >= 0) {
			Wide left = lower, right = upper;
			for (int v : {s, t}) {
				Wide low = -(values ? (*values)[v] : Wide(supply_[v])), high = low;
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
		if (values) balance = *values;
		Wide sum = 0;
		for (Wide b : balance) sum += b;
		if (sum) return std::nullopt;
		MinCostFlow<Residual, Cost, Total> graph(n + 2);
		graph.reserve(std::size_t(m) + n + (s >= 0));
		using BaseTotal = std::conditional_t<std::is_integral_v<Total> || std::is_same_v<Total, Wide>, __uint128_t, Total>;
		BaseTotal base_cost = 0;
		auto insert = [&](const StoredEdge& e, Cost cost) {
			bool reverse = cost < 0;
			Cap base = reverse ? e.upper : e.lower;
			balance[e.from] -= Wide(base), balance[e.to] += Wide(base);
			base_cost += BaseTotal(Wide(base) * Wide(cost));
			graph.add_edge(reverse ? e.to : e.from, reverse ? e.from : e.to,
				capacity(e.lower, e.upper), reverse ? -cost : cost);
		};
		for (const auto& e : edges_) insert(e, objective ? Cost(0) : e.cost);
		if (s >= 0) insert(StoredEdge{t, s, lower, upper, Cost(objective)}, Cost(objective));
		Wide need = 0;
		const Wide maximum = std::numeric_limits<Residual>::max();
		for (int v = 0; v < n; ++v) {
			if (balance[v] < -maximum || maximum < balance[v]) return solve_fallback(s, t, lower, upper, objective, values, initial);
			if (balance[v] > 0) {
				need += balance[v];
				if (need > maximum) return solve_fallback(s, t, lower, upper, objective, values, initial);
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
		return solve(s, t, result->value);
	}

  public:
	explicit MinCostBFlow(int n = 0) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() - 2);
		supply_.assign(n, Cap(0));
	}
	MinCostBFlow(const MinCostBFlow& other) : supply_(other.supply_), edges_(other.edges_), active_(other.active_) {
		if (active_) state_ = other.state_, delta_ = other.delta_;
	}
	MinCostBFlow& operator=(const MinCostBFlow& other) {
		if (this != &other) {
			supply_ = other.supply_, edges_ = other.edges_, active_ = other.active_;
			if (active_) state_ = other.state_, delta_ = other.delta_;
		}
		return *this;
	}
	MinCostBFlow(MinCostBFlow&&) = default;
	MinCostBFlow& operator=(MinCostBFlow&&) = default;
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
		if (active_) {
			state_->flow.push_back(Cap(0));
			state_->potential.clear();
		}
		return id;
	}
	int add_edge(int from, int to, Cap upper, Cost cost) { return add_edge(from, to, Cap(0), upper, cost); }
	int add_edge_lower_bound(int from, int to, Cap lower, Cost cost) {
		return add_edge(from, to, lower, std::numeric_limits<Cap>::max(), cost);
	}
	Edge get_edge(int i) const {
		index(i);
		const auto& e = edges_[i];
		return {e.from, e.to, e.upper, active_ ? state_->flow[i] : Cap(0), e.cost, e.lower, e.upper};
	}
	std::vector<Edge> edges() const {
		std::vector<Edge> result;
		result.reserve(edges_.size());
		for (int i = 0; i < edge_count(); ++i) result.push_back(get_edge(i));
		return result;
	}
	void change_edge(int i, Cap lower, Cap upper, Cost cost) {
		index(i);
		capacity(lower, upper);
		assert(cost > std::numeric_limits<Cost>::min());
		edges_[i].lower = lower, edges_[i].upper = upper, edges_[i].cost = cost;
		reset_flow();
	}
	Cap supply(int v) const { vertex(v); return supply_[v]; }
	const std::vector<Cap>& supplies() const { return supply_; }
	void set_supply(int v, Cap amount) { vertex(v); supply_[v] = amount; reset_flow(); }
	void add_supply(int v, Cap amount) { vertex(v); supply_[v] = narrow(Wide(supply_[v]) + amount); reset_flow(); }
	void reset_supply() { std::fill(supply_.begin(), supply_.end(), Cap(0)); reset_flow(); }
	void reset_flow() { active_ = false; }
	const std::vector<Cost>& potentials() const {
		static const std::vector<Cost> empty;
		return active_ ? state_->potential : empty;
	}
	void clear() { edges_.clear(); reset_supply(); }
	std::optional<Result> solve() const { return solve_impl(-1, -1, 0, 0, 0); }
	std::optional<Result> solve(int s, int t, Cap amount) const { return solve(s, t, amount, amount); }
	std::optional<Result> solve(int s, int t, Cap lower, Cap upper) const {
		vertex(s), vertex(t);
		assert(s != t);
		assert(lower <= upper);
		return solve_impl(s, t, lower, upper, 0);
	}
	std::optional<std::pair<Cap, Total>> flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
		vertex(s), vertex(t);
		assert(s != t && limit >= 0);
		std::vector<Wide> values;
		if (active_) {
			values.assign(supply_.begin(), supply_.end());
			for (int v = 0; v < size(); ++v) values[v] += delta_[v];
		}
		const auto* balance = active_ ? &values : nullptr;
		const auto* initial = active_ ? &state_->flow : nullptr;
		auto maximum = solve_impl(s, t, Cap(0), limit, -1, balance, initial);
		if (!maximum) return std::nullopt;
		auto result = solve_impl(s, t, maximum->value, maximum->value, 0, balance, initial);
		assert(result);
		Cap sent = result->value;
		Total cost = result->cost - (active_ ? state_->cost : Total(0));
		if (!active_) delta_.assign(size(), Wide(0));
		delta_[s] += Wide(sent), delta_[t] -= Wide(sent);
		state_ = std::move(result);
		active_ = true;
		return std::pair<Cap, Total>{sent, cost};
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


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/min_cost_b_flow.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	MinCostBFlow<> graph(n);
	graph.reserve(m);
	for (int v = 0; v < n; ++v) {
		std::int64_t supply = 0;
		input.read(supply);
		graph.set_supply(v, supply);
	}
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t lower = 0, upper = 0, cost = 0;
		input.read(u), input.read(v), input.read(lower), input.read(upper), input.read(cost);
		graph.add_edge(u, v, lower, upper, cost);
	}
	auto result = graph.solve();
	if (!result) { output.println("infeasible"); return 0; }
	output.println(result->cost);
	for (auto potential : result->potential) output.println(potential);
	for (auto flow : result->flow) output.println(flow);
}
