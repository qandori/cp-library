#ifndef KRUSKAL_HPP
#define KRUSKAL_HPP

#include "../data_structure/union_find.hpp"
#include "../components/radix_sort.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

template<class Cost = std::int64_t>
class Kruskal {
	static_assert((std::numeric_limits<Cost>::is_integer || std::is_floating_point_v<Cost>) && !std::is_same_v<Cost, bool>);
	int n_;

  public:
	struct Edge { int from, to; Cost cost; };
	struct Result {
		Cost cost = 0;
		std::vector<int> edges;
		int components = 0;
		bool connected() const { return components <= 1; }
	};

  private:
	std::vector<Edge> edges_;
	static void add_cost(Cost &sum, Cost value) {
		if constexpr (std::numeric_limits<Cost>::is_integer) {
#if defined(__GNUC__) || defined(__clang__)
			Cost next;
			if (__builtin_add_overflow(sum, value, &next)) throw std::overflow_error("Kruskal cost");
			sum = next;
#else
			if ((value > 0 && sum > std::numeric_limits<Cost>::max() - value) ||
				(std::numeric_limits<Cost>::is_signed && value < 0 && sum < std::numeric_limits<Cost>::lowest() - value))
				throw std::overflow_error("Kruskal cost");
			sum += value;
#endif
		} else sum += value;
	}

  public:
	explicit Kruskal(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max()));
		edges_.reserve(m);
	}
	int add_edge(int u, int v, Cost cost) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		if constexpr (std::is_floating_point_v<Cost>) assert(cost == cost);
		int id = edge_count();
		edges_.push_back({u, v, cost});
		return id;
	}
	Edge get_edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	Result solve() const {
		std::vector<int> order;
		order.reserve(edges_.size());
		for (int id = 0; id < edge_count(); ++id) if (edges_[id].from != edges_[id].to) order.push_back(id);
		auto compare = [&](int a, int b) {
			if (edges_[a].cost < edges_[b].cost) return true;
			if (edges_[b].cost < edges_[a].cost) return false;
			return a < b;
		};
		if constexpr (std::is_integral_v<Cost> && sizeof(Cost) <= sizeof(std::uint64_t)) {
			if (order.size() < 256) std::sort(order.begin(), order.end(), compare);
			else if (std::is_sorted(order.begin(), order.end(), [&](int a, int b) { return edges_[b].cost < edges_[a].cost; })) {
				std::reverse(order.begin(), order.end());
				for (std::size_t first = 0; first < order.size();) {
					std::size_t last = first + 1;
					while (last < order.size() && edges_[order[first]].cost == edges_[order[last]].cost) ++last;
					std::reverse(order.begin() + first, order.begin() + last);
					first = last;
				}
			} else {
				using Key = std::make_unsigned_t<Cost>;
				stable_radix_sort(order, [&](int id) {
					Key key = static_cast<Key>(edges_[id].cost);
					if constexpr (std::is_signed_v<Cost>) key ^= Key(1) << (std::numeric_limits<Key>::digits - 1);
					return key;
				});
			}
		} else std::sort(order.begin(), order.end(), compare);
		UnionFind uf(n_);
		Result result;
		result.edges.reserve(std::min(order.size(), std::size_t(n_ ? n_ - 1 : 0)));
		for (int id : order) {
			const auto &e = edges_[id];
			int u = uf.leader(e.from), v = uf.leader(e.to);
			if (u == v) continue;
			add_cost(result.cost, e.cost);
			uf.merge(u, v);
			result.edges.push_back(id);
			if (uf.count() == 1) break;
		}
		result.components = uf.count();
		return result;
	}
};

#endif
