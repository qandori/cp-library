#include "data_structure/dynamic_connectivity.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

using T = int64_t;
T add(T a, T b) { return a + b; }
T zero() { return 0; }
T minimum(T a, T b) { return min(a, b); }
T inf() { return numeric_limits<T>::max(); }

template <auto op, auto e>
void random_test(uint64_t seed) {
	mt19937_64 rng(seed);
	for (int trial = 0; trial < 100; ++trial) {
		int n = 1 + rng() % 36;
		vector<T> values(n);
		for (auto &x : values) x = int(rng() % 1001) - 500;
		DynamicConnectivity<T, op, e> g(values);
		DynamicConnectivity<> plain(n);
		if (trial & 1) g.reserve(n * 2), plain.reserve(n * 2);
		vector<vector<int>> edges(n, vector<int>(n));
		int64_t m = 0;
		auto component = [&](int start) {
			vector<int> seen(n), vertices{start};
			seen[start] = 1;
			for (int i = 0; i < (static_cast<int>(vertices.size())); ++i)
				for (int v = 0; v < n; ++v)
					if (edges[vertices[i]][v] && !seen[v]) seen[v] = 1, vertices.push_back(v);
			return vertices;
		};
		for (int step = 0; step < 1200; ++step) {
			if (step == 600) g.reserve(n * 4), plain.reserve(n * 4);
			int u = rng() % n, v = rng() % n, type = rng() % 12;
			auto prior = component(u);
			bool before = find(prior.begin(), prior.end(), v) != prior.end();
			if (type < 5 && m < n * 2) {
				bool expected = !before;
				assert(g.link(u, v) == expected);
				assert(plain.link(v, u) == expected);
				++edges[u][v];
				if (u != v) ++edges[v][u];
				++m;
			} else if (type < 10) {
				if (rng() & 1) {
					for (int k = 0; k < n; ++k)
						if (edges[u][(v + k) % n]) {
							v = (v + k) % n;
							break;
						}
				}
				before = find(prior.begin(), prior.end(), v) != prior.end();
				if (edges[u][v]) {
					--edges[u][v];
					if (u != v) --edges[v][u];
					--m;
				}
				auto members = component(u);
				bool expected = before && find(members.begin(), members.end(), v) == members.end();
				assert(g.cut(v, u) == expected);
				assert(plain.cut(u, v) == expected);
			} else {
				values[u] = int(rng() % 2001) - 1000;
				g.set(u, values[u]);
				assert(g.get(u) == values[u]);
			}
			assert(g.edge_count() == m && plain.edge_count() == m);
			assert(g.count(u, v) == edges[u][v] && plain.count(v, u) == edges[u][v]);
			auto members = component(u);
			T expected = e();
			for (int x : members) expected = op(expected, values[x]);
			assert(g.size(u) == static_cast<int>(members.size()) && plain.size(u) == static_cast<int>(members.size()));
			assert(g.prod(u) == expected);
			bool connected = find(members.begin(), members.end(), v) != members.end();
			assert(g.same(u, v) == connected && plain.same(u, v) == connected);
			if (step % 37 == 0) {
				vector<int> seen(n);
				int count = 0;
				for (int i = 0; i < n; ++i)
					if (!seen[i]) {
						++count;
						for (int x : component(i)) seen[x] = 1;
					}
				assert(g.component_count() == count && plain.component_count() == count);
			}
		}
		auto copy = g;
		g.link(0, 0);
		assert(copy.edge_count() == m && copy.prod(0) == g.prod(0));
	}
}

struct NoDefault {
	T value;
	explicit NoDefault(T x) : value(x) {}
};

void replacement_test() {
	DynamicConnectivity<T, add, zero> graph(vector<T>{1, 2, 3, 4, 5, 6, 7, 8});
	for (int v = 1; v < 8; ++v) assert(graph.link(v - 1, v));
	for (int v = 0; v < 4; ++v) assert(!graph.link(v, v + 4));
	for (auto [u, v] : vector<pair<int, int>>{{0, 2}, {1, 3}, {4, 6}, {5, 7}}) assert(!graph.link(u, v));
	assert(!graph.cut(3, 4));
	for (int v = 0; v < 8; ++v) assert(graph.size(v) == 8 && graph.prod(v) == 36);
	for (int v = 0; v < 4; ++v) assert(graph.cut(v, v + 4) == (v == 3));
	assert(graph.component_count() == 2 && graph.prod(0) == 10 && graph.prod(7) == 26);
	for (auto [u, v] : vector<pair<int, int>>{{0, 2}, {1, 3}, {4, 6}, {5, 7}}) assert(!graph.cut(u, v));
	assert(graph.cut(1, 2));
	assert(!graph.same(0, 3) && graph.same(4, 7));
	assert(graph.component_count() == 3 && graph.size(0) == 2 && graph.prod(3) == 7);
	graph.set(0, 99);
	assert(graph.prod(1) == 101 && graph.prod(3) == 7);
}

int main() {
	DynamicConnectivity<> empty;
	assert(empty.size() == 0 && empty.component_count() == 0 && empty.edge_count() == 0);
	empty.reserve(20);
	DynamicConnectivity<T, minimum, inf> initial(4);
	assert(initial.prod(0) == inf());
	assert(initial.link(0, 1));
	initial.set(0, 7);
	initial.set(1, 2);
	assert(initial.prod(0) == 2);
	assert(initial.cut(0, 1) && initial.prod(0) == 7 && initial.prod(1) == 2);
	random_test<add, zero>(7123);
	random_test<minimum, inf>(3291);
	replacement_test();
	constexpr auto op = [](NoDefault a, NoDefault b) { return NoDefault(a.value ^ b.value); };
	constexpr auto e = [] { return NoDefault(0); };
	DynamicConnectivity<NoDefault, op, e> custom(vector<NoDefault>{NoDefault(3), NoDefault(5), NoDefault(8)});
	assert(custom.link(0, 1) && custom.prod(0).value == 6);
	assert(custom.link(1, 2) && !custom.link(0, 2));
	assert(!custom.cut(0, 1) && custom.prod(0).value == 14);
	assert(custom.cut(0, 2) && custom.prod(0).value == 3);
	custom.set(1, NoDefault(1));
	assert(custom.prod(2).value == 9);
	const int star_n = 4096;
	DynamicConnectivity<T, add, zero> star(vector<T>(star_n, 1));
	for (int v = 1; v < star_n; ++v) assert(star.link(0, v));
	for (int v = 1; v < (star_n - 1); ++v) assert(!star.link(v, v + 1));
	for (int v = 1; v < star_n; ++v) assert(star.cut(0, v) == (v == star_n - 1));
	assert(star.size(0) == 1 && star.prod(1) == star_n - 1 && star.component_count() == 2);
	for (int v = 1; v < (star_n - 1); ++v) assert(star.cut(v, v + 1));
	assert(star.component_count() == star_n);
	const int n = 200000;
	DynamicConnectivity<T, add, zero> path(vector<T>(n, 1));
	path.reserve(n);
	for (int i = 1; i < n; ++i) assert(path.link(i - 1, i));
	assert(path.prod(0) == n && path.size(n - 1) == n);
	for (int i = 1; i < n; i += 2) assert(path.cut(i, i - 1));
	assert(path.component_count() == n / 2 + 1);
	for (int i = 1; i < n; i += 2) assert(path.link(i, i - 1));
	assert(path.component_count() == 1 && path.prod(n / 2) == n);
	for (int i = 1; i < n; ++i) assert(path.cut(i - 1, i));
	assert(path.component_count() == n);
	for (int i = 0; i < n; ++i) assert(path.size(i) == 1 && path.prod(i) == 1);
}
