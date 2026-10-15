#include "data_structure/dynamic_connectivity.hpp"

using T = int64_t;
T add(T a, T b) { return a + b; }
T zero() { return 0; }
T minimum(T a, T b) { return min(a, b); }
T inf() { return numeric_limits<T>::max(); }

template <auto op, auto e>
void random_test(uint64_t seed) {
	mt19937_64 rng(seed);
	rep(trial, 100) {
		int n = 1 + rng() % 36;
		vec<T> values(n);
		for (auto &x : values) x = int(rng() % 1001) - 500;
		DynamicConnectivity<T, op, e> g(values);
		DynamicConnectivity<> plain(n);
		if (trial & 1) g.reserve(n * 2), plain.reserve(n * 2);
		vec<vec<int>> edges(n, vec<int>(n));
		int64_t m = 0;
		auto component = [&](int start) {
			vec<int> seen(n), vertices{start};
			seen[start] = 1;
			rep(i, sz(vertices)) rep(v, n) if (edges[vertices[i]][v] && !seen[v]) seen[v] = 1, vertices.push_back(v);
			return vertices;
		};
		rep(step, 1200) {
			if (step == 600) g.reserve(n * 4), plain.reserve(n * 4);
			int u = rng() % n, v = rng() % n, type = rng() % 12;
			auto prior = component(u);
			bool before = find(all(prior), v) != prior.end();
			if (type < 5 && m < n * 2) {
				bool expected = !before;
				assert(g.link(u, v) == expected);
				assert(plain.link(v, u) == expected);
				++edges[u][v];
				if (u != v) ++edges[v][u];
				++m;
			} else if (type < 10) {
				if (rng() & 1) {
					rep(k, n) if (edges[u][(v + k) % n]) {
						v = (v + k) % n;
						break;
					}
				}
				before = find(all(prior), v) != prior.end();
				if (edges[u][v]) {
					--edges[u][v];
					if (u != v) --edges[v][u];
					--m;
				}
				auto members = component(u);
				bool expected = before && find(all(members), v) == members.end();
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
			assert(g.size(u) == sz(members) && plain.size(u) == sz(members));
			assert(g.prod(u) == expected);
			bool connected = find(all(members), v) != members.end();
			assert(g.same(u, v) == connected && plain.same(u, v) == connected);
			if (step % 37 == 0) {
				vec<int> seen(n);
				int count = 0;
				rep(i, n) if (!seen[i]) {
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
	DynamicConnectivity<T, add, zero> graph(vec<T>{1, 2, 3, 4, 5, 6, 7, 8});
	rep(v, 1, 8) assert(graph.link(v - 1, v));
	rep(v, 4) assert(!graph.link(v, v + 4));
	for (auto [u, v] : vec<pair<int, int>>{{0, 2}, {1, 3}, {4, 6}, {5, 7}}) assert(!graph.link(u, v));
	assert(!graph.cut(3, 4));
	rep(v, 8) assert(graph.size(v) == 8 && graph.prod(v) == 36);
	rep(v, 4) assert(graph.cut(v, v + 4) == (v == 3));
	assert(graph.component_count() == 2 && graph.prod(0) == 10 && graph.prod(7) == 26);
	for (auto [u, v] : vec<pair<int, int>>{{0, 2}, {1, 3}, {4, 6}, {5, 7}}) assert(!graph.cut(u, v));
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
	DynamicConnectivity<NoDefault, op, e> custom(vec<NoDefault>{NoDefault(3), NoDefault(5), NoDefault(8)});
	assert(custom.link(0, 1) && custom.prod(0).value == 6);
	assert(custom.link(1, 2) && !custom.link(0, 2));
	assert(!custom.cut(0, 1) && custom.prod(0).value == 14);
	assert(custom.cut(0, 2) && custom.prod(0).value == 3);
	custom.set(1, NoDefault(1));
	assert(custom.prod(2).value == 9);
	const int star_n = 4096;
	DynamicConnectivity<T, add, zero> star(vec<T>(star_n, 1));
	rep(v, 1, star_n) assert(star.link(0, v));
	rep(v, 1, star_n - 1) assert(!star.link(v, v + 1));
	rep(v, 1, star_n) assert(star.cut(0, v) == (v == star_n - 1));
	assert(star.size(0) == 1 && star.prod(1) == star_n - 1 && star.component_count() == 2);
	rep(v, 1, star_n - 1) assert(star.cut(v, v + 1));
	assert(star.component_count() == star_n);
	const int n = 200000;
	DynamicConnectivity<T, add, zero> path(vec<T>(n, 1));
	path.reserve(n);
	rep(i, 1, n) assert(path.link(i - 1, i));
	assert(path.prod(0) == n && path.size(n - 1) == n);
	for (int i = 1; i < n; i += 2) assert(path.cut(i, i - 1));
	assert(path.component_count() == n / 2 + 1);
	for (int i = 1; i < n; i += 2) assert(path.link(i, i - 1));
	assert(path.component_count() == 1 && path.prod(n / 2) == n);
	rep(i, 1, n) assert(path.cut(i - 1, i));
	assert(path.component_count() == n);
	rep(i, n) assert(path.size(i) == 1 && path.prod(i) == 1);
}
