#include "tree/dsu_on_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

void solve_dsu_on_tree_mode(FastInput &input, FastOutput &output) {
	int n = 0, m = 0, f = 0;
	std::uint64_t state = 0;
	input.read(n), input.read(state), input.read(m), input.read(f);
	auto next = [&] { state = (state * 1103515245 + 12345) & 0x7fffffff; };
	DSUOnTree tree(n);
	for (int v = 1; v < n; ++v) {
		int p = 0;
		if (v < m) { input.read(p); --p; }
		else { p = int(state % v); next(); }
		tree.add_edge(p, v);
	}
	std::vector<int> color(n), count(n), touched;
	for (int v = 0; v < n; ++v) {
		if (v < m) { input.read(color[v]); --color[v]; }
		else { color[v] = int(state % f); next(); }
	}
	tree.build();
	int maximum = 0, kinds = 0;
	std::uint64_t result = 0;
	tree.run_clear([&](int v) {
		int c = color[v];
		if (!count[c]) touched.push_back(c);
		int x = ++count[c];
		if (x > maximum) maximum = x, kinds = 1;
		else if (x == maximum) ++kinds;
	}, [&](int v) {
		result = (result + std::uint64_t(maximum ^ (v + 1)) * (kinds ^ (v + 1))) % 998244353;
	}, [&] {
		for (int c : touched) count[c] = 0;
		touched.clear();
		maximum = kinds = 0;
	});
	output.println(result);
}

#ifndef DSU_ON_TREE_MODE_TEST
int main() {
	FastInput input;
	FastOutput output;
	solve_dsu_on_tree_mode(input, output);
}
#endif
