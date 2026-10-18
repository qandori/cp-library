#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum"
#include "../data_structure/dynamic_connectivity.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

using namespace std;

int64_t op(int64_t a, int64_t b) { return a + b; }
int64_t e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	vector<int64_t> values(n);
	for (auto &v : values) input.read(v);
	DynamicConnectivity<int64_t, op, e> graph(values);
	graph.reserve(q);
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int type = 0, u = 0;
		input.read(type); input.read(u);
		if (type < 2) {
			int v = 0;
			input.read(v);
			if (type == 0) graph.link(u, v);
			else graph.cut(u, v);
		} else if (type == 2) {
			int64_t x = 0;
			input.read(x);
			graph.set(u, graph.get(u) + x);
		} else output.println(graph.prod(u));
	}
}
