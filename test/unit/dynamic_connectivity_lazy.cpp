#include "data_structure/dynamic_connectivity.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

void require(bool ok, int line = __builtin_LINE()) {
	if (!ok) { std::fprintf(stderr, "line %d\n", line); std::abort(); }
}

using T = std::int64_t;
T add(T a, T b) { return a + b; }
T zero() { return 0; }

struct Value {
	T x;
	explicit Value(T x) : x(x) {}
};
Value combine(Value a, Value b) { return Value(a.x ^ b.x); }
Value identity() { return Value(0); }

int main() {
	const int n = 200000, k = 23;
	std::vector<T> values(n);
	for (int i = 0; i < n; ++i) values[i] = i % 37 - 18;
	DynamicConnectivity<T, add, zero> graph(values);
	DynamicConnectivity<> plain(n);
	graph.set(n - 1, 123);
	values[n - 1] = 123;
	require(graph.prod(n - 1) == 123 && graph.size(n - 1) == 1);
	require(!graph.same(0, n - 1) && graph.same(n - 1, n - 1));
	require(!graph.link(n - 2, n - 2));
	require(!graph.cut(n - 2, n - 2));
	require(graph.prod(n - 2) == values[n - 2]);
	std::vector<int> ids(k);
	for (int i = 0; i < k; ++i) ids[i] = n - 1 - i * 7919;
	std::vector<std::vector<int>> edges(k, std::vector<int>(k));
	std::mt19937 random(20261018);
	T edge_count = 0;
	auto component = [&](int start) {
		std::vector<int> seen(k), members{start};
		seen[start] = 1;
		for (int i = 0; i < static_cast<int>(members.size()); ++i)
			for (int v = 0; v < k; ++v)
				if (edges[members[i]][v] && !seen[v]) seen[v] = 1, members.push_back(v);
		return seen;
	};
	for (int step = 0; step < 10000; ++step) {
		int u = random() % k, v = random() % k, type = random() % 10;
		bool before = component(u)[v];
		if (type < 3) {
			require(graph.link(ids[u], ids[v]) == !before);
			require(plain.link(ids[v], ids[u]) == !before);
			++edges[u][v];
			if (u != v) ++edges[v][u];
			++edge_count;
		} else if (type < 7) {
			if (edges[u][v]) {
				--edges[u][v];
				if (u != v) --edges[v][u];
				--edge_count;
			}
			bool split = before && !component(u)[v];
			require(graph.cut(ids[v], ids[u]) == split);
			require(plain.cut(ids[u], ids[v]) == split);
		} else {
			values[ids[u]] = int(random() % 1000001) - 500000;
			graph.set(ids[u], values[ids[u]]);
		}
		auto seen = component(u);
		T total = 0;
		int size = 0;
		for (int i = 0; i < k; ++i) if (seen[i]) total += values[ids[i]], ++size;
		require(graph.prod(ids[u]) == total && graph.size(ids[u]) == size);
		require(plain.size(ids[u]) == size);
		require(graph.same(ids[u], ids[v]) == bool(seen[v]));
		require(plain.same(ids[u], ids[v]) == bool(seen[v]));
		require(graph.edge_count() == edge_count && plain.edge_count() == edge_count);
		require(graph.count(ids[u], ids[v]) == edges[u][v]);
		if (step % 257 == 0) {
			auto copy = graph;
			require(copy.prod(ids[u]) == total && copy.edge_count() == edge_count);
			graph.reserve(step + 1);
		}
	}
	DynamicConnectivity<Value, combine, identity> custom(1000);
	custom.set(999, Value(7));
	custom.set(456, Value(11));
	require(custom.prod(999).x == 7 && custom.prod(456).x == 11);
	require(custom.link(999, 456) && custom.prod(456).x == 12);
	require(custom.cut(999, 456));
	custom.set(456, Value(2));
	require(custom.prod(456).x == 2 && custom.prod(999).x == 7);
	require(custom.link(999, 456) && custom.prod(999).x == 5);
}
