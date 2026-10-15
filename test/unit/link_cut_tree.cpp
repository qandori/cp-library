#include "data_structure/link_cut_tree.hpp"

int64_t add(int64_t a, int64_t b) { return a + b; }
int64_t zero() { return 0; }
string concat(const string &a, const string &b) { return a + b; }
string empty_string() { return {}; }
using SumTree = LinkCutTree<int64_t, add, zero, true>;
using WordTree = LinkCutTree<string, concat, empty_string>;

vec<int> path(const vec<vec<int>> &adj, int u, int v) {
	int n = sz(adj);
	vec<int> parent(n, -1), queue{u};
	parent[u] = u;
	rep(i, sz(queue)) rep(to, n) if (adj[queue[i]][to] && parent[to] == -1) {
		parent[to] = queue[i];
		queue.push_back(to);
	}
	if (parent[v] == -1) return {};
	vec<int> result{v};
	while (result.back() != u) result.push_back(parent[result.back()]);
	reverse(all(result));
	return result;
}

struct Value {
	int value;
	explicit Value(int value) : value(value) {}
};

int main() {
	SumTree empty;
	assert(empty.size() == 0);
	mt19937 random(123);
	rep(trial, 40) {
		int n = 1 + random() % 24;
		vec<int64_t> values(n);
		vec<string> words(n);
		rep(v, n) values[v] = int(random() % 2001) - 1000, words[v] = string(1, 'A' + random() % 26);
		SumTree sums(values);
		WordTree strings(words);
		vec<vec<int>> adj(n, vec<int>(n));
		rep(v, 1, n) {
			int p = random() % v;
			bool a = sums.link(p, v), b = strings.link(p, v);
			assert(a && b);
			adj[p][v] = adj[v][p] = 1;
		}
		rep(step, 1500) {
			int u = random() % n, v = random() % n, type = random() % 10;
			auto vertices = path(adj, u, v);
			if (type == 0) {
				bool expected = vertices.empty();
				bool a = sums.link(u, v), b = strings.link(u, v);
				assert(a == expected && b == expected);
				if (expected) adj[u][v] = adj[v][u] = 1;
			} else if (type == 1) {
				bool expected = adj[u][v];
				bool a = sums.cut(u, v), b = strings.cut(u, v);
				assert(a == expected && b == expected);
				if (expected) adj[u][v] = adj[v][u] = 0;
			} else if (type == 2) {
				values[u] = int(random() % 2001) - 1000;
				words[u] = string(1, 'A' + random() % 26);
				sums.set(u, values[u]);
				strings.set(u, words[u]);
			} else if (type == 3 && !vertices.empty()) {
				int64_t sum = 0;
				string word;
				for (int x : vertices) sum += values[x], word += words[x];
				assert(sums.prod(u, v) == sum);
				assert(strings.prod(u, v) == word);
				reverse(all(word));
				assert(strings.prod(v, u) == word);
			} else if (type == 4 && !vertices.empty()) {
				int k = int(random() % (sz(vertices) + 4)) - 2;
				int expected = k >= 0 && k < sz(vertices) ? vertices[k] : -1;
				assert(sums.jump(u, v, k) == expected);
				assert(strings.jump(u, v, k) == expected);
			} else if (type == 5) {
				assert(sums.same(u, v) == !vertices.empty());
				assert(strings.same(u, v) == !vertices.empty());
			} else if (type == 6) {
				sums.make_root(u);
				strings.make_root(u);
				assert(sums.root(u) == u && strings.root(u) == u);
				if (!vertices.empty()) assert(sums.root(v) == u && strings.root(v) == u);
			} else if (type == 7 && !vertices.empty()) {
				int r = random() % n;
				auto a = path(adj, r, u), b = path(adj, r, v);
				if (a.empty()) continue;
				int i = 0;
				while (i < min(sz(a), sz(b)) && a[i] == b[i]) ++i;
				assert(sums.lca(r, u, v) == a[i - 1]);
				assert(strings.lca(r, u, v) == a[i - 1]);
			} else if (type == 8 && !vertices.empty()) {
				assert(sums.path_size(u, v) == sz(vertices));
				assert(strings.distance(u, v) == sz(vertices) - 1);
			} else {
				assert(sums.get(u) == values[u]);
				assert(strings.get(u) == words[u]);
			}
		}
	}
	const int n = 200000;
	SumTree large(vec<int64_t>(n, 1));
	rep(v, 1, n) {
		bool linked = large.link(v - 1, v);
		assert(linked);
	}
	assert(large.prod(0, n - 1) == n);
	assert(large.path_size(0, n - 1) == n);
	assert(large.jump(0, n - 1, n / 2) == n / 2);
	assert(large.jump(n - 1, 0, n / 2) == n / 2 - 1);
	assert(large.lca(n / 2, 0, n - 1) == n / 2);
	bool cut = large.cut(n / 2 - 1, n / 2);
	assert(cut && !large.same(0, n - 1));
	bool linked = large.link(0, n - 1);
	assert(linked && large.prod(n / 2 - 1, n / 2) == n);
	large.set(0, 10);
	assert(large.prod(0, n - 1) == 11);
	SumTree original(vec<int64_t>{1, 2, 3});
	original.link(0, 1);
	original.link(1, 2);
	auto copy = original;
	copy.cut(0, 1);
	assert(!copy.same(0, 2) && original.prod(0, 2) == 6);
	auto moved = std::move(copy);
	assert(moved.prod(1, 2) == 5 && copy.size() == 0);
	constexpr auto combine = [](Value a, Value b) { return Value(a.value + b.value); };
	constexpr auto identity = [] { return Value(0); };
	LinkCutTree<Value, combine, identity> custom(vec<Value>{Value(4), Value(7)});
	custom.link(0, 1);
	assert(custom.prod(0, 1).value == 11);
	LinkCutTree<Value, combine, identity> initialized(3);
	assert(initialized.get(2).value == 0);
	constexpr auto disjoin = [](bool a, bool b) { return a || b; };
	constexpr auto no = [] { return false; };
	LinkCutTree<bool, disjoin, no, true> bits(vec<bool>{false, true});
	bits.link(0, 1);
	assert(bits.prod(1, 0));
	constexpr auto minimum = [](int a, int b) { return min(a, b); };
	constexpr auto infinity = [] { return numeric_limits<int>::max(); };
	LinkCutTree<int, minimum, infinity, true> minima(4);
	minima.set(0, 7);
	minima.set(1, -3);
	minima.set(2, 11);
	minima.set(3, 2);
	minima.link(0, 1);
	minima.link(1, 2);
	minima.link(1, 3);
	assert(minima.prod(0, 2) == -3 && minima.prod(0, 0) == 7);
	minima.set(1, 9);
	assert(minima.prod(0, 2) == 7 && minima.prod(3, 2) == 2);
	bool removed = minima.cut(1, 2);
	assert(removed && minima.prod(2, 2) == 11);
	minima.link(0, 2);
	assert(minima.prod(3, 2) == 2);
}
