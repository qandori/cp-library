#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "../template.hpp"

inline constexpr int64_t DIJKSTRA_INF = 1'000'000'000'000'000'000;

template <class T>
vec<T> dijkstra(const vec<vec<pair<int, T>>> &g, int st, T inf = static_cast<T>(min<int64_t>(DIJKSTRA_INF, numeric_limits<T>::max()))) {
	assert(g.size() <= static_cast<size_t>(numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<size_t>(st) < g.size());
	assert(inf > 0);
	vec<T> d(g.size(), inf);
	d[st] = 0;
	using State = pair<T, int>;
	priority_queue<State, vec<State>, greater<State>> pq;
	pq.push({0, st});
	while (!pq.empty()) {
		const auto [cost, v] = pq.top();
		pq.pop();
		if (d[v] != cost) continue;
		for (const auto &[to, c] : g[v]) {
			assert(to >= 0 && static_cast<size_t>(to) < g.size());
			assert(c >= 0);
			if (c >= inf - cost) continue;
			if (chmin(d[to], cost + c)) pq.push({d[to], to});
		}
	}
	for (T &val : d)
		if (val == inf) val = -1;
	return d;
}

#endif
