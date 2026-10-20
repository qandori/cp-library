#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"
#include "tree/dsu_on_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	DSUOnTree tree(n);
	std::vector<std::int64_t> initial(n), bit(q + 2), answers;
	for (auto& x : initial) input.read(x);
	for (int v = 1; v < n; ++v) {
		int p = 0;
		input.read(p);
		tree.add_edge(p, v);
	}
	std::vector<std::vector<std::pair<int, std::int64_t>>> updates(n);
	std::vector<std::vector<std::pair<int, int>>> queries(n);
	for (int t = 0; t < q; ++t) {
		int kind = 0, v = 0;
		input.read(kind), input.read(v);
		if (kind == 0) {
			std::int64_t x = 0;
			input.read(x);
			updates[v].emplace_back(t + 1, x);
		} else {
			queries[v].emplace_back(t + 1, int(answers.size()));
			answers.push_back(0);
		}
	}
	auto change = [&](int p, std::int64_t x) {
		for (++p; p < int(bit.size()); p += p & -p) bit[p] += x;
	};
	auto prefix = [&](int r) {
		std::int64_t result = 0;
		for (; r; r -= r & -r) result += bit[r];
		return result;
	};
	tree.build();
	tree.run(
		[&](int v) {
			change(0, initial[v]);
			for (auto [t, x] : updates[v]) change(t, x);
		},
		[&](int v) {
			change(0, -initial[v]);
			for (auto [t, x] : updates[v]) change(t, -x);
		},
		[&](int v) { for (auto [t, id] : queries[v]) answers[id] = prefix(t); }, true
	);
	for (auto x : answers) output.println(x);
}
