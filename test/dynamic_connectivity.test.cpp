#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum"
#include "../data_structure/dynamic_connectivity.hpp"

int64_t op(int64_t a, int64_t b) { return a + b; }
int64_t e() { return 0; }

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vec<int64_t> values(n);
	for (auto &v : values) cin >> v;
	DynamicConnectivity<int64_t, op, e> graph(values);
	graph.reserve(q);
	rep(q) {
		int type, u;
		cin >> type >> u;
		if (type < 2) {
			int v;
			cin >> v;
			if (type == 0) graph.link(u, v);
			else graph.cut(u, v);
		} else if (type == 2) {
			int64_t x;
			cin >> x;
			graph.set(u, graph.get(u) + x);
		} else cout << graph.prod(u) << '\n';
	}
}
