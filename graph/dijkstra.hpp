#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>

inline constexpr std::int64_t DIJKSTRA_INF = 1'000'000'000'000'000'000;

namespace dijkstra_detail {
template <class T>
struct RadixHeap {
	using State = std::pair<T, int>;
	std::vector<State> bucket[65];
	std::uint64_t last = 0;
	std::size_t count = 0;
	int index(T cost) const {
		const std::uint64_t x = static_cast<std::uint64_t>(cost) ^ last;
		return x ? 64 - __builtin_clzll(x) : 0;
	}
	bool empty() const { return count == 0; }
	void push(State s) {
		bucket[index(s.first)].push_back(s);
		++count;
	}
	State top() {
		if (bucket[0].empty()) {
			int i = 1;
			while (bucket[i].empty()) ++i;
			last = static_cast<std::uint64_t>(bucket[i][0].first);
			for (const auto &[cost, v] : bucket[i]) last = std::min(last, static_cast<std::uint64_t>(cost));
			for (const auto &s : bucket[i]) bucket[index(s.first)].push_back(s);
			bucket[i].clear();
		}
		return bucket[0].back();
	}
	void pop() {
		bucket[0].pop_back();
		--count;
	}
};
} // namespace dijkstra_detail

template <class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = static_cast<T>(std::min<std::int64_t>(DIJKSTRA_INF, std::numeric_limits<T>::max()))) {
	assert(g.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < g.size());
	assert(inf > 0);
	std::vector<T> d(g.size(), inf);
	d[st] = 0;
	using State = std::pair<T, int>;
	std::conditional_t<std::is_integral_v<T> && sizeof(T) <= 8, dijkstra_detail::RadixHeap<T>, std::priority_queue<State, std::vector<State>, std::greater<State>>> pq;
	pq.push({0, st});
	while (!pq.empty()) {
		const auto [cost, v] = pq.top();
		pq.pop();
		if (d[v] != cost) continue;
		for (const auto &[to, c] : g[v]) {
			assert(to >= 0 && static_cast<std::size_t>(to) < g.size());
			assert(c >= 0);
			if (c >= inf - cost) continue;
			T next = cost + c;
			if (next < d[to]) d[to] = next, pq.push({next, to});
		}
	}
	for (T &val : d)
		if (val == inf) val = -1;
	return d;
}

#endif
