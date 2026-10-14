#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "../template.hpp"

inline constexpr int64_t DIJKSTRA_INF = 1'000'000'000'000'000'000;

namespace dijkstra_detail {
template <class T>
struct RadixHeap {
	using State = pair<T, int>;
	vec<State> bucket[65];
	uint64_t last = 0;
	size_t count = 0;
	int index(T cost) const {
		const uint64_t x = static_cast<uint64_t>(cost) ^ last;
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
			last = static_cast<uint64_t>(bucket[i][0].first);
			for (const auto &[cost, v] : bucket[i]) chmin(last, static_cast<uint64_t>(cost));
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
vec<T> dijkstra(const vec<vec<pair<int, T>>> &g, int st, T inf = static_cast<T>(min<int64_t>(DIJKSTRA_INF, numeric_limits<T>::max()))) {
	assert(g.size() <= static_cast<size_t>(numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<size_t>(st) < g.size());
	assert(inf > 0);
	vec<T> d(g.size(), inf);
	d[st] = 0;
	using State = pair<T, int>;
	conditional_t<is_integral_v<T> && sizeof(T) <= 8, dijkstra_detail::RadixHeap<T>, priority_queue<State, vec<State>, greater<State>>> pq;
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
