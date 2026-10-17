#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "../components/RadixHeap.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

inline constexpr std::int64_t DIJKSTRA_INF = 1'000'000'000'000'000'000;

template <class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = static_cast<T>(std::min<std::int64_t>(DIJKSTRA_INF, std::numeric_limits<T>::max()))) {
	assert(g.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < g.size());
	assert(inf > 0);
	std::vector<T> d(g.size(), inf);
	d[st] = 0;
	using State = std::pair<T, int>;
	constexpr bool radix = std::is_integral_v<T> && sizeof(T) <= 8;
	std::conditional_t<radix, RadixHeap<T>, std::vector<State>> pq;
	auto push = [&](T cost, int v) {
		if constexpr (radix) pq.push({cost, v});
		else {
			pq.emplace_back(std::move(cost), v);
			std::push_heap(pq.begin(), pq.end(), std::greater<State>());
		}
	};
	push(0, st);
	while (!pq.empty()) {
		const auto [cost, v] = [&] {
			if constexpr (radix) return pq.pop_value();
			else {
				std::pop_heap(pq.begin(), pq.end(), std::greater<State>());
				State s = std::move(pq.back());
				pq.pop_back();
				return s;
			}
		}();
		if (d[v] != cost) continue;
		T allowance = inf - cost;
		for (const auto &[to, c] : g[v]) {
			assert(to >= 0 && static_cast<std::size_t>(to) < g.size());
			assert(c >= 0);
			if (c >= allowance) continue;
			T next = cost + c;
			if (next < d[to]) d[to] = next, push(std::move(next), to);
		}
	}
	for (T &val : d)
		if (val == inf) val = -1;
	return d;
}

#endif
