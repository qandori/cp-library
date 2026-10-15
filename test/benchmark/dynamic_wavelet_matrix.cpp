#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

class TreapBitVector {
	struct Node {
		int left = 0, right = 0, size = 0, ones = 0;
		std::uint32_t priority = 0;
		bool value = false;
	};
	std::vector<Node> nodes = std::vector<Node>(1);
	int root = 0, free = 0;
	std::uint32_t random = 713;
	void pull(int k) {
		auto &a = nodes[k];
		a.size = nodes[a.left].size + nodes[a.right].size + 1;
		a.ones = nodes[a.left].ones + nodes[a.right].ones + a.value;
	}
	int make(bool value) {
		random ^= random << 13, random ^= random >> 17, random ^= random << 5;
		int k;
		if (free) k = free, free = nodes[k].left;
		else k = int(nodes.size()), nodes.emplace_back();
		nodes[k] = {0, 0, 1, int(value), random, value};
		return k;
	}
	std::pair<int, int> split(int k, int p) {
		if (!k) return {};
		int size = nodes[nodes[k].left].size;
		if (p <= size) {
			auto [a, b] = split(nodes[k].left, p);
			nodes[k].left = b;
			pull(k);
			return {a, k};
		}
		auto [a, b] = split(nodes[k].right, p - size - 1);
		nodes[k].right = a;
		pull(k);
		return {k, b};
	}
	int merge(int a, int b) {
		if (!a || !b) return a ? a : b;
		if (nodes[a].priority > nodes[b].priority) {
			nodes[a].right = merge(nodes[a].right, b);
			pull(a);
			return a;
		}
		nodes[b].left = merge(a, nodes[b].left);
		pull(b);
		return b;
	}
	void pull_tree(int k) {
		if (!k) return;
		pull_tree(nodes[k].left), pull_tree(nodes[k].right);
		pull(k);
	}

  public:
	void reserve(int n) { nodes.reserve(std::size_t(n) + 1); }
	void clear() {
		nodes.resize(1);
		root = free = 0;
		random = 713;
	}
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	void build(int n, const std::vector<std::uint64_t> &packed) {
		clear();
		reserve(n);
		std::vector<int> stack;
		for (int i = 0; i < n; ++i) {
			int k = make((packed[i >> 6] >> (i & 63)) & 1), previous = 0;
			while (!stack.empty() && nodes[stack.back()].priority < nodes[k].priority) previous = stack.back(), stack.pop_back();
			nodes[k].left = previous;
			if (!stack.empty()) nodes[stack.back()].right = k;
			stack.push_back(k);
		}
		root = stack.empty() ? 0 : stack.front();
		pull_tree(root);
	}
	int rank1(int p) const {
		int k = root, result = 0;
		while (k) {
			int size = nodes[nodes[k].left].size;
			if (p <= size) k = nodes[k].left;
			else result += nodes[nodes[k].left].ones + nodes[k].value, p -= size + 1, k = nodes[k].right;
		}
		return result;
	}
	std::pair<int, int> rank1(int l, int r) const { return {rank1(l), rank1(r)}; }
	std::pair<int, bool> access_rank(int p) const {
		int k = root, before = 0;
		for (;;) {
			int size = nodes[nodes[k].left].size;
			if (p < size) k = nodes[k].left;
			else {
				before += nodes[nodes[k].left].ones;
				if (p == size) return {before, nodes[k].value};
				before += nodes[k].value;
				p -= size + 1, k = nodes[k].right;
			}
		}
	}
	int insert(int p, bool value) {
		auto [a, b] = split(root, p);
		int result = nodes[a].ones, k = make(value);
		root = merge(merge(a, k), b);
		return result;
	}
	std::pair<int, bool> erase_rank(int p) {
		auto [a, b] = split(root, p);
		auto [k, c] = split(b, 1);
		bool value = nodes[k].value;
		int before = nodes[a].ones;
		nodes[k].left = free, free = k;
		root = merge(a, c);
		return {before, value};
	}
	std::pair<int, bool> set_rank(int p, bool value) {
		std::array<int, 128> path;
		int k = root, before = 0, depth = 0;
		for (;;) {
			path[depth++] = k;
			int size = nodes[nodes[k].left].size;
			if (p < size) k = nodes[k].left;
			else {
				before += nodes[nodes[k].left].ones;
				if (p == size) break;
				before += nodes[k].value;
				p -= size + 1, k = nodes[k].right;
			}
		}
		bool old = nodes[k].value;
		nodes[k].value = value;
		while (depth) pull(path[--depth]);
		return {before, old};
	}
	int select(bool value, int p) const {
		int k = root, before = 0;
		if (p < 0 || p >= (value ? nodes[k].ones : nodes[k].size - nodes[k].ones)) return -1;
		while (k) {
			int left = nodes[k].left, size = nodes[left].size;
			int count = value ? nodes[left].ones : size - nodes[left].ones;
			if (p < count) k = left;
			else {
				p -= count;
				if (nodes[k].value == value) {
					if (!p) return before + size;
					--p;
				}
				before += size + 1, k = nodes[k].right;
			}
		}
		return -1;
	}
};

struct SeparateRanks : DynamicBitVector<> {
	using DynamicBitVector<>::rank1;
	std::pair<int, int> rank1(int l, int r) const { return {rank1(l), rank1(r)}; }
};

struct Operation {
	int type, p, l, r, k;
	unsigned value;
};

template <class BV, bool replace = true>
void run(const std::vector<unsigned> &a, const std::vector<Operation> &operations) {
	auto start = std::chrono::steady_clock::now();
	DynamicWaveletMatrix<unsigned, 20, BV> wm(a);
	auto built = std::chrono::steady_clock::now();
	std::uint64_t checksum = 0;
	for (auto [type, p, l, r, k, value] : operations) {
		if (type == 0) wm.insert(p, value);
		else if (type == 1) checksum += wm.erase(p);
		else if (type == 2) {
			if constexpr (replace) wm.set(p, value);
			else wm.erase(p), wm.insert(p, value);
		} else if (type == 3) checksum += wm.kth_smallest(l, r, k);
		else if (type == 4) checksum += wm.range_freq(l, r, value);
		else if (type == 5) checksum += wm.count(l, r, value);
		else if (type == 6) checksum += wm.access(p);
		else checksum += wm.select(value, 0);
	}
	auto finished = std::chrono::steady_clock::now();
	for (int i = 0; i < std::min(256, wm.size()); ++i) checksum = checksum * 713 + wm[i];
	std::cout << std::fixed << std::setprecision(3)
			  << std::chrono::duration<double, std::milli>(built - start).count() << ' '
			  << std::chrono::duration<double, std::milli>(finished - built).count() << ' '
			  << wm.memory_usage() << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	std::string mode = argc > 1 ? argv[1] : "library", kind = argc > 2 ? argv[2] : "mixed";
	int n = argc > 3 ? std::atoi(argv[3]) : 100000, q = argc > 4 ? std::atoi(argv[4]) : 100000;
	if (n < 1 || q < 0) return 1;
	std::mt19937 random(713);
	unsigned mask = kind == "duplicates" ? 15 : (1U << 20) - 1;
	std::vector<unsigned> a(n);
	for (auto &v : a) v = random() & mask;
	std::vector<Operation> operations;
	for (int i = 0, size = n; i < q; ++i) {
		int type = kind == "access" ? 6 : kind == "select"					 ? 7
									  : kind == "updates"					 ? 2
									  : kind == "queries" || kind == "short" ? 3 + random() % 3
																			 : random() % 6;
		if (!size) type = 0;
		int p = random() % (size + (type == 0));
		if (kind == "front") p = 0;
		if (kind == "append") p = size - (type != 0);
		int l = random() % (size + 1), r = random() % (size + 1);
		if (l > r) std::swap(l, r);
		if (type == 3 && l == r) l = 0, r = size;
		if (kind == "short") r = std::min(r, l + 16);
		if (type == 3 && l == r) l = 0, r = std::min(16, size);
		operations.push_back({type, p, l, r, l < r ? int(random() % (r - l)) : 0, kind == "select" ? a[p] : unsigned(random() & mask)});
		size += (type == 0) - (type == 1);
	}
	std::cout << mode << ' ' << kind << ' ';
	if (mode == "library") run<DynamicBitVector<>>(a, operations);
	else if (mode == "treap") run<TreapBitVector>(a, operations);
	else if (mode == "separate") run<SeparateRanks>(a, operations);
	else if (mode == "erase_insert") run<DynamicBitVector<>, false>(a, operations);
	else if (mode == "128x4") run<DynamicBitVector<128, 4>>(a, operations);
	else if (mode == "128x8") run<DynamicBitVector<128, 8>>(a, operations);
	else if (mode == "256x4") run<DynamicBitVector<256, 4>>(a, operations);
	else if (mode == "256x8") run<DynamicBitVector<256, 8>>(a, operations);
	else if (mode == "256x16") run<DynamicBitVector<256, 16>>(a, operations);
	else if (mode == "512x4") run<DynamicBitVector<512, 4>>(a, operations);
	else if (mode == "512x8") run<DynamicBitVector<512, 8>>(a, operations);
	else if (mode == "512x16") run<DynamicBitVector<512, 16>>(a, operations);
	else if (mode == "512x32") run<DynamicBitVector<512, 32>>(a, operations);
	else if (mode == "1024x8") run<DynamicBitVector<1024, 8>>(a, operations);
	else if (mode == "1024x16") run<DynamicBitVector<1024, 16>>(a, operations);
	else return 1;
}
