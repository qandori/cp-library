#include "variants.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using U = std::uint64_t;
struct Query { int u, v, k, a; };
struct Input {
	std::vector<std::pair<int, int>> edges;
	std::vector<Query> queries;
	std::array<std::vector<U>, 4> expected;
};

template <int op, class Tree>
[[gnu::noinline]] double queries(const Tree &tree, const Input &input, const std::vector<U> &prefix, std::vector<U> &result) {
	auto start = Clock::now();
	if constexpr (op == 0) for (std::size_t i = 0; i < result.size(); ++i) result[i] = tree.lca(input.queries[i].u, input.queries[i].v);
	if constexpr (op == 1) for (std::size_t i = 0; i < result.size(); ++i) result[i] = tree.ancestor(input.queries[i].u, input.queries[i].a);
	if constexpr (op == 2) for (std::size_t i = 0; i < result.size(); ++i) result[i] = tree.jump(input.queries[i].u, input.queries[i].v, input.queries[i].k);
	if constexpr (op == 3) for (std::size_t i = 0; i < result.size(); ++i) {
		U sum = 0;
		tree.for_each_path(input.queries[i].u, input.queries[i].v, [&](int l, int r, bool) { sum += prefix[r] - prefix[l]; });
		result[i] = sum;
	}
	auto end = Clock::now();
	return std::chrono::duration<double>(end - start).count();
}

template <class Tree>
void measure(const char *name, const std::string &shape, int n, int trial, const Input &input) {
	Tree tree(n);
	tree.edges = input.edges;
	auto start = Clock::now();
	tree.build(n / 3);
	auto end = Clock::now();
	auto report = [&](const char *op, double seconds) {
		std::cout << name << ',' << shape << ',' << n << ',' << input.queries.size() << ',' << trial << ',' << op << ',' << seconds * 1000 << '\n';
	};
	report("build", std::chrono::duration<double>(end - start).count());
	std::vector<U> prefix(n + 1), result(input.queries.size());
	for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + U(tree.order[i]) * 1000000007 + 123;
	std::array<decltype(&queries<0, Tree>), 4> operations{queries<0, Tree>, queries<1, Tree>, queries<2, Tree>, queries<3, Tree>};
	for (int op = 0; op < 4; ++op) {
		double elapsed = operations[op](tree, input, prefix, result);
		if (result != input.expected[op]) {
			std::cerr << "Mismatch: " << name << ' ' << shape << ' ' << op << '\n';
			std::abort();
		}
		report(std::array{"lca", "ancestor", "jump", "path"}[op], elapsed);
	}
}

int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 200000;
	int q = argc > 2 ? std::atoi(argv[2]) : 500000;
	int trials = argc > 3 ? std::atoi(argv[3]) : 5;
	std::cout << "variant,shape,n,q,trial,operation,ms\n" << std::fixed << std::setprecision(6);
	for (const std::string shape : {"line", "star", "balanced", "random", "broom"}) {
		std::mt19937 random(713);
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), random);
		Input input;
		for (int v = 1; v < n; ++v) {
			int p = shape == "line" ? v - 1 : shape == "star" ? 0 : shape == "balanced" ? (v - 1) / 2 : shape == "broom" && v < n / 2 ? v - 1 : random() % v;
			input.edges.emplace_back(labels[p], labels[v]);
		}
		std::shuffle(input.edges.begin(), input.edges.end(), random);
		HLD_baseline baseline(n);
		baseline.edges = input.edges;
		baseline.build(n / 3);
		std::vector<U> prefix(n + 1);
		for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + U(baseline.order[i]) * 1000000007 + 123;
		for (int i = 0; i < q; ++i) {
			int u = random() % n, v = random() % n;
			int length = baseline.dist(u, v);
			int k = int(random() % (length + 3)) - 1;
			if (i % 8 == 0) k = 0;
			int a = int(random() % (baseline.depth[u] + 3)) - 1;
			input.queries.push_back({u, v, k, a});
			input.expected[0].push_back(baseline.lca(u, v));
			input.expected[1].push_back(baseline.ancestor(u, a));
			input.expected[2].push_back(baseline.jump(u, v, k));
			U sum = 0;
			baseline.for_each_path(u, v, [&](int l, int r, bool) { sum += prefix[r] - prefix[l]; });
			input.expected[3].push_back(sum);
		}
		using Function = void (*)(const char *, const std::string &, int, int, const Input &);
		std::array<std::pair<const char *, Function>, 11> variants{{
			{"baseline", measure<HLD_baseline>}, {"preorder", measure<HLD_preorder>}, {"fused", measure<HLD_fused>},
			{"peel", measure<HLD_peel>}, {"peel_preorder", measure<HLD_peel_preorder>}, {"peel_fused", measure<HLD_peel_fused>}, {"peel_zero", measure<HLD_peel_zero>}, {"packed", measure<HLD_packed>}, {"shapes", measure<HLD_shapes>}, {"packed_shapes", measure<HLD_packed_shapes>}, {"current", measure<HLD_current>}
		}};
		for (int trial = 0; trial < trials; ++trial) {
			std::shuffle(variants.begin(), variants.end(), random);
			for (auto [name, function] : variants) function(name, shape, n, trial, input);
		}
	}
}
