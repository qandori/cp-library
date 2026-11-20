#ifndef TWO_EDGE_CONNECTED_COMPONENTS_HPP
#define TWO_EDGE_CONNECTED_COMPONENTS_HPP

#include "../components/default_init_allocator.hpp"
#include "../components/undirected_csr.hpp"
#include <algorithm>

class TwoEdgeConnectedComponents {
	struct Frame { int v, parent, next; };
	bool built_ = false;
	std::vector<int, DefaultInitAllocator<int>> vertices_;
	std::vector<int> boundaries_;
	void check_([[maybe_unused]] int v) const { assert(built_ && 0 <= v && v < n); }

  public:
	using Arc = UndirectedCSR::Arc;
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> component, bridges;
	explicit TwoEdgeConnectedComponents(int n = 0) : n(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		assert(edges.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		built_ = false;
		int id = static_cast<int>(edges.size());
		edges.emplace_back(u, v);
		return id;
	}
	void build() {
		built_ = false;
		UndirectedCSR graph(n, edges);
		std::vector<int> ord(n, -1);
		std::vector<Frame> stack;
		stack.reserve(n);
		vertices_.resize(n), component.resize(n), bridges.clear();
		bridges.reserve(std::min(edges.size(), std::size_t(n ? n - 1 : 0)));
		boundaries_.reserve(std::size_t(n) + 1);
		boundaries_.clear(), boundaries_.push_back(n);
		int timer = 0, top = 0, end = n;
		for (int root = 0; root < n; ++root) if (ord[root] == -1) {
			ord[root] = component[root] = timer++;
			vertices_[top++] = root;
			stack.push_back({root, -1, graph.start[root]});
			while (!stack.empty()) {
				auto &frame = stack.back();
				int v = frame.v;
				if (frame.next == graph.start[v + 1]) {
					int parent = frame.parent;
					stack.pop_back();
					if (component[v] == ord[v]) {
						int id = static_cast<int>(boundaries_.size()) - 1, u;
						do {
							u = vertices_[--top];
							component[u] = id, vertices_[--end] = u;
						} while (u != v);
						boundaries_.push_back(end);
						if (parent != -1) bridges.push_back(parent);
					} else {
						int p = stack.back().v;
						component[p] = std::min(component[p], component[v]);
					}
					continue;
				}
				auto [to, id] = graph.adj[frame.next++];
				if (id == frame.parent) continue;
				if (ord[to] != -1) component[v] = std::min(component[v], ord[to]);
				else {
					ord[to] = component[to] = timer++;
					vertices_[top++] = to;
					stack.push_back({to, id, graph.start[to]});
				}
			}
		}
		built_ = true;
	}
	int component_count() const { assert(built_); return static_cast<int>(boundaries_.size()) - 1; }
	bool same(int u, int v) const { check_(u), check_(v); return component[u] == component[v]; }
	bool is_bridge(int id) const {
		assert(built_ && 0 <= id && id < static_cast<int>(edges.size()));
		auto [u, v] = edges[id];
		return component[u] != component[v];
	}
	std::span<const int> group(int id) const {
		assert(built_ && 0 <= id && id < component_count());
		return {vertices_.data() + boundaries_[id + 1], std::size_t(boundaries_[id] - boundaries_[id + 1])};
	}
	std::vector<std::vector<int>> groups() const {
		std::vector<std::vector<int>> result(component_count());
		for (int i = 0; i < component_count(); ++i) {
			auto members = group(i);
			result[i].assign(members.begin(), members.end());
		}
		return result;
	}
	std::vector<std::vector<Arc>> forest() const {
		std::vector<std::vector<Arc>> result(component_count());
		for (int id : bridges) {
			auto [u, v] = edges[id];
			int a = component[u], b = component[v];
			result[a].push_back({b, id}), result[b].push_back({a, id});
		}
		return result;
	}
};

#endif
