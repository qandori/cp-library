#include "data_structure/dynamic_segtree.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using I = int64_t;
I add(I a, I b) { return a + b; }
I zero() { return 0; }

struct Arena {
	struct Node {
		I sum = 0;
		uint32_t left = 0, right = 0;
	};
	vector<Node> nodes{Node{}};
	I size;
	uint32_t root = 0;
	explicit Arena(I size) : size(size) {}
	void reserve(size_t n) { nodes.reserve(4 * n + 1); }
	uint32_t set(uint32_t k, I l, I r, I p, I value) {
		if (!k) {
			k = nodes.size();
			nodes.emplace_back();
		}
		if (r - l == 1) {
			nodes[k].sum = value;
			return k;
		}
		I m = l + (r - l) / 2;
		if (p < m) nodes[k].left = set(nodes[k].left, l, m, p, value);
		else nodes[k].right = set(nodes[k].right, m, r, p, value);
		nodes[k].sum = nodes[nodes[k].left].sum + nodes[nodes[k].right].sum;
		return k;
	}
	void set(I p, I value) { root = set(root, 0, size, p, value); }
	I prod(uint32_t k, I l, I r, I a, I b) const {
		if (!k || b <= l || r <= a) return 0;
		if (a <= l && r <= b) return nodes[k].sum;
		I m = l + (r - l) / 2;
		return prod(nodes[k].left, l, m, a, b) + prod(nodes[k].right, m, r, a, b);
	}
	I prod(I l, I r) const { return prod(root, 0, size, l, r); }
	size_t node_count() const { return nodes.size() - 1; }
	size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

struct PointTree {
	struct Node {
		I key, value, sum;
		uint32_t left = 0, right = 0;
	};
	vector<Node> nodes{{0, 0, 0}};
	I size;
	uint32_t root = 0;
	explicit PointTree(I size) : size(size) {}
	void reserve(size_t n) { nodes.reserve(n + 1); }
	uint32_t set(uint32_t k, I l, I r, I p, I value) {
		if (!k) {
			k = nodes.size();
			nodes.push_back({p, value, value});
			return k;
		}
		if (nodes[k].key == p) nodes[k].value = value;
		else {
			I m = l + (r - l) / 2;
			if (p < m) {
				if (nodes[k].key < p) swap(p, nodes[k].key), swap(value, nodes[k].value);
				nodes[k].left = set(nodes[k].left, l, m, p, value);
			} else {
				if (p < nodes[k].key) swap(p, nodes[k].key), swap(value, nodes[k].value);
				nodes[k].right = set(nodes[k].right, m, r, p, value);
			}
		}
		nodes[k].sum = nodes[nodes[k].left].sum + nodes[k].value + nodes[nodes[k].right].sum;
		return k;
	}
	void set(I p, I value) { root = set(root, 0, size, p, value); }
	I prefix(uint32_t k, I r) const {
		I sum = 0;
		while (k) {
			if (r <= nodes[k].key) k = nodes[k].left;
			else {
				sum += nodes[nodes[k].left].sum + nodes[k].value;
				k = nodes[k].right;
			}
		}
		return sum;
	}
	I suffix(uint32_t k, I l) const {
		I sum = 0;
		while (k) {
			if (nodes[k].key < l) k = nodes[k].right;
			else {
				sum += nodes[k].value + nodes[nodes[k].right].sum;
				k = nodes[k].left;
			}
		}
		return sum;
	}
	I prod(I l, I r) const {
		auto k = root;
		while (k) {
			if (nodes[k].key < l) k = nodes[k].right;
			else if (r <= nodes[k].key) k = nodes[k].left;
			else return suffix(nodes[k].left, l) + nodes[k].value + prefix(nodes[k].right, r);
		}
		return 0;
	}
	size_t node_count() const { return nodes.size() - 1; }
	size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

template <class F>
double timed(F f) {
	auto start = chrono::steady_clock::now();
	f();
	return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}

template <class Tree>
void bench(int n, int q, int pattern, bool reserve_nodes) {
	constexpr I size = I(1) << 62;
	mt19937_64 rng(38947268);
	vector<I> keys(n);
	for (int i = 0; i < n; ++i) keys[i] = pattern == 0 ? I(rng() % size) : pattern == 1 ? (I(1) << 60) + i
																						: I((__int128_t(size) * i) / n);
	if (pattern < 2) shuffle(keys.begin(), keys.end(), rng);
	if (pattern == 3) reverse(keys.begin(), keys.end());
	vector<int> changes(q);
	vector<pair<I, I>> ranges(q);
	for (int &i : changes) i = rng() % n;
	for (auto &[l, r] : ranges) {
		if (pattern == 1) l = (I(1) << 60) + rng() % n, r = (I(1) << 60) + rng() % n;
		else l = rng() % size, r = rng() % size;
		if (l > r) swap(l, r);
		++r;
	}
	Tree tree(size);
	if (reserve_nodes) tree.reserve(n);
	double build = timed([&] { for (int i = 0; i < n; ++i) tree.set(keys[i], i % 1000); });
	double update = timed([&] { for (int i = 0; i < q; ++i) tree.set(keys[changes[i]], i % 1000); });
	uint64_t checksum = 0;
	double query = timed([&] { for (auto [l, r] : ranges) checksum += tree.prod(l, r); });
	cout << fixed << setprecision(3) << "build_ms=" << build << " update_ms=" << update << " query_ms=" << query << " nodes=" << tree.node_count() << " bytes=" << tree.memory_usage() << " checksum=" << checksum << '\n';
}

int main(int argc, char **argv) {
	int variant = argc > 1 ? atoi(argv[1]) : 2;
	int pattern = argc > 2 ? atoi(argv[2]) : 0;
	int n = argc > 3 ? atoi(argv[3]) : 100000;
	int q = argc > 4 ? atoi(argv[4]) : 200000;
	bool reserve = argc <= 5 || atoi(argv[5]);
	if (n <= 0 || q < 0 || variant < 0 || variant > 2 || pattern < 0 || pattern > 3) return 1;
	if (variant == 0) bench<Arena>(n, q, pattern, reserve);
	else if (variant == 1) bench<PointTree>(n, q, pattern, reserve);
	else bench<DynamicSegtree<I, add, zero>>(n, q, pattern, reserve);
}
