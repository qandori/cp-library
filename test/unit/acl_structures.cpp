#include "data_structure/union_find.hpp"
#include "data_structure/fenwick_tree.hpp"
#include "data_structure/lazy_segtree.hpp"
#include "graph/scc.hpp"
#include "graph/two_sat.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <string>

using U = std::uint64_t;
struct Sum { U value = 0; int length = 0; };
Sum op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum identity() { return {}; }
struct Affine { U a = 1, b = 0; };
Sum mapping(Affine f, Sum x) { return {f.a * x.value + f.b * U(x.length), x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine lazy_id() { return {}; }
using Tree = cp_acl::lazy_segtree<Sum, op, identity, Affine, mapping, composition, lazy_id>;

std::string concat(std::string a, std::string b) { return a + b; }
std::string empty_string() { return {}; }
struct Shift { unsigned value = 0; };
std::string shift_string(Shift f, std::string x) {
	for (char &c : x) c = char('a' + (unsigned(c - 'a') + f.value) % 26);
	return x;
}
Shift shift_composition(Shift f, Shift g) { return {(f.value + g.value) % 26}; }
Shift shift_id() { return {}; }

void noncommutative_lazy() {
	using Strings = LazySegtree<std::string, concat, empty_string, Shift, shift_string, shift_composition, shift_id>;
	std::mt19937 rng(756723);
	for (int n : {1, 3, 8, 17, 33}) {
		std::string a(n, 'a');
		Strings tree(std::vector<std::string>(n, "a"));
		for (int q = 0; q < 1000; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			Shift f{unsigned(rng() % 26)};
			tree.apply(l, r, f);
			for (int i = l; i < r; ++i) a[i] = char('a' + (unsigned(a[i] - 'a') + f.value) % 26);
			assert(tree.prod(l, r) == a.substr(l, r - l));
			assert(tree.all_prod() == a);
			int k = rng() % (r - l + 1);
			std::string prefix = a.substr(l, k), suffix = a.substr(r - k, k);
			assert(tree.max_right(l, [&](const std::string &x) { return prefix.starts_with(x); }) == l + k);
			assert(tree.min_left(r, [&](const std::string &x) { return suffix.ends_with(x); }) == r - k);
		}
	}
}

void union_find() {
	std::mt19937 rng(34568);
	for (int n : {0, 1, 2, 3, 17, 100}) {
		cp_acl::dsu uf(n);
		std::vector<int> id(n);
		std::iota(id.begin(), id.end(), 0);
		assert(uf.groups().size() == std::size_t(n));
		for (int q = 0; n && q < 2000; ++q) {
			int a = rng() % n, b = rng() % n;
			int from = id[b], to = id[a];
			int root = uf.merge(a, b);
			for (int &x : id) if (x == from) x = to;
			assert(uf.leader(a) == root && uf.leader(b) == root);
			for (int v = 0; v < n; ++v) {
				assert(uf.same(a, v) == (id[a] == id[v]));
				assert(uf.size(v) == std::count(id.begin(), id.end(), id[v]));
			}
			auto groups = uf.groups();
			assert(int(groups.size()) == uf.count());
			int vertices = 0;
			for (const auto &group : groups) {
				assert(!group.empty() && std::is_sorted(group.begin(), group.end()));
				for (int v : group) assert(id[v] == id[group.front()]);
				vertices += group.size();
			}
			assert(vertices == n);
		}
	}
	UnionFind uf(500000);
	for (int width = 1; width < uf.size(); width *= 2)
		for (int v = 0; v + width < uf.size(); v += width * 2) uf.merge(v, v + width);
	assert(uf.count() == 1 && uf.groups()[0].size() == 500000);
	UnionFind copy = uf;
	assert(copy.size(uf.size() - 1) == uf.size());
}

template<bool Padded> void fenwick() {
	std::mt19937 rng(238219);
	for (int n : {0, 1, 2, 3, 31, 32, 33, 1023, 1024, 1025, 2048}) {
		std::vector<std::int64_t> initial(n);
		std::vector<U> a(n);
		for (int i = 0; i < n; ++i) a[i] = U(initial[i] = std::int64_t(rng() % 100) - 50);
		FenwickTree<std::int64_t, Padded> tree(initial);
		for (int q = 0; q < 2000; ++q) {
			if (n && (rng() & 1)) {
				int p = rng() % n;
				std::int64_t x = std::int64_t(rng() % 100) - 50;
				a[p] += U(x);
				tree.add(p, x);
			}
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			U expected = 0;
			for (int i = l; i < r; ++i) expected += a[i];
			assert(U(tree.sum(l, r)) == expected);
			assert(U(tree.sum(r)) - U(tree.sum(l)) == expected);
		}
	}
	FenwickTree<int, Padded> wrapping(2);
	wrapping.add(0, std::numeric_limits<int>::max());
	wrapping.add(1, 1);
	assert(wrapping.sum(0, 2) == std::numeric_limits<int>::min());
#if defined(__SIZEOF_INT128__)
	FenwickTree<__int128_t, Padded> wide(1);
	wide.add(0, __int128_t(1) << 100);
	assert(wide.sum(0, 1) == (__int128_t(1) << 100));
#endif
}

void lazy() {
	std::mt19937 rng(721561);
	Tree empty;
	assert(empty.prod(0, 0).value == 0 && empty.all_prod().length == 0);
	assert(empty.max_right<[](Sum x) { return x.value == 0; }>(0) == 0);
	assert(empty.min_left<[](Sum x) { return x.value == 0; }>(0) == 0);
	for (int n : {1, 2, 3, 7, 8, 9, 31, 32, 33, 100}) {
		std::vector<U> a(n);
		std::vector<Sum> values(n);
		for (int i = 0; i < n; ++i) values[i] = {a[i] = rng() % 8, 1};
		Tree tree(values);
		for (int q = 0; q < 3000; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1), p = rng() % n;
			if (l > r) std::swap(l, r);
			switch (rng() % 5) {
			case 0: {
				Affine f{U(rng() % 2), U(rng() % 5)};
				tree.apply(l, r, f);
				for (int i = l; i < r; ++i) a[i] = f.a * a[i] + f.b;
				break;
			}
			case 1: tree.set(p, {a[p] = rng() % 8, 1}); break;
			case 2: tree.apply(p, {0, a[p] = rng() % 8}); break;
			case 3: assert(tree.get(p).value == a[p]); break;
			default: break;
			}
			U expected = std::accumulate(a.begin() + l, a.begin() + r, U(0));
			assert(tree.prod(l, r).value == expected);
			assert(tree.all_prod().value == std::accumulate(a.begin(), a.end(), U(0)));
			U limit = rng() % 100;
			int right = l, left = r;
			U sum = 0;
			while (right < n && sum + a[right] <= limit) sum += a[right++];
			sum = 0;
			while (left > 0 && sum + a[left - 1] <= limit) sum += a[--left];
			assert(tree.max_right(l, [=](Sum x) { return x.value <= limit; }) == right);
			assert(tree.min_left(r, [=](Sum x) { return x.value <= limit; }) == left);
		}
		Tree copy = tree;
		copy.apply(0, n, {0, 7});
		assert(copy.all_prod().value == U(n * 7));
	}
}

void check_scc(int n, const std::vector<std::pair<int, int>> &edges) {
	SCCGraph graph(n);
	std::vector<std::vector<bool>> reach(n, std::vector<bool>(n));
	for (int v = 0; v < n; ++v) reach[v][v] = true;
	for (auto [a, b] : edges) graph.add_edge(a, b), reach[a][b] = true;
	for (int k = 0; k < n; ++k) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
		reach[i][j] = reach[i][j] || (reach[i][k] && reach[k][j]);
	auto [count, id] = graph.scc_ids();
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
		assert((id[i] == id[j]) == (reach[i][j] && reach[j][i]));
	for (auto [a, b] : edges) assert(id[a] <= id[b]);
	auto groups = graph.scc();
	assert(int(groups.size()) == count);
	std::vector<bool> used(n);
	for (int k = 0; k < count; ++k) {
		assert(!groups[k].empty() && std::is_sorted(groups[k].begin(), groups[k].end()));
		for (int v : groups[k]) { assert(!used[v] && id[v] == k); used[v] = true; }
	}
	assert(std::all_of(used.begin(), used.end(), [](bool x) { return x; }));
	assert(graph.scc_ids().second == id);
}

void scc() {
	for (int n = 0; n <= 4; ++n) for (int mask = 0; mask < (1 << (n * n)); ++mask) {
		std::vector<std::pair<int, int>> edges;
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
			if ((mask >> (i * n + j)) & 1) edges.emplace_back(i, j);
		check_scc(n, edges);
	}
	std::mt19937 rng(296528);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + rng() % 16;
		std::vector<std::pair<int, int>> edges;
		for (int i = 0, m = rng() % (n * n * 2 + 1); i < m; ++i) edges.emplace_back(rng() % n, rng() % n);
		check_scc(n, edges);
	}
	const int n = 500000;
	SCCGraph cycle(n);
	for (int v = 0; v < n; ++v) cycle.add_edge(v, (v + 1) % n);
	assert(cycle.scc_ids().first == 1);
	cycle.add_edge(0, 2);
	assert(cycle.scc_ids().first == 1);
}

void two_sat() {
	TwoSAT incremental(1);
	incremental.add_clause(0, true, 0, false);
	assert(incremental.satisfiable());
	incremental.add_clause(0, true, 0, true);
	assert(incremental.satisfiable() && incremental.answer()[0]);
	auto copy = incremental;
	copy.add_clause(0, false, 0, false);
	assert(!copy.satisfiable());
	assert(incremental.satisfiable() && incremental.answer()[0]);
	std::mt19937 rng(827635);
	for (int n = 0; n <= 8; ++n) for (int trial = 0; trial < 500; ++trial) {
		TwoSAT solver(n);
		std::vector<std::array<int, 4>> clauses;
		int m = n ? rng() % (n * 5 + 1) : 0;
		for (int q = 0; q < m; ++q) {
			int i = rng() % n, j = rng() % n, f = rng() % 2, g = rng() % 2;
			solver.add_clause(i, f, j, g);
			clauses.push_back({i, f, j, g});
		}
		bool possible = false;
		for (int mask = 0; mask < (1 << n); ++mask) {
			bool good = true;
			for (auto [i, f, j, g] : clauses) good &= ((mask >> i) & 1) == f || ((mask >> j) & 1) == g;
			possible |= good;
		}
		assert(solver.satisfiable() == possible);
		if (possible) {
			auto answer = solver.answer();
			for (auto [i, f, j, g] : clauses) assert(answer[i] == bool(f) || answer[j] == bool(g));
		}
		assert(solver.satisfiable() == possible);
	}
}

int main() {
	union_find();
	fenwick<true>();
	fenwick<false>();
	lazy();
	noncommutative_lazy();
	scc();
	two_sat();
}
