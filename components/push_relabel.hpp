#ifndef PUSH_RELABEL_HPP
#define PUSH_RELABEL_HPP

#include "flow_graph.hpp"
#include <cstdint>

namespace flow_detail {
template<class Cap> Cap push_relabel(Graph<Cap>& graph, int s, int t) {
	graph.build();
	int n = graph.n;
	assert(n <= std::numeric_limits<int>::max() / 2 - 1);
	int dead = 2 * n;
	std::vector<__uint128_t> excess(n);
	std::vector<int> height(n), current(n), count(std::size_t(dead) + 1), head(std::size_t(dead) + 1, -1), next(n), queue(n);
	std::vector<unsigned char> active(n);
	int highest = -1;
	std::size_t work = 0, threshold = std::max<std::size_t>(1, 4 * graph.arcs.size() + n);
	auto activate = [&](int v) {
		if (v == s || v == t || active[v] || !excess[v] || height[v] == dead) return;
		active[v] = 1;
		next[v] = head[height[v]], head[height[v]] = v;
		highest = std::max(highest, height[v]);
	};
	auto buckets = [&] {
		std::fill(head.begin(), head.end(), -1);
		std::fill(active.begin(), active.end(), 0);
		highest = -1;
		for (int v = 0; v < n; ++v) activate(v);
	};
	auto global = [&] {
		std::fill(height.begin(), height.end(), n + 1);
		std::fill(count.begin(), count.end(), 0);
		std::copy_n(graph.start.begin(), n, current.begin());
		height[s] = n, height[t] = 0;
		int first = 0, last = 0;
		queue[last++] = t;
		while (first < last) {
			int v = queue[first++];
			for (int j = graph.start[v]; j < graph.start[v + 1]; ++j) {
				int i = graph.order[j], u = graph.arcs[i].to;
				if (u != s && height[u] == n + 1 && graph.arcs[i ^ 1].cap) height[u] = height[v] + 1, queue[last++] = u;
			}
		}
		for (int h : height) ++count[h];
		buckets();
		work = 0;
	};
	for (int j = graph.start[s]; j < graph.start[s + 1]; ++j) {
		int i = graph.order[j];
		auto& e = graph.arcs[i];
		if (e.to == s || !e.cap) continue;
		excess[e.to] += e.cap, graph.arcs[i ^ 1].cap += e.cap, e.cap = 0;
	}
	global();
	while (highest >= 0) {
		if (head[highest] < 0) { --highest; continue; }
		int v = head[highest];
		head[highest] = next[v];
		if (!active[v] || height[v] != highest) continue;
		active[v] = 0;
		while (excess[v] && height[v] < dead) {
			if (current[v] == graph.start[v + 1]) {
				int old = height[v], value = dead, best = graph.start[v];
				for (int j = graph.start[v]; j < graph.start[v + 1]; ++j) {
					const auto& e = graph.arcs[graph.order[j]];
					if (e.cap && height[e.to] + 1 < value) value = height[e.to] + 1, best = j;
				}
				work += graph.start[v + 1] - graph.start[v];
				--count[old], ++count[value], height[v] = value, current[v] = graph.start[v];
				if (old < n && !count[old]) {
					for (int u = 0; u < n; ++u) if (old < height[u] && height[u] < n) {
						--count[height[u]], height[u] = n + 1, ++count[n + 1], current[u] = graph.start[u];
					}
					buckets();
					break;
				}
				if (value < dead) work += best - graph.start[v], current[v] = best;
			} else {
				++work;
				int i = graph.order[current[v]];
				auto& e = graph.arcs[i];
				if (!e.cap || height[v] != height[e.to] + 1) { ++current[v]; continue; }
				Cap amount = Cap(std::min(excess[v], __uint128_t(e.cap)));
				e.cap -= amount, graph.arcs[i ^ 1].cap += amount;
				excess[v] -= amount, excess[e.to] += amount;
				activate(e.to);
			}
		}
		activate(v);
		if (work >= threshold) global();
	}
	assert(excess[t] <= __uint128_t(std::numeric_limits<Cap>::max()));
	return Cap(excess[t]);
}
}

#endif
