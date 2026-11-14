#ifndef MAX_FLOW_HPP
#define MAX_FLOW_HPP

#include "../components/flow_graph.hpp"
#include "../components/push_relabel.hpp"
#include <cstdint>
#include <type_traits>

template<class Cap = std::int64_t> class MaxFlow {
	static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool> && sizeof(Cap) <= 8);
	flow_detail::Graph<Cap> graph_;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < graph_.n); }
	int index(int i) const { assert(0 <= i && std::size_t(i) < graph_.arcs.size() / 2); return 2 * i; }

  public:
	using capacity_type = Cap;
	struct Edge { int from, to; Cap cap, flow; };
	explicit MaxFlow(int n = 0) : graph_(n) {}
	int size() const { return graph_.n; }
	int edge_count() const { return int(graph_.arcs.size() / 2); }
	void reserve(std::size_t m) { graph_.reserve(m); }
	int add_edge(int from, int to, Cap cap) { return graph_.add_edge(from, to, cap); }
	Edge get_edge(int i) const {
		int j = index(i);
		const auto& a = graph_.arcs[j];
		const auto& b = graph_.arcs[j ^ 1];
		return {b.to, a.to, Cap(a.cap + b.cap), b.cap};
	}
	std::vector<Edge> edges() const {
		std::vector<Edge> result;
		result.reserve(edge_count());
		for (int i = 0; i < edge_count(); ++i) result.push_back(get_edge(i));
		return result;
	}
	void change_edge(int i, Cap cap, Cap flow) {
		assert(0 <= flow && flow <= cap);
		int j = index(i);
		graph_.arcs[j].cap = cap - flow, graph_.arcs[j ^ 1].cap = flow;
	}
	void reset_flow() { graph_.reset_flow(); }
	Cap flow(int s, int t) {
		vertex(s), vertex(t);
		assert(s != t);
		return flow_detail::push_relabel(graph_, s, t);
	}
	Cap flow(int s, int t, Cap limit) {
		vertex(s), vertex(t);
		assert(s != t && limit >= 0);
		graph_.build();
		auto& g = graph_;
		std::vector<int> level(g.n), iter(g.n), queue(g.n);
		struct Frame { int v; Cap limit, sent; };
		std::vector<Frame> stack;
		stack.reserve(g.n);
		Cap total = 0;
		while (total < limit) {
			std::fill(level.begin(), level.end(), -1);
			level[s] = 0;
			int head = 0, tail = 0;
			queue[tail++] = s;
			while (head < tail && level[t] < 0) {
				int v = queue[head++];
				for (int i = g.start[v]; i < g.start[v + 1]; ++i) {
					const auto& e = g.arcs[g.order[i]];
					if (!e.cap || level[e.to] >= 0) continue;
					level[e.to] = level[v] + 1;
					queue[tail++] = e.to;
				}
			}
			if (level[t] < 0) break;
			std::copy_n(g.start.begin(), g.n, iter.begin());
			stack.push_back({t, Cap(limit - total), Cap(0)});
			while (!stack.empty()) {
				auto& frame = stack.back();
				int v = frame.v;
				if (v == s || frame.sent == frame.limit || iter[v] == g.start[v + 1]) {
					Cap sent = v == s ? frame.limit : frame.sent;
					if (v != s && sent < frame.limit) level[v] = g.n;
					stack.pop_back();
					if (stack.empty()) { total += sent; break; }
					auto& parent = stack.back();
					int id = g.order[iter[parent.v]];
					g.arcs[id].cap += sent, g.arcs[id ^ 1].cap -= sent;
					parent.sent += sent;
					if (parent.sent < parent.limit) ++iter[parent.v];
				} else {
					int id = g.order[iter[v]];
					const auto& e = g.arcs[id];
					if (level[e.to] < 0 || level[e.to] >= level[v] || !g.arcs[id ^ 1].cap) ++iter[v];
					else stack.push_back({e.to, std::min<Cap>(frame.limit - frame.sent, g.arcs[id ^ 1].cap), Cap(0)});
				}
			}
		}
		return total;
	}
	std::vector<bool> min_cut(int s) {
		vertex(s);
		graph_.build();
		std::vector<bool> visited(graph_.n);
		std::vector<int> queue{ s };
		visited[s] = true;
		for (std::size_t head = 0; head < queue.size(); ++head) {
			int v = queue[head];
			for (int i = graph_.start[v]; i < graph_.start[v + 1]; ++i) {
				const auto& e = graph_.arcs[graph_.order[i]];
				if (e.cap && !visited[e.to]) visited[e.to] = true, queue.push_back(e.to);
			}
		}
		return visited;
	}
};

#endif
